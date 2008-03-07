// Copyright (c) 2005, Niels Martin Hansen
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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

#include <wx/string.h>
#include <wx/datetime.h>
#include "version.h"

#ifdef __WINDOWS__
#include "config.h"
#include "../build/svn-revision.h"
#else
#ifdef __APPLE__
#include "macosx/config.h"
#include "../build/svn-revision.h"

#else

#ifndef BUILD_SVN_REVISION
#define BUILD_SVN_REVISION 0
#endif
#endif
#endif

#define _T_rec(X) _T(X)

#define BUILD_TIMESTAMP _T_rec(__DATE__) _T(" ") _T_rec(__TIME__)

// Define FINAL_RELEASE to mark a build as a "final" version, ie. not pre-release version
// In that case it won't include the SVN revision information

struct VersionInfoStruct {
	// Some raw data
	wxChar *VersionNumber;
	bool IsDebug;
	bool IsRelease;
	int SvnRev;
	wxChar *BuildTime;
	wxChar *BuildCredit;

	// Nice strings for display
	wxString LongVersionString;
	wxString ShortVersionString;

	// Generate the above data
	VersionInfoStruct() {
		wxString SCMStr, VersionStr;

		// Update this whenever a new version is released
		VersionNumber = _T("v2.1.1");
#ifdef _DEBUG
		IsDebug = true;
#else
		IsDebug = false;
#endif
		SvnRev = BUILD_SVN_REVISION;
#ifdef BUILD_SVN_DATE
		BuildTime = _T_rec(BUILD_SVN_DATE);
#else
		BuildTime = BUILD_TIMESTAMP;
#endif
#ifdef BUILD_CREDIT
		BuildCredit = _T_rec(BUILD_CREDIT);
#else
		BuildCredit = _T("anonymous");
#endif

		if (SvnRev > 0) {
			SCMStr = wxString::Format(_T("SVN r%d"), SvnRev);
#ifdef BUILD_SVN_LOCALMODS
			SCMStr += BUILD_SVN_LOCALMODS ? _T("M") : _T("");
#endif
#ifdef BUILD_DARCS
		} else {
			SCMStr = _T("darcs");
#endif
		}

#ifdef FINAL_RELEASE
		IsRelease = true;
#else
		IsRelease = false;
#endif
		VersionStr = wxString::Format(_T("%s%s"), VersionNumber, IsRelease ? _T("") : _T(" RELEASE PREVIEW"));

		LongVersionString = wxString::Format(_T("%s (%s%s, %s)"), VersionStr.c_str(), IsDebug ? _T("debug, ") : _T(""), SCMStr.c_str(), BuildCredit);
		ShortVersionString = wxString::Format(_T("%s %s%s"), VersionStr.c_str(), SCMStr.c_str(), IsDebug ? _T(" debug") : _T(""));

		if (IsRelease && !IsDebug)
			ShortVersionString = LongVersionString = VersionStr;
	}
};


VersionInfoStruct versioninfo;


wxString GetAegisubLongVersionString() {
	return versioninfo.LongVersionString;
}

wxString GetAegisubShortVersionString() {
	return versioninfo.ShortVersionString;
}

wxString GetAegisubBuildTime() {
	return versioninfo.BuildTime;
}

wxString GetAegisubBuildCredit() {
	return versioninfo.BuildCredit;
}

bool GetIsOfficialRelease() {
	return versioninfo.IsRelease;
}

wxString GetVersionNumber() {
	return versioninfo.VersionNumber;
}

int GetSVNRevision() {
	return versioninfo.SvnRev;
}
