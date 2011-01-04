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
// $Id: platform_unix.cpp 3592 2009-09-27 04:15:41Z greg $

/// @file platform_unix.cpp
/// @brief Unix Platform extension.
/// @ingroup unix

#ifndef R_PRECOMP
#include <wx/string.h>
#include <wx/app.h>
#include <wx/apptrait.h>
#endif

#include "include/platform.h"
#include "platform_windows.h"

std::string PlatformWindows::OSVersion() {
	return "";
}

std::string PlatformWindows::DesktopEnvironment() {
	return wxTheApp->GetTraits()->GetDesktopEnvironment();
}

std::string PlatformWindows::CPUId() {
	return "";
};

std::string PlatformWindows::CPUSpeed() {
	return "";
};

std::string PlatformWindows::CPUCores() {
	return "";
};

std::string PlatformWindows::CPUCount() {
	return "";
};

std::string PlatformWindows::CPUFeatures() {
	return "";
};

std::string PlatformWindows::CPUFeatures2() {
	return "";
};

std::string PlatformWindows::Memory() {
	return "";
};

std::string PlatformWindows::ServicePack() {
	return "";
};

std::string PlatformWindows::DriverGraphicsVersion() {
	return "";
};

std::string PlatformWindows::DirectShowFilters() {
	return "";
};

std::string PlatformWindows::AntiVirus() {
	return "";
};

std::string PlatformWindows::Firewall() {
	return "";
};

std::string PlatformWindows::DLLVersions() {
	return "";
};

std::string PlatformWindows::OpenGLVendor() {
	return "";
};

std::string PlatformWindows::OpenGLRenderer() {
	return "";
};

std::string PlatformWindows::OpenGLVersion() {
	return "";
};

std::string PlatformWindows::OpenGLExt() {
	return "";
};
