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


////////////
// Includes
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#ifdef __WINDOWS__
#include <shlobj.h>
#endif
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


///////////////
// Constructor
DialogFontsCollector::DialogFontsCollector(wxWindow *parent)
: wxDialog(parent,-1,_("Fonts Collector"),wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	// Parent
	main = (FrameMain*) parent;

	// Destination box
	wxString dest = Options.AsText(_T("Fonts Collector Destination"));
	if (dest == _T("?script")) {
		wxFileName filename(AssFile::top->filename);
		dest = filename.GetPath();
	}
	DestBox = new wxTextCtrl(this,-1,dest,wxDefaultPosition,wxSize(250,20),0);
	BrowseButton = new wxButton(this,BROWSE_BUTTON,_("&Browse..."));
	AttachmentCheck = new wxCheckBox(this,ATTACHMENT_CHECK,_("As attachments"),wxDefaultPosition);
	AttachmentCheck->SetValue(Options.AsBool(_T("Fonts Collector Attachment")));
	ArchiveCheck = new wxCheckBox(this,ARCHIVE_CHECK,_("As a zipped archive"),wxDefaultPosition);
	ArchiveCheck->SetValue(Options.AsBool(_T("Fonts Collector Archive")));
	if (ArchiveCheck->GetValue()) AttachmentCheck->SetValue(false);
	wxSizer *DestBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	DestLabel = new wxStaticText(this,-1,_("Choose the folder where the fonts will be collected to.\nIt will be created if it doesn't exist."));
	DestBottomSizer->Add(DestBox,1,wxEXPAND | wxRIGHT,5);
	DestBottomSizer->Add(BrowseButton,0,0,0);
	wxSizer *DestSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Destination"));
	DestSizer->Add(DestLabel,0,wxEXPAND | wxBOTTOM,5);
	DestSizer->Add(DestBottomSizer,0,wxEXPAND,0);
	DestSizer->Add(AttachmentCheck,0,wxTOP,5);
	DestSizer->Add(ArchiveCheck,0,wxTOP,5);

	// Log box
	LogBox = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(300,210),wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	wxSizer *LogSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Log"));
	LogSizer->Add(LogBox,1,wxEXPAND,0);

	// Buttons sizer
	StartButton = new wxButton(this,START_BUTTON,_("&Start!"));
	StartButton->SetDefault();
	CloseButton = new wxButton(this,wxID_CANCEL,_T("Close"));
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifdef __WXMAC__ 
	ButtonSizer->Add(CloseButton,0,wxRIGHT,5);
	ButtonSizer->Add(StartButton);
#else
	ButtonSizer->Add(StartButton,0,wxRIGHT,5);
	ButtonSizer->Add(CloseButton);
#endif

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(DestSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->Add(LogSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);

	// Run dummy event to update label
	Update();
}


//////////////
// Destructor
DialogFontsCollector::~DialogFontsCollector() {
}


////////////////////////////////
// Get fonts from ass overrides
void FontsCollectorThread::GetFonts (wxString tagName,int par_n,AssOverrideParameter *param,void *usr) {
	if (tagName == _T("\\fn")) {
		instance->AddFont(param->AsText(),false);
	}
}


///////////////
// Adds a font
void FontsCollectorThread::AddFont(wxString fontname,bool isStyle) {
	if (fonts.Index(fontname) == wxNOT_FOUND) {
		fonts.Add(fontname);

		// Dialogue
		if (!isStyle) {
			collector->LogBox->AppendText(wxString(_T("\"")) + fontname + _("\" found on dialogue line ") + wxString::Format(_T("%i"),curLine) + _T(".\n"));
		}
	}
}


///////////////////
// Static instance
FontsCollectorThread *FontsCollectorThread::instance;


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogFontsCollector, wxDialog)
	EVT_BUTTON(START_BUTTON,DialogFontsCollector::OnStart)
	EVT_BUTTON(BROWSE_BUTTON,DialogFontsCollector::OnBrowse)
	EVT_BUTTON(wxID_CLOSE,DialogFontsCollector::OnClose)
	EVT_CHECKBOX(ATTACHMENT_CHECK,DialogFontsCollector::OnCheckAttach)
	EVT_CHECKBOX(ARCHIVE_CHECK,DialogFontsCollector::OnCheckArchive)
