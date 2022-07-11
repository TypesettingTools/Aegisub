// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "crash_writer.h"

#include "version.h"

#include <libaegisub/format.h>
#include <libaegisub/fs.h>
#include <libaegisub/make_unique.h>
#include <libaegisub/util.h>

#include <atomic>
#include <boost/filesystem/fstream.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <DbgHelp.h>
#include <Windows.h>

extern EXCEPTION_POINTERS *wxGlobalSEInformation;

namespace {
wchar_t crash_dump_path[MAX_PATH];
agi::fs::path crashlog_path;

using MiniDumpWriteDump = BOOL(WINAPI *)(
	HANDLE hProcess,
	DWORD dwPid,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

struct dump_thread_state {
	std::mutex start_mutex;
	std::condition_variable start_cv;

	std::atomic<bool> exit = false;
	EXCEPTION_POINTERS *ep = nullptr;
	DWORD thread_id = 0;

	// Must be last so everything else is initialized before it
	std::thread thread;

	dump_thread_state() : thread([&] { main(); }) { }

	void main() {
		auto module = LoadLibrary(L"dbghelp.dll");
		if (!module) return;

		auto fn = reinterpret_cast<MiniDumpWriteDump>(GetProcAddress(module, "MiniDumpWriteDump"));
		if (!fn) {
			FreeLibrary(module);
			return;
		}

		std::unique_lock<std::mutex> lock(start_mutex);
		start_cv.wait(lock, [&] { return ep || exit; });
		if (ep)
			write_dump(fn);
		FreeLibrary(module);
	}

	void write_dump(MiniDumpWriteDump fn) {
		auto file = CreateFile(crash_dump_path,
			GENERIC_WRITE,
			0,  // no sharing
			nullptr,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (file == INVALID_HANDLE_VALUE) return;

		MINIDUMP_EXCEPTION_INFORMATION info;
		info.ThreadId = thread_id;
		info.ExceptionPointers = ep;
		info.ClientPointers = FALSE;

		fn(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, nullptr, nullptr);

		CloseHandle(file);
	}
};

std::unique_ptr<dump_thread_state> dump_thread;
}

namespace crash_writer {
void Initialize(agi::fs::path const& path) {
	crashlog_path = path / "crashlog.txt";

	auto dump_path = path / "crashdumps";
	agi::fs::CreateDirectory(dump_path);

	const auto path_str = (dump_path / GetVersionNumber()).wstring();
	wcscpy_s(crash_dump_path, path_str.c_str());
	auto len = path_str.size();

	const auto t = time(nullptr);
	struct tm tm;
	localtime_s(&tm, &t);

	len += wcsftime(crash_dump_path + len, MAX_PATH - len, L"-%Y-%m-%d-%H-%M-%S-", &tm);
	len += swprintf_s(crash_dump_path + len, MAX_PATH - len, L"%d", GetCurrentProcessId());
	wcscpy_s(crash_dump_path + len, MAX_PATH - len, L".dmp");

	if (!dump_thread)
		dump_thread = agi::make_unique<dump_thread_state>();
}

void Cleanup() {
	dump_thread->exit = true;
	dump_thread->start_cv.notify_all();
	dump_thread->thread.join();
	dump_thread.reset();
}

void Write() {
	dump_thread->ep = wxGlobalSEInformation;
	dump_thread->thread_id = GetCurrentThreadId();
	dump_thread->start_cv.notify_all();
	dump_thread->thread.join();
	dump_thread.reset();
}

void Write(std::string const& error) {
	boost::filesystem::ofstream file(crashlog_path, std::ios::app);
	if (file.is_open()) {
		file << agi::util::strftime("--- %y-%m-%d %H:%M:%S ------------------\n");
		agi::format(file, "VER - %s\n", GetAegisubLongVersionString());
		agi::format(file, "EXC - Aegisub has crashed with unhandled exception \"%s\".\n", error);
		file << "----------------------------------------\n\n";
	}
}
}
