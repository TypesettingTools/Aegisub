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

/// @@file platform_unix_bsd.cpp
/// @brief BSD Platform extensions.

#ifndef R_PRECOMP
#include <wx/string.h>
#endif

extern "C" {
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysctl.h>
}

#include "include/platform.h"
#include "platform_unix.h"
#include "platform_unix_bsd.h"


wxString PlatformUnixBSD::CPUId() {
	char id[300];
	size_t len = sizeof(id);
	sysctlbyname("hw.model", &id, &len, NULL, 0);
	return wxString::Format("%s", id);
};

wxString PlatformUnixBSD::CPUSpeed() {
	return "";
};

wxString PlatformUnixBSD::CPUCores() {
	return "";
};

wxString PlatformUnixBSD::CPUCount() {
	int proc;
	size_t len = sizeof(proc);
	sysctlbyname("hw.ncpu", &proc, &len, NULL, 0);
	return wxString::Format("%d", proc);
};

wxString PlatformUnixBSD::CPUFeatures() {
	return "";
};

wxString PlatformUnixBSD::CPUFeatures2() {
	return "";
};

wxString PlatformUnixBSD::Memory() {
	uint64_t memory;
	size_t len = sizeof(memory);
	sysctlbyname("hw.physmem", &memory, &len, NULL, 0);
	return wxString::Format("%d", memory);
	return "";
};

wxString PlatformUnixBSD::Video() {
	return "";
};

wxString PlatformUnixBSD::UnixLibraries() {
	return "";
};
