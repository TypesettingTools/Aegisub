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

/// @file persist_location.h
/// @see persist_location.cpp
/// @ingroup utility

namespace agi { class OptionValue; }
class wxDialog;

/// @class PersistLocation
/// @brief Automatically save and restore the location of a dialog
///
/// This class saves the location of the supplied dialog to the preferences
/// file with the given prefix, then restores the saved position when it is
/// recreated in the future. This class should always have lifetime equal to
/// the associated dialog, as it does not unbind its events.
class PersistLocation {
	agi::OptionValue *x_opt;
	agi::OptionValue *y_opt;
	agi::OptionValue *maximize_opt;

	void OnMove(wxMoveEvent &evt);
	void OnMinimize(wxIconizeEvent &evt);

public:
	/// Persist the location of a dialog
	/// @param dialog The dialog to save and restore the position of
	/// @param options_prefix Prefix for the options names to store the location
	PersistLocation(wxDialog *dialog, std::string options_prefix);
};
