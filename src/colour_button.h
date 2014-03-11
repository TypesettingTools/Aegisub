// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <wx/button.h>

#include <libaegisub/color.h>

/// Emitted by ColourButton when the user picks a new color, with the chosen
/// color set to the event payload
wxDECLARE_EVENT(EVT_COLOR, wxThreadEvent);

/// A button which displays a currently-selected color and lets the user pick
/// a new color when clicked
class ColourButton: public wxButton {
	wxImage bmp;       ///< The button's bitmap label
	agi::Color colour; ///< The current colour

	/// Update the bitmap label after the color is changed
	void UpdateBitmap();

public:
	/// Constructor
	/// @param parent Parent window
	/// @param size Size of the bitmap (note: not the size of the button)
	/// @param alpha Let the user adjust the color's alpha
	/// @param color Initial color to display
	ColourButton(wxWindow *parent, wxSize const& size, bool alpha, agi::Color color = agi::Color(), wxValidator const& validator = wxDefaultValidator);

	/// Get the currently selected color
	agi::Color GetColor() { return colour; }
};

struct ColorValidator : public wxValidator {
	agi::Color *color;
	ColorValidator(agi::Color *color) : color(color) { }
	wxValidator *Clone() const override { return new ColorValidator(color); }
	bool Validate(wxWindow*) override { return true; }
	bool TransferToWindow() override { return true; }

	bool TransferFromWindow() override {
		*color = static_cast<ColourButton*>(GetWindow())->GetColor();
		return true;
	}
};
