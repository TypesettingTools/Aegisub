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

#include <libaegisub/signal.h>

class wxStyledTextCtrl;
class wxStyledTextEvent;

class TextSelectionController {
	int selection_start = 0;
	int selection_end = 0;
	int insertion_point = 0;
	bool changing = false;

	wxStyledTextCtrl *ctrl = nullptr;

	void UpdateUI(wxStyledTextEvent &evt);

	agi::signal::Signal<> AnnounceSelectionChanged;

public:
	void SetSelection(int start, int end);
	void SetInsertionPoint(int point);

	int GetSelectionStart() const { return selection_start; }
	int GetSelectionEnd() const { return selection_end; }
	int GetInsertionPoint() const { return insertion_point; }

	void SetControl(wxStyledTextCtrl *ctrl);
	~TextSelectionController();

	DEFINE_SIGNAL_ADDERS(AnnounceSelectionChanged, AddSelectionListener)
};
