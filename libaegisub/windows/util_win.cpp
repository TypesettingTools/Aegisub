// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

#include "libaegisub/util.h"

#include "libaegisub/charset_conv_win.h"

#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace agi {
namespace util {

using agi::charset::ConvertW;

std::string ErrorString(int error) {
	LPWSTR lpstr = nullptr;

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, 0,
	                 reinterpret_cast<LPWSTR>(&lpstr), 0, nullptr) == 0) {
		/// @todo Return the actual 'unknown error' string from windows.
		return "Unknown Error";
	}

	std::string str = ConvertW(lpstr);
	LocalFree(lpstr);
	return str;
}

#define MS_VC_EXCEPTION 0x406d1388

/// Parameters for setting the thread name
struct THREADNAME_INFO {
	DWORD dwType;     ///< must be 0x1000
	LPCSTR szName;    ///< pointer to name (in same addr space)
	DWORD dwThreadID; ///< thread ID (-1 caller thread)
	DWORD dwFlags;    ///< reserved for future use, most be zero
};

void SetThreadName(LPCSTR szThreadName) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = -1;
	info.dwFlags = 0;
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*)&info);
	} __except(EXCEPTION_CONTINUE_EXECUTION) {
	}
}

void sleep_for(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace util
} // namespace agi
