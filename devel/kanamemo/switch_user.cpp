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
#include <wx/filename.h>
#include <wx/dir.h>
#include <stdio.h>
#include "switch_user.h"
#include "game_display.h"
#include "main.h"


///////////////
// Constructor
SwitchUserDialog::SwitchUserDialog(GameDisplay *dspl)
: wxDialog(NULL,-1,_T("Select user..."),wxDefaultPosition,wxDefaultSize)
{
	display = dspl;

	// Sizers
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *listSizer = new wxStaticBoxSizer(wxVERTICAL,this,_T("Users:"));

	// Load list
	wxArrayString choices;
	wxArrayString files;
	wxDir::GetAllFiles(KanaMemo::folderName,&files,_T("*.usr"),wxDIR_FILES);
	for (size_t i=0;i<files.Count();i++) {
		wxFileName fname(files[i]);
		choices.Add(fname.GetName());
	}

	// List sizer
	listBox = new wxListBox(this,1337,wxDefaultPosition,wxSize(200,150),choices,wxLB_SINGLE | wxLB_SORT );
	listSizer->Add(listBox,1,wxEXPAND,0);

	// Button sizer
	buttonSizer->AddStretchSpacer(1);
	okButton = new wxButton(this,wxID_OK);
	buttonSizer->Add(new wxButton(this,wxID_NEW),0,0,0);
	buttonSizer->Add(okButton,0,0,0);
	buttonSizer->Add(new wxButton(this,wxID_CANCEL),0,0,0);

	// Main sizer
	mainSizer->Add(listSizer,1,wxEXPAND | wxALL,5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);

	// If list is empty, prompt for new user
	if (listBox->GetCount() == 0) NewUser();
	UpdateButtons();
}


//////////////
// Destructor
SwitchUserDialog::~SwitchUserDialog() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(SwitchUserDialog,wxDialog)
	EVT_BUTTON(wxID_NEW,SwitchUserDialog::OnButtonCreate)
	EVT_BUTTON(wxID_OK,SwitchUserDialog::OnButtonOK)
	EVT_BUTTON(wxID_CANCEL,SwitchUserDialog::OnButtonCancel)
	EVT_LISTBOX(1337, SwitchUserDialog::OnClickList)
	EVT_LISTBOX_DCLICK(1337, SwitchUserDialog::OnDoubleClickList)
END_EVENT_TABLE()


//////////
// Create
void SwitchUserDialog::OnButtonCreate(wxCommandEvent &event) {
	NewUser();
}


//////
// OK
void SwitchUserDialog::OnButtonOK(wxCommandEvent &event) {
	Select();
}


//////////
// Cancel
void SwitchUserDialog::OnButtonCancel(wxCommandEvent &event) {
	Destroy();
}


////////////
// New user
void SwitchUserDialog::NewUser() {
	wxString userName = wxGetTextFromUser(_T("Enter the name of the new user:"),_T("New user"),_T(""),this);
	if (!userName.IsEmpty()) {
		// Create file
		wxString path = KanaMemo::folderName + _T("/") + userName + _T(".usr");
		FILE *fp = fopen(path.mb_str(wxConvLocal),"wb");
		fclose(fp);

		// Update stuff
		listBox->Append(userName);
		listBox->SetStringSelection(userName);
		UpdateButtons();
	}
}


////////////////
// List clicked
void SwitchUserDialog::OnClickList(wxCommandEvent &event) {
	UpdateButtons();
}


///////////////////////
// List double clicked
void SwitchUserDialog::OnDoubleClickList(wxCommandEvent &event) {
	Select();
}


//////////////////
// Update buttons
void SwitchUserDialog::UpdateButtons() {
	okButton->Enable(!listBox->GetStringSelection().IsEmpty());
}


///////////////
// Select user
void SwitchUserDialog::Select() {
	wxString user = listBox->GetStringSelection();
	if (!user.IsEmpty()) {
		display->playerName = user;
		display->Load();
		display->EnableGame(true);
		Destroy();
	}
}