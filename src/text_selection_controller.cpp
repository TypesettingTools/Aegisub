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

#include "text_selection_controller.h"

#include <wx/stc/stc.h>

void TextSelectionController::SetControl(wxStyledTextCtrl *ctrl) {
	this->ctrl = ctrl;
	if (ctrl)
		ctrl->Bind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
}

TextSelectionController::~TextSelectionController() {
	if (ctrl) ctrl->Unbind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
}

#define GET(var, new_value) do { \
	int tmp = new_value;      \
	if (tmp != var) {         \
		var = tmp;            \
		changed = true;       \
	}                         \
} while(false)

#define SET(var, new_value, Setter) do { \
	if (var != new_value) {              \
		var = new_value;                 \
		if (ctrl) ctrl->Setter(var);     \
	}                                    \
} while (false)

void TextSelectionController::UpdateUI(wxStyledTextEvent &evt) {
	if (changing) return;

	bool changed = false;
	GET(insertion_point, ctrl->GetInsertionPoint());
	if (evt.GetUpdated() & wxSTC_UPDATE_SELECTION) {
		GET(selection_start, ctrl->GetSelectionStart());
		GET(selection_end, ctrl->GetSelectionEnd());
	}
	else {
		GET(selection_start, insertion_point);
		GET(selection_end, insertion_point);
	}
	if (changed) AnnounceSelectionChanged();
}

void TextSelectionController::SetInsertionPoint(int position) {
	changing = true;
	SET(insertion_point, position, SetInsertionPoint);
	changing = false;
	AnnounceSelectionChanged();
}

void TextSelectionController::SetSelection(int start, int end) {
	changing = true;
	SET(selection_start, start, SetSelectionStart);
	SET(selection_end, end, SetSelectionEnd);
	changing = false;
	AnnounceSelectionChanged();
}
