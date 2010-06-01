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
/// @brief Unix logging
/// @ingroup libaegisub


#include <stdio.h>
#include <time.h>
#include <string.h>

#include "libaegisub/log.h"
#include "libaegisub/util.h"

namespace agi {
	namespace log {

void EmitSTDOUT::log(SinkMessage *sm) {
	tm tmtime;
	localtime_r(&sm->tv.tv_sec, &tmtime);

//		tmtime.tm_year+1900,
//		tmtime.tm_mon,
//		tmtime.tm_mday,

	printf("%c %02d:%02d:%02d %ld <%-25s> [%s:%s:%d]  %s\n",
		Severity_ID[sm->severity],
		tmtime.tm_hour,
		tmtime.tm_min,
		tmtime.tm_sec,
		sm->tv.tv_usec,
		sm->section,
		sm->file,
		sm->func,
		sm->line,
		strndup(sm->message,
		sm->len));
}

	} // namespace log
} // namespace agi
