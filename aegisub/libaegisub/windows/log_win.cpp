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
//
// $Id$

/// @file log.cpp
/// @brief Windows logging
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <stdio.h>
#include <time.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "libaegisub/log.h"
#include "libaegisub/util.h"

namespace agi {
	namespace log {

void EmitSTDOUT::log(SinkMessage *sm) {
	tm tmtime;
	time_t time = sm->tv.tv_sec;
	localtime_s(&tmtime, &time);

	char buff[1024];
	_snprintf_s(buff, _TRUNCATE, "%s (%d): %c %02d:%02d:%02d %-6ld <%-25s> [%s]  %.*s\n",
		sm->file,
		sm->line,
		Severity_ID[sm->severity],
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		sm->tv.tv_usec,
		sm->section,
		sm->func,
		sm->len,
		sm->message);
	OutputDebugStringA(buff);
}
	} // namespace log
} // namespace agi
