// Copyright (c) 2005, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file timeedit_ctrl.h
/// @see timeedit_ctrl.cpp
/// @ingroup custom_control
///

#ifndef AGI_PRE
#include <wx/textctrl.h>
#endif

#include "ass_time.h"

#include <libaegisub/signal.h>

namespace agi {
	class OptionValue;
	struct Context;
}

/// @brief A text edit control for editing AssTime objects
///
/// This control constrains values to valid times, and can display the time
/// being edited as either a h:mm:ss.cc formatted time, or a frame number
class TimeEdit : public wxTextCtrl {
	agi::Context *c; ///< Project context
	bool byFrame;    ///< Is the time displayed as a frame number?
	bool isEnd;      ///< Should the time be treated as an end time for time <-> frame conversions?
	AssTime time;    ///< The time, which may be displayed as either a frame number or time
	bool insert;     ///< If true, disable overwriting behavior in time mode

	agi::signal::Connection insert_opt;

	void CopyTime();
	void PasteTime();

	/// Set the value of the text box from the current time and byFrame setting
	void UpdateText();

	void OnContextMenu(wxContextMenuEvent &event);
	void OnFocusLost(wxFocusEvent &evt);
	void OnInsertChanged(agi::OptionValue const& opt);
	void OnKeyDown(wxKeyEvent &event);
	void OnModified(wxCommandEvent &event);

public:
	/// Get the current time as an AssTime object
	AssTime GetTime() const { return time; }
	/// Set the time
	void SetTime(AssTime time);

	/// Set whether the time is displayed as a time or the corresponding frame number
	/// @param enableByFrame If true, frame numbers are displayed
	void SetByFrame(bool enableByFrame);

	/// Constructor
	/// @param parent Parent window
	/// @param id Window id
	/// @param c Project context
	/// @param value Initial value. Must be a valid time string or empty
	/// @param size Initial control size
	/// @param asEnd Treat the time as a line end time (rather than start) for time <-> frame number conversions
	TimeEdit(wxWindow* parent, wxWindowID id, agi::Context *c, const wxString& value = "", const wxSize& size = wxDefaultSize, bool asEnd = false);
};
