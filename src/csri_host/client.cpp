// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file client.cpp
/// @brief Launching and talking to a sandboxed aegisub-csri-host process
///
/// Each host runs in:
///  - an AppContainer with no capabilities, so it has no access to the
///    network, to the user's files and registry, or to other Aegisub
///    processes' hosts
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
#include <combaseapi.h>
#include <sddl.h>
#include <userenv.h>

#include "client.h"
#include "protocol.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <iterator>
#include <mutex>
#include <random>
#include <utility>

namespace csri_host {
namespace {
/// How long the host may take to handle a single operation before it's
/// assumed to have hung and is killed
constexpr DWORD timeout_ms = 30000;
/// The hosts' AppContainer profile is named
/// <prefix><Aegisub's pid>.<Aegisub's creation time>, so that ones left behind
/// by a crash can be recognised and deleted. Windows stores the names in
/// lowercase.
constexpr wchar_t container_prefix[] = L"aegisub.csrihost.";
constexpr wchar_t container_mappings_key[] =
	L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppContainer\\Mappings";
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
	// Before version 1809, CreateProcess fails when given both an AppContainer
	// and mitigation policies
	return info.dwMajorVersion > 10 || (info.dwMajorVersion == 10 && info.dwBuildNumber >= 17763);
}

struct UserEnv {
	decltype(&CreateAppContainerProfile) create = nullptr;
	decltype(&DeleteAppContainerProfile) remove = nullptr;
	decltype(&GetAppContainerFolderPath) folder = nullptr;
	decltype(&DeriveAppContainerSidFromAppContainerName) derive = nullptr;