END_EVENT_TABLE()


////////////////////
// Start processing
void DialogFontsCollector::OnStart(wxCommandEvent &event) {
	// Check if it's OK to do it
	wxString foldername = DestBox->GetValue();
	wxFileName folder(foldername);
	bool zipOut = ArchiveCheck->IsChecked();

	// Make folder if it doesn't exist
	if (!zipOut && !folder.DirExists()) folder.Mkdir(0777,wxPATH_MKDIR_FULL);

	// Start
	if (zipOut || folder.DirExists()) {
		// Start thread
		wxThread *worker = new FontsCollectorThread(AssFile::top,foldername,this);
		worker->Create();
		worker->Run();

		// Set options
		wxString dest = foldername;
		wxFileName filename(AssFile::top->filename);
		if (filename.GetPath() == dest) {
			dest = _T("?script");
		}
		Options.SetText(_T("Fonts Collector Destination"),dest);
		Options.Save();

		// Set buttons
		StartButton->Enable(false);
		BrowseButton->Enable(false);
		DestBox->Enable(false);
		CloseButton->Enable(false);
		AttachmentCheck->Enable(false);
		ArchiveCheck->Enable(false);
		if (!worker->IsDetached()) worker->Wait();
	}

	// Folder not available
	else {
		wxMessageBox(_("Invalid destination"),_("Error"),wxICON_EXCLAMATION | wxOK);
	}
}


////////////////
// Close dialog
void DialogFontsCollector::OnClose(wxCommandEvent &event) {
	EndModal(0);
}


