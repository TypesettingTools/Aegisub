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

/// @file platform_unix.cpp
/// @brief Unix Platform extension.

#ifndef R_PRECOMP
#include <wx/string.h>
#include <wx/apptrait.h>
#endif

#include "include/platform.h"
#include "platform_unix.h"

extern "C" {
#include <sys/utsname.h>
}

wxString PlatformUnix::OSVersion() {
	struct utsname name;
	if (uname(&name) != -1) {
		return name.release;
	}
	return "";
}

wxString PlatformUnix::DesktopEnvironment() {
	return wxTheApp->GetTraits()->GetDesktopEnvironment();
}

wxString PlatformUnix::CPUId() {
	return "";
};

wxString PlatformUnix::CPUSpeed() {
	return "";
};

wxString PlatformUnix::CPUCores() {
	return "";
};

wxString PlatformUnix::CPUCount() {
	return "";
};

wxString PlatformUnix::CPUFeatures() {
	return "";
};

wxString PlatformUnix::CPUFeatures2() {
	return "";
};

wxString PlatformUnix::Memory() {
	return "";
};

wxString PlatformUnix::Video() {
	return "";
};

wxString PlatformUnix::UnixLibraries() {
	return "";
};
