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

/// @file persist_location.h
/// @see persist_location.cpp
/// @ingroup utility

#pragma once

namespace agi { class OptionValue; }
class wxMoveEvent;
class wxSizeEvent;
class wxTopLevelWindow;

#include <string>

/// @class PersistLocation
/// @brief Automatically save and restore the location of a window
///
/// This class saves the location of the supplied window to the preferences
/// file with the given prefix, then restores the saved position when it is
/// recreated in the future. This class must not outlive the associated window.
class PersistLocation {
	agi::OptionValue *x_opt;
	agi::OptionValue *y_opt;
	agi::OptionValue *w_opt;
	agi::OptionValue *h_opt;
	agi::OptionValue *maximize_opt;
	wxTopLevelWindow *window;

	/// Is the window in a state whose geometry shouldn't be remembered?
	bool IsTransient() const;
	void SavePosition();

	void OnMove(wxMoveEvent&);
	void OnSize(wxSizeEvent&);

public:
	/// Persist the location of a window
	/// @param window The window to save and restore the position of
	/// @param options_prefix Prefix for the options names to store the location
	/// @param size_too Save and restore the size in addition to position
	PersistLocation(wxTopLevelWindow *window, std::string options_prefix, bool size_too = false);
	~PersistLocation();
};