	UserEnv() {
		// userenv's AppContainer functions don't exist on Windows 7, so they
		// can't be linked directly
		auto userenv = LoadLibraryExW(L"userenv.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (!userenv)
			throw_win32("Could not load userenv.dll");
		create = reinterpret_cast<decltype(create)>(GetProcAddress(userenv, "CreateAppContainerProfile"));
		remove = reinterpret_cast<decltype(remove)>(GetProcAddress(userenv, "DeleteAppContainerProfile"));
		folder = reinterpret_cast<decltype(folder)>(GetProcAddress(userenv, "GetAppContainerFolderPath"));
		derive = reinterpret_cast<decltype(derive)>(GetProcAddress(userenv, "DeriveAppContainerSidFromAppContainerName"));
		if (!create || !remove || !folder || !derive)
			throw Error("AppContainers are not supported on this version of Windows");
	}
};

/// The creation time of a process, which together with its pid identifies it
ULONGLONG process_creation_time(HANDLE process) {
	FILETIME created, exited, kernel, user;
	if (!GetProcessTimes(process, &created, &exited, &kernel, &user))
		return 0;
	return (ULONGLONG(created.dwHighDateTime) << 32) | created.dwLowDateTime;
}

/// Whether the Aegisub process which created a profile might still be running
bool container_owner_running(DWORD pid, ULONGLONG creation_time) {
	unique_handle process(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid));
	if (!process)
		// Any error other than there being no such process means it might
		// be running
		return GetLastError() != ERROR_INVALID_PARAMETER;
	DWORD exit_code = 0;
	if (GetExitCodeProcess(process.get(), &exit_code) && exit_code != STILL_ACTIVE)
		return false;
	auto actual_creation_time = process_creation_time(process.get());
	return actual_creation_time == 0 || actual_creation_time == creation_time;
}

/// Delete the AppContainer profiles of Aegisub processes which exited without
/// cleaning them up
void delete_stale_containers(UserEnv const& userenv) {
	HKEY key;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, container_mappings_key, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return;

	std::vector<std::wstring> stale;
	for (DWORD i = 0; ; ++i) {
		wchar_t sid[256];
		DWORD sid_len = static_cast<DWORD>(std::size(sid));
		LONG err = RegEnumKeyExW(key, i, sid, &sid_len, nullptr, nullptr, nullptr, nullptr);
		if (err == ERROR_NO_MORE_ITEMS)
			break;
		if (err != ERROR_SUCCESS)
			continue;

		wchar_t moniker[128];
		DWORD moniker_size = sizeof moniker;
		if (RegGetValueW(key, sid, L"Moniker", RRF_RT_REG_SZ, nullptr, moniker, &moniker_size) != ERROR_SUCCESS)
			continue;

		constexpr size_t prefix_len = std::size(container_prefix) - 1;
		if (wcsncmp(moniker, container_prefix, prefix_len) != 0)
			continue;
		unsigned long pid = 0;
		unsigned long long creation_time = 0;
		int length = 0;
		auto rest = moniker + prefix_len;
		// Only names which Aegisub generates, with nothing after them
		if (swscanf_s(rest, L"%lu.%llx%n", &pid, &creation_time, &length) != 2 || rest[length] != L'\0')
			continue;
		if (!container_owner_running(pid, creation_time))
			stale.emplace_back(moniker);
	}
	RegCloseKey(key);

	for (auto const& name : stale)
		userenv.remove(name.c_str());
}

/// Whether the DACL lets every AppContainer read and execute the file
bool containers_have_access(PACL dacl, PSID any_package) {
	if (!dacl)
		return true;

	constexpr ACCESS_MASK needed = FILE_READ_DATA | FILE_EXECUTE;
	for (DWORD i = 0; i < dacl->AceCount; ++i) {
		void *ace_ptr = nullptr;
		if (!GetAce(dacl, i, &ace_ptr))
			continue;
		auto header = static_cast<ACE_HEADER *>(ace_ptr);
		if (header->AceType != ACCESS_ALLOWED_ACE_TYPE || (header->AceFlags & INHERIT_ONLY_ACE))
			continue;
		auto ace = static_cast<ACCESS_ALLOWED_ACE *>(ace_ptr);
		if ((ace->Mask & needed) == needed && EqualSid(&ace->SidStart, any_package))
			return true;
	}
	return false;
}

/// Let AppContainers read and execute path if they can't already.
///
/// Installed copies of Aegisub are readable by every AppContainer, either via
/// Program Files' permissions or ones added by the installer, but portable
/// copies usually live somewhere which AppContainers can't read. Access is
/// granted to all AppContainers rather than just the hosts' as each Aegisub
/// process has a different one.
void ensure_container_access(std::wstring const& path, bool directory) {
	BYTE any_package[SECURITY_MAX_SID_SIZE];
	DWORD any_package_size = sizeof any_package;
	if (!CreateWellKnownSid(WinBuiltinAnyPackageSid, nullptr, any_package, &any_package_size))
		return;

	PACL dacl = nullptr;
	PSECURITY_DESCRIPTOR sd = nullptr;
	if (GetNamedSecurityInfoW(path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
	                          nullptr, nullptr, &dacl, nullptr, &sd) != ERROR_SUCCESS)
		return;
	std::unique_ptr<void, local_freer> sd_holder(sd);
	if (containers_have_access(dacl, any_package))
		return;

	EXPLICIT_ACCESSW access{};
	access.grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_EXECUTE;
	access.grfAccessMode = GRANT_ACCESS;
	access.grfInheritance = directory ? SUB_CONTAINERS_AND_OBJECTS_INHERIT : NO_INHERITANCE;
	access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	access.Trustee.ptstrName = reinterpret_cast<LPWSTR>(any_package);

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
	UserEnv userenv;
	std::wstring exe;
	std::wstring working_directory;
	/// Name of this process's AppContainer profile
	std::wstring container_name;

	Environment() {
		if (!supported_windows_version())
			throw Error("Sandboxed CSRI renderers require Windows 10 version 1809 or newer");

		delete_stale_containers(userenv);
		wchar_t name[64];
		swprintf_s(name, L"%ls%lu.%llx", csri_host::container_prefix,
			GetCurrentProcessId(), process_creation_time(GetCurrentProcess()));
		container_name = name;

		auto directory = module_directory();
		exe = directory + host_exe_name;
		ensure_container_access(exe, false);
		ensure_container_access(directory + L"csri", true);

		// The host can't necessarily access Aegisub's directory, but it can
		// always access System32
		working_directory = system_directory();
	}
};

Environment& environment() {
	// If this throws it'll be retried on the next call
	static Environment env;
	return env;
}

/// The folder of the AppContainer profile with the given SID
std::wstring container_folder(PSID sid) {
	wchar_t *sid_string = nullptr;
	if (!ConvertSidToStringSidW(sid, &sid_string))
		throw_win32("Could not get the subtitle renderer's AppContainer");
	std::unique_ptr<void, local_freer> sid_string_holder(sid_string);

	wchar_t *path = nullptr;
	HRESULT hr = environment().userenv.folder(sid_string, &path);
	if (FAILED(hr))
		throw Error("Could not get the subtitle renderer's AppContainer folder: " + error_string(static_cast<DWORD>(hr)));
	std::wstring folder = path;
	CoTaskMemFree(path);
	return folder;
}

/// A host's use of this process's AppContainer profile, which is shared by
/// all of its hosts and deleted once none are running, so that nothing they
/// store persists. Other Aegisub processes have their own, so they can't
/// interfere with each other's hosts or delete each other's profile.
class Container {
	struct Shared {
		std::mutex mutex;
		size_t users = 0;
		PSID sid = nullptr;
		std::wstring folder;
	};
	static Shared& shared() {
		static Shared s;
		return s;
	}

