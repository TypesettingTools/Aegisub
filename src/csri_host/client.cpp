// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file client.cpp
/// @brief Launching and talking to a sandboxed aegisub-csri-host process
///
/// The host runs in:
///  - an AppContainer with no capabilities, so it has no access to the
///    network or to the user's files and registry
///  - a job object which kills it when Aegisub closes it, stops it from
///    creating other processes, limits its memory use and blocks access to
///    other processes' windows, the clipboard and system settings
///  - a set of process mitigation policies, plus code integrity and
///    arbitrary code guards which the host enables itself once it has
///    loaded the renderers
///
/// It can't be denied access to win32k, as VSFilter uses GDI to rasterize
/// text, so a kernel bug reachable through GDI remains a way out.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
// AppContainers and most of the mitigation policies are only declared for
// newer targets. Everything newer than Windows 7 is either loaded at runtime
// or only used after checking the Windows version, so that Aegisub itself
// still starts on older versions.
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#undef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000002 // NTDDI_WIN10_RS1
#include <windows.h>
#include <aclapi.h>
#include <userenv.h>

#include "client.h"
#include "protocol.h"

#include <algorithm>
#include <cstring>
#include <random>
#include <utility>

namespace csri_host {
namespace {
/// How long the host may take to handle a single request before it's assumed
/// to have hung and is killed
constexpr DWORD timeout_ms = 30000;
constexpr SIZE_T memory_limit = SIZE_T(4) * 1024 * 1024 * 1024;
constexpr wchar_t container_name[] = L"Aegisub.CsriHost";
constexpr wchar_t host_exe_name[] = L"aegisub-csri-host.exe";

struct handle_closer {
	void operator()(HANDLE h) const { if (h && h != INVALID_HANDLE_VALUE) CloseHandle(h); }
};
using unique_handle = std::unique_ptr<void, handle_closer>;

struct local_freer {
	void operator()(void *p) const { LocalFree(p); }
};

struct view_unmapper {
	void operator()(unsigned char *p) const { UnmapViewOfFile(p); }
};

std::string error_string(DWORD err) {
	wchar_t *buf = nullptr;
	DWORD len = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, 0, reinterpret_cast<wchar_t *>(&buf), 0, nullptr);
	std::unique_ptr<void, local_freer> holder(buf);
	while (len && (buf[len - 1] == L'\r' || buf[len - 1] == L'\n' || buf[len - 1] == L'.'))
		--len;
	if (!len)
		return "error " + std::to_string(err);

