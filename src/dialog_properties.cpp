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

#include "ass_file.h"
#include "async_video_provider.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "project.h"
#include "resolution_resampler.h"
#include "validators.h"
#include "video_controller.h"

#include <boost/algorithm/string/predicate.hpp>
#include <vector>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
class DialogProperties {
	wxDialog d;
	agi::Context *c; ///< Project this dialog is adjusting the properties of

	/// Pairs of a script property and a text control for that property
	std::vector<std::pair<std::string, wxTextCtrl*>> properties;

	// Things that effect rendering
	wxComboBox *WrapStyle;   ///< Wrapping style for long lines
	wxTextCtrl *ResX;        ///< Script x resolution
	wxTextCtrl *ResY;        ///< Script y resolution
	wxTextCtrl *LayoutResX;  ///< Layout x resolution
	wxTextCtrl *LayoutResY;  ///< Layout y resolution
	wxCheckBox *ScaleBorder; ///< If script resolution != video resolution how should borders be handled
	wxComboBox *YCbCrMatrix;

	/// OK button handler
	void OnOK(wxCommandEvent &event);
	/// Set script resolution to video resolution button
	void OnSetFromVideo(wxCommandEvent &event);
	/// Set layout resolution to video resolution button
	void OnSetLayoutResFromVideo(wxCommandEvent &event);
	/// Set a script info field
	/// @param key Name of field
	/// @param value New value
	/// @return Did the value actually need to be changed?
	int SetInfoIfDifferent(std::string_view key, std::string_view value);

	/// Add a property with label and text box for updating the property
	/// @param parent Parent to construct the label and control with
	/// @param sizer Sizer to add the label and control to
	/// @param label Label text to use
	/// @param property Script info property name
	void AddProperty(wxWindow *parent, wxSizer *sizer, wxString const& label, std::string_view property);

public:
	/// Constructor
	/// @param c Project context
	DialogProperties(agi::Context *c);
	void ShowModal() { d.ShowModal(); }
};

