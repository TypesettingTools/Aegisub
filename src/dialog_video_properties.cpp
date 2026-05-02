// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
// Copyright (c) 2026, arch1t3cht
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
#include "include/aegisub/context.h"
#include "format.h"
#include "help_button.h"
#include "options.h"
#include "project.h"
#include "resolution_resampler.h"

#include <wx/choicdlg.h>
#include <wx/dialog.h>
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace ycbcr = agi::ycbcr;

namespace {

// This may look a bit silly with there being five copies of almost the same enum,
// (as well as very copies of very similar-looking code below), but the different
// mismatch cases are intentionally kept separate here so it's easier to reason about them
// and to adjust individual cases separately.

enum class ResolutionMismatchAction {
	Ignore,
	Prompt,
	Resample,
};

enum class ResolutionMismatchFix {
	Ignore,
	Resample,
};


enum class MissingLayoutResAction {
	Ignore,
	Prompt,
	Set,
	SetToPlayRes,
};

enum class MissingLayoutResFix {
	Ignore,
	Set,
	SetToPlayRes,
};


enum class MissingYCbCrMatrixAction {
	Ignore,
	Prompt,
	Set,
};

enum class MissingYCbCrMatrixFix {
	Ignore,
	Set,
};


enum class LayoutResMismatchPromptCondition {
	Never,
	OnARMismatch,
	Always,
};

enum class LayoutResMismatchFix {
	Ignore,
	Set,
};


enum class YCbCrMatrixMismatchAction {
	Ignore,
	Prompt,
	Set,
};

enum class YCbCrMatrixMismatchFix {
	Ignore,
	Set,
};


bool check_ar_changed(int sx, int sy, int vx, int vy) {
	auto sar = double(sx) / sy;
	auto var = double(vx) / vy;
	return std::abs(sar - var) / var > .01;
}


bool update_ycbcr_matrix(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	// When opening dummy video only want to set the script properties if
	// they were previously unset
	if (!new_provider->ShouldSetVideoProperties()) {
		return false;
	}

	auto VideoCS = new_provider->GetRealColorSpace();
	auto [guessCM, guessCR] = VideoCS;
	ycbcr::guess_colorspace(guessCM, guessCR, new_provider->GetWidth(), new_provider->GetHeight());

	if (VideoCS.matrix == agi::ycbcr_matrix::Unspecified && OPT_GET("Video/Untagged Matrix Warning")->GetBool()) {
		// Warn on an untagged matrix but not on an untagged range:
		// No sane player will ever guess a full range, so an untagged range is not really an issue in practice.

		wxMessageBox(
			fmt_tl(
			/* TRANSLATORS: Keep the space between the two line breaks; it's required for the message
			                to display correctly on Windows. */
			"The video you have loaded has no specified color matrix. "
			"Aegisub will guess the color matrix to be %s, but there is no guarantee that other programs will guess the same matrix. "
			"This may make the video appear with different colors in different media players and can prevent subtitle colors from matching video colors."
			"\n \n"
			"Consider tagging your video with a color matrix to ensure that your video displays consistently in all players and that subtitle colors can reliably match video colors."
			, ycbcr::matrix_to_string(guessCM)), _("Untagged video"), wxICON_WARNING | wxOK | wxCENTER);
	}

	if (new_provider->IsHDRorWCG() && OPT_GET("Video/HDR Video Warning")->GetBool()) {
		wxMessageBox(_(
			"The video you have loaded has HDR and/or WCG colors. "
			"While ordinary dialogue subtitles will work fine on such videos, "
			"matching video colors using subtitles will not be possible on HDR and/or WCG footage."
			), _("HDR and/or WCG video"), wxICON_WARNING | wxOK | wxCENTER);
	}

	auto script_matrix = file->GetYCbCrMatrix();
	auto recommended_matrix = ycbcr::Header(guessCM, guessCR).to_best_practice();

	if (std::holds_alternative<ycbcr::header_missing>(script_matrix) || std::holds_alternative<ycbcr::header_invalid>(script_matrix)) {
		// Script has no proper matrix set yet, so find out if the user wants to set one
		auto action = static_cast<MissingYCbCrMatrixAction>(OPT_GET("Video/No YCbCr Matrix in Script")->GetInt());

		if (action == MissingYCbCrMatrixAction::Prompt) {
			int Choice = wxGetSingleChoiceIndex(
				fmt_tl(
					"The current subtitle file has no YCbCr Matrix set.\n\n"
					"If you plan to use this subtitle file on this newly loaded video file,\nyou should set the subtitle file's YCbCr Matrix to the video's color space (\"%s\").\n\n"
					"If you plan to use a different video file, you may want to\nleave the YCbCr Matrix unset until you load the correct video file.\n\n"
					"This prompt can be configured in the preferences.",
				recommended_matrix.to_string().value()),
				_("Set script's YCbCr Matrix?"),
				{_("Leave YCbCr Matrix unset"), fmt_tl("Set script's YCbCr Matrix to the video's (\"%s\")", recommended_matrix.to_string().value())},
				OPT_GET("Video/Last No YCbCr Matrix in Script Choice")->GetInt(),
				parent
			);

			if (Choice == -1) {
				Choice = static_cast<int>(MissingYCbCrMatrixFix::Ignore);
			} else {
				OPT_SET("Video/Last No YCbCr Matrix in Script Choice")->SetInt(Choice);
			}

			switch (static_cast<MissingYCbCrMatrixFix>(Choice)) {
				case MissingYCbCrMatrixFix::Ignore:
					action = MissingYCbCrMatrixAction::Ignore;
					break;
				case MissingYCbCrMatrixFix::Set:
					action = MissingYCbCrMatrixAction::Set;
					break;
			}
		}

		if (action == MissingYCbCrMatrixAction::Set) {
			file->SetScriptInfo("YCbCr Matrix", recommended_matrix.to_string().value());
			return true;
		}
	} else if (std::holds_alternative<ycbcr::header_colorspace>(script_matrix) && script_matrix != recommended_matrix) {
		// If the script's YCbCr Matrix is None, we assume that the user opts out of all color mangling
		// and do not attempt (or ask) to change the YCbCr Matrix further.
		//
		// If the script's YCbCr Matrix is some actual color space, see if the video's color space is different from it.
		auto action = static_cast<YCbCrMatrixMismatchAction>(OPT_GET("Video/No YCbCr Matrix in Script")->GetInt());

		// This may be the most complicated case of all the video property mismatches.
		// There are the following relevant cases:
		// 1. The opened video is not a video on which the subtitles are meant to be played back. (Say, it's some test video or the user selected the wrong video by mistake.)
		//	  This is the easy case, there the user can just leave the script unchanged.
		// 2. The script was originally authored on (say) a BT.601 encode of some RGB video, and the user is now loading a (say) BT.709 encode of the same RGB video.
		//	  In this case, the RGB colors in the subtitle file remain accurate to the new video (decoded with its tagged matrix), so the correct action is to set the new YCbCr Matrix header without changing colors.
		// 3. The video the script was initially authored for was actually (say) BT.601 but mistagged as (say) BT.709 (or untagged and wrongly guessed),
		//	  and the user is now loading the same YCbCr stream, but this time correctly tagged as BT.601.
		//    In this case, the RGB colors in the subtitle file are *not* accurate to the video, so the correct action is to either leave the YCbCr Matrix unchanged or resample the colors to the new YCbCr Matrix.
		//
		//    We imagine that this case is the rarer one. (At least when exclusively working with new files, where Aegisub will also no longer force BT.601 and will warn on untagged videos.
		//    Old, mistagged subtitle files are a different story.)
		//
		// (In all of these cases the base assumption is that the script looked correct on whatever video it was originally authored on.
		// If this is not the case, all bets are off and the user will need to manually fix the matrix in the script properties dialog.)

		// FIXME: Also offer a "resample" option here. The added difficulty here is that not all color spaces can be resampled to (see dialog_resample.cpp).
		//		  As explained above, in the cases where this is necessary the YCbCr Matrix can also be left unchanged, so it's left as a lower priority for now.

		if (action == YCbCrMatrixMismatchAction::Prompt) {
			int Choice = wxGetSingleChoiceIndex(
				fmt_tl(
					"This video's recommended YCbCr Matrix (\"%s\") differs from the subtitle file's YCbCr Matrix (\"%s\").\n\n"
					"If this new video's colors look identical to the video the subtitle file was originally authored for,\n"
					"then the subtitle file's YCbCr Matrix should be set to the video's YCbCr Matrix.",
				recommended_matrix.to_string().value(), script_matrix.to_string().value()),
				_("YCbCr Matrix mismatch"),
				{fmt_tl("Leave script's YCbCr Matrix as it is (\"%s\")", script_matrix.to_string().value()), fmt_tl("Set script's YCbCr Matrix to the video's (\"%s\")", recommended_matrix.to_string().value())},
				OPT_GET("Video/Last YCbCr Matrix Mismatch Choice")->GetInt(),
				parent
			);

			if (Choice == -1) {
				Choice = static_cast<int>(MissingYCbCrMatrixFix::Ignore);
			} else {
				OPT_SET("Video/Last YCbCr Matrix Mismatch Choice")->SetInt(Choice);
			}

			switch (static_cast<YCbCrMatrixMismatchFix>(Choice)) {
				case YCbCrMatrixMismatchFix::Ignore:
					action = YCbCrMatrixMismatchAction::Ignore;
					break;
				case YCbCrMatrixMismatchFix::Set:
					action = YCbCrMatrixMismatchAction::Set;
					break;
			}
		}

		if (action == YCbCrMatrixMismatchAction::Set) {
			file->SetScriptInfo("YCbCr Matrix", recommended_matrix.to_string().value());
			return true;
		}
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

	bool ar_changed = check_ar_changed(sx, sy, vx, vy);

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

bool update_layout_res(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	// When opening dummy video only want to set the script properties if
	// they were previously unset
	if (!new_provider->ShouldSetVideoProperties())
		return false;

	int spx = file->GetScriptInfoAsInt("PlayResX");
	int spy = file->GetScriptInfoAsInt("PlayResY");

	int sx = file->GetScriptInfoAsInt("LayoutResX");
	int sy = file->GetScriptInfoAsInt("LayoutResY");

	auto [vx, vy] = new_provider->GetDisplayResolution();

	if (sx == 0 || sy == 0) {
		// Script has no LayoutRes set yet, so find out if the user wants to set one
		auto action = static_cast<MissingLayoutResAction>(OPT_GET("Video/No LayoutRes in Script")->GetInt());

		if (action == MissingLayoutResAction::Prompt) {
			int Choice = wxGetSingleChoiceIndex(
				fmt_tl(
					"The current subtitle file has no layout resolution set.\n\n"
					"If you plan to use this subtitle file on this newly loaded video file,\n"
					"you should set the subtitle file's layout resolution to the video's display resolution.\n"
					"\n"
					"If you plan to use a different video file, you may want to leave\n"
					"the layout resolution unset until you load the correct video file.\n"
					"\n"
					"If you are adapting an existing old subtitle file that does not yet have\n"
					"a layout resolution set, you should set its layout resolution to the resolution\n"
					"of the video the subtitle file was intended to be played back on.\n"
					"Often, this is the same as the file's script resolution. (%s \u00D7 %s)\n"
					"\n"
					"This prompt can be configured in the preferences."
				, spx, spy),
				_("Set script's layout resolution?"),
				{_("Leave layout resolution unset"),
				 fmt_tl("Set script's layout resolution to the video's (%d \u00D7 %d)", vx, vy), // U+00D7 multiplication sign
				 fmt_tl("Set script's layout resolution to its script resolution (%d \u00D7 %d)", spx, spy)},
				OPT_GET("Video/Last No LayoutRes in Script Choice")->GetInt(),
				parent
			);

			if (Choice == -1) {
				Choice = static_cast<int>(MissingLayoutResFix::Ignore);
			} else {
				OPT_SET("Video/Last No LayoutRes in Script Choice")->SetInt(Choice);
			}

			switch (static_cast<MissingLayoutResFix>(Choice)) {
				case MissingLayoutResFix::Ignore:
					action = MissingLayoutResAction::Ignore;
					break;
				case MissingLayoutResFix::Set:
					action = MissingLayoutResAction::Set;
					break;
				case MissingLayoutResFix::SetToPlayRes:
					action = MissingLayoutResAction::SetToPlayRes;
					break;
			}
		}

		switch (action) {
			case MissingLayoutResAction::Set:
				file->SetScriptInfo("LayoutResX", std::to_string(vx));
				file->SetScriptInfo("LayoutResY", std::to_string(vy));
				return true;
			case MissingLayoutResAction::SetToPlayRes:
				file->SetScriptInfo("LayoutResX", std::to_string(spx));
				file->SetScriptInfo("LayoutResY", std::to_string(spy));
				return true;
			default:
				break;
		}
	} else if (sx != vx || sx != vy) {
		// The video's (display) resolution differs from the LayoutRes.
		//
		// If the aspect ratio stays the same this is usually not a problem
		// (after all, the entire point of LayoutRes is that subtitle files will correctly display
		// on all video resolutions), so we do not show any alert here by default.
		// However, if the user wants to be prompted in these cases, they can enable this in the settings.
		//
		// If the aspect ratio changes, we definitely need user intervention here.

		auto condition = static_cast<LayoutResMismatchPromptCondition>(OPT_GET("Video/LayoutRes Mismatch")->GetInt());

		if (condition == LayoutResMismatchPromptCondition::Never)
			return false;

		bool ar_changed = check_ar_changed(sx, sy, vx, vy);

		if (!ar_changed && condition == LayoutResMismatchPromptCondition::OnARMismatch)
			return false;

		wxString prompt = !ar_changed ?
			fmt_tl(
				"The resolution of the loaded video and the layout resolution specified for the subtitles don't match.\n\n"
				"Video resolution:\t%d \u00D7 %d\n"     // U+00D7 multiplication sign
				"Subtitle layout resolution:\t%d \u00D7 %d\n\n"
				"Usually, this is not an issue, and you may leave the subtitle file's layout resolution as it is.\n"
				"If your subtitle file does not contain any significant formatting and you now intend to perform\n"
				"formatting on the newly loaded video, you may want to set the script's layout resolution\n"
				"to the video's display resolution."
			, vx, vy, sx, sy)
			:
			// FIXME: offer more options in this case, like cropping or resampling to the new aspect ratio
			fmt_tl(
				"The resolution of the loaded video and the layout resolution specified for the subtitles differ in aspect ratios.\n\n"
				"Video resolution:\t%d \u00D7 %d\n"     // U+00D7 multiplication sign
				"Subtitle layout resolution:\t%d \u00D7 %d\n\n"
				"Change layout resolution of subtitles to match video?"
			, vx, vy, sx, sy);

		int Choice = wxGetSingleChoiceIndex(
			prompt,
			_("Layout resolution mismatch"),
			{fmt_tl("Leave layout resolution as it is (%s \u00D7 %s)", sx, sy), fmt_tl("Set script's layout resolution to the video's (%d \u00D7 %d)", vx, vy)}, // U+00D7 multiplication sign
			OPT_GET("Video/Last LayoutRes Mismatch Choice")->GetInt(),
			parent
		);

		if (Choice == -1) {
			Choice = static_cast<int>(LayoutResMismatchFix::Ignore);
		} else {
			OPT_SET("Video/Last LayoutRes Mismatch Choice")->SetInt(Choice);
		}

		if (static_cast<LayoutResMismatchFix>(Choice) == LayoutResMismatchFix::Set) {
			file->SetScriptInfo("LayoutResX", std::to_string(vx));
			file->SetScriptInfo("LayoutResY", std::to_string(vy));
			return true;
		}
	}

	return false;
}

}

void UpdateVideoProperties(AssFile *file, const AsyncVideoProvider *new_provider, wxWindow *parent) {
	if (update_ycbcr_matrix(file, new_provider, parent))
		file->Commit(_("change ycbcr matrix"), AssFile::COMMIT_SCRIPTINFO);

	if (update_play_res(file, new_provider, parent))
		file->Commit(_("change script resolution"), AssFile::COMMIT_SCRIPTINFO);

	if (update_layout_res(file, new_provider, parent))
		file->Commit(_("change layout resolution"), AssFile::COMMIT_SCRIPTINFO);
}
