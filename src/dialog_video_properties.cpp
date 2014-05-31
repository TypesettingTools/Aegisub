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
#include "options.h"
#include "resolution_resampler.h"

#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace {
enum {
	MISMATCH_IGNORE,
	MISMATCH_PROMPT,
	MISMATCH_RESAMPLE,
	MISMATCH_SET
};
enum {
	FIX_IGNORE,
	FIX_SET,
	FIX_RESAMPLE
};

int prompt(wxWindow *parent, bool ar_changed, int sx, int sy, int vx, int vy) {
	wxDialog d(parent, -1, _("Resolution mismatch"));

	auto label_text = fmt_tl("The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\nVideo resolution:\t%d x %d\nScript resolution:\t%d x %d\n\nChange subtitles resolution to match video?", vx, vy, sx, sy);

	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(new wxStaticText(&d, -1, label_text), wxSizerFlags().Border());

	wxRadioBox *rb;
	if (ar_changed) {
		wxString choices[] = {
			_("Set to video resolution"),
			_("Resample script (stretch to new aspect ratio)"),
			_("Resample script (add borders)"),
			_("Resample script (remove borders)")
		};
		rb = new wxRadioBox(&d, -1, "", wxDefaultPosition, wxDefaultSize, 4, choices, 1);
	}
	else {
		wxString choices[] = {
			_("Set to video resolution"),
			_("Resample script"),
		};
		rb = new wxRadioBox(&d, -1, "", wxDefaultPosition, wxDefaultSize, 2, choices, 1);
	}
	sizer->Add(rb, wxSizerFlags().Border(wxALL & ~wxTOP).Expand());
	sizer->Add(d.CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP), wxSizerFlags().Border().Expand());

	unsigned int sel = OPT_GET("Video/Last Script Resolution Mismatch Choice")->GetInt();
	rb->SetSelection(std::min(sel - 1, rb->GetCount()));

	d.SetSizerAndFit(sizer);
	d.CenterOnParent();

	d.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { d.EndModal(rb->GetSelection() + 1); }, wxID_OK);
	d.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { d.EndModal(0); }, wxID_CANCEL);

	return d.ShowModal();
}

bool update_video_properties(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	bool commit_subs = false;

	// When opening dummy video only want to set the script properties if
	// they were previously unset
	bool set_properties = new_provider->ShouldSetVideoProperties();

	auto matrix = new_provider->GetColorSpace();
	if (set_properties && matrix != file->GetScriptInfo("YCbCr Matrix")) {
		file->SetScriptInfo("YCbCr Matrix", matrix);
		commit_subs = true;
	}

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

	if (!set_properties)
		return false;

	// Treat exact multiples of the video resolution as equaling the resolution
	// for the people who use that for subpixel precision (which is mostly
	// pointless these days due to decimals being supported almost everywhere)
	if (sx % vx == 0 && sy % vy == 0)
		return commit_subs;

	auto sar = double(sx) / sy;
	auto var = double(vx) / vy;
	bool ar_changed = abs(sar - var) / var > .01;

	switch (OPT_GET("Video/Script Resolution Mismatch")->GetInt()) {
	case MISMATCH_IGNORE: default:
		return commit_subs;

	case MISMATCH_SET:
		file->SetScriptInfo("PlayResX", std::to_string(vx));
		file->SetScriptInfo("PlayResY", std::to_string(vy));
		return true;

	case MISMATCH_RESAMPLE:
		// Fallthrough to prompt if the AR changed
		if (!ar_changed) {
			ResampleResolution(file, {
				{0, 0, 0, 0},
				sx, sy, vx, vy,
				ResampleARMode::Stretch,
				YCbCrMatrix::rgb, YCbCrMatrix::rgb
			});
			return true;
		}

	case MISMATCH_PROMPT:
		int res = prompt(parent, ar_changed, sx, sy, vx, vy);
		if (res == FIX_IGNORE) return commit_subs;
		OPT_SET("Video/Last Script Resolution Mismatch Choice")->SetInt(res);

		ResampleResolution(file, {
			{0, 0, 0, 0},
			sx, sy, vx, vy,
			static_cast<ResampleARMode>(res - FIX_RESAMPLE),
			YCbCrMatrix::rgb, YCbCrMatrix::rgb
		});
		return true;
	}
}
}

void UpdateVideoProperties(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	if (update_video_properties(file, new_provider, parent))
		file->Commit(_("change script resolution"), AssFile::COMMIT_SCRIPTINFO);
}
