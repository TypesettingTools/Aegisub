// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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
//
// $Id$

#ifndef AGI_PRE
#include <wx/pen.h>
#endif

#include <libaegisub/signal.h>

namespace agi { class OptionValue; }

/// @class Pen
/// @brief A simple wrapper around wxPen to bind the colour and width to the
/// value of an option
class Pen {
	wxPen impl;
	agi::signal::Connection colour_con;
	agi::signal::Connection width_con;

	void OnColourChanged(agi::OptionValue const& opt);
	void OnWidthChanged(agi::OptionValue const& opt);

public:
	/// Constructor
	/// @param colour_opt Option name to get the colour from
	/// @param width_opt Option name to get the width from
	/// @param style Pen style
	Pen(const char *colour_opt, const char *width_opt, wxPenStyle style = wxPENSTYLE_SOLID);

	/// Constructor
	/// @param colour_opt Option name to get the colour from
	/// @param width Pen width
	/// @param style Pen style
	Pen(const char *colour_opt, int width = 1, wxPenStyle style = wxPENSTYLE_SOLID);

	/// Implicit conversion to wxPen
	operator wxPen const&() const { return impl; }
};