	/// Delete the profile. If this fails it's reused by the next host, or
	/// deleted the next time Aegisub starts.
	static void release(Shared& s) {
		FreeSid(s.sid);
		s.sid = nullptr;
		environment().userenv.remove(environment().container_name.c_str());
	}

	PSID sid;
	/// The profile's folder, which only AppContainers using the profile can
	/// write to
	std::wstring folder;

public:
	/// Must not be destroyed until the host has exited
	Container() {
		auto& s = shared();
		std::lock_guard<std::mutex> lock(s.mutex);
		if (s.users == 0) {
			auto& env = environment();
			auto name = env.container_name.c_str();
			HRESULT hr = env.userenv.create(name, L"Aegisub subtitle renderer",
				L"Sandbox for a third-party subtitle renderer used by Aegisub", nullptr, 0, &s.sid);
			// Deleting it failed when the last host exited
			if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
				hr = env.userenv.derive(name, &s.sid);
			if (FAILED(hr))
				throw Error("Could not create an AppContainer for the subtitle renderer: " + error_string(static_cast<DWORD>(hr)));
		}
		try {
			if (s.users == 0)
				s.folder = container_folder(s.sid);
			folder = s.folder;
		}
		catch (...) {
			if (s.users == 0)
				release(s);
			throw;
		}
		++s.users;
		sid = s.sid;
	}

	~Container() {
		auto& s = shared();
		std::lock_guard<std::mutex> lock(s.mutex);
		if (--s.users == 0)
			release(s);
	}

	Container(Container const&) = delete;
	Container& operator=(Container const&) = delete;

	PSID Sid() const { return sid; }
	std::wstring const& Folder() const { return folder; }
};

/// Build an environment block for the host. It doesn't get Aegisub's, which
/// could contain things like credentials.
std::wstring host_environment(std::wstring const& profile_folder) {
	std::vector<std::wstring> vars;
	// Some system DLLs need these
	for (auto name : {L"SystemDrive", L"SystemRoot", L"windir"}) {
		wchar_t value[MAX_PATH];
		DWORD len = GetEnvironmentVariableW(name, value, MAX_PATH);
		if (len > 0 && len < MAX_PATH)
			vars.push_back(std::wstring(name) + L"=" + value);
	}
	// Windows 10 fails to create AppContainer processes with
	// ERROR_ENVVAR_NOT_FOUND if this is missing. Rather than replacing it,
	// Windows appends \Packages\<name>\AC to it and then sets TEMP and TMP to
	// the Temp folder in that, so it has to be what the profile folder is in.
	// The suffix is checked from the end, skipping <name>.
	auto local_app_data = profile_folder;
	const wchar_t *const suffix[] = {L"AC", nullptr, L"Packages"};
	for (auto expected : suffix) {
		auto sep = local_app_data.find_last_of(L'\\');
		if (sep == local_app_data.npos || (expected &&
		    CompareStringOrdinal(local_app_data.c_str() + sep + 1, -1, expected, -1, TRUE) != CSTR_EQUAL))
			throw Error("The subtitle renderer's AppContainer folder is in an unexpected place");
		local_app_data.resize(sep);
	}
	vars.push_back(L"LOCALAPPDATA=" + local_app_data);

	// The block has to be sorted case-insensitively by name
	std::sort(vars.begin(), vars.end(), [](std::wstring const& a, std::wstring const& b) {
		return CompareStringOrdinal(a.c_str(), -1, b.c_str(), -1, TRUE) == CSTR_LESS_THAN;
	});

	std::wstring block;
	for (auto const& var : vars) {
		block += var;
		block += L'\0';
	}
	block += L'\0';
	return block;
}

/// Closes a process handle after terminating the process and waiting for it
/// to exit
struct process_killer {
	void operator()(HANDLE h) const {
		if (!h) return;
		TerminateProcess(h, 1);
		WaitForSingleObject(h, 5000);
		CloseHandle(h);
	}
};

/// Frame memory which can be mapped into hosts so that they can draw on it
class SharedFrameStorage final : public FrameStorage {
	unique_handle section;
	std::unique_ptr<unsigned char, view_unmapper> view;
	size_t capacity;
	/// Identifies this to hosts. Never reused.
	uint64_t id;
	/// Expires when this is destroyed, so that hosts know to unmap it
	std::shared_ptr<void> alive = std::make_shared<char>();

