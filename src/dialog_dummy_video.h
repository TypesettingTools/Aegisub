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
// Aegisub Project http://www.aegisub.org/

/// @file dialog_dummy_video.h
/// @see dialog_dummy_video.cpp
/// @ingroup secondary_ui
///

#include <wx/dialog.h>

#include <libaegisub/color.h>

class wxFlexGridSizer;
class wxStaticText;

class DialogDummyVideo : public wxDialog {
	DialogDummyVideo(wxWindow *parent);

	double fps;
	int width;
	int height;
	int length;
	agi::Color color;
	bool pattern;

	wxStaticText *length_display;
	wxFlexGridSizer *sizer;

	template<typename T>
	void AddCtrl(wxString const& label, T *ctrl);

	void OnResolutionShortcut(wxCommandEvent &evt);
	void UpdateLengthDisplay();

public:
	static std::string CreateDummyVideo(wxWindow *parent);
};
