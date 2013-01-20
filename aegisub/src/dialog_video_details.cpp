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
// Aegisub Project http://www.aegisub.org/

/// @file dialog_video_details.cpp
/// @brief Video Details dialogue box
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_video_details.h"

#include "ass_time.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "utils.h"
#include "video_context.h"
#include "video_provider_manager.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

static void make_field(wxWindow *parent, wxSizer *sizer, wxString const& name, wxString const& value) {
	sizer->Add(new wxStaticText(parent, -1, name), 0, wxALIGN_CENTRE_VERTICAL);
	sizer->Add(new wxTextCtrl(parent, -1, value, wxDefaultPosition, wxSize(300,-1), wxTE_READONLY), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
}

static wxString pretty_ar(int width, int height) {
	int limit = (int)ceil(sqrt(double(std::min(width, height))));
	for (int i=2;i<=limit;i++) {
		while (width % i == 0 && height % i == 0) {
			width /= i;
			height /= i;
		}
	}
	return wxString::Format("%d:%d", width, height);
}

DialogVideoDetails::DialogVideoDetails(agi::Context *c)
: wxDialog(c->parent , -1, _("Video Details"))
{
	auto width = c->videoController->GetWidth();
	auto height = c->videoController->GetHeight();
	auto framecount = c->videoController->GetLength();
	auto fps = c->videoController->FPS();

	wxFlexGridSizer *fg = new wxFlexGridSizer(2, 5, 10);
	make_field(this, fg, _("File name:"), c->videoController->GetVideoName().wstring());
	make_field(this, fg, _("FPS:"), wxString::Format("%.3f", fps.FPS()));
	make_field(this, fg, _("Resolution:"), wxString::Format("%dx%d (%s)", width, height, pretty_ar(width, height)));
	make_field(this, fg, _("Length:"), wxString::Format(_("%d frames (%s)"), framecount, to_wx(AssTime(fps.TimeAtFrame(framecount - 1)).GetAssFormated(true))));
	make_field(this, fg, _("Decoder:"), to_wx(c->videoController->GetProvider()->GetDecoderName()));

	wxStaticBoxSizer *video_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Video"));
	video_sizer->Add(fg);

	wxBoxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(video_sizer, 1, wxALL|wxEXPAND, 5);
	main_sizer->Add(CreateSeparatedButtonSizer(wxOK), 0, wxALL|wxEXPAND, 5);
	SetSizerAndFit(main_sizer);

	CenterOnParent();
}
