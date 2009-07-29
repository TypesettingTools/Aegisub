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

/// @file dialog_fonts_collector.cpp
/// @brief Font collector dialogue box
/// @ingroup tools_ui font_collector
///


////////////
// Includes
#include "config.h"

#include <wx/config.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/fontenum.h>
#include "ass_override.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "dialog_fonts_collector.h"
#include "utils.h"
#include "options.h"
#include "frame_main.h"
#include "subs_grid.h"
#include "main.h"
#include "font_file_lister.h"
#include "utils.h"
#include "help_button.h"
#include "scintilla_text_ctrl.h"
#include "libresrc/libresrc.h"




/// DOCME
enum IDs {

	/// DOCME
	START_BUTTON = 1150,

	/// DOCME
	BROWSE_BUTTON,

	/// DOCME
	RADIO_BOX
};


/////////
// Event
DECLARE_EVENT_TYPE(EVT_ADD_TEXT, -1)
DEFINE_EVENT_TYPE(EVT_ADD_TEXT)



/// @brief Constructor 
/// @param parent 
///
DialogFontsCollector::DialogFontsCollector(wxWindow *parent)
: wxDialog(parent,-1,_("Fonts Collector"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(font_collector_button_24)));

	// Parent
	main = (FrameMain*) parent;

	// Destination box
	wxString dest = Options.AsText(_T("Fonts Collector Destination"));
	if (dest == _T("?script")) {
		wxFileName filename(AssFile::top->filename);
		dest = filename.GetPath();
	}
	while (dest.Right(1) == _T("/")) dest = dest.Left(dest.Length()-1);
	DestBox = new wxTextCtrl(this,-1,dest,wxDefaultPosition,wxSize(250,20),0);
	BrowseButton = new wxButton(this,BROWSE_BUTTON,_("&Browse..."));
	wxSizer *DestBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	DestLabel = new wxStaticText(this,-1,_("Choose the folder where the fonts will be collected to.\nIt will be created if it doesn't exist."));
	DestBottomSizer->Add(DestBox,1,wxEXPAND | wxRIGHT,5);
	DestBottomSizer->Add(BrowseButton,0,0,0);
	wxSizer *DestSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Destination"));
	DestSizer->Add(DestLabel,0,wxEXPAND | wxBOTTOM,5);
	DestSizer->Add(DestBottomSizer,0,wxEXPAND,0);

	// Action radio box
	wxArrayString choices;
	choices.Add(_("Check fonts for availability"));
	choices.Add(_("Copy fonts to folder"));
	choices.Add(_("Copy fonts to zipped archive"));
	choices.Add(_("Attach fonts to current subtitles"));
#ifdef __WXDEBUG__
	choices.Add(_("DEBUG: Verify all fonts in system"));
