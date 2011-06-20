// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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
#include "config.h"

#include <wx/tokenzr.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "dialog_export.h"
#include "ass_file.h"
#include "ass_exporter.h"
#include "frame_main.h"
#include "help_button.h"


///////////////
// Constructor
DialogExport::DialogExport (wxWindow *parent)
: wxDialog (parent, -1, _("Export"), wxDefaultPosition, wxSize(200,100), wxCAPTION | wxCLOSE_BOX, _T("Export"))
{
	// Filter list
	wxSizer *TopSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Filters"));
	Export = new AssExporter(AssFile::top);
	wxArrayString filters = Export->GetAllFilterNames();
	FilterList = new wxCheckListBox(this, Filter_List_Box, wxDefaultPosition, wxSize(200,100), filters);

	// Get selected filters
	wxString selected = Export->GetOriginalSubs()->GetScriptInfo(_T("Export filters"));
	wxStringTokenizer token(selected, _T("|"));
	int n = 0;
	while (token.HasMoreTokens()) {
		wxString cur = token.GetNextToken();
		if (!cur.IsEmpty()) {
			n++;
			for (unsigned int i=0;i<FilterList->GetCount();i++) {
				if (FilterList->GetString(i) == cur) {
					FilterList->Check(i);
					break;
				}
			}
		}
	}

	// No filters listed on header, select all
	/*if (n == 0) {
		for (unsigned int i=0;i<FilterList->GetCount();i++) {
			FilterList->Check(i);
		}
	}*/

	// Top buttons
	wxSizer *TopButtons = new wxBoxSizer(wxHORIZONTAL);
	TopButtons->Add(new wxButton(this,Button_Move_Up,_("Move up"),wxDefaultPosition,wxSize(90,-1)),1,wxEXPAND | wxRIGHT,0);
	TopButtons->Add(new wxButton(this,Button_Move_Down,_("Move down"),wxDefaultPosition,wxSize(90,-1)),1,wxEXPAND | wxRIGHT,5);
	TopButtons->Add(new wxButton(this,Button_Select_All,_("Select all"),wxDefaultPosition,wxSize(80,-1)),1,wxEXPAND | wxRIGHT,0);
	TopButtons->Add(new wxButton(this,Button_Select_None,_("Select none"),wxDefaultPosition,wxSize(80,-1)),1,wxEXPAND | wxRIGHT,0);

	// Description field
	Description = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(200,60), wxTE_MULTILINE | wxTE_READONLY);

	// Charset dropdown list
	wxStaticText *charset_list_label = new wxStaticText(this, -1, _("Text encoding:"));
	CharsetList = new wxChoice(this, Charset_List_Box, wxDefaultPosition, wxDefaultSize, FrameMain::GetEncodings());
	wxSizer *charset_list_sizer = new wxBoxSizer(wxHORIZONTAL);
	charset_list_sizer->Add(charset_list_label, 0, wxALIGN_CENTER | wxRIGHT, 5);
	charset_list_sizer->Add(CharsetList, 1, wxEXPAND);
	if (!CharsetList->SetStringSelection(Export->GetOriginalSubs()->GetScriptInfo(_T("Export Encoding")))) {
		CharsetList->SetStringSelection(_T("UTF-8"));
	}

	// Top sizer
	TopSizer->Add(FilterList,1,wxEXPAND,0);
	TopSizer->Add(TopButtons,0,wxEXPAND,0);
	TopSizer->Add(Description,0,wxEXPAND | wxTOP,5);
	TopSizer->Add(charset_list_sizer, 0, wxEXPAND | wxTOP, 5);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	wxButton *process = new wxButton(this,Button_Process,_("Export..."));
	ButtonSizer->AddButton(process);
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Export")));
	ButtonSizer->SetAffirmativeButton(process);
	ButtonSizer->Realize();

	// Draw stuff sizer
	HorizSizer = new wxBoxSizer(wxHORIZONTAL);
	OptionsSizer = new wxBoxSizer(wxVERTICAL);
	Export->DrawSettings(this,OptionsSizer);
	HorizSizer->Add(TopSizer,0,wxEXPAND | wxLEFT | wxTOP | wxBOTTOM,5);
	HorizSizer->Add(OptionsSizer,1,wxEXPAND | wxTOP | wxRIGHT | wxBOTTOM,5);

	// Main sizer
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(HorizSizer,1,wxEXPAND,0);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	RefreshOptions();
	CenterOnParent();
	FilterList->SetFocus();
}


//////////////
// Destructor
DialogExport::~DialogExport() {
	// Set script info data
	int n = 0;
	wxString infoList;
	for (unsigned int i=0;i<FilterList->GetCount();i++) {
		if (FilterList->IsChecked(i)) {
			infoList += FilterList->GetString(i) + _T("|");
			n++;
		}
	}
	if (n > 0) infoList = infoList.Left(infoList.Length()-1);
	Export->GetOriginalSubs()->SetScriptInfo(_T("Export filters"),infoList);

	// Delete exporter
	if (Export) delete Export;
	Export = NULL;
}


