// Copyright (c) 2007, Alysson Souza e Silva (demi_alucard)
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
#include "dialog_video_details.h"
#include "video_context.h"
#include "video_provider_manager.h"
#include "audio_provider_manager.h"
#include "audio_box.h"
#include "utils.h"


///////////////
// Constructor
DialogVideoDetails::DialogVideoDetails(wxWindow *parent)
: wxDialog(parent , -1, _("Video Details"),wxDefaultPosition,wxDefaultSize)
{
	// Main controls
	wxFlexGridSizer *fg = new wxFlexGridSizer(2, 5, 10);
	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *video_sizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Video"));
	VideoProvider *vprovider = VideoContext::Get()->GetProvider();

	int width = vprovider->GetWidth();
	int height = vprovider->GetHeight();
	int framecount = vprovider->GetFrameCount();
	double fps = vprovider->GetFPS();

	wxTextCtrl *fname_text = new wxTextCtrl(this, -1, VideoContext::Get()->videoName, wxDefaultPosition, wxSize(300,-1), wxTE_READONLY);
	wxTextCtrl *fps_text = new wxTextCtrl(this, -1, wxString::Format(_T("%.3f"), fps), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	wxTextCtrl *resolution_text = new wxTextCtrl(this, -1, wxString::Format(_T("%dx%d (%s)"), width, height, PrettyAR(width, height).c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	wxTextCtrl *length_text = new wxTextCtrl(this, -1, wxString::Format(_T("%d frames (%s)"), framecount, PrettyTimeStamp(framecount, fps).c_str()), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	wxTextCtrl *decoder_text = new wxTextCtrl(this, -1, vprovider->GetDecoderName().c_str(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

	fg->Add(new wxStaticText(this, -1, _("File name:")), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(fname_text, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("FPS:")), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(fps_text, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Resolution:")), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(resolution_text, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Length:")), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(length_text, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(new wxStaticText(this, -1, _("Decoder:")), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	fg->Add(decoder_text, 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);

	video_sizer->Add(fg);

	main_sizer->Add(video_sizer, 1, wxALL|wxEXPAND, 5);
	main_sizer->Add(CreateSeparatedButtonSizer(wxOK), 0, wxALL|wxEXPAND, 5);
	main_sizer->SetSizeHints(this);
	SetSizer(main_sizer);

	CenterOnParent();
}


////////////
// PrettyAR
wxString DialogVideoDetails::PrettyAR(int width, int height)
{
	int limit = (int)ceil(sqrt(double(MIN(width, height))));
	for (int i=2;i<=limit;i++) {
		while (width % i == 0 && height % i == 0) {
			width /= i;
			height /= i;
		}
	}
	return wxString::Format(_T("%d:%d"), width, height);
}


///////////////////
// PrettyTimeStamp
wxString DialogVideoDetails::PrettyTimeStamp(int frames, double fps)
{
	int tt = int(frames / fps * 1000);
	int cs = tt % 1000; tt /= 1000;
	int s = tt % 60; tt /= 60;
	int m = tt % 60; tt /= 60;
	int h = tt;
	return wxString::Format(_T("%d:%02d:%02d.%03d"), h, m, s, cs);
}
