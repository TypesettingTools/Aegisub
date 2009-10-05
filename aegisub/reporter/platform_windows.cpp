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

wxString PlatformWindows::OSVersion() {
	return "";
}

wxString PlatformWindows::DesktopEnvironment() {
	return wxTheApp->GetTraits()->GetDesktopEnvironment();
}

wxString PlatformWindows::CPUId() {
	return "";
};

wxString PlatformWindows::CPUSpeed() {
	return "";
};

wxString PlatformWindows::CPUCores() {
	return "";
};

wxString PlatformWindows::CPUCount() {
	return "";
};

wxString PlatformWindows::CPUFeatures() {
	return "";
};

wxString PlatformWindows::CPUFeatures2() {
	return "";
};

wxString PlatformWindows::Memory() {
	return "";
};

wxString PlatformWindows::ServicePack() {
	return "";
};

wxString PlatformWindows::GraphicsVersion() {
	return "";
};

wxString PlatformWindows::DShowFilters() {
	return "";
};

wxString PlatformWindows::AntiVirus() {
	return "";
};

wxString PlatformWindows::Firewall() {
	return "";
};
wxString PlatformWindows::OpenGLVendor() {
	return "";
};

wxString PlatformWindows::OpenGLRenderer() {
	return "";
};

wxString PlatformWindows::OpenGLVersion() {
	return "";
};

wxString PlatformWindows::OpenGLExt() {
	return "";
};
