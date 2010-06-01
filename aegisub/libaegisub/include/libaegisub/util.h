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

/// @file util.h
/// @brief Public interface for general utilities.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <string>
#include <stdio.h>
#ifdef _WIN32
#  include <time.h>
#else
#  include <sys/time.h>
#endif // _WIN32
#endif // LAGI_PRE

#include <libaegisub/access.h>

namespace agi {
	namespace util {

	const std::string DirName(const std::string& path);
	void Rename(const std::string& from, const std::string& to);
	void time_log(timeval &tv);

	} // namespace util
} // namespace agi
