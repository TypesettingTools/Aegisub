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

/// @file dialog_resample.h
/// @see dialog_resample.cpp
/// @ingroup tools_ui
///

#include <wx/dialog.h>

namespace agi { struct Context; }
class AssFile;
class wxCheckBox;
class wxSpinCtrl;
struct ResampleSettings;

/// @class DialogResample
/// @brief Configuration dialog for resolution resampling
///
/// Populate a ResampleSettings structure with data from the user
class DialogResample final : public wxDialog {
	agi::Context *c; ///< Project context

	int script_w;
	int script_h;
	int video_w = 0;
	int video_h = 0;

	wxSpinCtrl *source_x;
	wxSpinCtrl *source_y;
	wxSpinCtrl *dest_x;
	wxSpinCtrl *dest_y;
	wxCheckBox *symmetrical;
	wxCheckBox *change_ar;
	wxSpinCtrl *margin_ctrl[4];

	wxButton *from_script;
	wxButton *from_video;

	void SetSourceFromScript(wxCommandEvent &);
	/// Set the destination resolution to the video's resolution
	void SetDestFromVideo(wxCommandEvent &);
	/// Symmetrical checkbox toggle handler
	void OnSymmetrical(wxCommandEvent &);
	/// Copy margin values over if symmetrical is enabled
	void OnMarginChange(wxSpinCtrl *src, wxSpinCtrl *dst);
	void UpdateButtons();

public:
	/// Constructor
	/// @param context Project context
	/// @param[out] settings Settings struct to populate
	DialogResample(agi::Context *context, ResampleSettings &settings);
};