	int size = WideCharToMultiByte(CP_UTF8, 0, buf, static_cast<int>(len), nullptr, 0, nullptr, nullptr);
	std::string msg(size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, buf, static_cast<int>(len), msg.data(), size, nullptr, nullptr);
	return msg;
}

[[noreturn]] void throw_win32(const char *what) {
	DWORD err = GetLastError();
	throw Error(std::string(what) + ": " + error_string(err));
}

bool supported_windows_version() {
	// GetVersionEx lies depending on the manifest, so ask ntdll directly
	using RtlGetVersionFn = LONG (WINAPI *)(OSVERSIONINFOW *);
	auto ntdll = GetModuleHandleW(L"ntdll.dll");
	auto get_version = ntdll ? reinterpret_cast<RtlGetVersionFn>(GetProcAddress(ntdll, "RtlGetVersion")) : nullptr;
	OSVERSIONINFOW info{};
	info.dwOSVersionInfoSize = sizeof info;
	if (!get_version || get_version(&info) != 0)
		return false;
	// Version 1607 added the child process policy and image load mitigations
	return info.dwMajorVersion > 10 || (info.dwMajorVersion == 10 && info.dwBuildNumber >= 14393);
}

PSID create_container_sid() {
	// userenv's AppContainer functions don't exist on Windows 7, so they
	// can't be linked directly
	auto userenv = LoadLibraryExW(L"userenv.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (!userenv)
		throw_win32("Could not load userenv.dll");
	auto create = reinterpret_cast<decltype(&CreateAppContainerProfile)>(
		GetProcAddress(userenv, "CreateAppContainerProfile"));
	auto derive = reinterpret_cast<decltype(&DeriveAppContainerSidFromAppContainerName)>(
		GetProcAddress(userenv, "DeriveAppContainerSidFromAppContainerName"));
	if (!create || !derive)
		throw Error("AppContainers are not supported on this version of Windows");

	PSID sid = nullptr;
	HRESULT hr = create(container_name, L"Aegisub subtitle renderer",
		L"Sandbox for the third-party subtitle renderers used by Aegisub", nullptr, 0, &sid);
	if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
		hr = derive(container_name, &sid);
	if (FAILED(hr))
		throw Error("Could not create the AppContainer profile: " + error_string(static_cast<DWORD>(hr)));
	return sid;
}

/// Does the DACL let the AppContainer read and execute the file?
bool container_has_access(PACL dacl, PSID sid) {
	if (!dacl)
		return true;

	BYTE any_package_buf[SECURITY_MAX_SID_SIZE];
	DWORD any_package_size = sizeof any_package_buf;
	PSID any_package = any_package_buf;
	if (!CreateWellKnownSid(WinBuiltinAnyPackageSid, nullptr, any_package, &any_package_size))
		return false;

	constexpr ACCESS_MASK needed = FILE_READ_DATA | FILE_EXECUTE;
	for (DWORD i = 0; i < dacl->AceCount; ++i) {
		void *ace_ptr = nullptr;
		if (!GetAce(dacl, i, &ace_ptr))
			continue;
		auto header = static_cast<ACE_HEADER *>(ace_ptr);
		if (header->AceType != ACCESS_ALLOWED_ACE_TYPE || (header->AceFlags & INHERIT_ONLY_ACE))
			continue;
		auto ace = static_cast<ACCESS_ALLOWED_ACE *>(ace_ptr);
		PSID ace_sid = &ace->SidStart;
		if ((ace->Mask & needed) == needed && (EqualSid(ace_sid, sid) || EqualSid(ace_sid, any_package)))
			return true;
	}
	return false;
}

/// Let the AppContainer read and execute path if it can't already.
///
/// Installed copies of Aegisub are readable by every AppContainer, either via
/// Program Files' permissions or ones added by the installer, but portable
/// copies usually live somewhere which AppContainers can't read.
void ensure_container_access(std::wstring const& path, PSID sid, bool directory) {
	PACL dacl = nullptr;
	PSECURITY_DESCRIPTOR sd = nullptr;
	if (GetNamedSecurityInfoW(path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
	                          nullptr, nullptr, &dacl, nullptr, &sd) != ERROR_SUCCESS)
		return;
	std::unique_ptr<void, local_freer> sd_holder(sd);
	if (container_has_access(dacl, sid))
		return;

	EXPLICIT_ACCESSW access{};
	access.grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
	access.grfAccessMode = GRANT_ACCESS;
	access.grfInheritance = directory ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
	access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
	access.Trustee.ptstrName = static_cast<LPWSTR>(sid);

	PACL new_dacl = nullptr;
	if (SetEntriesInAclW(1, &access, dacl, &new_dacl) != ERROR_SUCCESS)
		return;
	std::unique_ptr<void, local_freer> dacl_holder(new_dacl);
	// If this fails the host will fail to start, which is reported then
	SetNamedSecurityInfoW(const_cast<LPWSTR>(path.c_str()), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
	                      nullptr, nullptr, new_dacl, nullptr);
}

std::wstring module_directory() {
	std::wstring path(MAX_PATH, L'\0');
	for (;;) {
		DWORD len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
		if (len == 0)
			throw_win32("Could not get the Aegisub directory");
		if (len < path.size()) {
			path.resize(len);
			break;
		}
		path.resize(path.size() * 2);
	}
	path.resize(path.find_last_of(L'\\') + 1);
	return path;
}

std::wstring system_directory() {
	wchar_t buf[MAX_PATH];
	UINT len = GetSystemDirectoryW(buf, MAX_PATH);
	if (len == 0 || len >= MAX_PATH)
		throw_win32("Could not get the system directory");
	return std::wstring(buf, len);
}

/// Per-session setup shared by all hosts
struct Environment {
	PSID sid = nullptr; // Deliberately never freed
	std::wstring exe;
	std::wstring working_directory;

	Environment() {
		if (!supported_windows_version())
			throw Error("Sandboxed CSRI renderers require Windows 10 version 1607 or newer");
		sid = create_container_sid();

		auto directory = module_directory();
		exe = directory + host_exe_name;
		ensure_container_access(exe, sid, false);
		ensure_container_access(directory + L"csri", sid, true);

		// The host can't necessarily access Aegisub's directory, but it can
		// always access System32
		working_directory = system_directory();
	}
};

Environment const& environment() {
	// If this throws it'll be retried on the next call
	static Environment env;
	return env;
}

std::pair<unique_handle, unique_handle> create_pipe() {
	std::random_device rd;
	std::wstring name = L"\\\\.\\pipe\\aegisub-csri-host-" + std::to_wstring(GetCurrentProcessId());
	for (int i = 0; i < 4; ++i)
		name += L"-" + std::to_wstring(rd());

	unique_handle server(CreateNamedPipeW(name.c_str(),
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
		1, 64 * 1024, 64 * 1024, 0, nullptr));
	if (server.get() == INVALID_HANDLE_VALUE)
		throw_win32("Could not create a pipe for the subtitle renderer");

	// The host's end has to be inheritable to be passed to it. The handle
	// list given to CreateProcess keeps it from being inherited by anything
	// else the host might start, but a process started by Aegisub at the
	// same time could also inherit it, which would only keep the pipe open.
	SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
	unique_handle client(CreateFileW(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &sa, OPEN_EXISTING,
		SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS, nullptr));
	if (client.get() == INVALID_HANDLE_VALUE)
		throw_win32("Could not connect a pipe for the subtitle renderer");

	return {std::move(server), std::move(client)};
}

unique_handle create_job() {
	unique_handle job(CreateJobObjectW(nullptr, nullptr));
	if (!job)
		throw_win32("Could not create a job object for the subtitle renderer");

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
	limits.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION |
		JOB_OBJECT_LIMIT_ACTIVE_PROCESS |
		JOB_OBJECT_LIMIT_PROCESS_MEMORY;
	limits.BasicLimitInformation.ActiveProcessLimit = 1;
	limits.ProcessMemoryLimit = memory_limit;
	if (!SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof limits))
		throw_win32("Could not set the subtitle renderer's job limits");

	JOBOBJECT_BASIC_UI_RESTRICTIONS ui{};
	ui.UIRestrictionsClass =
		JOB_OBJECT_UILIMIT_DESKTOP |
		JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
		JOB_OBJECT_UILIMIT_EXITWINDOWS |
		JOB_OBJECT_UILIMIT_GLOBALATOMS |
		JOB_OBJECT_UILIMIT_HANDLES |
		JOB_OBJECT_UILIMIT_READCLIPBOARD |
		JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS |
		JOB_OBJECT_UILIMIT_WRITECLIPBOARD;
	if (!SetInformationJobObject(job.get(), JobObjectBasicUIRestrictions, &ui, sizeof ui))
		throw_win32("Could not set the subtitle renderer's UI restrictions");

	return job;
}

class AttributeList {
	std::vector<char> buffer;

public:
	explicit AttributeList(DWORD count) {
		SIZE_T size = 0;
		InitializeProcThreadAttributeList(nullptr, count, 0, &size);
		buffer.resize(size);
		if (!InitializeProcThreadAttributeList(get(), count, 0, &size))
			throw_win32("Could not initialize the process attribute list");
	}
	~AttributeList() { DeleteProcThreadAttributeList(get()); }
	AttributeList(AttributeList const&) = delete;
	AttributeList& operator=(AttributeList const&) = delete;

