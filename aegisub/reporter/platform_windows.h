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
// $Id: platform_unix.h 3592 2009-09-27 04:15:41Z greg $

/// @file platform_unix.h
/// @see platform_unix.cpp
/// @ingroup unix

class Platform;

/// @brief General Windows functions.
class PlatformWindows : public Platform {
public:
	PlatformWindows() {};
	virtual ~PlatformWindows() {};
	const std::string OSVersion();
	std::string DesktopEnvironment();

	// Hardware
	virtual std::string CPUId();
	virtual std::string CPUSpeed();
	virtual std::string CPUCores();
	virtual std::string CPUCount();
	virtual std::string CPUFeatures();
	virtual std::string CPUFeatures2();
	virtual std::string Memory();

	// OpenGL
	virtual std::string OpenGLVendor();
	virtual std::string OpenGLRenderer();
	virtual std::string OpenGLVersion();
	virtual std::string OpenGLExt();

	// Windows Specific
	virtual std::string ServicePack();
	virtual std::string DriverGraphicsVersion();
	virtual std::string DirectShowFilters();
	virtual std::string AntiVirus();
	virtual std::string Firewall();
	virtual std::string DLLVersions();
};
