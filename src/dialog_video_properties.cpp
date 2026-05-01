// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "ass_file.h"
#include "async_video_provider.h"
#include "format.h"
#include "help_button.h"
#include "options.h"
#include "resolution_resampler.h"

#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace {
enum class ResolutionMismatchAction {
	Ignore,
	Prompt,
	Resample,
};

enum class ResolutionMismatchFix {
	Ignore,
	Resample,
};


bool update_ycbcr_matrix(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *) {
	// When opening dummy video only want to set the script properties if
	// they were previously unset
	if (!new_provider->ShouldSetVideoProperties()) {
		return false;
	}

	auto VideoCS = new_provider->GetRealColorSpace();
	auto [guessCM, guessCR] = VideoCS;
	agi::ycbcr::guess_colorspace(guessCM, guessCR, new_provider->GetWidth(), new_provider->GetHeight());

	if (VideoCS.matrix == agi::ycbcr_matrix::Unspecified && OPT_GET("Video/Untagged Matrix Warning")->GetBool()) {
		wxString title = _("Untagged video");

		// Warn on an untagged matrix but not on an untagged range:
		// No sane player will ever guess a full range, so an untagged range is not really an issue in practice.
		wxMessageBox(
#ifdef __WXMSW__
			// On Windows, wxMessageBoxes containing new lines highlight the first line as a title.
			title + "\n" +
#endif
			fmt_tl(
			"The video you have loaded has no specified color matrix. "
			"Aegisub will guess the color matrix to be %s, but there is no guarantee that other programs will guess the same matrix. "
			"This may make the video appear with different colors in different media players and can prevent subtitle colors from matching video colors."
			"\n\n"
			"Consider tagging your video with a color matrix to ensure that your video displays consistently in all players and that subtitle colors can reliably match video colors."
			, agi::ycbcr::matrix_to_string(guessCM)), title, wxICON_WARNING | wxOK | wxCENTER);
	}

	if (new_provider->IsHDRorWCG() && OPT_GET("Video/HDR Video Warning")->GetBool()) {
		wxMessageBox(_(
			"The video you have loaded has HDR and/or WCG colors. "
			"While ordinary dialogue subtitles will work fine on such videos, "
			"matching video colors using subtitles will not be possible on HDR and/or WCG footage."
			), _("HDR and/or WCG video"), wxICON_WARNING | wxOK | wxCENTER);
	}

	agi::ycbcr::Header video_matrix(new_provider->GetColorSpace());
	if (video_matrix != file->GetYCbCrMatrix()) {
		file->SetScriptInfo("YCbCr Matrix", video_matrix.to_existing_string());
		return true;
	}

	return false;
}

ResolutionMismatchFix prompt_play_res(wxWindow *parent, bool ar_changed, int sx, int sy, int vx, int vy) {
	wxDialog d(parent, -1, _("Resolution mismatch"));

	auto label_text = fmt_tl(
        "The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\n"
        "Video resolution:\t%d \u00D7 %d\n"     // U+00D7 multiplication sign
        "Script resolution:\t%d \u00D7 %d\n\n"
        "Change subtitles resolution to match video?", vx, vy, sx, sy);

	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxStaticText(&d, -1, label_text), wxSizerFlags().Border());

	wxRadioBox *rb;
	if (ar_changed) {
		wxString choices[] = {
			_("Leave resolution as it is"),
			_("Resample script (stretch to new aspect ratio)"),
			_("Resample script (add borders)"),
			_("Resample script (remove borders)")
		};
		rb = new wxRadioBox(&d, -1, "", wxDefaultPosition, wxDefaultSize, 4, choices, 1);
	}
	else {
		wxString choices[] = {
			_("Leave resolution as it is"),
			_("Resample script"),
		};
		rb = new wxRadioBox(&d, -1, "", wxDefaultPosition, wxDefaultSize, 2, choices, 1);
	}
	sizer->Add(rb, wxSizerFlags().Border(wxALL & ~wxTOP).Expand());
	sizer->Add(d.CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP), wxSizerFlags().Border().Expand());

	unsigned int sel = OPT_GET("Video/Last PlayRes Mismatch Choice")->GetInt();
	rb->SetSelection(std::min(sel, rb->GetCount()));

	d.SetSizerAndFit(sizer);
	d.CenterOnParent();

	d.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { d.EndModal(rb->GetSelection()); }, wxID_OK);
	d.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { d.EndModal(0); }, wxID_CANCEL);
	d.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { HelpButton::OpenPage("Resolution mismatch"); }, wxID_HELP);

	return static_cast<ResolutionMismatchFix>(d.ShowModal());
}

bool update_play_res(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	// Check that the script resolution matches the video resolution
	int sx = file->GetScriptInfoAsInt("PlayResX");
	int sy = file->GetScriptInfoAsInt("PlayResY");
	int vx = new_provider->GetWidth();
	int vy = new_provider->GetHeight();

	// If the script resolution hasn't been set at all just force it to the
	// video resolution
	if (sx == 0 && sy == 0) {
		file->SetScriptInfo("PlayResX", std::to_string(vx));
		file->SetScriptInfo("PlayResY", std::to_string(vy));
		return true;
	}

	// When opening dummy video only want to set the script properties if
	// they were previously unset
	if (!new_provider->ShouldSetVideoProperties())
		return false;

	// Treat exact multiples of the video resolution as equaling the resolution
	// for the people who use that for subpixel precision (which is mostly
	// pointless these days due to decimals being supported almost everywhere)
	if (sx % vx == 0 && sy % vy == 0)
		return false;

	auto sar = double(sx) / sy;
	auto var = double(vx) / vy;
	bool ar_changed = std::abs(sar - var) / var > .01;

	switch (static_cast<ResolutionMismatchAction>(OPT_GET("Video/PlayRes Mismatch")->GetInt())) {
	case ResolutionMismatchAction::Ignore: default:
		return false;

	case ResolutionMismatchAction::Resample:
		// Fallthrough to prompt if the AR changed
		if (!ar_changed) {
			ResampleResolution(file, {
				{0, 0, 0, 0},
				sx, sy, vx, vy,
				ResampleARMode::Stretch,
				std::nullopt,
			});
			return true;
		}
		[[fallthrough]];

	case ResolutionMismatchAction::Prompt:
		auto res = prompt_play_res(parent, ar_changed, sx, sy, vx, vy);
		if (res == ResolutionMismatchFix::Ignore) return false;
		OPT_SET("Video/Last PlayRes Mismatch Choice")->SetInt(static_cast<int>(res));

		ResampleResolution(file, {
			{0, 0, 0, 0},
			sx, sy, vx, vy,
			static_cast<ResampleARMode>(static_cast<int>(res) - static_cast<int>(ResolutionMismatchFix::Resample)),
			std::nullopt,
		});
		return true;
	}
}
}

void UpdateVideoProperties(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	if (update_ycbcr_matrix(file, new_provider, parent))
		file->Commit(_("change ycbcr matrix"), AssFile::COMMIT_SCRIPTINFO);

	if (update_play_res(file, new_provider, parent))
		file->Commit(_("change script resolution"), AssFile::COMMIT_SCRIPTINFO);
}