	LPPROC_THREAD_ATTRIBUTE_LIST get() { return reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data()); }

	/// value must remain valid until the process has been created
	void Set(DWORD_PTR attribute, void *value, size_t size) {
		if (!UpdateProcThreadAttribute(get(), 0, attribute, value, size, nullptr, nullptr))
			throw_win32("Could not set a process attribute");
	}
};
}

struct Host::Impl {
	// Closing the job kills the host, so it's declared first to be destroyed last
	unique_handle job;
	unique_handle process;
	unique_handle pipe;
	unique_handle event;
	std::unique_ptr<unsigned char, view_unmapper> frame;
	size_t frame_size = 0;

	Impl();

	void Transfer(bool write, char *data, size_t size);
	void Read(void *data, size_t size) { Transfer(false, static_cast<char *>(data), size); }
	void Write(const void *data, size_t size) { Transfer(true, static_cast<char *>(const_cast<void *>(data)), size); }
	Response Call(Request const& req, const void *payload = nullptr);
	void EnsureFrameBuffer(size_t size);
};

Host::Impl::Impl() {
	auto const& env = environment();
	auto [server, client] = create_pipe();
	job = create_job();

	SECURITY_CAPABILITIES capabilities{};
	capabilities.AppContainerSid = env.sid;

	HANDLE inherited[] = {client.get()};

	DWORD64 mitigations =
		PROCESS_CREATION_MITIGATION_POLICY_FORCE_RELOCATE_IMAGES_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_HEAP_TERMINATE_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_BOTTOM_UP_ASLR_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_HIGH_ENTROPY_ASLR_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_STRICT_HANDLE_CHECKS_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_EXTENSION_POINT_DISABLE_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_IMAGE_LOAD_NO_REMOTE_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_IMAGE_LOAD_NO_LOW_LABEL_ALWAYS_ON |
		PROCESS_CREATION_MITIGATION_POLICY_IMAGE_LOAD_PREFER_SYSTEM32_ALWAYS_ON;

	DWORD child_process_policy = PROCESS_CREATION_CHILD_PROCESS_RESTRICTED;

	AttributeList attributes(4);
	attributes.Set(PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, &capabilities, sizeof capabilities);
	attributes.Set(PROC_THREAD_ATTRIBUTE_HANDLE_LIST, inherited, sizeof inherited);
	attributes.Set(PROC_THREAD_ATTRIBUTE_MITIGATION_POLICY, &mitigations, sizeof mitigations);
	attributes.Set(PROC_THREAD_ATTRIBUTE_CHILD_PROCESS_POLICY, &child_process_policy, sizeof child_process_policy);

	STARTUPINFOEXW si{};
	si.StartupInfo.cb = sizeof si;
	si.lpAttributeList = attributes.get();

	std::wstring cmdline = L"\"" + env.exe + L"\" --pipe=" +
		std::to_wstring(reinterpret_cast<uintptr_t>(client.get()));

	PROCESS_INFORMATION pi{};
	if (!CreateProcessW(env.exe.c_str(), cmdline.data(), nullptr, nullptr, TRUE,
	                    CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr,
	                    env.working_directory.c_str(), &si.StartupInfo, &pi))
		throw_win32("Could not start aegisub-csri-host.exe");
	process.reset(pi.hProcess);
	unique_handle thread(pi.hThread);

	if (!AssignProcessToJobObject(job.get(), process.get())) {
		TerminateProcess(process.get(), 1);
		throw_win32("Could not add the subtitle renderer to its job object");
	}
	ResumeThread(thread.get());

	// Once the host has the only handle to its end of the pipe, its exit
	// breaks the pipe and fails any I/O waiting on it
	client.reset();
	pipe = std::move(server);

	event.reset(CreateEventW(nullptr, TRUE, FALSE, nullptr));
	if (!event)
		throw_win32("Could not create an event");

	Hello hello;
	Read(&hello, sizeof hello);
	if (hello.magic != hello_magic || hello.version != protocol_version)
		throw Error("aegisub-csri-host.exe is from a different version of Aegisub");
}

