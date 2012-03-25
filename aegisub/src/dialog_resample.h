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
// $Id$

/// @file dialog_resample.h
/// @see dialog_resample.cpp
/// @ingroup tools_ui
///

#ifndef AGI_PRE
#include <wx/dialog.h>
#endif

namespace agi { struct Context; }
class AssFile;
class wxCheckBox;
class wxSpinCtrl;

/// Configuration parameters for a resample
struct ResampleSettings {
	/// Amount to add to each margin
	int margin[4];
	/// New X resolution
	int script_x;
	/// New Y resolution
	int script_y;
	/// Should the aspect ratio of the subs be changed?
	bool change_ar;
};

/// Resample the subtitles in the project
/// @param file Subtitles to resample
/// @param settings Resample configuration settings
void ResampleResolution(AssFile *file, ResampleSettings const& settings);

/// @class DialogResample
/// @brief Configuration dialog for resolution resampling
///
/// Populate a ResampleSettings structure with data from the user
class DialogResample : public wxDialog {
	agi::Context *c; ///< Project context

	wxSpinCtrl *res_x;
	wxSpinCtrl *res_y;
	wxCheckBox *symmetrical;
	wxSpinCtrl *margin_ctrl[4];

	/// Set the destination resolution to the video's resolution
	void SetDestFromVideo(wxCommandEvent &);
	/// Symmetrical checkbox toggle handler
	void OnSymmetrical(wxCommandEvent &);
	/// Copy margin values over if symmetrical is enabled
	void OnMarginChange(wxSpinCtrl *src, wxSpinCtrl *dst);

public:
	/// Constructor
	/// @param context Project context
	/// @param[out] settings Settings struct to populate
	DialogResample(agi::Context *context, ResampleSettings &settings);
};