DialogProperties::DialogProperties(agi::Context *c)
: d(c->parent, -1, _("Script Properties"))
, c(c)
{
	d.SetIcons(GETICONS(properties_toolbutton));

	// Button sizer
	// Create buttons first. See:
	//  https://github.com/wangqr/Aegisub/issues/6
	//  https://trac.wxwidgets.org/ticket/18472#comment:9
	auto ButtonSizer = d.CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	d.Bind(wxEVT_BUTTON, &DialogProperties::OnOK, this, wxID_OK);
	d.Bind(wxEVT_BUTTON, std::bind(&HelpButton::OpenPage, "Properties"), wxID_HELP);

	// Script details crap
	wxStaticBoxSizer *TopSizer = new wxStaticBoxSizer(wxHORIZONTAL,&d,_("Script"));
	wxWindow *TopSizerBox = TopSizer->GetStaticBox();
	auto TopSizerGrid = new wxFlexGridSizer(0,2,5,5);

	AddProperty(TopSizerBox, TopSizerGrid, _("Title:"), "Title");
	AddProperty(TopSizerBox, TopSizerGrid, _("Original script:"), "Original Script");
	AddProperty(TopSizerBox, TopSizerGrid, _("Translation:"), "Original Translation");
	AddProperty(TopSizerBox, TopSizerGrid, _("Editing:"), "Original Editing");
	AddProperty(TopSizerBox, TopSizerGrid, _("Timing:"), "Original Timing");
	AddProperty(TopSizerBox, TopSizerGrid, _("Synch point:"), "Synch Point");
	AddProperty(TopSizerBox, TopSizerGrid, _("Updated by:"), "Script Updated By");
	AddProperty(TopSizerBox, TopSizerGrid, _("Update details:"), "Update Details");

	TopSizerGrid->AddGrowableCol(1,1);
	TopSizer->Add(TopSizerGrid,1,wxALL | wxEXPAND,0);

	// Resolution box
	wxStaticBoxSizer *res_box_sizer = new wxStaticBoxSizer(wxVERTICAL, &d, _("Resolution"));
	wxWindow *res_box = res_box_sizer->GetStaticBox();

	ResX = new wxTextCtrl(res_box,-1,"",wxDefaultPosition,wxDefaultSize,0,IntValidator(c->ass->GetScriptInfoAsInt("PlayResX")));
	ResY = new wxTextCtrl(res_box,-1,"",wxDefaultPosition,wxDefaultSize,0,IntValidator(c->ass->GetScriptInfoAsInt("PlayResY")));

	LayoutResX = new wxTextCtrl(res_box,-1,"",wxDefaultPosition,wxDefaultSize,0,IntValidator(c->ass->GetScriptInfoAsInt("LayoutResX")));
	LayoutResY = new wxTextCtrl(res_box,-1,"",wxDefaultPosition,wxDefaultSize,0,IntValidator(c->ass->GetScriptInfoAsInt("LayoutResY")));

	wxButton *FromVideo = new wxButton(res_box,-1,_("From &video"));
	if (!c->project->VideoProvider())
		FromVideo->Enable(false);
	else
		FromVideo->Bind(wxEVT_BUTTON, &DialogProperties::OnSetFromVideo, this);

	auto res_sizer = new wxFlexGridSizer(5, 5, 5);
	res_sizer->AddGrowableCol(1, 1);
	res_sizer->AddGrowableCol(3, 1);
	res_sizer->Add(new wxStaticText(res_box, -1, _("Script: ")), wxSizerFlags().Center().Left());
	res_sizer->Add(ResX, 1, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
	res_sizer->Add(new wxStaticText(res_box, -1, _(L"\u00D7")), 0, wxALIGN_CENTER | wxRIGHT, 2); // U+00D7 multiplication sign
	res_sizer->Add(ResY, 1, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
	res_sizer->Add(FromVideo, 1, 0, 0);

	wxButton *LayoutResFromVideo = new wxButton(res_box,-1,_("From video"));
	if (!c->project->VideoProvider())
		LayoutResFromVideo->Enable(false);
	else
		LayoutResFromVideo->Bind(wxEVT_BUTTON, &DialogProperties::OnSetLayoutResFromVideo, this);

	res_sizer->Add(new wxStaticText(res_box, -1, _("Layout: ")), wxSizerFlags().Center().Left());
	res_sizer->Add(LayoutResX, 1, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
	res_sizer->Add(new wxStaticText(res_box, -1, _(L"\u00D7")), 0, wxALIGN_CENTER | wxRIGHT, 2); // U+00D7 multiplication sign
	res_sizer->Add(LayoutResY, 1, wxRIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND, 2);
	res_sizer->Add(LayoutResFromVideo, 1, 0, 0);

	YCbCrMatrix = new wxComboBox(res_box, -1, to_wx(c->ass->GetScriptInfo("YCbCr Matrix")),
		 wxDefaultPosition, wxDefaultSize, to_wx(MatrixNames()), wxCB_READONLY);

	auto matrix_sizer = new wxBoxSizer(wxHORIZONTAL);
	matrix_sizer->Add(new wxStaticText(res_box, -1, _("YCbCr Matrix:")), wxSizerFlags().Center());
	matrix_sizer->Add(YCbCrMatrix, wxSizerFlags(1).Expand().Border(wxLEFT));

	res_box_sizer->Add(res_sizer, wxSizerFlags().Expand());
	res_box_sizer->Add(matrix_sizer, wxSizerFlags().Border(wxTOP).Expand());

	// Options
	wxStaticBoxSizer *optionsSizer = new wxStaticBoxSizer(wxHORIZONTAL,&d,_("Options"));
	wxWindow *optionsBox = optionsSizer->GetStaticBox();
	auto optionsGrid = new wxFlexGridSizer(3,2,5,5);
	wxString wrap_opts[] = {
		_("0: Smart wrapping, top line is wider"),
		_("1: End-of-line word wrapping, only \\N breaks"),
		_("2: No word wrapping, both \\n and \\N break"),
		_("3: Smart wrapping, bottom line is wider")
	};
	WrapStyle = new wxComboBox(optionsBox, -1, "", wxDefaultPosition, wxDefaultSize, 4, wrap_opts, wxCB_READONLY);
	WrapStyle->SetSelection(c->ass->GetScriptInfoAsInt("WrapStyle"));
	optionsGrid->Add(new wxStaticText(optionsBox,-1,_("Wrap Style: ")),0,wxALIGN_CENTER_VERTICAL,0);
	optionsGrid->Add(WrapStyle,1,wxEXPAND,0);

	ScaleBorder = new wxCheckBox(optionsBox,-1,_("Scale Border and Shadow"));
	ScaleBorder->SetToolTip(_("Scale border and shadow together with script/render resolution. If this is unchecked, relative border and shadow size will depend on renderer."));
	ScaleBorder->SetValue(boost::iequals(c->ass->GetScriptInfo("ScaledBorderAndShadow"), "yes"));
	optionsGrid->AddSpacer(0);
	optionsGrid->Add(ScaleBorder,1,wxEXPAND,0);
	optionsGrid->AddGrowableCol(1,1);
	optionsSizer->Add(optionsGrid,1,wxEXPAND,0);

	// MainSizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(res_box_sizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(optionsSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);

	d.SetSizerAndFit(MainSizer);
	d.CenterOnParent();
}

void DialogProperties::AddProperty(wxWindow *parent, wxSizer *sizer, wxString const& label, std::string_view property) {
	wxTextCtrl *ctrl = new wxTextCtrl(parent, -1, to_wx(c->ass->GetScriptInfo(property)));
	sizer->Add(new wxStaticText(parent, -1, label), wxSizerFlags().Center().Left());
	sizer->Add(ctrl, wxSizerFlags(1).Expand());
	properties.emplace_back(property, ctrl);
}

void DialogProperties::OnOK(wxCommandEvent &) {
	int count = 0;
	for (auto const& prop : properties)
		count += SetInfoIfDifferent(prop.first, from_wx(prop.second->GetValue()));

	count += SetInfoIfDifferent("PlayResX", from_wx(ResX->GetValue()));
	count += SetInfoIfDifferent("PlayResY", from_wx(ResY->GetValue()));
	count += SetInfoIfDifferent("LayoutResX", from_wx(LayoutResX->GetValue()));
	count += SetInfoIfDifferent("LayoutResY", from_wx(LayoutResY->GetValue()));
	count += SetInfoIfDifferent("WrapStyle", std::to_string(WrapStyle->GetSelection()));
	count += SetInfoIfDifferent("ScaledBorderAndShadow", ScaleBorder->GetValue() ? "yes" : "no");
	count += SetInfoIfDifferent("YCbCr Matrix", from_wx(YCbCrMatrix->GetValue()));

	if (count) c->ass->Commit(_("property changes"), AssFile::COMMIT_SCRIPTINFO);

	d.EndModal(!!count);
}

int DialogProperties::SetInfoIfDifferent(std::string_view key, std::string_view value) {
	if (c->ass->GetScriptInfo(key) != value) {
		c->ass->SetScriptInfo(key, value);
		return 1;
	}
	return 0;
}

std::pair<int, int> GetVideoDisplayResolution(agi::Context *c) {
	double dar = c->videoController->GetAspectRatioValue();
	int width = c->project->VideoProvider()->GetWidth();
	int height = c->project->VideoProvider()->GetHeight();
	double sar = double(width) / double(height);

	return std::make_pair(
		width * std::max(1., dar / sar),
		height * std::max(1., sar / dar)
	);
}

void DialogProperties::OnSetFromVideo(wxCommandEvent &) {
	auto [width, height] = GetVideoDisplayResolution(c);
	ResX->SetValue(std::to_wstring(width));
	ResY->SetValue(std::to_wstring(height));
}

void DialogProperties::OnSetLayoutResFromVideo(wxCommandEvent &) {
	auto [width, height] = GetVideoDisplayResolution(c);
	LayoutResX->SetValue(std::to_wstring(width));
	LayoutResY->SetValue(std::to_wstring(height));
}
}

void ShowPropertiesDialog(agi::Context *c) {
	DialogProperties(c).ShowModal();
}
