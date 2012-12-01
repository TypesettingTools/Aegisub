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

/// @file dialog_shift_times.h
/// @see dialog_shift_times.cpp
/// @ingroup secondary_ui
///

#include <deque>

#include <wx/dialog.h>

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

#include "selection_controller.h"

class AssDialogue;
class TimeEdit;
class wxListBox;
class wxRadioBox;
class wxRadioButton;
class wxTextCtrl;
namespace agi { struct Context; }
namespace json {
	class UnknownElement;
	typedef std::deque<UnknownElement> Array;
}

/// DOCME
/// @class DialogShiftTimes
/// @brief DOCME
///
/// DOCME
class DialogShiftTimes : public wxDialog {
	agi::Context *context;

	std::string history_filename;
	agi::scoped_ptr<json::Array> history;
	agi::vfr::Framerate fps;
	agi::signal::Connection timecodes_loaded_slot;
	agi::signal::Connection selected_set_changed_slot;

	TimeEdit *shift_time;
	wxTextCtrl *shift_frames;
	wxRadioButton *shift_by_time;
	wxRadioButton *shift_by_frames;
	wxRadioButton *shift_forward;
	wxRadioButton *shift_backward;
	wxRadioBox *selection_mode;
	wxRadioBox *time_fields;
	wxListBox *history_box;

	void SaveHistory(json::Array const& shifted_blocks);
	void LoadHistory();
	void Process(wxCommandEvent&);
	int Shift(int initial_time, int shift, bool by_time, agi::vfr::Time type);

	void OnClear(wxCommandEvent&);
	void OnByTime(wxCommandEvent&);
	void OnByFrames(wxCommandEvent&);
	void OnHistoryClick(wxCommandEvent&);

	void OnSelectedSetChanged();
	void OnTimecodesLoaded(agi::vfr::Framerate const& new_fps);

public:
	DialogShiftTimes(agi::Context *context);
	~DialogShiftTimes();
};
