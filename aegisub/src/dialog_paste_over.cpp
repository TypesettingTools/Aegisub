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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_paste_over.cpp
/// @brief Paste Over set-up dialogue box
/// @ingroup secondary_ui
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/config.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#endif

#include "dialog_paste_over.h"
#include "help_button.h"
#include "main.h"
#include "options.h"


/// @brief Constructor 
/// @param parent 
///
DialogPasteOver::DialogPasteOver (wxWindow *parent)
: wxDialog (parent,-1,_("Select Fields to Paste Over"),wxDefaultPosition,wxDefaultSize)
{
	// Script mode
	int mode = 1; // ASS

	// Label and list sizer
	wxSizer *ListSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Fields"));
	wxStaticText *label = new wxStaticText(this,-1,_("Please select the fields that you want to paste over:"),wxDefaultPosition,wxDefaultSize);
	ListSizer->Add(label,0,wxEXPAND,0);
	
	// List box
	wxArrayString choices;
	choices.Add(_("Layer"));
	choices.Add(_("Start Time"));
	choices.Add(_("End Time"));
	choices.Add(_("Style"));
	choices.Add(_("Actor"));
	choices.Add(_("Margin Left"));
	choices.Add(_("Margin Right"));
	if (mode == 1) {
		choices.Add(_("Margin Vertical"));
	}
	else {
		choices.Add(_("Margin Top"));
		choices.Add(_("Margin Bottom"));
	}
	choices.Add(_("Effect"));
	choices.Add(_("Text"));
	ListBox = new wxCheckListBox(this,-1,wxDefaultPosition,wxSize(250,170), choices);
	ListSizer->Add(ListBox,0,wxEXPAND|wxTOP,5);

	// Load checked items
	/// @todo This assumes a static set of fields.
	std::vector<bool> choice_array;
	OPT_GET("Tool/Paste Lines Over/Fields")->GetListBool(choice_array);
	for (unsigned int i=0;i<choices.Count();i++) ListBox->Check(i,choice_array.at(i));

	// Top buttons
	wxSizer *TopButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	TopButtonSizer->Add(new wxButton(this, Paste_Over_All, _("All")),1,0,0);
	TopButtonSizer->Add(new wxButton(this, Paste_Over_None, _("None")),1,0,0);
	TopButtonSizer->Add(new wxButton(this, Paste_Over_Times, _("Times")),1,0,0);
	TopButtonSizer->Add(new wxButton(this, Paste_Over_Text, _("Text")),1,0,0);

	// Buttons
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this, wxID_OK));
	ButtonSizer->AddButton(new wxButton(this, wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Paste Over")));
	ButtonSizer->Realize();

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(ListSizer,0,wxEXPAND | wxLEFT | wxRIGHT,5);
	MainSizer->Add(TopButtonSizer,0,wxLEFT | wxRIGHT | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxALL | wxEXPAND,5);
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	Center();
}



/// @brief Destructor 
///
DialogPasteOver::~DialogPasteOver() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogPasteOver, wxDialog)
	EVT_BUTTON(wxID_OK,DialogPasteOver::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogPasteOver::OnCancel)
	EVT_BUTTON(Paste_Over_All,DialogPasteOver::OnAll)
	EVT_BUTTON(Paste_Over_None,DialogPasteOver::OnNone)
	EVT_BUTTON(Paste_Over_Text,DialogPasteOver::OnText)
	EVT_BUTTON(Paste_Over_Times,DialogPasteOver::OnTimes)
END_EVENT_TABLE()



/// @brief OK pressed 
/// @param event 
///
void DialogPasteOver::OnOK(wxCommandEvent &event) {

	std::vector<bool> map;
	for (int i=0;i<10;i++) {
		map[i] = ListBox->IsChecked(i);
	}
	OPT_SET("Tool/Paste Lines Over/Fields")->SetListBool(map);

	EndModal(1);
}



/// @brief Cancel pressed 
/// @param event 
///
void DialogPasteOver::OnCancel(wxCommandEvent &event) {
	EndModal(0);
}



/// @brief Select Text 
/// @param event 
///
void DialogPasteOver::OnText(wxCommandEvent &event) {
	for (int i=0;i<9;i++) ListBox->Check(i,false);
	ListBox->Check(9,true);
}



/// @brief Select Times 
/// @param event 
///
void DialogPasteOver::OnTimes(wxCommandEvent &event) {
	for (int i=0;i<10;i++) ListBox->Check(i,false);
	ListBox->Check(1,true);
	ListBox->Check(2,true);
}



/// @brief Select All 
/// @param event 
///
void DialogPasteOver::OnAll(wxCommandEvent &event) {
	for (int i=0;i<10;i++) ListBox->Check(i,true);
}



/// @brief Select None 
/// @param event 
///
void DialogPasteOver::OnNone(wxCommandEvent &event) {
	for (int i=0;i<10;i++) ListBox->Check(i,false);
}



/// @brief Get options 
///
wxArrayInt DialogPasteOver::GetOptions() {
	return options;
}


