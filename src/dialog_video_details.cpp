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

#include "ass_time.h"
#include "async_video_provider.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "project.h"

#include <boost/rational.hpp>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
/// @class DialogVideoDetails
/// @brief Display information about the video in a dialog
struct DialogVideoDetails final : wxDialog {
	DialogVideoDetails(agi::Context *c);
};

DialogVideoDetails::DialogVideoDetails(agi::Context *c)
: wxDialog(c->parent , -1, _("Video Details"))
{
	auto provider = c->project->VideoProvider();
	auto width = provider->GetWidth();
	auto height = provider->GetHeight();
	auto framecount = provider->GetFrameCount();
	auto fps = provider->GetFPS();
	boost::rational<int> ar(width, height);

	auto fg = new wxFlexGridSizer(2, 5, 10);
	auto make_field = [&](wxString const& name, wxString const& value) {
		fg->Add(new wxStaticText(this, -1, name), 0, wxALIGN_CENTRE_VERTICAL);
		fg->Add(new wxTextCtrl(this, -1, value, wxDefaultPosition, wxSize(300,-1), wxTE_READONLY), 0, wxALIGN_CENTRE_VERTICAL | wxEXPAND);
	};
	make_field(_("File name:"), c->project->VideoName().wstring());
	make_field(_("FPS:"), wxString::Format("%.3f", fps.FPS()));
	make_field(_("Resolution:"), wxString::Format("%dx%d (%d:%d)", width, height, ar.numerator(), ar.denominator()));
	make_field(_("Length:"), wxString::Format(_("%d frames (%s)"), framecount, to_wx(AssTime(fps.TimeAtFrame(framecount - 1)).GetAssFormated(true))));
	make_field(_("Decoder:"), to_wx(provider->GetDecoderName()));

	wxStaticBoxSizer *video_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Video"));
	video_sizer->Add(fg);

	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(video_sizer, 1, wxALL|wxEXPAND, 5);
	main_sizer->Add(CreateSeparatedButtonSizer(wxOK), 0, wxALL|wxEXPAND, 5);
	SetSizerAndFit(main_sizer);

	CenterOnParent();
}
}

void ShowVideoDetailsDialog(agi::Context *c) {
	DialogVideoDetails(c).ShowModal();
}
