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

/// @file dialog_version_check.cpp
/// @brief Version Checker dialogue box and logic
/// @ingroup configuration_ui
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/event.h>
#include <wx/filesys.h>
#include <wx/fs_inet.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>
#endif

#include "dialog_version_check.h"
#include "main.h"
#include "version.h"


/// DOCME
bool DialogVersionCheck::dialogRunning = false;


/// @brief Constructor 
/// @param parent 
/// @param hidden 
/// @return 
///
DialogVersionCheck::DialogVersionCheck(wxWindow *parent,bool hidden)
: wxDialog(parent,-1,_("Version Checker"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE)
{
	// Is already running?
	if (dialogRunning) {
		Destroy();
		return;
	}
	dialogRunning = true;

	// Controls
	wxSizer *controlSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Log"));
	logBox = new wxTextCtrl(this,Log_Box,_T(""),wxDefaultPosition,wxSize(400,150),wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_AUTO_URL);
	controlSizer->Add(logBox,1,wxEXPAND,0);

	// Buttons
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(new wxButton(this,wxID_OK),0,0,0);

	// Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(controlSizer,1,wxEXPAND | wxALL,5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxALL,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);

	// Show
	visible = false;
	if (!hidden) {
		Show();
		CenterOnParent();
		visible = true;
	}

	// Check version
	CheckVersion();
}



/// @brief Destructor 
///
DialogVersionCheck::~DialogVersionCheck() {
	dialogRunning = false;
}



/// @brief Check Version 
///
void DialogVersionCheck::CheckVersion() {
	if (!wxFileSystem::HasHandlerForPath(_T("http://www.aegisub.net/"))) wxFileSystem::AddHandler(new wxInternetFSHandler);
	thread = new VersionCheckThread(this);
	thread->Create();
	thread->Run();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogVersionCheck,wxDialog)
	EVT_BUTTON (wxID_OK, DialogVersionCheck::OnOK)
	EVT_CLOSE (DialogVersionCheck::OnClose)
	EVT_TEXT_URL(Log_Box,DialogVersionCheck::OnURL)
END_EVENT_TABLE()



/// @brief On close 
/// @param event 
///
void DialogVersionCheck::OnClose(wxCloseEvent &event) {
	dialogRunning = false;
	if (thread) thread->alive = false;
	Destroy();
}

/// @brief DOCME
/// @param event 
///
void DialogVersionCheck::OnOK(wxCommandEvent &event) {
	dialogRunning = false;
	if (thread) thread->alive = false;
	Destroy();
}



/// @brief On Click URL 
/// @param event 
///
void DialogVersionCheck::OnURL(wxTextUrlEvent &event) {
	wxMouseEvent mEvent = event.GetMouseEvent();
	if (mEvent.LeftDown()) {
		wxString url = logBox->GetValue().Mid(event.GetURLStart(),event.GetURLEnd()-event.GetURLStart()+1);
		AegisubApp::OpenURL(url);
	}
}



/// @brief Worker thread constructor 
/// @param par 
///
VersionCheckThread::VersionCheckThread(DialogVersionCheck *par)
: wxThread(wxTHREAD_DETACHED)
{
	parent = par;
	alive = true;
}



/// @brief Worker thread 
///
wxThread::ExitCode VersionCheckThread::Entry() {
	// List of paths
	wxArrayString paths;
	paths.Add(_T("http://updates.aegisub.org/latest.txt"));
	paths.Add(_T("http://files.aegisub.net/latest.txt"));
	wxFSFile *fp = NULL;
	
	// Try each path until it finds one that works
	for (unsigned int i=0;i<paths.Count();i++) {
		// Get path and make sure that it has a handle for it
		wxString path = wxString::Format(_T("%s?current=%d"), paths[i].c_str(), GetSVNRevision());
		wxMutexGuiEnter();
		if (!alive) goto endThread;
		if (!wxFileSystem::HasHandlerForPath(path)) {
			parent->logBox->AppendText(_("Could not open Internet File system. Aborting.\n"));
			goto endThread;
		}
		wxMutexGuiLeave();

		// Open file
#ifdef __WXDEBUG__
		wxMutexGuiEnter();
		if (!alive) goto endThread;
		parent->logBox->AppendText(wxString::Format(_("Attempting to open \"%s\"..."), path.c_str()));
		wxMutexGuiLeave();
#endif
		wxFileSystem fs;
		fp = fs.OpenFile(path);

		// Failed?
		if (!fp) {
#ifdef __WXDEBUG__
			wxMutexGuiEnter();
			if (!alive) goto endThread;
			parent->logBox->AppendText(_("Failed.\n"));
			wxMutexGuiLeave();
#endif
			continue;
		}

		// OK
#ifdef __WXDEBUG__
		wxMutexGuiEnter();
		if (!alive) goto endThread;
		parent->logBox->AppendText(_("OK.\n"));
		wxMutexGuiLeave();
#endif

		// Get text input stream
		wxInputStream *fstream = fp->GetStream();
		wxTextInputStream text(*fstream);
		wxString contents;

		// What kind of version is this?
		wxString versionKind;
		double versionN;
		int svnRev = GetSVNRevision();
		wxString temp = GetVersionNumber();
		temp.ToDouble(&versionN);
		if (GetIsOfficialRelease()) versionKind = _T("release");
		else versionKind = _T("svn");
		
		// Read lines
		while (!fstream->Eof()) {
			// Parse
			contents = text.ReadLine();
			wxStringTokenizer tkn(contents,_T(";"),wxTOKEN_RET_EMPTY_ALL);
			wxArrayString parsed;
			while (tkn.HasMoreTokens()) {
				parsed.Add(tkn.GetNextToken());
			}
			if (parsed.Count() != 4) continue;

			// See if it's interesting
			if (parsed[0] == versionKind || parsed[0] == _T("release")) {
				bool newer = false;
				
				// Check
				if (parsed[0] == _T("svn")) {
					long temp;
					parsed[1].ToLong(&temp);
					if (temp > svnRev) newer = true;
				}
				else {
					int pos = parsed[1].Find(_T('-'));
					wxString vName = parsed[1].Left(pos);
					wxString svnName = parsed[1].Mid(pos+1);
					if (versionKind == _T("svn")) {
						long temp;
						svnName.ToLong(&temp);
						if (temp >= svnRev) newer = true;
					}
					else {
						double temp;
						vName.ToDouble(&temp);
						if (temp > versionN) newer = true;
					}
				}

				// Is newer?
				if (newer) {
					wxMutexGuiEnter();
					if (!alive) goto endThread;
					parent->logBox->AppendText(_("New version found!\n"));
					parent->logBox->AppendText(parsed[3] + _T("\n"));
					parent->logBox->AppendText(wxString::Format(_("Please go to the following URL to download it: %s\n"), parsed[2].c_str()));
					if (!parent->visible) {
						parent->Show();
						parent->Raise();
						parent->visible = true;
					}
					goto endThread;
				}
			}
		}

		// No new updates
		wxMutexGuiEnter();
		if (!alive) goto endThread;
		parent->logBox->AppendText(_("No new version has been found.\n"));
		wxMutexGuiLeave();

		// Delete file
		delete fp;
		fp = NULL;
		break;
	}

	// Finish thread
	wxMutexGuiEnter();
endThread:
	if (alive && !parent->visible) parent->Destroy();
	wxMutexGuiLeave();
	delete fp;
	fp = NULL;
	return 0;
}


