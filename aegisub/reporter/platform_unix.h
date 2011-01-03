// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
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

/// @file platform_unix.h
/// @see platform_unix.cpp
/// @ingroup unix

class Platform;

/// @brief General Unix functions.
class PlatformUnix : public Platform {
public:
	PlatformUnix() {};
	virtual ~PlatformUnix() {};
	const std::string OSVersion();
	const char* DesktopEnvironment();

	// Hardware
	virtual std::string CPUId() { return ""; }
	virtual std::string CPUSpeed() { return ""; }
	virtual int CPUCores() { return 0; }
	virtual int CPUCount() { return 0; }
	virtual std::string CPUFeatures() { return ""; }
	virtual std::string CPUFeatures2() { return ""; }
	virtual uint64_t Memory() { return 0; }

	// Unix Specific
	virtual std::string UnixLibraries()  { return ""; };
};
