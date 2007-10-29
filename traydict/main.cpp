// Copyright (c) 2006, Rodrigo Braz Monteiro
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
//   * Neither the name of the TrayDict Group nor the names of its contributors
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
// TRAYDICT
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/filename.h>
#include "main.h"
#include "dict_window.h"


/////////////////
// Implement app
IMPLEMENT_APP(TrayDict)


/////////////
// Libraries
#if __VISUALC__ >= 1200
#ifdef __WXDEBUG__
#pragma comment(lib, "wxzlibd.lib")
#pragma comment(lib, "wxpngd.lib")
#else
#pragma comment(lib, "wxzlib.lib")
#pragma comment(lib, "wxpng.lib")
#endif

#ifdef __WXDEBUG__
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase28ud.lib")
#pragma comment(lib, "wxbase28ud_net.lib")
#pragma comment(lib, "wxmsw28ud_media.lib")
#pragma comment(lib, "wxmsw28ud_core.lib")
#pragma comment(lib, "wxmsw28ud_adv.lib")
#pragma comment(lib, "wxmsw28ud_aui.lib")
#pragma comment(lib, "wxmsw28ud_html.lib")
#pragma comment(lib, "wxbase28ud_xml.lib")
#pragma comment(lib, "wxexpatd.lib")
#else
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase28u.lib")
#pragma comment(lib, "wxbase28u_net.lib")
#pragma comment(lib, "wxmsw28u_media.lib")
#pragma comment(lib, "wxmsw28u_core.lib")
#pragma comment(lib, "wxmsw28u_adv.lib")
#pragma comment(lib, "wxmsw28u_aui.lib")
#pragma comment(lib, "wxmsw28u_html.lib")
#pragma comment(lib, "wxbase28u_xml.lib")
#pragma comment(lib, "wxexpat.lib")
#endif

#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wsock32.lib")
#endif


//////////////
// Initialize
bool TrayDict::OnInit() {
	// Configuration
	SetAppName(_T("TrayDict"));
	srand((unsigned)time(NULL));

	// Get path
	GetFullPath(argv[0]);
	GetFolderName();

	// Create the window
	DictWindow *window = new DictWindow();
	window->Show();

	// Initialization OK
	return true;
}


/////////////////////////////
// Gets and stores full path
void TrayDict::GetFullPath(wxString arg) {
	if (wxIsAbsolutePath(arg)) {
		fullPath = arg;
		return;
	}

	// Is it a relative path?
	wxString currentDir(wxFileName::GetCwd());
	if (currentDir.Last() != wxFILE_SEP_PATH) currentDir += wxFILE_SEP_PATH;
	wxString str = currentDir + arg;
	if (wxFileExists(str)) {
		fullPath = str;
		return;
	}

    // OK, it's neither an absolute path nor a relative path.
    // Search PATH.
    wxPathList pathList;
    pathList.AddEnvList(_T("PATH"));
    str = pathList.FindAbsoluteValidPath(arg);
	if (!str.IsEmpty()) {
		fullPath = str;
		return;
	}

	fullPath = _T("");
	return;
}


///////////////////////////////////
// Gets folder name from full path
void TrayDict::GetFolderName () {
	folderName = _T("");
	wxFileName path(fullPath);
	folderName += path.GetPath(wxPATH_GET_VOLUME);
	folderName += _T("/");
}


///////////
// Statics
wxString TrayDict::folderName;
wxString TrayDict::fullPath;