void Host::Impl::Transfer(bool write, char *data, size_t size) {
	ULONGLONG deadline = GetTickCount64() + timeout_ms;
	while (size) {
		OVERLAPPED ov{};
		ov.hEvent = event.get();
		DWORD chunk = static_cast<DWORD>(std::min<size_t>(size, 1 << 20));
		BOOL ok = write
			? WriteFile(pipe.get(), data, chunk, nullptr, &ov)
			: ReadFile(pipe.get(), data, chunk, nullptr, &ov);
		if (!ok && GetLastError() != ERROR_IO_PENDING)
			throw_win32("Lost the connection to the subtitle renderer");

		ULONGLONG now = GetTickCount64();
		DWORD wait = now < deadline ? static_cast<DWORD>(deadline - now) : 0;
		if (WaitForSingleObject(event.get(), wait) != WAIT_OBJECT_0) {
			// The I/O has to be finished before ov goes out of scope
			CancelIoEx(pipe.get(), &ov);
			DWORD ignored;
			GetOverlappedResult(pipe.get(), &ov, &ignored, TRUE);
			throw Error("The subtitle renderer stopped responding");
		}

		DWORD transferred = 0;
		if (!GetOverlappedResult(pipe.get(), &ov, &transferred, FALSE))
			throw_win32("Lost the connection to the subtitle renderer");
		if (transferred == 0)
			throw Error("Lost the connection to the subtitle renderer");
		data += transferred;
		size -= transferred;
	}
}

