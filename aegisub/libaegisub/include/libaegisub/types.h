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

/// @file types.h
/// @brief Platform specific types.
/// @ingroup libaegisub

#ifndef LAGI_PRE

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#else
#  include <time.h>
#endif

#endif // LAGI_PRE

#pragma once

namespace agi {
	namespace type {

		namespace win {
typedef struct timeval {
	long tv_sec;	///< seconds
	long tv_usec;	///< microseconds
} timeval;


typedef struct tm {
    int tm_sec;			///< seconds (0-59)
    int tm_min;			///< minutes (0-59)
    int tm_hour;		///< hours (0-23)
    int tm_mday;		///< day of the month (1-31)
    int tm_mon;			///< months since january (0-11)
    int tm_year;		///< years since 1900
    int tm_wday;		///< day of the week since sunday (0-6)
    int tm_yday;		///< days since january 1 (0-365)
    int tm_isdst;		///< whether in DST or not
    long    tm_gmtoff;	///< GMT offset in seconds
    char    *tm_zone;	///< TZ abrivation
} tm;

		} // namespace win



		// u_nix beacuse some compilers set "unix" to 1 if it's unix. -> ARGH.
		namespace u_nix {
		} // namespace unix

		namespace osx {
		} // namespace osx


	} // namespace type
} // namespace agi


#ifdef _WIN32

typedef agi::type::win::timeval agi_timeval;

#else // Unix / OSX

typedef timeval agi_timeval;

#endif // if _WIN32
