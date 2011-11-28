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
#include <algorithm>

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

DialogProperties::DialogProperties(agi::Context *c)
: wxDialog(c->parent, -1, _("Script Properties"))
, c(c)
{
	SetIcon(BitmapToIcon(GETIMAGE(properties_toolbutton_24)));

	// Script details crap
	wxSizer *TopSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Script"));
	wxFlexGridSizer *TopSizerGrid = new wxFlexGridSizer(0,2,5,5);

	AddProperty(TopSizerGrid, _("Title:"), "Title");
	AddProperty(TopSizerGrid, _("Original script:"), "Original Script");
	AddProperty(TopSizerGrid, _("Translation:"), "Original Translation");
	AddProperty(TopSizerGrid, _("Editing:"), "Original Editing");
	AddProperty(TopSizerGrid, _("Timing:"), "Original Timing");
	AddProperty(TopSizerGrid, _("Synch point:"), "Synch Point");
	AddProperty(TopSizerGrid, _("Updated by:"), "Script Updated By");
	AddProperty(TopSizerGrid, _("Update details:"), "Update Details");

	TopSizerGrid->AddGrowableCol(1,1);
	TopSizer->Add(TopSizerGrid,1,wxALL | wxEXPAND,0);

	// Resolution box
	wxSizer *ResSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Resolution"));
	ResX = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,NumValidator(c->ass->GetScriptInfo("PlayResX")));
	ResY = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,NumValidator(c->ass->GetScriptInfo("PlayResY")));
	wxStaticText *ResText = new wxStaticText(this,-1,"x");

	wxButton *FromVideo = new wxButton(this,-1,_("From &video"));
	if (!c->videoController->IsLoaded())
		FromVideo->Enable(false);
	else
		FromVideo->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnSetFromVideo, this);

	ResSizer->Add(ResX,1,wxRIGHT,5);
	ResSizer->Add(ResText,0,wxALIGN_CENTER | wxRIGHT,5);
	ResSizer->Add(ResY,1,wxRIGHT,5);
	ResSizer->Add(FromVideo,1,0,0);

	// Options
	wxSizer *optionsBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	wxFlexGridSizer *optionsGrid = new wxFlexGridSizer(3,2,5,5);
	wxString wrap_opts[] = {
		_("0: Smart wrapping, top line is wider"),
		_("1: End-of-line word wrapping, only \\N breaks"),
		_("2: No word wrapping, both \\n and \\N break"),
		_("3: Smart wrapping, bottom line is wider")
	};
	WrapStyle = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, 4, wrap_opts, wxCB_READONLY);
	WrapStyle->SetSelection(c->ass->GetScriptInfoAsInt("WrapStyle"));
	optionsGrid->Add(new wxStaticText(this,-1,_("Wrap Style: ")),0,wxALIGN_CENTER_VERTICAL,0);
	optionsGrid->Add(WrapStyle,1,wxEXPAND,0);

	wxString coll_opts[] = { _("Normal"), _("Reverse") };
	collision = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, 2, coll_opts, wxCB_READONLY);
	collision->SetSelection(c->ass->GetScriptInfo("Collisions").Lower() == "reverse");
	optionsGrid->Add(new wxStaticText(this,-1,_("Collision: ")),0,wxALIGN_CENTER_VERTICAL,0);
	optionsGrid->Add(collision,1,wxEXPAND,0);

	ScaleBorder = new wxCheckBox(this,-1,_("Scale Border and Shadow"));
	ScaleBorder->SetToolTip(_("Scale border and shadow together with script/render resolution. If this is unchecked, relative border and shadow size will depend on renderer."));
	ScaleBorder->SetValue(c->ass->GetScriptInfo("ScaledBorderAndShadow").Lower() == "yes");
	optionsGrid->AddSpacer(0);
	optionsGrid->Add(ScaleBorder,1,wxEXPAND,0);
	optionsGrid->AddGrowableCol(1,1);
	optionsBox->Add(optionsGrid,1,wxEXPAND,0);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&HelpButton::OpenPage, "Properties"), wxID_HELP);

	// MainSizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ResSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(optionsBox,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);

	SetSizerAndFit(MainSizer);
	CenterOnParent();
}

void DialogProperties::AddProperty(wxSizer *sizer, wxString const& label, wxString const& property) {
	wxTextCtrl *ctrl = new wxTextCtrl(this, -1, c->ass->GetScriptInfo(property), wxDefaultPosition, wxSize(200, 20));
	sizer->Add(new wxStaticText(this, -1, label), wxSizerFlags().Center().Left());
	sizer->Add(ctrl, wxSizerFlags(1).Expand());
	properties.push_back(std::make_pair(property, ctrl));
}

void DialogProperties::OnOK(wxCommandEvent &event) {
	int count = 0;
	for (size_t i = 0; i < properties.size(); ++i)
		count += SetInfoIfDifferent(properties[i].first, properties[i].second->GetValue());

	count += SetInfoIfDifferent("PlayResX", ResX->GetValue());
	count += SetInfoIfDifferent("PlayResY", ResY->GetValue());
	count += SetInfoIfDifferent("WrapStyle", wxString::Format("%d", WrapStyle->GetSelection()));
	wxString col[2] = { "Normal", "Reverse" };
	count += SetInfoIfDifferent("Collisions", col[collision->GetSelection()]);
	count += SetInfoIfDifferent("ScaledBorderAndShadow", ScaleBorder->GetValue()? "yes" : "no");

	if (count) c->ass->Commit(_("property changes"), AssFile::COMMIT_SCRIPTINFO);

	EndModal(!!count);
}

int DialogProperties::SetInfoIfDifferent(wxString key,wxString value) {
	if (c->ass->GetScriptInfo(key) != value) {
		c->ass->SetScriptInfo(key, value);
		return 1;
	}
	return 0;
}

void DialogProperties::OnSetFromVideo(wxCommandEvent &event) {
	ResX->SetValue(wxString::Format("%d", c->videoController->GetWidth()));
	ResY->SetValue(wxString::Format("%d", c->videoController->GetHeight()));
}