Response Host::Impl::Call(Request const& req, const void *payload) {
	Write(&req, sizeof req);
	if (req.payload_size)
		Write(payload, static_cast<size_t>(req.payload_size));

	Response response;
	Read(&response, sizeof response);
	if (response.status != Status::Ok && response.status != Status::Failed)
		throw Error("The subtitle renderer sent an invalid response");
	if (response.payload_size && req.command != Command::ListRenderers)
		throw Error("The subtitle renderer sent an invalid response");
	return response;
}

void Host::Impl::EnsureFrameBuffer(size_t size) {
	if (size <= frame_size)
		return;

	unique_handle section(CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		static_cast<DWORD>(uint64_t(size) >> 32), static_cast<DWORD>(size), nullptr));
	if (!section)
		throw_win32("Could not allocate a frame buffer for the subtitle renderer");
	std::unique_ptr<unsigned char, view_unmapper> view(static_cast<unsigned char *>(
		MapViewOfFile(section.get(), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size)));
	if (!view)
		throw_win32("Could not map the subtitle renderer's frame buffer");

	// Only allow mapping the section, not anything like querying or extending it
	HANDLE host_section = nullptr;
	if (!DuplicateHandle(GetCurrentProcess(), section.get(), process.get(), &host_section,
	                     FILE_MAP_READ | FILE_MAP_WRITE, FALSE, 0))
		throw_win32("Could not share the frame buffer with the subtitle renderer");

	frame = std::move(view);
	frame_size = size;

	Request req{};
	req.command = Command::SetFrameBuffer;
	req.section_handle = reinterpret_cast<uintptr_t>(host_section);
	req.section_size = size;
	Call(req);
}

Host::Host() : impl(std::make_unique<Impl>()) { }
Host::~Host() = default;

std::vector<std::string> Host::ListRenderers() {
	Request req{};
	req.command = Command::ListRenderers;
	auto response = impl->Call(req);
	if (response.payload_size > max_renderer_list_size)
		throw Error("The subtitle renderer sent an invalid response");

	std::string payload(response.payload_size, '\0');
	impl->Read(payload.data(), payload.size());

	std::vector<std::string> names;
	size_t start = 0;
	while (start < payload.size()) {
		size_t end = payload.find('\0', start);
		if (end == payload.npos || end == start || end - start >= max_renderer_name_size)
			throw Error("The subtitle renderer sent an invalid response");
		std::string name(payload, start, end - start);
		// The names are shown in the UI and stored in the config
		if (!std::all_of(name.begin(), name.end(), [](char c) { return c >= 0x20 && c < 0x7f; }))
			throw Error("The subtitle renderer sent an invalid response");
		names.push_back(std::move(name));
		start = end + 1;
	}
	return names;
}

bool Host::OpenRenderer(std::string const& name) {
	if (name.size() >= max_renderer_name_size)
		return false;
	Request req{};
	req.command = Command::OpenRenderer;
	req.payload_size = name.size();
	return impl->Call(req, name.data()).status == Status::Ok;
}

void Host::LoadSubtitles(const char *data, size_t len) {
	if (len > max_script_size)
		throw Error("The subtitles are too large to render");
	Request req{};
	req.command = Command::LoadSubtitles;
	req.payload_size = len;
	impl->Call(req, data);
}

void Host::Render(unsigned char *data, int width, int height, int pitch, bool flipped, double time) {
	if (width <= 0 || height <= 0 || width > int(max_frame_dimension) || height > int(max_frame_dimension))
		return;
	size_t row_size = size_t(width) * 4;
	if (pitch < 0 || size_t(pitch) < row_size)
		return;

	impl->EnsureFrameBuffer(row_size * height);

	// The host gets a top-down frame without padding regardless of how the
	// frame is laid out
	auto row = [&](int y) { return data + size_t(flipped ? height - 1 - y : y) * pitch; };
	unsigned char *frame = impl->frame.get();
	for (int y = 0; y < height; ++y)
		memcpy(frame + y * row_size, row(y), row_size);

	Request req{};
	req.command = Command::Render;
	req.width = width;
	req.height = height;
	req.time = time;
	if (impl->Call(req).status != Status::Ok)
		return;

	for (int y = 0; y < height; ++y)
		memcpy(row(y), frame + y * row_size, row_size);
}

}