///////////////////
// Browse location
void DialogFontsCollector::OnBrowse(wxCommandEvent &event) {
	// Chose file name
	if (ArchiveCheck->IsChecked()) {
		wxFileName fname(DestBox->GetValue());
		wxString dest = wxFileSelector(_("Select archive file name"),DestBox->GetValue(),fname.GetFullName(),_T(".zip"),_T("Zip Archives (*.zip)|*.zip"),wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
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


////////////////////
// Check Attachment
void DialogFontsCollector::OnCheckAttach(wxCommandEvent &event) {
	bool check = AttachmentCheck->IsChecked();
	BrowseButton->Enable(!check);
	DestBox->Enable(!check);
	if (check) {
		ArchiveCheck->SetValue(false);
		Update();
	}
}


/////////////////
// Check Archive
void DialogFontsCollector::OnCheckArchive(wxCommandEvent &event) {
	bool check = ArchiveCheck->IsChecked();
	if (check) {
		BrowseButton->Enable(check);
		DestBox->Enable(check);
	}
	Update();
}


///////////////////
// Update controls
void DialogFontsCollector::Update() {
	bool check = ArchiveCheck->IsChecked();
	if (check) {
		AttachmentCheck->SetValue(false);
		DestLabel->SetLabel(_("Enter the name of the destination zip file to collect the fonts to.\nIf a folder is entered, a default name will be used."));
	}
	else {
		// Set label
		DestLabel->SetLabel(_("Choose the folder where the fonts will be collected to.\nIt will be created if it doesn't exist."));

		// Remove filename from browser box
		wxFileName fname1(DestBox->GetValue()+_T("/"));
		if (fname1.DirExists()) {
			DestBox->SetValue(fname1.GetPath());
		}
		else {
			wxFileName fname2(DestBox->GetValue());
			if (fname2.DirExists()) {
				DestBox->SetValue(fname2.GetPath());
			}
			else DestBox->SetValue(((AegisubApp*)wxTheApp)->folderName);
		}
	}
}


//////////////////////
// Get font filenames
wxArrayString FontsCollectorThread::GetFontFiles (wxString face) {
	wxArrayString files;
	int n = 0;

	for (FontMap::iterator entry = regFonts.begin();entry != regFonts.end();entry++) {
		wxString curData = (*entry).first;
		if (face == curData.Left(face.Length())) {
			files.Add((*entry).second);
			n++;
		}
	}
	if (n==0) throw wxString(_T("Font not found"));

	return files;
}


///////////////////////
// Collect font files
void FontsCollectorThread::CollectFontData () {
#ifdef __WINDOWS__
	// Prepare key
	wxRegKey *reg = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"));
	// Try win9x
	if (!reg->Exists()) {
		delete reg;
		reg = new wxRegKey(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts"));
		if (!reg->Exists()) {
			delete reg;
			throw _T("Could not locate fonts directory");
		}
	}
	wxString curName;
	wxString curVal;
	long index;
	int n = 0;

	// Iterate through
	bool ok = reg->GetFirstValue(curName,index);
	while (ok) {
		reg->QueryValue(curName,curVal);
		//AddFontData(curName,curVal);
		regFonts[curName] = curVal;
		ok = reg->GetNextValue(curName,index);
		n++;
	}

	// Clean up
	delete reg;
#endif
}


////////////////////
// Collector thread
FontsCollectorThread::FontsCollectorThread(AssFile *_subs,wxString _destination,DialogFontsCollector *_collector)
: wxThread(wxTHREAD_DETACHED)
{
	subs = _subs;
	destination = _destination;
	collector = _collector;
	instance = this;
}


////////////////
// Thread entry
wxThread::ExitCode FontsCollectorThread::Entry() {
	// Collect
	Collect();
	collector->CloseButton->Enable(true);

	// Return
	if (IsDetached()) Delete();
	return 0;
}


///////////
// Collect
void FontsCollectorThread::Collect() {
	// Prepare
	bool attaching = collector->AttachmentCheck->IsChecked();
	bool zipOut = collector->ArchiveCheck->IsChecked();

	// Make sure there is a separator at the end
	if (!zipOut) destination += _T("\\");

	// For zipped files, enter a default name if none was given
	else {
		wxFileName dest(destination);
		wxString subsPath = subs->filename;
		if (subsPath.IsEmpty()) subsPath = AegisubApp::folderName + _T("/unnamed.ass");
		wxFileName subsname(subsPath);

		// Folder picked
		if (dest.IsDir()) {
			if (!dest.DirExists()) destination = subsname.GetPath() + _T("/");
			destination += _T("/") + subsname.GetName() + _T(".zip");
		}

		// File picked
		else {
			if (!dest.DirExists()) destination = subsname.GetPath();
			else destination = dest.GetPath();
			destination += _T("/") + dest.GetName() + _T(".zip");
		}

		// Clean up name
		wxFileName finalDest(destination);
		destination = finalDest.GetFullPath();
	}

	// Reset log box
	wxTextCtrl *LogBox = collector->LogBox;
	wxMutexGuiEnter();
	LogBox->SetValue(_T(""));
	LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,0,180)));
	LogBox->AppendText(_("Searching for fonts in file...\n"));
	LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,0,0)));
	LogBox->Refresh();
	LogBox->Update();
	wxSafeYield();
	wxMutexGuiLeave();

	// Scans file
	bool fileModified = false;
	AssStyle *curStyle;
	AssDialogue *curDiag;
	curLine = 0;
	for (std::list<AssEntry*>::iterator cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		// Collect from style
		curStyle = AssEntry::GetAsStyle(*cur);
		if (curStyle) {
			AddFont(curStyle->font,true);
			wxMutexGuiEnter();
			LogBox->AppendText(wxString(_T("\"")) + curStyle->font + _("\" found on style \"") + curStyle->name + _T("\".\n"));
			LogBox->Refresh();
			LogBox->Update();
			wxSafeYield();
			wxMutexGuiLeave();
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

#ifdef __WINDOWS__
	// Collect font data
	wxMutexGuiEnter();
	LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,0,180)));
	LogBox->AppendText(_("\nReading fonts from registry...\n"));
	LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,0,0)));
	wxSafeYield();
	wxMutexGuiLeave();
	CollectFontData();

	// Get fonts folder
	wxString source;
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS,NULL,0,szPath))) {
		source = wxString(szPath);
	}
	else source = wxGetOSDirectory() + _T("\\fonts");
	source += _T("\\");

	// Open zip stream if saving to compressed archive
	wxFFileOutputStream *out = NULL;
	wxZipOutputStream *zip = NULL;
	if (zipOut) {
		out = new wxFFileOutputStream(destination);
		zip = new wxZipOutputStream(*out);
	}

	// Get font file names
	wxArrayString work;
	wxArrayString copied;
	for (size_t i=0;i<fonts.GetCount();i++) {
		try {
			work = GetFontFiles(fonts[i]);
			for (size_t j=0;j<work.GetCount();j++) {
				// Get path to font file
				wxString srcFile,dstFile;
				wxFileName srcFileName(work[j]);
				if (srcFileName.FileExists() && srcFileName.IsAbsolute()) {
					srcFile = work[j];
					dstFile = destination + srcFileName.GetFullName();
				}
				else {
					srcFile = source + work[j];
					dstFile = destination + work[j];
				}

				if (copied.Index(work[j]) == wxNOT_FOUND) {
					copied.Add(work[j]);

					// Check if it exists
					if (!attaching && !zipOut && wxFileName::FileExists(dstFile)) {
						wxMutexGuiEnter();
						LogBox->SetDefaultStyle(wxTextAttr(wxColour(255,128,0)));
						LogBox->AppendText(wxString(_T("\"")) + work[j] + _("\" already exists on destination.\n"));
						LogBox->Refresh();
						LogBox->Update();
						wxSafeYield();
						wxMutexGuiLeave();
					}

					// Copy
					else {
						// Attach to file
						bool success;
						if (attaching) {
							try {
								subs->InsertAttachment(srcFile);
								fileModified = true;
								success = true;
							}
							catch (...) { success = false; }
						}

						// Copy to zip destination
						else if (zipOut) {
							// Open file
							wxFFileInputStream in(srcFile);

							// Write to archive
							zip->PutNextEntry(work[j]);
							zip->Write(in);
						}

						// Copy to destination
						else {
							success = Copy(srcFile,dstFile);
						}

						// Report
						wxMutexGuiEnter();
						if (success) {
							LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,180,0)));
							LogBox->AppendText(wxString(_T("\"")) + work[j] + _("\" copied.\n"));
						}
						else {
							LogBox->SetDefaultStyle(wxTextAttr(wxColour(220,0,0)));
							LogBox->AppendText(wxString(_("Failed copying \"")) + srcFile + _T("\".\n"));
						}
						LogBox->Refresh();
						LogBox->Update();
						wxSafeYield();
						wxMutexGuiLeave();
					}
				}
			}
		}

		catch (...) {
			wxMutexGuiEnter();
			LogBox->SetDefaultStyle(wxTextAttr(wxColour(220,0,0)));
			LogBox->AppendText(wxString(_("Could not find font ")) + fonts[i] + _T("\n"));
			wxMutexGuiLeave();
		}
	}

	// Close ZIP archive
	if (zipOut) {
		zip->Close();
		delete zip;
		delete out;

		wxMutexGuiEnter();
		LogBox->SetDefaultStyle(wxTextAttr(wxColour(0,180,0)));
		LogBox->AppendText(wxString::Format(_("Finished writing to %s.\n"),destination.c_str()));
		wxMutexGuiLeave();
	}
#endif

	// Flag file as modified
	if (fileModified) {
		subs->FlagAsModified();
		collector->main->SubsBox->CommitChanges();
	}
}
