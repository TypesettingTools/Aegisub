// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once


////////////
// Includes
#include <wx/wxprec.h>
#include <map>


//////////////
// Prototypes
class AssFile;
class AssOverrideParameter;
class DialogFontsCollector;
class FrameMain;


////////////
// Typedefs
typedef std::map<wxString,wxString> FontMap;


/////////////////
// Worker thread
class FontsCollectorThread : public wxThread {
private:
	AssFile *subs;
	wxString destination;
	DialogFontsCollector *collector;
	int curLine;

	static FontsCollectorThread *instance;

	std::map<wxString,wxString> regFonts;
	wxArrayString fonts;

	void Collect();
	wxArrayString GetFontFiles (wxString face);
	void AddFont(wxString fontname,bool isStyle);
	void CollectFontData();

public:
	FontsCollectorThread(AssFile *subs,wxString destination,DialogFontsCollector *collector);
	wxThread::ExitCode Entry();

	static void GetFonts (wxString tagName,int par_n,AssOverrideParameter *param,void *usr);
};


////////////////////
// Class definition
class DialogFontsCollector : public wxDialog {
	friend class FontsCollectorThread;

private:
	wxTextCtrl *DestBox;
	wxTextCtrl *LogBox;
	wxButton *BrowseButton;
	wxButton *StartButton;
	wxButton *CloseButton;
	wxCheckBox *AttachmentCheck;
	wxCheckBox *ArchiveCheck;
	wxStaticText *DestLabel;
	FrameMain *main;

	void OnStart(wxCommandEvent &event);
	void OnClose(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnCheckAttach(wxCommandEvent &event);
	void OnCheckArchive(wxCommandEvent &event);
	void Update();

public:
	DialogFontsCollector(wxWindow *parent);
	~DialogFontsCollector();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	BROWSE_BUTTON = 1100,
	START_BUTTON,
	ATTACHMENT_CHECK,
	ARCHIVE_CHECK
};
