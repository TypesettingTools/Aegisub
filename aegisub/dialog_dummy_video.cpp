// Copyright (c) 2007, Niels Martin Hansen
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
// Contact: mailto:jiifurusu@gmail.com
//


#include "dialog_dummy_video.h"
#include <wx/datetime.h>
#include <wx/sizer.h>
#include <wx/log.h>
#include "options.h"
#include "help_button.h"


struct ResolutionShortcut {
	const wxChar *name;
	int width;
	int height;
};
static ResolutionShortcut resolutions[] = {
	{_T("640x480 (SD fullscreen)"), 640, 480},
	{_T("704x480 (SD anamorphic)"), 704, 480},
	{_T("640x360 (SD widescreen)"), 640, 360},
	{_T("704x396 (SD widescreen)"), 704, 396},
	{_T("640x352 (SD widescreen MOD16)"), 640, 352},
	{_T("704x400 (SD widescreen MOD16)"), 704, 400},
	{_T("1280x720 (HD 720p)"), 1280, 720},
	{_T("1920x1080 (HD 1080p)"), 1920, 1080},
	{_T("1024x576 (SuperPAL widescreen)"), 1024, 576},
	{0, 0, 0}
};


bool DialogDummyVideo::CreateDummyVideo(wxWindow *parent, wxString &out_filename)
{
	DialogDummyVideo dlg(parent);
	if (dlg.ShowModal() == wxID_OK) {
		double fps;
		long width, height, length;
		wxColour colour;
		bool pattern;

		// Read back values and check sensibility
		if (!dlg.fps->GetValue().ToDouble(&fps) || fps <= 0) {
			wxLogWarning(_T("Invalid framerate specified, assuming 23.976"));
			fps = 24/1.001;
		}
		if (!dlg.width->GetValue().ToLong(&width) || width <= 0) {
			wxLogWarning(_T("Invalid width specified"));
			width = 0;
		}
		if (!dlg.height->GetValue().ToLong(&height) || height <= 0) {
			wxLogWarning(_T("Invalid height specified"));
			height = 0;
		}
		if (width == 0 && height == 0) {
			wxLogWarning(_T("Assuming 640x480"));
			width = 640; height = 480;
		} else if (width == 0) {
			width = height * 4 / 3;
			wxLogWarning(_T("Assuming 4:3 fullscreen, %dx%d"), width, height);
		} else if (height == 0) {
			height = width * 3 / 4;
			wxLogWarning(_T("Assuming 4:3 fullscreen, %dx%d"), width, height);
		}
		if ((length = dlg.length->GetValue()) <= 0) {
			wxLogWarning(_T("Invalid duration, assuming 2 frames"));
			length = 2;
		}
		colour = dlg.colour->GetColour();
		pattern = dlg.pattern->GetValue();

		// Write to options
		Options.SetFloat(_T("Video Dummy Last FPS"), fps);
		Options.SetInt(_T("Video Dummy Last Width"), width);
		Options.SetInt(_T("Video Dummy Last Height"), height);
		Options.SetInt(_T("Video Dummy Last Length"), length);
		Options.SetColour(_T("Video Dummy Last Colour"), colour);
		Options.SetBool(_T("Video Dummy Pattern"), pattern);

		out_filename = DummyVideoProvider::MakeFilename(fps, length, width, height, colour, pattern);
		return true;
	} else {
		return false;
	}
}


