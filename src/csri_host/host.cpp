// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

/// @file host.cpp
/// @brief aegisub-csri-host, which runs CSRI renderers in a sandbox
///
/// Aegisub launches this in a sandbox (see client.cpp). It loads the
/// renderers, enables the mitigations which would have prevented loading
/// them, and then serves requests from Aegisub until the pipe is closed. Any error simply exits the process, which
/// Aegisub treats as a crash.

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
// The process mitigation policy declarations require a Windows 8+ target.
// Aegisub only launches this on Windows 10 1809 or newer.
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#undef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000002 // NTDDI_WIN10_RS1
#include <windows.h>
#include <psapi.h>

#include "protocol.h"

#include <csri/csri.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace csri_host;

namespace {
HANDLE channel = INVALID_HANDLE_VALUE;

[[noreturn]] void fail() {
	ExitProcess(1);
}

void read_exact(void *data, size_t size) {
	auto ptr = static_cast<char *>(data);
	while (size) {
		DWORD read = 0;
		DWORD chunk = static_cast<DWORD>(size < (1u << 20) ? size : (1u << 20));
		if (!ReadFile(channel, ptr, chunk, &read, nullptr) || read == 0) {
			// The pipe closing is how Aegisub tells us to exit
			if (GetLastError() == ERROR_BROKEN_PIPE)
				ExitProcess(0);
			fail();
		}
		ptr += read;
		size -= read;
	}
}

void write_exact(const void *data, size_t size) {
	auto ptr = static_cast<const char *>(data);
	while (size) {
		DWORD written = 0;
		DWORD chunk = static_cast<DWORD>(size < (1u << 20) ? size : (1u << 20));
		if (!WriteFile(channel, ptr, chunk, &written, nullptr) || written == 0)
			fail();
		ptr += written;
		size -= written;
	}
}

void respond(Status status, std::string const& payload = {}) {
	Response response{status, static_cast<uint32_t>(payload.size())};
	write_exact(&response, sizeof response);
	write_exact(payload.data(), payload.size());
}

std::vector<char> read_payload(Request const& req, uint64_t max_size) {
	if (req.payload_size > max_size)
		fail();
	std::vector<char> payload(static_cast<size_t>(req.payload_size));
	read_exact(payload.data(), payload.size());
	return payload;
}

// AppContainers can't open the root of the drive, so GetVolumeInformation
// fails on it. MFC's CFile::Open calls that on the file's drive and gives up
// if it fails, which stops VSFilter from loading the temporary file it writes
// scripts to. The renderers are given these instead, which report a volume
// with no particular name or file system when the real call fails. This
// doesn't give the host access to anything.
BOOL WINAPI volume_information_w(LPCWSTR root, LPWSTR name, DWORD name_size, LPDWORD serial,
                                 LPDWORD max_component_length, LPDWORD flags, LPWSTR fs_name, DWORD fs_name_size) {
	if (GetVolumeInformationW(root, name, name_size, serial, max_component_length, flags, fs_name, fs_name_size))
		return TRUE;
	if (name && name_size) *name = L'\0';
	if (serial) *serial = 0;
	if (max_component_length) *max_component_length = MAX_PATH;
	if (flags) *flags = FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK;
	if (fs_name && fs_name_size) *fs_name = L'\0';
	return TRUE;
}

BOOL WINAPI volume_information_a(LPCSTR root, LPSTR name, DWORD name_size, LPDWORD serial,
                                 LPDWORD max_component_length, LPDWORD flags, LPSTR fs_name, DWORD fs_name_size) {
	if (GetVolumeInformationA(root, name, name_size, serial, max_component_length, flags, fs_name, fs_name_size))
		return TRUE;
	if (name && name_size) *name = '\0';
	if (serial) *serial = 0;
	if (max_component_length) *max_component_length = MAX_PATH;
	if (flags) *flags = FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK;
	if (fs_name && fs_name_size) *fs_name = '\0';
	return TRUE;
}

/// Point module's imports of functions from kernel32 which have a
/// replacement at the replacement. Only affects imports which are bound at
/// load time, not ones looked up with GetProcAddress.
void patch_imports(HMODULE module) {
	struct Replacement {
		const char *name;
		void *function;
	};
	static const Replacement replacements[] = {
		{"GetVolumeInformationW", reinterpret_cast<void *>(&volume_information_w)},
		{"GetVolumeInformationA", reinterpret_cast<void *>(&volume_information_a)},
	};

	auto base = reinterpret_cast<unsigned char *>(module);
	auto dos = reinterpret_cast<IMAGE_DOS_HEADER *>(base);
	auto nt = reinterpret_cast<IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
	auto const& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (!dir.VirtualAddress) return;

	for (auto desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(base + dir.VirtualAddress); desc->Name; ++desc) {
		if (_stricmp(reinterpret_cast<const char *>(base + desc->Name), "kernel32.dll") != 0 || !desc->OriginalFirstThunk)
			continue;
		auto names = reinterpret_cast<IMAGE_THUNK_DATA *>(base + desc->OriginalFirstThunk);
		auto addresses = reinterpret_cast<IMAGE_THUNK_DATA *>(base + desc->FirstThunk);
		for (; names->u1.AddressOfData; ++names, ++addresses) {
			if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
			auto import = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(base + names->u1.AddressOfData);
			for (auto const& r : replacements) {
				if (strcmp(reinterpret_cast<const char *>(import->Name), r.name) != 0) continue;
				DWORD old;
				if (!VirtualProtect(&addresses->u1.Function, sizeof addresses->u1.Function, PAGE_READWRITE, &old))
					break;
				addresses->u1.Function = reinterpret_cast<ULONG_PTR>(r.function);
				VirtualProtect(&addresses->u1.Function, sizeof addresses->u1.Function, old, &old);
				break;
			}
		}
	}
}

/// Patch the imports of the renderers, which are the modules loaded from the
/// csri directory
void patch_renderers() {
	std::vector<wchar_t> path(MAX_PATH);
	for (;;) {
		DWORD len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
		if (len == 0) return;
		if (len < path.size()) break;
		path.resize(path.size() * 2);
	}
	std::wstring csri_dir(path.data());
	csri_dir.resize(csri_dir.find_last_of(L'\\') + 1);
	csri_dir += L"csri\\";

	std::vector<HMODULE> modules(256);
	DWORD needed = 0;
	for (;;) {
		if (!EnumProcessModules(GetCurrentProcess(), modules.data(), static_cast<DWORD>(modules.size() * sizeof(HMODULE)), &needed))
			return;
		if (needed <= modules.size() * sizeof(HMODULE)) break;
		modules.resize(needed / sizeof(HMODULE));
	}
	modules.resize(needed / sizeof(HMODULE));

	for (HMODULE module : modules) {
		DWORD len = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()));
		if (len == 0 || len >= path.size()) continue;
		if (len > csri_dir.size() && CompareStringOrdinal(path.data(), static_cast<int>(csri_dir.size()),
		                                                  csri_dir.c_str(), static_cast<int>(csri_dir.size()), TRUE) == CSTR_EQUAL)
			patch_imports(module);
	}
}