/////////////////////////////////
// Refresh displaying of options
void DialogExport::RefreshOptions() {
	int num = FilterList->GetCount();
	for (int i=0;i<num;i++) {
		wxSizer *sizer = Export->GetSettingsSizer(FilterList->GetString(i));
		if (sizer) OptionsSizer->Show(sizer,FilterList->IsChecked(i)?1:0,true);
	}
	Layout();
	MainSizer->Fit(this);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogExport,wxDialog)
	EVT_BUTTON(Button_Process,DialogExport::OnProcess)
	EVT_BUTTON(Button_Move_Up,DialogExport::OnMoveUp)
	EVT_BUTTON(Button_Move_Down,DialogExport::OnMoveDown)
	EVT_BUTTON(Button_Select_All,DialogExport::OnSelectAll)
	EVT_BUTTON(Button_Select_None,DialogExport::OnSelectNone)
	EVT_CHECKLISTBOX(Filter_List_Box,DialogExport::OnCheck)
	EVT_LISTBOX(Filter_List_Box, DialogExport::OnChange)
END_EVENT_TABLE()


/////////////////
// Process start
void DialogExport::OnProcess(wxCommandEvent &event) {
	// Get destination
	wxString filename = wxFileSelector(_("Export subtitles file"),_T(""),_T(""),_T(""),AssFile::GetWildcardList(2),wxFD_SAVE | wxFD_OVERWRITE_PROMPT,this);
	if (filename.empty()) return;

	// Add filters
	for (unsigned int i=0;i<FilterList->GetCount();i++) {
		if (FilterList->IsChecked(i)) {
			Export->AddFilter(FilterList->GetString(i));
		}
	}

	// Export
	try {
		wxBusyCursor busy;
		Export->GetOriginalSubs()->SetScriptInfo(_T("Export Encoding"), CharsetList->GetStringSelection());
		Export->Export(filename, CharsetList->GetStringSelection(), this);
	}
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error exporting subtitles"), wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error exporting subtitles"), wxOK | wxICON_ERROR, this);
	}

	// Close dialog
	EndModal(0);
}


/////////////////////////////
// Checked or unchecked item
void DialogExport::OnCheck(wxCommandEvent &event) {
	int n = event.GetInt();
	wxSizer *sizer = Export->GetSettingsSizer(FilterList->GetString(n));
	if (sizer) MainSizer->Show(sizer,FilterList->IsChecked(n)?1:0,true);
	Layout();
	MainSizer->Fit(this);
}


////////////////
// Changed item
void DialogExport::OnChange(wxCommandEvent &event) {
	int n = FilterList->GetSelection();
	if (n != wxNOT_FOUND) {
		wxString name = FilterList->GetString(n);
		//Description->SetValue(wxGetTranslation(Export->GetDescription(name)));
		Description->SetValue(Export->GetDescription(name));
	}
}


///////////
// Move up
void DialogExport::OnMoveUp(wxCommandEvent &event) {
	int pos = FilterList->GetSelection();
	if (pos <= 0) return;
	FilterList->Freeze();
	wxString tempname = FilterList->GetString(pos);
	bool tempval = FilterList->IsChecked(pos);
	FilterList->SetString(pos,FilterList->GetString(pos-1));
	FilterList->Check(pos,FilterList->IsChecked(pos-1));
	FilterList->SetString(pos-1,tempname);
	FilterList->Check(pos-1,tempval);
	FilterList->SetSelection(pos-1);
	FilterList->Thaw();
}


/////////////
// Move down
void DialogExport::OnMoveDown(wxCommandEvent &event) {
	int pos = FilterList->GetSelection();
	int n = FilterList->GetCount();
	if (pos == n-1 || pos == -1) return;
	FilterList->Freeze();
	wxString tempname = FilterList->GetString(pos);
	bool tempval = FilterList->IsChecked(pos);
	FilterList->SetString(pos,FilterList->GetString(pos+1));
	FilterList->Check(pos,FilterList->IsChecked(pos+1));
	FilterList->SetString(pos+1,tempname);
	FilterList->Check(pos+1,tempval);
	FilterList->SetSelection(pos+1);
	FilterList->Thaw();
}


//////////////
// Select all
void DialogExport::OnSelectAll(wxCommandEvent &event) {
	Freeze();
	FilterList->Freeze();
	for (unsigned int i=0;i<FilterList->GetCount();i++) {
		FilterList->Check(i,true);
		wxSizer *sizer = Export->GetSettingsSizer(FilterList->GetString(i));
		if (sizer) MainSizer->Show(sizer,true,true);
	}

	// Update dialog
	Layout();
	MainSizer->Fit(this);
	FilterList->Thaw();
	Thaw();
}


///////////////
// Select none
void DialogExport::OnSelectNone(wxCommandEvent &event) {
	Freeze();
	FilterList->Freeze();
	for (unsigned int i=0;i<FilterList->GetCount();i++) {
		FilterList->Check(i,false);
		wxSizer *sizer = Export->GetSettingsSizer(FilterList->GetString(i));
		if (sizer) MainSizer->Show(sizer,false,true);
	}

	// Update dialog
	FilterList->Thaw();
	Thaw();
	Layout();
	MainSizer->Fit(this);
}
