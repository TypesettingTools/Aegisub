// Copyright (c) 2026, Ryan Lucia <ryan@luciaonline.net>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

/// @file cli_launcher_win.cpp
/// @brief aegisub-cli.exe, which runs aegisub.exe in CLI mode from a console
///
/// aegisub.exe is a GUI-subsystem program so that starting it normally
/// doesn't open a console window, but that also means shells don't wait for
/// it to finish and don't show its output. This console program runs it and
/// waits for it, the same way as mpv.com does for mpv.exe.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdio>
#include <string>

int wmain() {
	// aegisub.exe is next to this program
	std::wstring exe(MAX_PATH, L'\0');
	for (;;) {
		DWORD len = GetModuleFileNameW(nullptr, exe.data(), static_cast<DWORD>(exe.size()));
		if (len == 0)
			return 1;
		if (len < exe.size()) {
			exe.resize(len);
			break;
		}
		exe.resize(exe.size() * 2);
	}
	exe.resize(exe.find_last_of(L'\\') + 1);
	exe += L"aegisub.exe";

	// Pass on redirected standard handles, e.g. pipes from a program capturing
	// the output. Console handles are left out, and aegisub.exe attaches to
	// this program's console for those instead.
	auto redirected = [](DWORD which) -> HANDLE {
		HANDLE handle = GetStdHandle(which);
		DWORD mode;
		return GetConsoleMode(handle, &mode) ? nullptr : handle;
	};
	STARTUPINFOW si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = redirected(STD_INPUT_HANDLE);
	si.hStdOutput = redirected(STD_OUTPUT_HANDLE);
	si.hStdError = redirected(STD_ERROR_HANDLE);

	// Take aegisub.exe down with this program if it's killed, e.g. by a
	// timeout in whatever is running it
	HANDLE job = CreateJobObjectW(nullptr, nullptr);
	if (job) {
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
		limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
	}

	// The command line is passed on unchanged, so aegisub.exe sees that it
	// was started as aegisub-cli, which implies --cli
	std::wstring command_line = GetCommandLineW();
	PROCESS_INFORMATION pi{};
	if (!CreateProcessW(exe.c_str(), command_line.data(), nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
		fwprintf(stderr, L"aegisub-cli: could not start %ls (error %lu)\n", exe.c_str(), GetLastError());
		return 1;
	}
	if (job)
		AssignProcessToJobObject(job, pi.hProcess);
	ResumeThread(pi.hThread);
	CloseHandle(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code = 1;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	return static_cast<int>(exit_code);
}
