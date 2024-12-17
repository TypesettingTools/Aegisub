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

/// @file timeedit_ctrl.cpp
/// @brief Edit-control for editing SSA-format timestamps
/// @ingroup custom_control
///

#include "timeedit_ctrl.h"

#include "compat.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "project.h"
#include "utils.h"

#include <libaegisub/ass/time.h>

#include <functional>
#include <wx/menu.h>
#include <wx/valtext.h>

#define TimeEditWindowStyle

// Check menu.h for id range allocation before editing this enum
enum {
	Time_Edit_Copy = (wxID_HIGHEST + 1) + 3000,
	Time_Edit_Paste
};

TimeEdit::TimeEdit(wxWindow* parent, wxWindowID id, agi::Context *c, const std::string& value, const wxSize& size, bool asEnd)
: wxTextCtrl(parent, id, to_wx(value), wxDefaultPosition, size, wxTE_CENTRE | wxTE_PROCESS_ENTER)
, c(c)
, isEnd(asEnd)
, insert(!OPT_GET("Subtitle/Time Edit/Insert Mode")->GetBool())
, insert_opt(OPT_SUB("Subtitle/Time Edit/Insert Mode", &TimeEdit::OnInsertChanged, this))
{
	// Set validator
	wxTextValidator val(wxFILTER_INCLUDE_CHAR_LIST);
	wxString includes[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", ".", ":", ","};
	val.SetIncludes(wxArrayString(std::size(includes), includes));
	SetValidator(val);

	// Other stuff
	if (value.empty()) SetValue(to_wx(time.GetAssFormatted()));

	Bind(wxEVT_MENU, std::bind(&TimeEdit::CopyTime, this), Time_Edit_Copy);
	Bind(wxEVT_MENU, std::bind(&TimeEdit::PasteTime, this), Time_Edit_Paste);
	Bind(wxEVT_TEXT, &TimeEdit::OnModified, this);
	Bind(wxEVT_CONTEXT_MENU, &TimeEdit::OnContextMenu, this);
	Bind(wxEVT_CHAR_HOOK, &TimeEdit::OnKeyDown, this);
	Bind(wxEVT_CHAR, &TimeEdit::OnChar, this);
	Bind(wxEVT_KILL_FOCUS, &TimeEdit::OnFocusLost, this);
}

void TimeEdit::SetTime(agi::Time new_time) {
	if (time != new_time) {
		time = new_time;
		UpdateText();
	}
}

int TimeEdit::GetFrame() const {
	return c->project->Timecodes().FrameAtTime(time, isEnd ? agi::vfr::END : agi::vfr::START);
}

void TimeEdit::SetFrame(int fn) {
	SetTime(c->project->Timecodes().TimeAtFrame(fn, isEnd ? agi::vfr::END : agi::vfr::START));
}

void TimeEdit::SetByFrame(bool enableByFrame) {
	if (enableByFrame == byFrame) return;

	byFrame = enableByFrame && c->project->Timecodes().IsLoaded();
	UpdateText();
}

void TimeEdit::OnModified(wxCommandEvent &event) {
	event.Skip();
	if (byFrame) {
		long temp = 0;
		GetValue().ToLong(&temp);
		time = c->project->Timecodes().TimeAtFrame(temp, isEnd ? agi::vfr::END : agi::vfr::START);
	}
	else if (insert)
		time = agi::Time(GetValue().utf8_str().data());
}

void TimeEdit::UpdateText() {
	if (byFrame)
		ChangeValue(std::to_wstring(c->project->Timecodes().FrameAtTime(time, isEnd ? agi::vfr::END : agi::vfr::START)));
	else
		ChangeValue(to_wx(time.GetAssFormatted()));
}

void TimeEdit::OnKeyDown(wxKeyEvent &event) {
	int kc = event.GetKeyCode();

	// Needs to be done here to trump user-defined hotkeys
	int key = event.GetUnicodeKey();
	if (event.CmdDown()) {
		if (key == 'C' || key == 'X')
			CopyTime();
		else if (key == 'V')
			PasteTime();
		else
			event.Skip();
		return;
	}

	// Shift-Insert would paste the stuff anyway
	// but no one updates the private "time" variable.
	if (event.ShiftDown() && kc == WXK_INSERT) {
		PasteTime();
		return;
	}

	if (byFrame || insert) {
		event.Skip();
		return;
	}
	// Overwrite mode stuff

	// On OS X backspace is reported as delete
#ifdef __APPLE__
	if (kc == WXK_DELETE)
		kc = WXK_BACK;
#endif

	// Back just moves cursor back one without deleting
	if (kc == WXK_BACK) {
		long start = GetInsertionPoint();
		if (start > 0)
			SetInsertionPoint(start - 1);
	}
	// Delete just does nothing
	else if (kc != WXK_DELETE)
		event.Skip();
}

void TimeEdit::OnChar(wxKeyEvent &event) {
	event.Skip();
	if (byFrame || insert) return;

	int key = event.GetUnicodeKey();
	if ((key < '0' || key > '9') && key != ';' && key != '.' && key != ',') return;

	event.Skip(false);

	long start = GetInsertionPoint();
	auto text = from_wx(GetValue());
	// Cursor is at the end so do nothing
	if (start >= (long)text.size()) return;

	// If the cursor is at punctuation, move it forward to the next digit
	if (text[start] == ':' || text[start] == '.' || text[start] == ',')
		++start;

	// : and . hop over punctuation but never insert anything
	if (key == ':' || key == ';' || key == '.' || key == ',') {
		SetInsertionPoint(start);
		return;
	}

	// Overwrite the digit
	text[start] = (char)key;
	time = agi::Time(text);
	SetValue(to_wx(time.GetAssFormatted()));
	SetInsertionPoint(start + 1);
}

void TimeEdit::OnInsertChanged(agi::OptionValue const& opt) {
	insert = !opt.GetBool();
}

void TimeEdit::OnContextMenu(wxContextMenuEvent &evt) {
	if (byFrame || insert) {
		evt.Skip();
		return;
	}

	wxMenu menu;
	menu.Append(Time_Edit_Copy, _("&Copy"));
	menu.Append(Time_Edit_Paste, _("&Paste"));
	PopupMenu(&menu);
}

void TimeEdit::OnFocusLost(wxFocusEvent &evt) {
	if (insert || byFrame)
		UpdateText();
	evt.Skip();
}

void TimeEdit::CopyTime() {
	SetClipboard(from_wx(GetValue()));
}

void TimeEdit::PasteTime() {
	if (byFrame) {
		Paste();
		return;
	}

	std::string text(GetClipboard());
	if (text.empty()) return;

	agi::Time tempTime(text);
	if (tempTime.GetAssFormatted() == text) {
		SetTime(tempTime);
		SetSelection(0, GetValue().size());

		wxCommandEvent evt(wxEVT_TEXT, GetId());
		evt.SetEventObject(this);
		HandleWindowEvent(evt);
	}
}