DialogDummyVideo::DialogDummyVideo(wxWindow *parent)
: wxDialog(parent, -1, _("Dummy video options"),wxDefaultPosition,wxDefaultSize)
{
	// Main controls
	length_display = 0;
	resolution_shortcuts = new wxComboBox(this, Dummy_Video_Resolution_Shortcut, _T(""), wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_READONLY);
	width = new wxTextCtrl(this, -1);
	height = new wxTextCtrl(this, -1);
	colour = new ColourButton(this, -1, wxSize(30, 17), Options.AsColour(_T("Video Dummy Last Colour")));
	pattern = new wxCheckBox(this, -1, _("Checkerboard pattern"));
	//fps = new wxComboBox(this, Dummy_Video_FPS, Options.AsText(_T("Video Dummy Last FPS")), wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_DROPDOWN);
	fps = new wxTextCtrl(this, Dummy_Video_FPS, Options.AsText(_T("Video Dummy Last FPS")));
	length = new wxSpinCtrl(this, Dummy_Video_Length);
	length_display = new wxStaticText(this, -1, _T(""));

	// Support controls and layout
	wxFlexGridSizer *fg = new wxFlexGridSizer(2, 5, 5);
	fg->Add(new wxStaticText(this, -1, _("Video resolution:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(resolution_shortcuts, 0, wxEXPAND);
	fg->AddStretchSpacer();
	wxBoxSizer *res_sizer = new wxBoxSizer(wxHORIZONTAL);
	res_sizer->Add(width, 0, wxEXPAND);
	res_sizer->Add(new wxStaticText(this, -1, _T(" x ")), 0, wxALIGN_CENTRE_VERTICAL|wxFIXED_MINSIZE);
	res_sizer->Add(height, 0, wxEXPAND);
	fg->Add(res_sizer, 0, wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Colour:")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(colour, 0, wxFIXED_MINSIZE|wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
	fg->AddStretchSpacer();
	fg->Add(pattern, 0, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
	fg->Add(new wxStaticText(this, -1, _("Frame rate (fps):")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(fps, 0, wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Duration (frames):")), 0, wxALIGN_CENTRE_VERTICAL);
	fg->Add(length, 0, wxEXPAND);
	fg->AddStretchSpacer();
	fg->Add(length_display, 0, wxEXPAND|wxALIGN_CENTRE_VERTICAL|wxALIGN_LEFT);
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(fg, 1, wxALL|wxEXPAND, 5);

	ok_button = new wxButton(this,wxID_OK);
	cancel_button = new wxButton(this,wxID_CANCEL);
	wxStdDialogButtonSizer *btnSizer = new wxStdDialogButtonSizer();
	btnSizer->AddButton(ok_button);
	btnSizer->AddButton(cancel_button);
	btnSizer->AddButton(new HelpButton(this,_T("Dummy Video")));
	btnSizer->Realize();
	main_sizer->Add(new wxStaticLine(this,wxHORIZONTAL),0,wxALL|wxEXPAND,5);
	main_sizer->Add(btnSizer,0,wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND,5);
//	main_sizer->Add(CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND, 5);
//	ok_button = static_cast<wxButton*>(FindWindow(wxID_OK));
//	cancel_button = static_cast<wxButton*>(FindWindow(wxID_CANCEL));

	// Initialise controls
	int lastwidth, lastheight, lastres = 0;
	lastwidth = Options.AsInt(_T("Video Dummy Last Width"));
	lastheight = Options.AsInt(_T("Video Dummy Last Height"));
	for (ResolutionShortcut *res = resolutions; res->name; ++res) {
		resolution_shortcuts->Append(res->name);
		if (res->width == lastwidth && res->height == lastheight)
			resolution_shortcuts->SetSelection(lastres);
		lastres++;
	}
	pattern->SetValue(Options.AsBool(_T("Video Dummy Pattern")));
	/*fps->Append(_T("23.976"));
	fps->Append(_T("29.97"));
	fps->Append(_T("24"));
	fps->Append(_T("25"));
	fps->Append(_T("30"));*/
	width->ChangeValue(Options.AsText(_T("Video Dummy Last Width")));
	height->ChangeValue(Options.AsText(_T("Video Dummy Last Height")));
	length->SetRange(0, 0x10000000);
	length->SetValue(Options.AsInt(_T("Video Dummy Last Length")));
	UpdateLengthDisplay();

	// Layout
	main_sizer->SetSizeHints(this);
	SetSizer(main_sizer);
	CenterOnParent();
}


DialogDummyVideo::~DialogDummyVideo()
{
}


BEGIN_EVENT_TABLE(DialogDummyVideo,wxDialog)
	EVT_COMBOBOX(Dummy_Video_Resolution_Shortcut, DialogDummyVideo::OnResolutionShortcut)
	EVT_TEXT(Dummy_Video_FPS, DialogDummyVideo::OnFpsChange)
	EVT_SPINCTRL(Dummy_Video_Length, DialogDummyVideo::OnLengthSpin)
	EVT_TEXT(Dummy_Video_Length, DialogDummyVideo::OnLengthChange)
END_EVENT_TABLE()


void DialogDummyVideo::OnResolutionShortcut(wxCommandEvent &evt)
{
	int rs = resolution_shortcuts->GetSelection();
	width->ChangeValue(wxString::Format(_T("%d"), resolutions[rs].width));
	height->ChangeValue(wxString::Format(_T("%d"), resolutions[rs].height));
}


void DialogDummyVideo::OnFpsChange(wxCommandEvent &evt)
{
	UpdateLengthDisplay();
}


void DialogDummyVideo::OnLengthSpin(wxSpinEvent &evt)
{
	UpdateLengthDisplay();
}


void DialogDummyVideo::OnLengthChange(wxCommandEvent &evt)
{
	UpdateLengthDisplay();
}


void DialogDummyVideo::UpdateLengthDisplay()
{
	double fpsval;
	int lengthval = 0;
	if (!length_display) return;
	bool valid = false;
	if (fps->GetValue().ToDouble(&fpsval)) {
		lengthval = length->GetValue();
		if (lengthval && fpsval > 0 && lengthval > 0) {
			valid = true;
			int tt = int(lengthval / fpsval * 1000); // frames / (frames/seconds) * 1000 = milliseconds
			// 32 bit signed int can hold almost 600 positive hours when counting milliseconds, ASS allows at most just below 10 hours, so we're safe
			int ms, s, m, h;
			ms = tt % 1000; tt /= 1000;
			s = tt % 60; tt /= 60;
			m = tt % 60; tt /= 60;
			h = tt;
			length_display->SetLabel(wxString::Format(_("Resulting duration: %d:%02d:%02d.%03d"), h, m, s, ms));
			ok_button->Enable();
		}
	}
	
	if (!valid) {
		length_display->SetLabel(_("Invalid fps or length value"));
		ok_button->Disable();
	}
}
