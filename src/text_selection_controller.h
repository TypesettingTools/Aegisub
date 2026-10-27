// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#pragma once

#include <libaegisub/signal.h>

class wxStyledTextCtrl;
class wxTextCtrl;
class wxEvent;

class TextSelectionController {
	int selection_start = 0;
	int selection_end = 0;
	int insertion_point = 0;
	bool changing = false;

	int staged_selection_start = 0;
	int staged_selection_end = 0;
	bool has_staged_selection = false;

	wxStyledTextCtrl *ctrl_stc = nullptr;
	wxTextCtrl *ctrl_text = nullptr;
	bool use_stc = true;

	void UpdateUI(wxEvent &evt);

	agi::signal::Signal<> AnnounceSelectionChanged;

public:
	void SetSelection(int start, int end);
	void SetInsertionPoint(int point);

	// This set of functions allows staging changes to the selection or insertion points, which can then be applied later.
	// This is useful when one is still waiting on other changes to be applied, but already listening for changes to the
	// selection in the eventually visible text.
	// They also provide a wrapper for setting a selection whose insertion point is on the left side.
	void StageSetSelection(int start, int end) { staged_selection_start = start; staged_selection_end = end; has_staged_selection = true; };
	void StageSetInsertionPoint(int point) { StageSetSelection(point, point); };
	void CommitStagedChanges();
	void DropStagedChanges() { has_staged_selection = false; };

	int GetSelectionStart() const { return selection_start; }
	int GetSelectionEnd() const { return selection_end; }
	int GetInsertionPoint() const { return insertion_point; }

	int GetStagedSelectionStart() const { return has_staged_selection ? staged_selection_start : selection_start; }
	int GetStagedSelectionEnd() const { return has_staged_selection ? staged_selection_end : selection_end; }
	int GetStagedInsertionPoint() const { return has_staged_selection ? staged_selection_end : insertion_point; }

	void SetControl(wxStyledTextCtrl *ctrl);
	void SetControl(wxTextCtrl *ctrl);
	~TextSelectionController();

	DEFINE_SIGNAL_ADDERS(AnnounceSelectionChanged, AddSelectionListener)
};
