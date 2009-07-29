// Copyright (c) 2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file standard_paths.cpp
/// @brief Encode and decode paths relative to various special locations
/// @ingroup utility
///


///////////
// Headers
#include "config.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "standard_paths.h"


////////////////
// Get instance
StandardPaths &StandardPaths::GetInstance() {
	static StandardPaths instance;
	return instance;
}


///////////////
// Constructor
StandardPaths::StandardPaths() {
   wxStandardPathsBase &paths = wxStandardPaths::Get();

#if defined(__UNIX__) && !defined(__APPLE__)
   // Relocation support, this is required to set the prefix to all
   // wx StandardPaths.
   static_cast<wxStandardPaths&>(paths).SetInstallPrefix(wxT(INSTALL_PREFIX));
#endif

	// Get paths
#ifdef __WINDOWS__
	wxString dataDir = paths.GetDataDir();
	wxString userDir = paths.GetUserDataDir();
#elif defined(__APPLE__)
	wxString dataDir = paths.GetDataDir();
	wxString userDir = paths.GetUserDataDir() + _T("-") + _T(AEGISUB_VERSION_DATA);
#else
	wxString dataDir = paths.GetDataDir() + _T("/") + _T(AEGISUB_VERSION_DATA);
	wxString userDir = paths.GetUserDataDir() + _T("-") + _T(AEGISUB_VERSION_DATA);
#endif
	wxString tempDir = paths.GetTempDir();

	// Set paths
	DoSetPathValue(_T("?data"),dataDir);
	DoSetPathValue(_T("?user"),userDir);
	DoSetPathValue(_T("?temp"),tempDir);

	// Create paths if they don't exist
	wxFileName folder(userDir + _T("/"));
	if (!folder.DirExists()) folder.Mkdir(0777,wxPATH_MKDIR_FULL);
}


///////////////
// Decode path
wxString StandardPaths::DoDecodePath(wxString path) {
	// Decode
	if (path[0] == _T('?')) {
		// Split ?part from rest
		path.Replace(_T("\\"),_T("/"));
		int pos = path.Find(_T("/"));
		wxString path1,path2;
		if (pos == wxNOT_FOUND) path1 = path;
		else {
			path1 = path.Left(pos);
			path2 = path.Mid(pos+1);
		}

		// Replace ?part if valid
		std::map<wxString,wxString>::iterator iter = paths.find(path1);
		if (iter == paths.end()) return path;
		wxString final = iter->second + _T("/") + path2;
		final.Replace(_T("//"),_T("/"));
#ifdef WIN32
		final.Replace(_T("/"),_T("\\"));
#endif
		return final;
	}

	// Nothing to decode
	else return path;
}


///////////////
// Encode path
wxString StandardPaths::DoEncodePath(const wxString &path) {
	// TODO
	return path;
}


/////////////////////////
// Set value of a ? path
void StandardPaths::DoSetPathValue(const wxString &path, const wxString &value) {
	paths[path] = value;
}


///////////////////////////////////////////////////////////////////////////
// Decode a path that for legacy reasons might be relative to another path
wxString StandardPaths::DecodePathMaybeRelative(const wxString &path, const wxString &relativeTo) {
	wxFileName res(DecodePath(path));
	if (res.IsRelative())
		res.Assign(DecodePath(relativeTo + _T("/") + path));
	return res.GetFullPath();
}

