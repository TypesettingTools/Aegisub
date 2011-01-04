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

/// @file platform_unix_bsd.cpp
/// @brief BSD Platform extensions.
/// @ingroup unix

#ifndef R_PRECOMP
#include <wx/string.h>
#endif

extern "C" {
#include <sys/types.h>
#include <sys/sysctl.h>
}

#include "include/platform.h"
#include "platform_unix.h"
#include "platform_unix_osx.h"


std::string PlatformUnixOSX::CPUId() {
	char id[300];
	size_t len = sizeof(id);
	sysctlbyname("machdep.cpu.brand_string", &id, &len, NULL, 0);
	return wxString::Format("%s", id);
};

std::string PlatformUnixOSX::CPUSpeed() {
	uint64_t speed;
	size_t len = sizeof(speed);
	sysctlbyname("hw.cpufrequency_max", &speed, &len, NULL, 0);
	return wxString::Format("%d", speed / (1000*1000));
};

std::string PlatformUnixOSX::CPUCores() {
	return "";
};

std::string PlatformUnixOSX::CPUCount() {
	int proc;
	size_t len = sizeof(proc);
	sysctlbyname("hw.ncpu", &proc, &len, NULL, 0);
	return wxString::Format("%d", proc);
};

std::string PlatformUnixOSX::CPUFeatures() {
	char feat[300];
	size_t len = sizeof(feat);
	sysctlbyname("machdep.cpu.features", &feat, &len, NULL, 0);
	return wxString::Format("%s", feat);
};

std::string PlatformUnixOSX::CPUFeatures2() {
	char feat[128];
	size_t len = sizeof(feat);
	sysctlbyname("machdep.cpu.extfeatures", &feat, &len, NULL, 0);
	return wxString::Format("%s", feat);
	return "";
};

std::string PlatformUnixOSX::Memory() {
	uint64_t memory;
	size_t len = sizeof(memory);
	sysctlbyname("hw.memsize", &memory, &len, NULL, 0);
	return wxString::Format("%llu", memory);
};

std::string PlatformUnixOSX::UnixLibraries() {
	return "";
};

std::string PlatformUnixOSX::PatchLevel() {
	return "";
}

std::string PlatformUnixOSX::QuickTimeExt() {
	return "";
}

std::string PlatformUnixOSX::HardwareModel() {
	char model[300];
	size_t len = sizeof(model);
	sysctlbyname("hw.model", &model, &len, NULL, 0);
	return wxString::Format("%s", model);
}

