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

#include "libaegisub/log.h"

#include "libaegisub/charset_conv_win.h"
#include "libaegisub/make_unique.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace agi { namespace log {
void EmitSTDOUT::log(SinkMessage const& sm) {
	tm tmtime;
	time_t time = sm.time / 1000000000;
	localtime_s(&tmtime, &time);

	char buff[65536];
	_snprintf_s(buff, _TRUNCATE, "%s (%d): %c %02d:%02d:%02d.%-3ld <%-25s> [%s]  %.*s\n",
		sm.file,
		sm.line,
		Severity_ID[sm.severity],
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		(long)(sm.time % 1000000000 / 1000000),
		sm.section,
		sm.func,
		(int)sm.message.size(),
		sm.message.c_str());
	OutputDebugStringW(charset::ConvertW(buff).c_str());
}
} }