	static std::atomic<uint64_t> next_id;

public:
	SharedFrameStorage(unique_handle section, std::unique_ptr<unsigned char, view_unmapper> view, size_t capacity)
	: section(std::move(section))
	, view(std::move(view))
	, capacity(capacity)
	, id(next_id++)
	{
	}

	unsigned char *Data() const override { return view.get(); }
	size_t Capacity() const override { return capacity; }

	HANDLE Section() const { return section.get(); }
	uint64_t Id() const { return id; }
	std::weak_ptr<void> Alive() const { return alive; }
};

std::atomic<uint64_t> SharedFrameStorage::next_id{1};

std::unique_ptr<SharedFrameStorage> create_shared_storage(size_t size) {
	unique_handle section(CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
		static_cast<DWORD>(uint64_t(size) >> 32), static_cast<DWORD>(size), nullptr));
	if (!section)
		throw_win32("Could not allocate a frame buffer for the subtitle renderer");
	std::unique_ptr<unsigned char, view_unmapper> view(static_cast<unsigned char *>(
		MapViewOfFile(section.get(), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size)));
	if (!view)
		throw_win32("Could not map a frame buffer for the subtitle renderer");
	return std::make_unique<SharedFrameStorage>(std::move(section), std::move(view), size);
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

unique_handle create_job(uint64_t memory_limit) {
	unique_handle job(CreateJobObjectW(nullptr, nullptr));
	if (!job)
		throw_win32("Could not create a job object for the subtitle renderer");

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
	limits.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
		JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION |
		JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
	limits.BasicLimitInformation.ActiveProcessLimit = 1;
	if (memory_limit) {
		limits.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
		limits.ProcessMemoryLimit = static_cast<SIZE_T>(std::min<uint64_t>(memory_limit, SIZE_MAX));
	}
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

/// A completion port which the job's notifications are posted to
unique_handle watch_job(HANDLE job) {
	// The port is polled from whichever thread is using the host. With a
	// concurrency limit, a queued notification isn't given to one thread
	// while others which polled the port before are running.
	unique_handle port(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, MAXDWORD));
	if (!port)
		throw_win32("Could not create a completion port for the subtitle renderer's job object");
	JOBOBJECT_ASSOCIATE_COMPLETION_PORT association{nullptr, port.get()};
	if (!SetInformationJobObject(job, JobObjectAssociateCompletionPortInformation, &association, sizeof association))
		throw_win32("Could not watch the subtitle renderer's job object");
	return port;
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
	// Members are destroyed in reverse order, and the process has to be
	// killed and have exited before its AppContainer is deleted
	Container container;
	/// Null if there's no memory limit
	unique_handle job_port;
	unique_handle job;
	std::unique_ptr<void, process_killer> process;
	unique_handle pipe;
	unique_handle event;
	ULONGLONG deadline = 0;
	/// In bytes, or 0 for none
	uint64_t memory_limit;
	bool memory_limit_reached = false;

	/// A buffer which frames that weren't allocated by SharedFrameAllocator()
	/// are copied into and out of
	std::unique_ptr<SharedFrameStorage> copy_buffer;

	struct MappedBuffer {
		uint64_t id;
		std::weak_ptr<void> alive;
		uint64_t last_used;
	};
	/// Buffers mapped into the host
	std::vector<MappedBuffer> mapped;
	uint64_t use_count = 0;

	Impl(uint64_t memory_limit);

	/// Start the timeout for an operation, which may consist of several requests
	void StartOperation() { deadline = GetTickCount64() + timeout_ms; }
	/// Throw if the host has reached its memory limit. It may have failed
	/// to allocate memory without crashing, so is no longer trusted to work.
	/// Windows doesn't guarantee that the notification is delivered, so
	/// this can miss it, in which case the host fails some other way.
	void CheckMemoryLimit();

	void Transfer(bool write, char *data, size_t size);
	void Read(void *data, size_t size) { Transfer(false, static_cast<char *>(data), size); }
	void Write(const void *data, size_t size) { Transfer(true, static_cast<char *>(const_cast<void *>(data)), size); }
	Response Call(Request const& req, const void *payload = nullptr);

	/// Make sure buffer is mapped into the host
	void Map(SharedFrameStorage const& buffer);
	void Unmap(size_t index);
	/// Have the host draw on buffer, returning false if it failed to
	bool Render(SharedFrameStorage const& buffer, uint32_t width, uint32_t height, uint32_t pitch, bool flipped, double time);
};

Host::Impl::Impl(uint64_t memory_limit) : memory_limit(memory_limit) {
	auto const& env = environment();
	auto [server, client] = create_pipe();
	job = create_job(memory_limit);
	if (memory_limit)
		job_port = watch_job(job.get());

	SECURITY_CAPABILITIES capabilities{};
	capabilities.AppContainerSid = container.Sid();

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
	auto environment_block = host_environment(container.Folder());

	unique_handle thread;
	auto start = [&](DWORD flags) {
		PROCESS_INFORMATION pi{};
		if (!CreateProcessW(env.exe.c_str(), cmdline.data(), nullptr, nullptr, TRUE,
		                    CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT | flags,
		                    environment_block.data(), env.working_directory.c_str(), &si.StartupInfo, &pi))
			return false;
		process.reset(pi.hProcess);
		thread.reset(pi.hThread);
		return true;
	};

	if (!start(0))
		throw_win32("Could not start aegisub-csri-host.exe");
	if (!AssignProcessToJobObject(job.get(), process.get())) {
		// If Aegisub is running in a job then the host starts in it too, and
		// a job with UI restrictions can't be nested inside another. The
		// host has to break away from Aegisub's job if that's allowed, and
		// otherwise isn't run at all, as it isn't safe without them.
		thread.reset();
		process.reset();
		if (!start(CREATE_BREAKAWAY_FROM_JOB))
			throw_win32("Could not start aegisub-csri-host.exe outside of the job object Aegisub is running in");
		if (!AssignProcessToJobObject(job.get(), process.get()))
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

	StartOperation();
	Hello hello;
	Read(&hello, sizeof hello);
	if (hello.magic != hello_magic || hello.version != protocol_version)
		throw Error("aegisub-csri-host.exe is from a different version of Aegisub");
}

void Host::Impl::Transfer(bool write, char *data, size_t size) {
	try {
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
	catch (Error const&) {
		// Running out of memory is the more useful thing to report
		CheckMemoryLimit();
		throw;
	}
}

void Host::Impl::CheckMemoryLimit() {
	if (job_port) {
		DWORD message;
		ULONG_PTR key;
		OVERLAPPED *ov;
		while (GetQueuedCompletionStatus(job_port.get(), &message, &key, &ov, 0)) {
			if (message == JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT)
				memory_limit_reached = true;
		}
	}
	if (memory_limit_reached)
		throw Error("The subtitle renderer ran out of memory. Its limit of " + std::to_string(memory_limit >> 20) +
			" MB can be raised in the Advanced > Video preferences");
}

Response Host::Impl::Call(Request const& req, const void *payload) {
	Write(&req, sizeof req);
	if (req.payload_size)
		Write(payload, static_cast<size_t>(req.payload_size));

	Response response;
	Read(&response, sizeof response);
	CheckMemoryLimit();

	if (response.status != Status::Ok && response.status != Status::Failed)
		throw Error("The subtitle renderer sent an invalid response");
	if (response.payload_size && req.command != Command::ListRenderers)
		throw Error("The subtitle renderer sent an invalid response");
	return response;
}

void Host::Impl::Map(SharedFrameStorage const& buffer) {
	// Unmap buffers which have been freed, which the host would otherwise
	// keep alive
	for (size_t i = mapped.size(); i-- > 0; ) {
		if (mapped[i].alive.expired())
			Unmap(i);
	}

	auto it = std::find_if(mapped.begin(), mapped.end(), [&](MappedBuffer const& m) { return m.id == buffer.Id(); });
	if (it != mapped.end()) {
		it->last_used = ++use_count;
		return;
	}

	if (mapped.size() >= max_buffers) {
		auto oldest = std::min_element(mapped.begin(), mapped.end(), [](MappedBuffer const& a, MappedBuffer const& b) {
			return a.last_used < b.last_used;
		});
		Unmap(static_cast<size_t>(oldest - mapped.begin()));
	}

	// Only allow mapping the section, not anything like querying or extending it
	HANDLE host_section = nullptr;
	if (!DuplicateHandle(GetCurrentProcess(), buffer.Section(), process.get(), &host_section,
	                     FILE_MAP_READ | FILE_MAP_WRITE, FALSE, 0))
		throw_win32("Could not share a frame buffer with the subtitle renderer");

	Request req{};
	req.command = Command::MapBuffer;
	req.buffer = buffer.Id();
	req.section_handle = reinterpret_cast<uintptr_t>(host_section);
	req.section_size = buffer.Capacity();
	Call(req);
	mapped.push_back({buffer.Id(), buffer.Alive(), ++use_count});
}

void Host::Impl::Unmap(size_t index) {
	Request req{};
	req.command = Command::UnmapBuffer;
	req.buffer = mapped[index].id;
	Call(req);
	mapped.erase(mapped.begin() + index);
}

bool Host::Impl::Render(SharedFrameStorage const& buffer, uint32_t width, uint32_t height, uint32_t pitch, bool flipped, double time) {
	Map(buffer);

	Request req{};
	req.command = Command::Render;
	req.buffer = buffer.Id();
	req.width = width;
	req.height = height;
	req.pitch = pitch;
	req.flipped = flipped;
	req.time = time;
	return Call(req).status == Status::Ok;
}

bool Supported() {
	return supported_windows_version();
}

Host::Host(uint64_t memory_limit) : impl(std::make_unique<Impl>(memory_limit)) { }
Host::~Host() = default;

std::vector<std::string> Host::ListRenderers() {
	Request req{};
	req.command = Command::ListRenderers;
	impl->StartOperation();
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
	impl->StartOperation();
	return impl->Call(req, name.data()).status == Status::Ok;
}

bool Host::LoadSubtitles(const char *data, size_t len) {
	if (len > max_script_size)
		throw Error("The subtitles are too large to render");
	Request req{};
	req.command = Command::LoadSubtitles;
	req.payload_size = len;
	// TODO: the timeout includes VSFilter parsing the script, which hasn't
	// been timed with very large scripts. If it can take longer than
	// timeout_ms, scale the deadline with len.
	impl->StartOperation();
	return impl->Call(req, data).status == Status::Ok;
}

void Host::Render(VideoFrame &frame, double time) {
	int width = frame.width, height = frame.height, pitch = frame.pitch;
	if (width <= 0 || height <= 0 || width > int(max_frame_dimension) || height > int(max_frame_dimension))
		return;
	size_t row_size = size_t(width) * 4;
	if (pitch < 0 || size_t(pitch) < row_size || uint64_t(pitch) * height > frame.data.size())
		return;

	impl->StartOperation();

	if (auto shared = dynamic_cast<SharedFrameStorage *>(frame.data.Storage())) {
		impl->Render(*shared, static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(pitch), frame.flipped, time);
		return;
	}

	size_t size = row_size * height;
	if (!impl->copy_buffer || impl->copy_buffer->Capacity() < size) {
		impl->copy_buffer.reset();
		impl->copy_buffer = create_shared_storage(size);
	}

	// The host gets a top-down frame without padding regardless of how the
	// frame is laid out
	auto row = [&](int y) { return frame.data.data() + size_t(frame.flipped ? height - 1 - y : y) * pitch; };
	unsigned char *buffer = impl->copy_buffer->Data();
	for (int y = 0; y < height; ++y)
		memcpy(buffer + y * row_size, row(y), row_size);

	if (!impl->Render(*impl->copy_buffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(row_size), false, time))
		return;

	for (int y = 0; y < height; ++y)
		memcpy(row(y), buffer + y * row_size, row_size);
}

FrameAllocator SharedFrameAllocator() {
	return [](size_t size) -> std::unique_ptr<FrameStorage> {
		if (size == 0)
			return nullptr;
		try {
			return create_shared_storage(size);
		}
		catch (Error const&) {
			// Use ordinary memory, which is copied when drawing on it
			return nullptr;
		}
	};
}

}
