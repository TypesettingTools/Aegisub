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

/// @file dialog_properties.cpp
/// @brief Dialogue box to set subtitle meta-data
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_properties.h"

#include "ass_file.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "validators.h"
#include "video_context.h"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

DialogProperties::DialogProperties(agi::Context *c)
: wxDialog(c->parent, -1, _("Script Properties"))
, c(c)
{
	SetIcon(GETICON(properties_toolbutton_16));

	// Script details crap
	wxSizer *TopSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Script"));
	auto TopSizerGrid = new wxFlexGridSizer(0,2,5,5);

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
	ResX = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,IntValidator(c->ass->GetScriptInfoAsInt("PlayResX")));
	ResY = new wxTextCtrl(this,-1,"",wxDefaultPosition,wxSize(50,20),0,IntValidator(c->ass->GetScriptInfoAsInt("PlayResY")));
	wxStaticText *ResText = new wxStaticText(this,-1,"x");

	wxButton *FromVideo = new wxButton(this,-1,_("From &video"));
	if (!c->videoController->IsLoaded())
		FromVideo->Enable(false);
	else
		FromVideo->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnSetFromVideo, this);

	ResSizer->Add(ResX,1,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
	ResSizer->Add(ResText,0,wxALIGN_CENTER | wxRIGHT,5);
	ResSizer->Add(ResY,1,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
	ResSizer->Add(FromVideo,1,0,0);

	// Options
	wxSizer *optionsBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	auto optionsGrid = new wxFlexGridSizer(3,2,5,5);
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

	ScaleBorder = new wxCheckBox(this,-1,_("Scale Border and Shadow"));
	ScaleBorder->SetToolTip(_("Scale border and shadow together with script/render resolution. If this is unchecked, relative border and shadow size will depend on renderer."));
	ScaleBorder->SetValue(boost::iequals(c->ass->GetScriptInfo("ScaledBorderAndShadow"), "yes"));
	optionsGrid->AddSpacer(0);
	optionsGrid->Add(ScaleBorder,1,wxEXPAND,0);
	optionsGrid->AddGrowableCol(1,1);
	optionsBox->Add(optionsGrid,1,wxEXPAND,0);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProperties::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&HelpButton::OpenPage, "Properties"), wxID_HELP);

	// MainSizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ResSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(optionsBox,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);

	SetSizerAndFit(MainSizer);
	CenterOnParent();
}

void DialogProperties::AddProperty(wxSizer *sizer, wxString const& label, std::string const& property) {
	wxTextCtrl *ctrl = new wxTextCtrl(this, -1, to_wx(c->ass->GetScriptInfo(property)), wxDefaultPosition, wxSize(200, 20));
	sizer->Add(new wxStaticText(this, -1, label), wxSizerFlags().Center().Left());
	sizer->Add(ctrl, wxSizerFlags(1).Expand());
	properties.push_back(std::make_pair(property, ctrl));
}

void DialogProperties::OnOK(wxCommandEvent &) {
	int count = 0;
	for (auto const& prop : properties)
		count += SetInfoIfDifferent(prop.first, from_wx(prop.second->GetValue()));

	count += SetInfoIfDifferent("PlayResX", from_wx(ResX->GetValue()));
	count += SetInfoIfDifferent("PlayResY", from_wx(ResY->GetValue()));
	count += SetInfoIfDifferent("WrapStyle", std::to_string(WrapStyle->GetSelection()));
	count += SetInfoIfDifferent("ScaledBorderAndShadow", ScaleBorder->GetValue() ? "yes" : "no");

	if (count) c->ass->Commit(_("property changes"), AssFile::COMMIT_SCRIPTINFO);

	EndModal(!!count);
}

int DialogProperties::SetInfoIfDifferent(std::string const& key, std::string const&value) {
	if (c->ass->GetScriptInfo(key) != value) {
		c->ass->SetScriptInfo(key, value);
		return 1;
	}
	return 0;
}

void DialogProperties::OnSetFromVideo(wxCommandEvent &) {
	ResX->SetValue(std::to_wstring(c->videoController->GetWidth()));
	ResY->SetValue(std::to_wstring(c->videoController->GetHeight()));
}
