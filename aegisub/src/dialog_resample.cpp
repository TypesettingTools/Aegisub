// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

/// @file dialog_resample.cpp
/// @brief Resample Resolution dialogue box and logic
/// @ingroup tools_ui
///

#include "config.h"

#include "dialog_resample.h"

#include "ass_file.h"
#include "include/aegisub/context.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "resolution_resampler.h"
#include "video_context.h"

#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

enum {
	LEFT = 0,
	RIGHT = 1,
	TOP = 2,
	BOTTOM = 3
};

DialogResample::DialogResample(agi::Context *c, ResampleSettings &settings)
: wxDialog(c->parent, -1, _("Resample Resolution"))
, c(c)
{
	SetIcon(GETICON(resample_toolbutton_16));

	memset(&settings, 0, sizeof(settings));
	c->ass->GetResolution(settings.script_x, settings.script_y);

	// Create all controls and set validators
	for (size_t i = 0; i < 4; ++i) {
		margin_ctrl[i] = new wxSpinCtrl(this, -1, "0", wxDefaultPosition, wxSize(50, -1), wxSP_ARROW_KEYS, -9999, 9999, 0);
		margin_ctrl[i]->SetValidator(wxGenericValidator(&settings.margin[i]));
	}

	symmetrical = new wxCheckBox(this, -1, _("&Symmetrical"));
	symmetrical->SetValue(true);

	margin_ctrl[RIGHT]->Enable(false);
	margin_ctrl[BOTTOM]->Enable(false);

	res_x = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxSize(50, -1), wxSP_ARROW_KEYS, 1, INT_MAX);
	res_y = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxSize(50, -1), wxSP_ARROW_KEYS, 1, INT_MAX);

	res_x->SetValidator(wxGenericValidator(&settings.script_x));
	res_y->SetValidator(wxGenericValidator(&settings.script_y));

	wxButton *from_video = new wxButton(this, -1, _("From &video"));
	from_video->Enable(c->videoController->IsLoaded());

	wxCheckBox *change_ar = new wxCheckBox(this, -1, _("&Change aspect ratio"));
	change_ar->SetValidator(wxGenericValidator(&settings.change_ar));

	// Position the controls
	wxSizer *margin_sizer = new wxGridSizer(3, 3, 5, 5);
	margin_sizer->AddSpacer(1);
	margin_sizer->Add(margin_ctrl[TOP], wxSizerFlags(1).Expand());
	margin_sizer->AddSpacer(1);
	margin_sizer->Add(margin_ctrl[LEFT], wxSizerFlags(1).Expand());
	margin_sizer->Add(symmetrical, wxSizerFlags(1).Expand());
	margin_sizer->Add(margin_ctrl[RIGHT], wxSizerFlags(1).Expand());
	margin_sizer->AddSpacer(1);
	margin_sizer->Add(margin_ctrl[BOTTOM], wxSizerFlags(1).Expand());
	margin_sizer->AddSpacer(1);

	wxSizer *margin_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Margin offset"));
	margin_box->Add(margin_sizer, wxSizerFlags(1).Expand().Border(wxBOTTOM));

	wxSizer *res_sizer = new wxBoxSizer(wxHORIZONTAL);
	res_sizer->Add(res_x, wxSizerFlags(1).Border(wxRIGHT).Align(wxALIGN_CENTER_VERTICAL));
	res_sizer->Add(new wxStaticText(this, -1, _("x")), wxSizerFlags().Center().Border(wxRIGHT));
	res_sizer->Add(res_y, wxSizerFlags(1).Border(wxRIGHT).Align(wxALIGN_CENTER_VERTICAL));
	res_sizer->Add(from_video, wxSizerFlags(1));

	wxSizer *res_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Resolution"));
	res_box->Add(res_sizer, wxSizerFlags(1).Expand().Border(wxBOTTOM));
	res_box->Add(change_ar);

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(margin_box, wxSizerFlags(1).Expand().Border());
	main_sizer->Add(res_box, wxSizerFlags(0).Expand().Border());
	main_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP), wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	SetSizerAndFit(main_sizer);
	CenterOnParent();

	// Bind events
	using std::bind;
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&HelpButton::OpenPage, "Resample resolution"), wxID_HELP);
	from_video->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogResample::SetDestFromVideo, this);
	symmetrical->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &DialogResample::OnSymmetrical, this);
	margin_ctrl[LEFT]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, bind(&DialogResample::OnMarginChange, this, margin_ctrl[LEFT], margin_ctrl[RIGHT]));
	margin_ctrl[TOP]->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, bind(&DialogResample::OnMarginChange, this, margin_ctrl[TOP], margin_ctrl[BOTTOM]));
}

void DialogResample::SetDestFromVideo(wxCommandEvent &) {
	res_x->SetValue(c->videoController->GetWidth());
	res_y->SetValue(c->videoController->GetHeight());
}

void DialogResample::OnSymmetrical(wxCommandEvent &) {
	bool state = !symmetrical->IsChecked();

	margin_ctrl[RIGHT]->Enable(state);
	margin_ctrl[BOTTOM]->Enable(state);

	if (!state) {
		margin_ctrl[RIGHT]->SetValue(margin_ctrl[LEFT]->GetValue());
		margin_ctrl[BOTTOM]->SetValue(margin_ctrl[TOP]->GetValue());
	}
}

void DialogResample::OnMarginChange(wxSpinCtrl *src, wxSpinCtrl *dst) {
	if (symmetrical->IsChecked())
		dst->SetValue(src->GetValue());
}
