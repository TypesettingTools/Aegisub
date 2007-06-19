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


///////////
// Headers
#include "dialog_associations.h"
#include <wx/config.h>


///////////////
// Constructor
DialogAssociations::DialogAssociations (wxWindow *parent)
: wxDialog (parent,-1,_("Associate extensions"),wxDefaultPosition,wxDefaultSize)
{
	// List box
	wxArrayString choices;
	choices.Add(_T("Advanced Substation Alpha (.ass)"));
	choices.Add(_T("Substation Alpha (.ssa)"));
	choices.Add(_T("SubRip (.srt)"));
	choices.Add(_T("MicroDVD (.sub)"));
	choices.Add(_T("MPEG-4 Timed Text (.ttxt)"));
	ListBox = new wxCheckListBox(this,-1,wxDefaultPosition,wxSize(200,80), choices);
	ListBox->Check(0,CheckAssociation(_T("ass")));
	ListBox->Check(1,CheckAssociation(_T("ssa")));
	ListBox->Check(2,CheckAssociation(_T("srt")));
	ListBox->Check(3,CheckAssociation(_T("sub")));
	ListBox->Check(4,CheckAssociation(_T("ttxt")));

	// Label and list sizer
	wxStaticText *label = new wxStaticText(this,-1,_("Please select the formats you want to\nassociate with Aegisub:"),wxDefaultPosition,wxDefaultSize);
	wxSizer *ListSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Associations"));
	ListSizer->Add(label,0,0,0);
	ListSizer->Add(ListBox,0,wxTOP,5);

	// Buttons
	wxButton *ok = new wxButton(this, wxID_OK);
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
	ButtonSizer->Add(ok,0,0,0);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(ListSizer,0,wxLEFT | wxRIGHT,5);
	MainSizer->Add(ButtonSizer,0,wxALL | wxEXPAND,5);
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	Center();
}


//////////////
// Destructor
DialogAssociations::~DialogAssociations() {
}


/////////////////////////////////////
// Associates a type with Aegisub
void DialogAssociations::AssociateType(wxString type) {
	type.Lower();
	wxRegKey *key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\.") + type);
	if (!key->Exists()) key->Create();
	key->SetValue(_T(""),_T("Aegisub"));
	delete key;
}


//////////////////////////////////////////////////
// Checks if a type is associated with Aegisub
bool DialogAssociations::CheckAssociation(wxString type) {
	type.Lower();
	wxRegKey *key = new wxRegKey(_T("HKEY_CURRENT_USER\\Software\\Classes\\.") + type);
	if (!key->Exists()) {
		delete key;
		return false;
	}
	else {
		wxString value;
		key->QueryValue(_T(""),value);
		delete key;
		return (value == _T("Aegisub"));
	}
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogAssociations, wxDialog)
	EVT_BUTTON(wxID_OK,DialogAssociations::OnOK)
END_EVENT_TABLE()


//////////////
// OK pressed
void DialogAssociations::OnOK(wxCommandEvent &event) {
	if (ListBox->IsChecked(0)) AssociateType(_T("ass"));
	if (ListBox->IsChecked(1)) AssociateType(_T("ssa"));
	if (ListBox->IsChecked(2)) AssociateType(_T("srt"));
	if (ListBox->IsChecked(3)) AssociateType(_T("sub"));
	if (ListBox->IsChecked(4)) AssociateType(_T("ttxt"));
	event.Skip();
}
