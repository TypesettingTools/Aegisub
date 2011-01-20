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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_properties.cpp
/// @brief Dialogue box to set subtitle meta-data
/// @ingroup secondary_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#endif

#include "dialog_properties.h"

#include "ass_file.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"

DialogProperties::DialogProperties(agi::Context *c)
: wxDialog(c->parent, -1, _("Script Properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
, c(c)
{
	SetIcon(BitmapToIcon(GETIMAGE(properties_toolbutton_24)));

	// Script details crap
	wxSizer *TopSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Script"));
	wxStaticText *TitleLabel = new wxStaticText(this,-1,_("Title:"));
	TitleEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Title"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *OrigScriptLabel = new wxStaticText(this,-1,_("Original script:"));
	OrigScriptEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Original Script"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *TranslationLabel = new wxStaticText(this,-1,_("Translation:"));
	TranslationEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Original Translation"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *EditingLabel = new wxStaticText(this,-1,_("Editing:"));
	EditingEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Original Editing"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *TimingLabel = new wxStaticText(this,-1,_("Timing:"));
	TimingEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Original Timing"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *SyncLabel = new wxStaticText(this,-1,_("Synch point:"));
	SyncEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Synch Point"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *UpdatedLabel = new wxStaticText(this,-1,_("Updated by:"));
	UpdatedEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Script Updated By"),wxDefaultPosition,wxSize(200,20));
	wxStaticText *UpdateDetailsLabel = new wxStaticText(this,-1,_("Update details:"));
	UpdateDetailsEdit = new wxTextCtrl(this,-1,c->ass->GetScriptInfo("Update Details"),wxDefaultPosition,wxSize(200,20));
	wxFlexGridSizer *TopSizerGrid = new wxFlexGridSizer(0,2,5,5);
	TopSizerGrid->Add(TitleLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(TitleEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(OrigScriptLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(OrigScriptEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(TranslationLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(TranslationEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(EditingLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(EditingEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(TimingLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(TimingEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(SyncLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(SyncEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(UpdatedLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(UpdatedEdit,1,wxEXPAND,0);
	TopSizerGrid->Add(UpdateDetailsLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL,0);
	TopSizerGrid->Add(UpdateDetailsEdit,1,wxEXPAND,0);
	TopSizerGrid->AddGrowableCol(1,1);
	TopSizer->Add(TopSizerGrid,1,wxALL | wxEXPAND,0);

	// Resolution box
	wxSizer *ResSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Resolution"));
	ResX = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,NumValidator(c->ass->GetScriptInfo("PlayResX")));
	ResY = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,NumValidator(c->ass->GetScriptInfo("PlayResY")));
	wxStaticText *ResText = new wxStaticText(this,-1,"x");
	wxButton *FromVideo = new wxButton(this,-1,_("From video"));
	if (!c->videoController->IsLoaded()) {
		FromVideo->Enable(false);
	}
	else {
		FromVideo->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnSetFromVideo, this);
	}
	ResSizer->Add(ResX,1,wxRIGHT,5);
	ResSizer->Add(ResText,0,wxALIGN_CENTER | wxRIGHT,5);
	ResSizer->Add(ResY,1,wxRIGHT,5);
	ResSizer->Add(FromVideo,1,0,0);

	// Options
	wxSizer *optionsBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	wxFlexGridSizer *optionsGrid = new wxFlexGridSizer(3,2,5,5);
	wxArrayString options;
	options.Add(_("0: Smart wrapping, top line is wider"));
	options.Add(_("1: End-of-line word wrapping, only \\N breaks"));
	options.Add(_("2: No word wrapping, both \\n and \\N break"));
	options.Add(_("3: Smart wrapping, bottom line is wider"));
	WrapStyle = new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,options,wxCB_READONLY);
	long n;
	c->ass->GetScriptInfo("WrapStyle").ToLong(&n);
	WrapStyle->SetSelection(n);
	optionsGrid->Add(new wxStaticText(this,-1,_("Wrap Style: ")),0,wxALIGN_CENTER_VERTICAL,0);
	optionsGrid->Add(WrapStyle,1,wxEXPAND,0);
	options.Clear();
	options.Add(_("Normal"));
	options.Add(_("Reverse"));
	collision = new wxComboBox(this,-1,"",wxDefaultPosition,wxDefaultSize,options,wxCB_READONLY);
	wxString col = c->ass->GetScriptInfo("Collisions");
	if (col.Lower() == "reverse") collision->SetSelection(1);
	else collision->SetSelection(0);
	optionsGrid->Add(new wxStaticText(this,-1,_("Collision: ")),0,wxALIGN_CENTER_VERTICAL,0);
	optionsGrid->Add(collision,1,wxEXPAND,0);
	ScaleBorder = new wxCheckBox(this,-1,_("Scale Border and Shadow"));
	ScaleBorder->SetToolTip(_("Scale border and shadow together with script/render resolution. If this is unchecked, relative border and shadow size will depend on renderer."));
	ScaleBorder->SetValue(c->ass->GetScriptInfo("ScaledBorderAndShadow").Lower() == "yes" ? 1 : 0);
	optionsGrid->AddSpacer(0);
	optionsGrid->Add(ScaleBorder,1,wxEXPAND,0);
	optionsGrid->AddGrowableCol(1,1);
	optionsBox->Add(optionsGrid,1,wxEXPAND,0);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this,wxID_OK));
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,"Properties"));
	ButtonSizer->Realize();
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnOK, this, wxID_OK);

	// MainSizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ResSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(optionsBox,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	CenterOnParent();
}

void DialogProperties::OnOK(wxCommandEvent &event) {
	int count = 0;
	count += SetInfoIfDifferent("Title",TitleEdit->GetValue());
	count += SetInfoIfDifferent("Original Script",OrigScriptEdit->GetValue());
	count += SetInfoIfDifferent("Original Translation",TranslationEdit->GetValue());
	count += SetInfoIfDifferent("Original Editing",EditingEdit->GetValue());
	count += SetInfoIfDifferent("Original Timing",TimingEdit->GetValue());
	count += SetInfoIfDifferent("Synch Point",SyncEdit->GetValue());
	count += SetInfoIfDifferent("Script Updated By",UpdatedEdit->GetValue());
	count += SetInfoIfDifferent("Update Details",UpdateDetailsEdit->GetValue());
	count += SetInfoIfDifferent("PlayResX",ResX->GetValue());
	count += SetInfoIfDifferent("PlayResY",ResY->GetValue());
	count += SetInfoIfDifferent("WrapStyle",wxString::Format("%i",WrapStyle->GetSelection()));
	wxString col[2] = { "Normal", "Reverse"};
	count += SetInfoIfDifferent("Collisions",col[collision->GetSelection()]);
	count += SetInfoIfDifferent("ScaledBorderAndShadow",ScaleBorder->GetValue()? "yes" : "no");

	if (count) c->ass->Commit(_("property changes"));

	EndModal(!!count);
}

int DialogProperties::SetInfoIfDifferent(wxString key,wxString value) {
	if (c->ass->GetScriptInfo(key) != value) {
		c->ass->SetScriptInfo(key,value);
		return 1;
	}
	return 0;
}

void DialogProperties::OnSetFromVideo(wxCommandEvent &event) {
	ResX->SetValue(wxString::Format("%i",c->videoController->GetWidth()));
	ResY->SetValue(wxString::Format("%i",c->videoController->GetHeight()));
	event.Skip();
}
