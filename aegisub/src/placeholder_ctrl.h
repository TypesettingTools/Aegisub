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
//
// $Id$

/// @file placeholder_ctrl.h
/// @ingroup custom_control
///

#ifndef AGI_PRE
#include <wx/settings.h>
#endif

/// @class Placeholder
/// @brief A wrapper around a control to add placeholder text
///
/// This control wraps a base control to add default greyed-out placeholder
/// text describing the control when the value would otherwise be empty, which
/// is removed when the control is focused to begin typing in it, and restored
/// when the control loses focus and the value is empty
template<class BaseCtrl>
class Placeholder : public BaseCtrl {
	wxString placeholder; ///< Placeholder string
	bool is_placeholder;  ///< Should the value be cleared on focus?

	/// Wrapper around Create to make it possible to override it for specific
	/// base classes
	inline void Create(wxWindow *parent, wxSize const& size, long style) {
		BaseCtrl::Create(parent, -1, placeholder, wxDefaultPosition, size, style);
	}

	/// Focus gained event handler
	void OnSetFocus(wxFocusEvent& evt) {
		evt.Skip();

		if (is_placeholder) {
			BaseCtrl::ChangeValue("");
			BaseCtrl::SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		}
	}

	/// Focus lost event handler
	void OnKillFocus(wxFocusEvent& evt) {
		evt.Skip();
		ChangeValue(BaseCtrl::GetValue());
	}

public:
	/// Constructor
	/// @param parent Parent window
	/// @param placeholder Placeholder string
	/// @param size Control size
	/// @param style Style flags to pass to the base control
	/// @param tooltip Tooltip string
	Placeholder(wxWindow *parent, wxString const& placeholder, wxSize const& size, long style, wxString const& tooltip)
	: placeholder(placeholder)
	, is_placeholder(true)
	{
		Create(parent, size, style);
		BaseCtrl::SetToolTip(tooltip);
		BaseCtrl::SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

		BaseCtrl::Bind(wxEVT_SET_FOCUS, &Placeholder::OnSetFocus, this);
		BaseCtrl::Bind(wxEVT_KILL_FOCUS, &Placeholder::OnKillFocus, this);
	}

	/// @brief Change the value of the control without triggering events
	/// @param new_value New value of the control
	///
	/// If new_value is empty, the control will switch to placeholder mode
	void ChangeValue(wxString new_value) {
		if (new_value.empty() && !this->HasFocus()) {
			is_placeholder = true;
			new_value = placeholder;
			BaseCtrl::SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		}
		else {
			is_placeholder = false;
			BaseCtrl::SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		}

		// This check should be pointless, but wxGTK is awesome and generates
		// change events in wxComboBox::ChangeValue
		if (new_value != BaseCtrl::GetValue())
			BaseCtrl::ChangeValue(new_value);
	}
};

template<> inline void Placeholder<wxComboBox>::Create(wxWindow *parent, wxSize const& size, long style) {
	wxComboBox::Create(parent, -1, "", wxDefaultPosition, size, 0, 0, style);
}