#endif
	CollectAction = new wxRadioBox(this,RADIO_BOX,_T("Action"),wxDefaultPosition,wxDefaultSize,choices,1);
	size_t lastAction = Options.AsInt(_T("Fonts Collector Action"));
	if (lastAction >= choices.GetCount()) lastAction = 0;
	CollectAction->SetSelection(lastAction);

	// Log box
	LogBox = new ScintillaTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(300,210));
	LogBox->SetWrapMode(wxSTC_WRAP_WORD);
	LogBox->SetMarginWidth(1,0);
	LogBox->SetReadOnly(true);
	LogBox->StyleSetForeground(1,wxColour(0,200,0));
	LogBox->StyleSetForeground(2,wxColour(200,0,0));
	LogBox->StyleSetForeground(3,wxColour(200,100,0));
	wxSizer *LogSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Log"));
	LogSizer->Add(LogBox,1,wxEXPAND,0);

	// Buttons sizer
	StartButton = new wxButton(this,START_BUTTON,_("&Start!"));
	CloseButton = new wxButton(this,wxID_CANCEL);
	StartButton->SetDefault();
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(StartButton);
	ButtonSizer->AddButton(CloseButton);
	ButtonSizer->AddButton(new HelpButton(this,_T("Fonts Collector")));
	ButtonSizer->SetAffirmativeButton(StartButton);
	ButtonSizer->Realize();

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(CollectAction,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->Add(DestSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->Add(LogSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	CenterOnParent();

	// Run dummy event to update label
	Update();
}



/// @brief Destructor 
///
DialogFontsCollector::~DialogFontsCollector() {
	FontFileLister::ClearData();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogFontsCollector, wxDialog)
	EVT_BUTTON(START_BUTTON,DialogFontsCollector::OnStart)
	EVT_BUTTON(BROWSE_BUTTON,DialogFontsCollector::OnBrowse)
	EVT_BUTTON(wxID_CLOSE,DialogFontsCollector::OnClose)
	EVT_RADIOBOX(RADIO_BOX,DialogFontsCollector::OnRadio)
	EVT_COMMAND(0,EVT_ADD_TEXT,DialogFontsCollector::OnAddText)
END_EVENT_TABLE()



/// @brief Start processing 
/// @param event 
/// @return 
///
void DialogFontsCollector::OnStart(wxCommandEvent &event) {
	// Clear
	LogBox->SetReadOnly(false);
	LogBox->ClearAll();
	LogBox->SetReadOnly(true);

	// Action being done
	int action = CollectAction->GetSelection();

	// Check if it's OK to do it
	wxString foldername = DestBox->GetValue();
	if (action == 1) foldername += _T("//");
	wxFileName folder(foldername);
	bool isFolder = folder.IsDir();

	// Check if it's a folder
	if (action == 1 && !isFolder) {
		wxMessageBox(_("Invalid destination."),_("Error"),wxICON_EXCLAMATION | wxOK);
		return;
	}

	// Make folder if it doesn't exist
	if (action == 1 || action == 2) {
		if (!folder.DirExists()) {
			folder.Mkdir(0777,wxPATH_MKDIR_FULL);
			if (!folder.DirExists()) {
				wxMessageBox(_("Could not create destination folder."),_("Error"),wxICON_EXCLAMATION | wxOK);
				return;
			}
		}
	}

	// Check if we have a valid archive name
	if (action == 2) {
		if (isFolder || folder.GetName().IsEmpty() || folder.GetExt() != _T("zip")) {
			wxMessageBox(_("Invalid path for .zip file."),_("Error"),wxICON_EXCLAMATION | wxOK);
			return;
		}
	}

	// Start thread
	wxThread *worker = new FontsCollectorThread(AssFile::top,foldername,this);
	worker->Create();
	worker->Run();

	// Set options
	if (action == 1 || action == 2) {
		wxString dest = foldername;
		wxFileName filename(AssFile::top->filename);
		if (filename.GetPath() == dest) {
			dest = _T("?script");
		}
		Options.SetText(_T("Fonts Collector Destination"),dest);
	}
	Options.SetInt(_T("Fonts Collector Action"),action);
	Options.Save();

	// Set buttons
	StartButton->Enable(false);
	BrowseButton->Enable(false);
	DestBox->Enable(false);
	CloseButton->Enable(false);
	CollectAction->Enable(false);
	DestLabel->Enable(false);
	if (!worker->IsDetached()) worker->Wait();
}



/// @brief Close dialog 
/// @param event 
///
void DialogFontsCollector::OnClose(wxCommandEvent &event) {
	EndModal(0);
}



/// @brief Browse location 
/// @param event 
///
void DialogFontsCollector::OnBrowse(wxCommandEvent &event) {
	// Chose file name
	if (CollectAction->GetSelection()==2) {
		wxFileName fname(DestBox->GetValue());
		wxString dest = wxFileSelector(_("Select archive file name"),DestBox->GetValue(),fname.GetFullName(),_T(".zip"),_("Zip Archives (*.zip)|*.zip"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (!dest.empty()) {
			DestBox->SetValue(dest);
		}
	}

	// Choose folder
	else {
		wxString dest = wxDirSelector(_("Select folder to save fonts on"),DestBox->GetValue(),0);
		if (!dest.empty()) {
			DestBox->SetValue(dest);
		}
	}
}



/// @brief Radio box changed 
/// @param event 
///
void DialogFontsCollector::OnRadio(wxCommandEvent &event) {
	Update(event.GetInt());
}



/// @brief Update controls 
/// @param value 
///
void DialogFontsCollector::Update(int value) {
	// Enable buttons
	CloseButton->Enable(true);
	StartButton->Enable(true);
	CollectAction->Enable(true);
	wxString dst = DestBox->GetValue();

	// Get value if -1
	if (value == -1) {
		value = CollectAction->GetSelection();
	}

	// Check or attach
	if (value == 0 || value == 3) {
		DestBox->Enable(false);
		BrowseButton->Enable(false);
		DestLabel->Enable(false);
		DestLabel->SetLabel(_T("N/A\n"));
	}

	// Collect to folder
	else if (value == 1) {
		DestBox->Enable(true);
		BrowseButton->Enable(true);
		DestLabel->Enable(true);
		DestLabel->SetLabel(_("Choose the folder where the fonts will be collected to.\nIt will be created if it doesn't exist."));

		// Remove filename from browse box
		if (dst.Right(4) == _T(".zip")) {
			wxFileName fn(dst);
			DestBox->SetValue(fn.GetPath());
		}
	}

	// Collect to zip
	else if (value == 2) {
		DestBox->Enable(true);
		BrowseButton->Enable(true);
		DestLabel->Enable(true);
		DestLabel->SetLabel(_("Enter the name of the destination zip file to collect the fonts to.\nIf a folder is entered, a default name will be used."));

		// Add filename to browse box
		if (dst.Right(4) != _T(".zip")) {
			wxFileName fn(dst + _T("//"));
			fn.SetFullName(_T("fonts.zip"));
			DestBox->SetValue(fn.GetFullPath());
		}
	}
}



/// @brief Add text 
/// @param event 
///
void DialogFontsCollector::OnAddText(wxCommandEvent &event) {
	ColourString *str = (ColourString*) event.GetClientData();
	LogBox->SetReadOnly(false);
	int pos = LogBox->GetReverseUnicodePosition(LogBox->GetLength());
	LogBox->AppendText(str->text);
	if (str->colour) {
		LogBox->StartUnicodeStyling(pos,31);
		LogBox->SetUnicodeStyling(pos,str->text.Length(),str->colour);
	}
	delete str;
	LogBox->GotoPos(pos);
	LogBox->SetReadOnly(true);
}



/// @brief Collect font files 
///
void FontsCollectorThread::CollectFontData () {
	FontFileLister::Initialize();
}



/// @brief Collector thread 
/// @param _subs        
/// @param _destination 
/// @param _collector   
///
FontsCollectorThread::FontsCollectorThread(AssFile *_subs,wxString _destination,DialogFontsCollector *_collector)
: wxThread(wxTHREAD_DETACHED)
{
	subs = _subs;
	destination = _destination;
	collector = _collector;
	instance = this;
}



/// @brief Thread entry 
/// @return 
///
wxThread::ExitCode FontsCollectorThread::Entry() {
	// Collect
	Collect();

	// After done, restore status
	collector->Update();

	// Return
	if (IsDetached() && TestDestroy()) Delete();
	return 0;
}



/// @brief Collect 
/// @return 
///
void FontsCollectorThread::Collect() {
	// Set destination folder
	int oper = collector->CollectAction->GetSelection();
	destFolder = collector->DestBox->GetValue();
	if (oper == 1 && !wxFileName::DirExists(destFolder)) {
		AppendText(_("Invalid destination directory."),1);
		return;
	}

	// Open zip stream if saving to compressed archive
	wxFFileOutputStream *out = NULL;
	zip = NULL;
	if (oper == 2) {
		out = new wxFFileOutputStream(destFolder);
		zip = new wxZipOutputStream(*out);
	}

	// Collect font data
	AppendText(_("Collecting font data from system. This might take a while, depending on the number of fonts installed. Results are cached and subsequent executions will be faster...\n"));
	CollectFontData();
	AppendText(_("Done collecting font data."));
	AppendText(_("Scanning file for fonts..."));

	// Scan file
	if (collector->CollectAction->GetSelection() != 4) {
		AssDialogue *curDiag;
		curLine = 0;
		for (std::list<AssEntry*>::iterator cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
			// Collect from style
			curStyle = AssEntry::GetAsStyle(*cur);
			if (curStyle) {
				AddFont(curStyle->font,0);
			}

			// Collect from dialogue
			else {
				curDiag = AssEntry::GetAsDialogue(*cur);
				if (curDiag) {
					curLine++;
					curDiag->ParseASSTags();
					curDiag->ProcessParameters(GetFonts);
					curDiag->ClearBlocks();
				}
			}
		}
	}

	// For maitenance, gather all on system
	else {
		wxArrayString fonts = wxFontEnumerator::GetFacenames();
		for (size_t i=0;i<fonts.Count();i++) AddFont(fonts[i],2);
	}

	// Copy fonts
	AppendText(wxString(_("Done.")) + _T("\n\n"));
	switch (oper) {
		case 0: AppendText(_("Checking fonts...\n")); break;
		case 1: AppendText(_("Copying fonts to folder...\n")); break;
		case 2: AppendText(_("Copying fonts to archive...\n")); break;
		case 3: AppendText(_("Attaching fonts to file...\n")); break;
	}
	bool ok = true;
	bool someOk = false;
	for (size_t i=0;i<fonts.Count();i++) {
		bool result = ProcessFont(fonts[i]);
		if (result) someOk = true;
		if (!result) ok = false;
	}

	// Close ZIP archive
	if (oper == 2) {
		zip->Close();
		delete zip;
		delete out;

		AppendText(wxString::Format(_("\nFinished writing to %s.\n"),destination.c_str()),1);
	}

	// Final result
	if (ok) {
		if (oper == 0) {
			AppendText(_("Done. All fonts found."),1);
		}
		else {
			AppendText(_("Done. All fonts copied."),1);

			// Modify file if it was attaching
			if (oper == 3 && someOk) {
				wxMutexGuiEnter();
				subs->FlagAsModified(_("font attachment"));
				collector->main->SubsBox->CommitChanges();
				wxMutexGuiLeave();
			}
		}
	}
	else {
		if (oper == 0) AppendText(_("Done. Some fonts could not be found."),2);
		else  AppendText(_("Done. Some fonts could not be copied."),2);
	}
}



/// @brief Process font 
/// @param name 
/// @return 
///
bool FontsCollectorThread::ProcessFont(wxString name) {
	// Action
	int action = collector->CollectAction->GetSelection();

	// Font name
	AppendText(wxString::Format(_T("\"%s\"... "),name.c_str()));

	// Get font list
	wxArrayString files = FontFileLister::GetFilesWithFace(name);
	bool result = files.Count() != 0;

	// No files found
	if (!result) {
		AppendText(_("Not found.\n"),2);
		return false;
	}

	// Just checking, found
	else if (action == 0 || action == 4) {
		AppendText(_("Found.\n"),1);
		return true;
	}

	// Copy font
	AppendText(_T("\n"));
	for (size_t i=0;i<files.Count();i++) {
		int tempResult = 0;
		switch (action) {
			case 1: tempResult = CopyFont(files[i]); break;
			case 2: tempResult = ArchiveFont(files[i]) ? 1 : 0; break;
			case 3: tempResult = AttachFont(files[i]) ? 1 : 0; break;
		}

		if (tempResult == 1) {
			AppendText(wxString::Format(_("* Copied %s.\n"),files[i].c_str()),1);
		}
		else if (tempResult == 2) {
			wxFileName fn(files[i]);
			AppendText(wxString::Format(_("* %s already exists on destination.\n"),fn.GetFullName().c_str()),3);
		}
		else {
			AppendText(wxString::Format(_("* Failed to copy %s.\n"),files[i].c_str()),2);
			result = false;
		}
	}

	// Done
	return result;
}



/// @brief Copy font 
/// @param filename 
/// @return 
///
int FontsCollectorThread::CopyFont(wxString filename) {
	wxFileName fn(filename);
	wxString dstName = destFolder + _T("//") + fn.GetFullName();
	if (wxFileName::FileExists(dstName)) return 2;
	return wxCopyFile(filename,dstName,true) ? 1 : 0;
}



/// @brief Archive font 
/// @param filename 
/// @return 
///
bool FontsCollectorThread::ArchiveFont(wxString filename) {
	// Open file
	wxFFileInputStream in(filename);
	if (!in.IsOk()) return false;

	// Write to archive
	try {
		wxFileName fn(filename);
		zip->PutNextEntry(fn.GetFullName());
		zip->Write(in);
	}
	catch (...) {
		return false;
	}

	return true;
}



/// @brief Attach font 
/// @param filename 
/// @return 
///
bool FontsCollectorThread::AttachFont(wxString filename) {
	try {
		subs->InsertAttachment(filename);
	}
	catch (...) {
		return false;
	}
	return true;
}



/// @brief Get fonts from ass overrides 
/// @param tagName 
/// @param par_n   
/// @param param   
/// @param usr     
///
void FontsCollectorThread::GetFonts (wxString tagName,int par_n,AssOverrideParameter *param,void *usr) {
	if (tagName == _T("\\fn")) {
		if (instance) instance->AddFont(param->AsText(),1);
	}
}



/// @brief Adds a font 
/// @param fontname 
/// @param mode     
///
void FontsCollectorThread::AddFont(wxString fontname,int mode) {
	// @-fonts (CJK vertical layout variations) should be listed as the non-@ name
	if (fontname.StartsWith(_T("@"), 0))
		fontname.Remove(0, 1);

	if (fonts.Index(fontname) == wxNOT_FOUND) {
		fonts.Add(fontname);

		if (mode == 0) AppendText(wxString::Format(_("\"%s\" found on style \"%s\".\n"), fontname.c_str(), curStyle->name.c_str()));
		else if (mode == 1) AppendText(wxString::Format(_("\"%s\" found on dialogue line \"%d\".\n"), fontname.c_str(), curLine));
		else AppendText(wxString::Format(_("\"%s\" found.\n"), fontname.c_str()));
	}
}



/// @brief Append text 
/// @param text   
/// @param colour 
///
void FontsCollectorThread::AppendText(wxString text,int colour) {
	ColourString *str = new ColourString;
	str->text = text;
	str->colour = colour;
	wxCommandEvent event(EVT_ADD_TEXT,0);
	event.SetClientData(str);
	collector->AddPendingEvent(event);
}



/// DOCME
FontsCollectorThread *FontsCollectorThread::instance;


