// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file subs_preview.cpp
/// @brief Preview control using a dummy video provider and subtitles provider to render a preview
/// @ingroup custom_control
///

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "dialog_progress.h"
#include "subs_preview.h"
#include "include/aegisub/subtitles_provider.h"
#include "video_frame.h"
#include "video_provider_dummy.h"

#include <libaegisub/make_unique.h>

#include <wx/dcclient.h>
#include <wx/msgdlg.h>

SubtitlesPreview::SubtitlesPreview(wxWindow *parent, wxSize size, int winStyle, agi::Color col)
: wxWindow(parent, -1, wxDefaultPosition, size, winStyle)
, style(new AssStyle)
, back_color(col)
, sub_file(agi::make_unique<AssFile>())
, line(new AssDialogue)
{
	line->Text = "{\\q2}preview";

	SetStyle(*style);

	sub_file->LoadDefault();
	sub_file->Styles.push_back(*style);
	sub_file->Events.push_back(*line);

	SetSizeHints(size.GetWidth(), size.GetHeight(), -1, -1);
	wxSizeEvent evt(size);
	OnSize(evt);
	UpdateBitmap();

	Bind(wxEVT_PAINT, &SubtitlesPreview::OnPaint, this);
	Bind(wxEVT_SIZE, &SubtitlesPreview::OnSize, this);
}

SubtitlesPreview::~SubtitlesPreview() {
}

void SubtitlesPreview::SetStyle(AssStyle const& new_style) {
	*style = new_style;
	style->name = "Default";
	style->alignment = 5;
	std::fill(style->Margin.begin(), style->Margin.end(), 0);
	style->UpdateData();
	UpdateBitmap();
}

void SubtitlesPreview::SetText(std::string const& text) {
	std::string new_text = "{\\q2}" + text;
	if (new_text != line->Text) {
		line->Text = new_text;
		UpdateBitmap();
	}
}

void SubtitlesPreview::SetColour(agi::Color col) {
	if (col != back_color) {
		back_color = col;
		vid = agi::make_unique<DummyVideoProvider>(0.0, 10, bmp->GetWidth(), bmp->GetHeight(), back_color, true);
		UpdateBitmap();
	}
}

void SubtitlesPreview::UpdateBitmap() {
	if (!vid) return;

	VideoFrame frame;
	vid->GetFrame(0, frame);

	if (provider) {
		try {
			provider->LoadSubtitles(sub_file.get());
			provider->DrawSubtitles(frame, 0.1);
		}
		catch (...) { }
	}

	// Convert frame to bitmap
	*bmp = static_cast<wxBitmap>(GetImage(frame));
	Refresh();
}

void SubtitlesPreview::OnPaint(wxPaintEvent &) {
	wxPaintDC(this).DrawBitmap(*bmp, 0, 0);
}

void SubtitlesPreview::OnSize(wxSizeEvent &evt) {
	if (bmp.get() && evt.GetSize() == bmp->GetSize()) return;

	int w = evt.GetSize().GetWidth();
	int h = evt.GetSize().GetHeight();

	bmp = agi::make_unique<wxBitmap>(w, h, -1);
	vid = agi::make_unique<DummyVideoProvider>(0.0, 10, w, h, back_color, true);
	try {
		if (!progress)
			progress = agi::make_unique<DialogProgress>(this);
		if (!provider)
			provider = SubtitlesProviderFactory::GetProvider(progress.get());
	}
	catch (...) {
		wxMessageBox(
			"Could not get any subtitles provider for the preview box. Make "
			"sure that you have a provider installed.",
			"No subtitles provider", wxOK | wxICON_ERROR | wxCENTER);
	}

	sub_file->SetScriptInfo("PlayResX", std::to_string(w));
	sub_file->SetScriptInfo("PlayResY", std::to_string(h));

	UpdateBitmap();
}
