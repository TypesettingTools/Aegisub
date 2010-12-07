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

/// @file dialog_fonts_collector.h
/// @see dialog_fonts_collector.cpp
/// @ingroup tools_ui font_collector
///

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>
#endif

class AssFile;
class AssOverrideParameter;
class DialogFontsCollector;
class wxZipOutputStream;
class ScintillaTextCtrl;

/// DOCME
/// @class FontsCollectorThread
/// @brief DOCME
///
/// DOCME
class FontsCollectorThread : public wxThread {
	/// DOCME
	AssFile *subs;

	/// DOCME
	AssStyle *curStyle;

	/// DOCME
	wxString destination;

	/// DOCME
	DialogFontsCollector *collector;

	/// DOCME
	wxZipOutputStream *zip;

	/// DOCME
	int curLine;

	/// DOCME
	wxString destFolder;


	/// DOCME
	static FontsCollectorThread *instance;


	/// DOCME
	wxArrayString fonts;

	bool ProcessFont(wxString fontname);
	int CopyFont(wxString filename);
	bool ArchiveFont(wxString filename);
	bool AttachFont(wxString filename);

	void Collect();
	void AddFont(wxString fontname,int mode);
	void CollectFontData();
	void AppendText(wxString text,int colour=0);

public:
	FontsCollectorThread(AssFile *subs,wxString destination,DialogFontsCollector *collector);
	wxThread::ExitCode Entry();

	static void GetFonts (wxString tagName,int par_n,AssOverrideParameter *param,void *usr);
};

/// DOCME
/// @class DialogFontsCollector
/// @brief DOCME
///
/// DOCME
class DialogFontsCollector : public wxDialog {
	friend class FontsCollectorThread;

	AssFile *subs;

	/// DOCME
	wxTextCtrl *DestBox;

	/// DOCME
	ScintillaTextCtrl *LogBox;

	/// DOCME
	wxButton *BrowseButton;

	/// DOCME
	wxButton *StartButton;

	/// DOCME
	wxButton *CloseButton;

	/// DOCME
	wxStaticText *DestLabel;

	/// DOCME
	wxRadioBox *CollectAction;

	void OnStart(wxCommandEvent &event);
	void OnClose(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnRadio(wxCommandEvent &event);
	void OnAddText(wxCommandEvent &event);
	void Update(int value=-1);

public:
	DialogFontsCollector(wxWindow *parent, AssFile *subs);
	~DialogFontsCollector();

	DECLARE_EVENT_TABLE()
};



/// DOCME
struct ColourString {

	/// DOCME
	wxString text;

	/// DOCME
	int colour;
};
