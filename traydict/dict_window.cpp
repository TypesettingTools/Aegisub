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


///////////
// Headers
#include <wx/wxprec.h>
#include <wx/taskbar.h>
#include "dict_window.h"
#include "systray.h"
#include "dictionary.h"
#include "main.h"


///////////////
// Constructor
DictWindow::DictWindow()
: wxFrame(NULL,-1,_T("TrayDict v0.06 - By Rodrigo Braz Monteiro"),wxDefaultPosition,wxSize(620,500),wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	// Icons
	SetIcon(wxICON(wxicon));
	systray = new Systray(this);

	// Menu bar
	//menu = new wxMenuBar();
	//SetMenuBar(menu);

	// Status bar
	//wxStatusBar *bar = CreateStatusBar(3,0);
	//int widths[] = { 85, -1, -1 };
	//bar->SetStatusWidths(3,widths);

	// Panel
	panel = new wxPanel(this);

	// Controls
	entry = new wxTextCtrl(panel,ENTRY_FIELD,_T(""),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
	results = new wxTextCtrl(panel,-1,_T(""),wxDefaultPosition,wxSize(280,400),wxTE_RICH2 | wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_READONLY);
	wxButton *searchButton = new wxButton(panel,BUTTON_SEARCH,_T("Search"),wxDefaultPosition,wxSize(80,-1));
	wxSizer *entrySizer = new wxBoxSizer(wxHORIZONTAL);
	entrySizer->Add(entry,1,wxEXPAND | wxRIGHT,5);
	entrySizer->Add(searchButton,0,wxEXPAND,0);

	// Options
	checkKanji = new wxCheckBox(panel,CHECK_KANJI,_T("Kanji"));
	checkKana = new wxCheckBox(panel,CHECK_KANA,_T("Kana"));
	checkRomaji = new wxCheckBox(panel,CHECK_ROMAJI,_T("Romaji"));
	checkEnglish = new wxCheckBox(panel,CHECK_ENGLISH,_T("English"));
	checkEdict = new wxCheckBox(panel,CHECK_ENGLISH,_T("EDICT"));
	checkEnamdict = new wxCheckBox(panel,CHECK_ENGLISH,_T("ENAMDICT"));
	checkCompdic = new wxCheckBox(panel,CHECK_ENGLISH,_T("COMPDIC"));
	checkJplaces = new wxCheckBox(panel,CHECK_ENGLISH,_T("J_PLACES"));
	checkKanji->SetValue(true);
	checkKana->SetValue(true);
	checkRomaji->SetValue(true);
	checkEnglish->SetValue(true);
	checkEdict->SetValue(true);
	checkEnamdict->SetValue(false);
	checkCompdic->SetValue(false);
	checkJplaces->SetValue(false);
	wxSizer *optionsSizer = new wxBoxSizer(wxHORIZONTAL);
	optionsSizer->Add(new wxStaticText(panel,-1,_T("Display:")),0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkKanji,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkKana,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkRomaji,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkEnglish,0,wxCENTER | wxRIGHT,20);
	optionsSizer->Add(new wxStaticText(panel,-1,_T("Dictionary:")),0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkEdict,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkEnamdict,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkCompdic,0,wxCENTER | wxRIGHT,5);
	optionsSizer->Add(checkJplaces,0,wxCENTER | wxRIGHT,0);
	optionsSizer->AddStretchSpacer(1);

	// Main sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(entrySizer,0,wxEXPAND | wxALL,5);
	mainSizer->Add(optionsSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->Add(results,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	panel->SetSizer(mainSizer);

	// Create dictionary files
	if (false) {
		Dictionary::Convert(TrayDict::folderName + _T("edict"),TrayDict::folderName + _T("edict.dic"));
		Dictionary::Convert(TrayDict::folderName + _T("enamdict"),TrayDict::folderName + _T("enamdict.dic"));
		Dictionary::Convert(TrayDict::folderName + _T("compdic"),TrayDict::folderName + _T("compdic.dic"));
		Dictionary::Convert(TrayDict::folderName + _T("j_places"),TrayDict::folderName + _T("j_places.dic"));
	}

	// Load dictionary files
	dict.push_back(new Dictionary(_T("edict"),checkEdict));
	dict.push_back(new Dictionary(_T("enamdict"),checkEnamdict));
	dict.push_back(new Dictionary(_T("compdic"),checkCompdic));
	dict.push_back(new Dictionary(_T("j_places"),checkJplaces));

	// Register hotkey
	RegisterHotKey(HOTKEY_ID,wxMOD_WIN,'Z');
}


//////////////
// Destructor
DictWindow::~DictWindow() {
	if (systray->IsOk()) {
		systray->RemoveIcon();
	}
	delete systray;

	for (size_t i=0;i<dict.size();i++) delete dict[i];
	dict.clear();
}


/////////////////
// Go to systray
void DictWindow::GoToTray() {
	if (systray->IsOk()) {
		Hide();
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DictWindow,wxFrame)
	EVT_ICONIZE(DictWindow::OnIconize)
	EVT_BUTTON(BUTTON_SEARCH,DictWindow::OnSearch)
	EVT_TEXT_ENTER(ENTRY_FIELD,DictWindow::OnSearch)
	EVT_HOTKEY(HOTKEY_ID,DictWindow::OnHotkey)
	EVT_CLOSE(DictWindow::OnClose)
END_EVENT_TABLE()


/////////////////
// Iconize event
void DictWindow::OnIconize(wxIconizeEvent &event) {
	if (event.Iconized()) {
		GoToTray();
	}
}


//////////////////
// Hotkey pressed
void DictWindow::OnHotkey(wxKeyEvent &event) {
	if (IsShown()) GoToTray();
	else systray->BringUp();
}


/////////////////////////
// Search button pressed
void DictWindow::OnSearch(wxCommandEvent &event) {
	Search(entry->GetValue());
}


//////////
// Search
void DictWindow::Search(wxString text) {
	// Prepare
	int bitmask = (checkKanji->GetValue() ? 1 : 0) | (checkKana->GetValue() ? 2 : 0) | (checkRomaji->GetValue() ? 4 : 0) | (checkEnglish->GetValue() ? 8 : 0);

	// Clear text
	results->Clear();
	entry->SetSelection(0,entry->GetValue().Length());

	// Search each dictionary
	for (size_t i=0;i<dict.size();i++) {
		if (dict[i]->check->GetValue() && dict[i]->check->IsEnabled()) {
			// Search
			ResultSet res;
			dict[i]->Search(res,text);

			// Sort results by relevancy
			res.results.sort();

			// Print
			res.Print(results,bitmask);
		}
	}

	// Show start
	results->ShowPosition(0);
	results->SetSelection(0,0);
}


////////////////
// Close window
void DictWindow::OnClose(wxCloseEvent &event) {
	if (event.CanVeto()) {
		event.Veto();
		GoToTray();
	}
	else Destroy();
}