/// Apply the mitigations that can't be enabled when the process is created
/// because they'd stop the renderers themselves from loading
void lock_down() {
	// Arbitrary code guard: no new executable memory. x86 and x64 code
	// emulated on ARM64 is translated at runtime, so this can't be used there.
	bool emulated = false;
#if !defined(_M_ARM64) && !defined(_M_ARM64EC)
	// Looked up at runtime so that the helper still loads if it's missing
	using IsWow64Process2Fn = BOOL (WINAPI *)(HANDLE, USHORT *, USHORT *);
	auto is_wow64_process2 = reinterpret_cast<IsWow64Process2Fn>(
		GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process2"));
	USHORT process_machine = 0, native_machine = 0;
	if (is_wow64_process2 && is_wow64_process2(GetCurrentProcess(), &process_machine, &native_machine))
		emulated = native_machine == IMAGE_FILE_MACHINE_ARM64;
#endif
	if (!emulated) {
		PROCESS_MITIGATION_DYNAMIC_CODE_POLICY dynamic_code{};
		dynamic_code.ProhibitDynamicCode = 1;
		if (!SetProcessMitigationPolicy(ProcessDynamicCodePolicy, &dynamic_code, sizeof dynamic_code))
			fail();
	}

	// Code integrity guard: no more DLLs which aren't signed by Microsoft
	PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY signature{};
	signature.MicrosoftSignedOnly = 1;
	if (!SetProcessMitigationPolicy(ProcessSignaturePolicy, &signature, sizeof signature))
		fail();
}

struct closer {
	void operator()(csri_inst *inst) { if (inst) csri_close(inst); }
};

/// A shared memory section mapped into this process
class Buffer {
	HANDLE section;
	unsigned char *view = nullptr;
	uint64_t size;

public:
	Buffer(HANDLE section, uint64_t size) : section(section), size(size) {
		if (size > SIZE_MAX)
			fail();
		view = static_cast<unsigned char *>(MapViewOfFile(section, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, static_cast<SIZE_T>(size)));
		if (!view)
			fail();
	}
	~Buffer() {
		UnmapViewOfFile(view);
		CloseHandle(section);
	}
	Buffer(Buffer const&) = delete;
	Buffer& operator=(Buffer const&) = delete;

	unsigned char *Data() const { return view; }
	uint64_t Size() const { return size; }
};

class Host {
	csri_rend *renderer = nullptr;
	std::unique_ptr<csri_inst, closer> instance;
	std::map<uint64_t, std::unique_ptr<Buffer>> buffers;

	void ListRenderers() {
		std::string names;
		for (csri_rend *cur = csri_renderer_default(); cur; cur = csri_renderer_next(cur)) {
			const char *name = csri_renderer_info(cur)->name;
			size_t len = strnlen(name, max_renderer_name_size);
			if (len == 0 || len == max_renderer_name_size) continue;
			if (names.size() + len + 1 > max_renderer_list_size) break;
			names.append(name, len);
			names.push_back('\0');
		}
		respond(Status::Ok, names);
	}

	void OpenRenderer(Request const& req) {
		auto payload = read_payload(req, max_renderer_name_size);
		std::string name(payload.begin(), payload.end());

		instance.reset();
		renderer = nullptr;
		for (csri_rend *cur = csri_renderer_default(); cur; cur = csri_renderer_next(cur)) {
			if (name == csri_renderer_info(cur)->name) {
				renderer = cur;
				break;
			}
		}
		respond(renderer ? Status::Ok : Status::Failed);
	}

	void LoadSubtitles(Request const& req) {
		auto script = read_payload(req, max_script_size);
		if (!renderer)
			fail();
		instance.reset(csri_open_mem(renderer, script.data(), script.size(), nullptr));
		respond(instance ? Status::Ok : Status::Failed);
	}

	void MapBuffer(Request const& req) {
		auto handle = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(req.section_handle));
		buffers.erase(req.buffer);
		if (buffers.size() >= max_buffers)
			fail();
		buffers[req.buffer] = std::make_unique<Buffer>(handle, req.section_size);
		respond(Status::Ok);
	}

	void UnmapBuffer(Request const& req) {
		buffers.erase(req.buffer);
		respond(Status::Ok);
	}

	void Render(Request const& req) {
		auto it = buffers.find(req.buffer);
		if (it == buffers.end())
			fail();
		auto const& buffer = *it->second;
		if (req.width == 0 || req.height == 0 || req.width > max_frame_dimension || req.height > max_frame_dimension)
			fail();
		if (req.pitch < uint64_t(req.width) * 4 || uint64_t(req.pitch) * req.height > buffer.Size())
			fail();

		if (instance) {
			csri_frame dst{};
			dst.planes[0] = buffer.Data();
			dst.strides[0] = static_cast<ptrdiff_t>(req.pitch);
			if (req.flipped) {
				dst.planes[0] += size_t(req.height - 1) * req.pitch;
				dst.strides[0] = -dst.strides[0];
			}
			dst.pixfmt = CSRI_F_BGR_;

			csri_fmt format = {dst.pixfmt, req.width, req.height};
			if (!csri_request_fmt(instance.get(), &format))
				csri_render(instance.get(), &dst, req.time);
		}
		respond(Status::Ok);
	}

public:
	void Run() {
		Hello hello{hello_magic, protocol_version};
		write_exact(&hello, sizeof hello);

		for (;;) {
			Request req;
			read_exact(&req, sizeof req);
			switch (req.command) {
				case Command::ListRenderers:  ListRenderers();      break;
				case Command::OpenRenderer:   OpenRenderer(req);    break;
				case Command::LoadSubtitles:  LoadSubtitles(req);   break;
				case Command::MapBuffer:      MapBuffer(req);       break;
				case Command::UnmapBuffer:    UnmapBuffer(req);     break;
				case Command::Render:         Render(req);          break;
				default: fail();
			}
		}
	}
};
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR cmdline, int) {
	// Crashing should just end the process rather than showing a dialog
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	// Don't load DLLs from the current directory
	SetDllDirectoryW(L"");

	const wchar_t *arg = wcsstr(cmdline, L"--pipe=");
	if (!arg)
		return 1;
	wchar_t *end = nullptr;
	auto handle = wcstoull(arg + 7, &end, 10);
	if (end == arg + 7 || handle == 0)
		return 1;
	channel = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(handle));

	// CSRI loads every renderer in the csri directory on the first call to
	// csri_renderer_default(), so do that before locking the process down
	csri_renderer_default();
	patch_renderers();
	lock_down();

	Host().Run();
	return 0;
}
