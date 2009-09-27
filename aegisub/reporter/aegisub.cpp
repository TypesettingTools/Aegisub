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

/// @file aegisub.cpp
/// @brief Aegisub specific configuration options and properties.
/// @ingroup base

#ifndef R_PRECOMP
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/stdpaths.h>
#endif

#include "aegisub.h"
#include "../acconf.h"

Aegisub::Aegisub() {
	wxStandardPathsBase &paths = wxStandardPaths::Get();
// Using ifdefs is a pain but it's much easier to centralise this.
#if defined(__APPLE__)
	wxString configdir =  wxString::Format("%s-%s", paths.GetUserDataDir(), _T(AEGISUB_VERSION_DATA));
#elif defined(__UNIX__)
	wxString configdir =  wxString::Format("%s/.aegisub-%s", paths.GetUserConfigDir(), _T(AEGISUB_VERSION_DATA));
#else
	wxString configdir =  wxString::Format("%s/Aegisub", paths.GetUserConfigDir());
#endif

	wxFileInputStream file(wxString::Format("%s/config.dat", configdir));
	conf = new wxFileConfig(file);
	conf->SetExpandEnvVars(false);
}

wxString Aegisub::Read(wxString key) {
	return conf->Read(key);
}
