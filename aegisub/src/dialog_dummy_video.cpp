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
//
// Aegisub Project http://www.aegisub.org/

/// @file dialog_dummy_video.cpp
/// @brief Set up dummy video provider
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_dummy_video.h"

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valnum.h>

#include "ass_time.h"
#include "colour_button.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "video_provider_dummy.h"

namespace {

struct ResolutionShortcut {
	const char *name;
	int width;
	int height;
};

static ResolutionShortcut resolutions[] = {
	{"640x480 (SD fullscreen)", 640, 480},
	{"704x480 (SD anamorphic)", 704, 480},
	{"640x360 (SD widescreen)", 640, 360},
	{"704x396 (SD widescreen)", 704, 396},
	{"640x352 (SD widescreen MOD16)", 640, 352},
	{"704x400 (SD widescreen MOD16)", 704, 400},
	{"1280x720 (HD 720p)", 1280, 720},
	{"1920x1080 (HD 1080p)", 1920, 1080},
	{"1024x576 (SuperPAL widescreen)", 1024, 576}
};

wxSpinCtrl *spin_ctrl(wxWindow *parent, int min, int max, int *value) {
	auto ctrl = new wxSpinCtrl(parent, -1, "", wxDefaultPosition, wxSize(50, -1), wxSP_ARROW_KEYS, min, max, *value);
	ctrl->SetValidator(wxGenericValidator(value));
	return ctrl;
}

wxControl *spin_ctrl(wxWindow *parent, double min, double max, double *value) {
	wxFloatingPointValidator<double> val(4, value);
	val.SetRange(min, max);
	return new wxTextCtrl(parent, -1, "", wxDefaultPosition, wxSize(50, -1), 0, val);
}

wxComboBox *resolution_shortcuts(wxWindow *parent, int width, int height) {
	wxComboBox *ctrl = new wxComboBox(parent, -1, "", wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_READONLY);

	for (auto const& res : resolutions) {
		ctrl->Append(res.name);
		if (res.width == width && res.height == height)
			ctrl->SetSelection(ctrl->GetCount() - 1);
	}

	return ctrl;
}

}

DialogDummyVideo::DialogDummyVideo(wxWindow *parent)
: wxDialog(parent, -1, _("Dummy video options"))
, fps(OPT_GET("Video/Dummy/FPS")->GetDouble())
, width(OPT_GET("Video/Dummy/Last/Width")->GetInt())
, height(OPT_GET("Video/Dummy/Last/Height")->GetInt())
, length(OPT_GET("Video/Dummy/Last/Length")->GetInt())
, color(OPT_GET("Colour/Video Dummy/Last Colour")->GetColor())
, pattern(OPT_GET("Video/Dummy/Pattern")->GetBool())
{
	SetIcon(GETICON(use_dummy_video_menu_16));

	wxBoxSizer *res_sizer = new wxBoxSizer(wxHORIZONTAL);
	res_sizer->Add(spin_ctrl(this, 1, 10000, &width), wxSizerFlags(1).Expand());
	res_sizer->Add(new wxStaticText(this, -1, " x "), wxSizerFlags().Center());
	res_sizer->Add(spin_ctrl(this, 1, 10000, &height), wxSizerFlags(1).Expand());

	wxBoxSizer *color_sizer = new wxBoxSizer(wxHORIZONTAL);
	ColourButton *color_btn = new ColourButton(this, -1, wxSize(30, 17), color);
	color_sizer->Add(color_btn, wxSizerFlags().DoubleBorder(wxRIGHT));
	color_sizer->Add(new wxCheckBox(this, -1, _("Checkerboard &pattern"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&pattern)), wxSizerFlags(1).Center());

	sizer = new wxFlexGridSizer(2, 5, 5);
	AddCtrl(_("Video resolution:"), resolution_shortcuts(this, width, height));
	AddCtrl("", res_sizer);
	AddCtrl(_("Color:"), color_sizer);
	AddCtrl(_("Frame rate (fps):"), spin_ctrl(this, .1, 1000.0, &fps));
	AddCtrl(_("Duration (frames):"), spin_ctrl(this, 2, 36000000, &length)); // Ten hours of 1k FPS
	AddCtrl("", length_display = new wxStaticText(this, -1, ""));

	wxStdDialogButtonSizer *btn_sizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	btn_sizer->GetHelpButton()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&HelpButton::OpenPage, "Dummy Video"));

	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(sizer, wxSizerFlags(1).Border().Expand());
	main_sizer->Add(new wxStaticLine(this, wxHORIZONTAL), wxSizerFlags().HorzBorder().Expand());
	main_sizer->Add(btn_sizer, wxSizerFlags().Expand().Border());

	UpdateLengthDisplay();

	SetSizerAndFit(main_sizer);
	CenterOnParent();

	Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &DialogDummyVideo::OnResolutionShortcut, this);
	color_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& e) {
		color = color_btn->GetColor();
		e.Skip();
	});
	Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, [=](wxCommandEvent&) {
		TransferDataFromWindow();
		UpdateLengthDisplay();
	});
}

template<typename T>
void DialogDummyVideo::AddCtrl(wxString const& label, T *ctrl) {
	if (!label)
		sizer->AddStretchSpacer();
	else
		sizer->Add(new wxStaticText(this, -1, label), wxSizerFlags().Center().Left());
	sizer->Add(ctrl, wxSizerFlags().Expand().Center().Left());
}

void DialogDummyVideo::OnResolutionShortcut(wxCommandEvent &e) {
	TransferDataFromWindow();
	int rs = e.GetSelection();
	width = resolutions[rs].width;
	height = resolutions[rs].height;
	TransferDataToWindow();
}

void DialogDummyVideo::UpdateLengthDisplay() {
	length_display->SetLabel(wxString::Format(_("Resulting duration: %s"), AssTime(length / fps * 1000).GetAssFormated(true)));
}

wxString DialogDummyVideo::CreateDummyVideo(wxWindow *parent) {
	DialogDummyVideo dlg(parent);
	if (dlg.ShowModal() != wxID_OK)
		return "";

	OPT_SET("Video/Dummy/FPS")->SetDouble(dlg.fps);
	OPT_SET("Video/Dummy/Last/Width")->SetInt(dlg.width);
	OPT_SET("Video/Dummy/Last/Height")->SetInt(dlg.height);
	OPT_SET("Video/Dummy/Last/Length")->SetInt(dlg.length);
	OPT_SET("Colour/Video Dummy/Last Colour")->SetColor(dlg.color);
	OPT_SET("Video/Dummy/Pattern")->SetBool(dlg.pattern);

	return DummyVideoProvider::MakeFilename(dlg.fps, dlg.length, dlg.width, dlg.height, dlg.color, dlg.pattern);
}
