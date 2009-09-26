// Copyright (c) 2009, Grigori Goronzy <greg@geekmind.org>
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

/// @file platform_unix_linux.cpp
/// @brief Linux Platform extensions.
/// @ingroup unix

#ifndef R_PRECOMP
#include <wx/string.h>
#include <wx/textfile.h>
#include <wx/log.h>
#endif

extern "C" {
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysctl.h>
}

#include "include/platform.h"
#include "platform_unix.h"
#include "platform_unix_linux.h"


wxString PlatformUnixLinux::CPUId() {
    return getProcValue("/proc/cpuinfo", "model name\t");
};

wxString PlatformUnixLinux::CPUSpeed() {
    return getProcValue("/proc/cpuinfo", "cpu MHz\t\t");
};

wxString PlatformUnixLinux::CPUCores() {
    return "";
};

wxString PlatformUnixLinux::CPUCount() {
    // This returns the index of the last processor.
    // Increment and return as string.
    wxString procIndex = getProcValue("/proc/cpuinfo", "processor\t");
    if (procIndex.IsNumber()) {
        long val;
        procIndex.ToLong(&val);
        return wxString::Format("%ld", val + 1);
    }

    // Fallback
    return "1";
};

wxString PlatformUnixLinux::CPUFeatures() {
    return getProcValue("/proc/cpuinfo", "flags\t\t");
};

wxString PlatformUnixLinux::CPUFeatures2() {
    return "";
};

wxString PlatformUnixLinux::Memory() {
    return getProcValue("/proc/meminfo", "MemTotal");
};

wxString PlatformUnixLinux::Video() {
    return "";
};

wxString PlatformUnixLinux::UnixLibraries() {
    return "";
};

/**
 * @brief Parse a /proc "key: value" style text file and extract a value.
 * @return The last valid value
 */
wxString PlatformUnixLinux::getProcValue(const wxString path, const wxString key) {
    const wxString prefix = wxString(key) + ":";
    wxTextFile *file = new wxTextFile(path);
    wxString val = wxString();

    file->Open();
    for (wxString str = file->GetFirstLine(); !file->Eof(); str = file->GetNextLine()) {
        str.Trim(false);
        if (str.StartsWith(prefix)) {
            val = wxString(str.Mid(key.Len() + 1));
            val.Trim(false);
        }
    }

    delete file;
    return val;
};

