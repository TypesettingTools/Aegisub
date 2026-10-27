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

#include <algorithm>
#include <wx/event.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>

void TextSelectionController::SetControl(wxStyledTextCtrl *ctrl) {
	if (ctrl_stc)
		ctrl_stc->Unbind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
	if (ctrl_text) {
		ctrl_text->Unbind(wxEVT_KEY_UP, &TextSelectionController::UpdateUI, this);
		ctrl_text->Unbind(wxEVT_LEFT_UP, &TextSelectionController::UpdateUI, this);
	}
	ctrl_stc = ctrl;
	ctrl_text = nullptr;
	use_stc = true;
	if (ctrl)
		ctrl->Bind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
}

void TextSelectionController::SetControl(wxTextCtrl *ctrl) {
	if (ctrl_stc)
		ctrl_stc->Unbind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
	if (ctrl_text) {
		ctrl_text->Unbind(wxEVT_KEY_UP, &TextSelectionController::UpdateUI, this);
		ctrl_text->Unbind(wxEVT_LEFT_UP, &TextSelectionController::UpdateUI, this);
	}
	ctrl_stc = nullptr;
	ctrl_text = ctrl;
	use_stc = false;
	if (ctrl) {
		ctrl->Bind(wxEVT_KEY_UP, &TextSelectionController::UpdateUI, this);
		ctrl->Bind(wxEVT_LEFT_UP, &TextSelectionController::UpdateUI, this);
	}
}

TextSelectionController::~TextSelectionController() {
	if (ctrl_stc)
		ctrl_stc->Unbind(wxEVT_STC_UPDATEUI, &TextSelectionController::UpdateUI, this);
	if (ctrl_text) {
		ctrl_text->Unbind(wxEVT_KEY_UP, &TextSelectionController::UpdateUI, this);
		ctrl_text->Unbind(wxEVT_LEFT_UP, &TextSelectionController::UpdateUI, this);
	}
}

#define GET(var, new_value) do { \
	int tmp = new_value;      \
	if (tmp != var) {         \
		var = tmp;            \
		changed = true;       \
	}                         \
} while(false)

void TextSelectionController::UpdateUI(wxEvent &evt) {
	evt.Skip();
	if (changing) return;

	bool changed = false;
	long current_insertion = 0, current_start = 0, current_end = 0;
	if (use_stc && ctrl_stc) {
		current_insertion = ctrl_stc->GetCurrentPos();
		current_start = ctrl_stc->GetSelectionStart();
		current_end = ctrl_stc->GetSelectionEnd();
	}
	else if (ctrl_text) {
		current_insertion = ctrl_text->GetInsertionPoint();
		ctrl_text->GetSelection(&current_start, &current_end);
		current_insertion = ctrl_text->GetRange(0, current_insertion).utf8_str().length();
		current_start = ctrl_text->GetRange(0, current_start).utf8_str().length();
		current_end = ctrl_text->GetRange(0, current_end).utf8_str().length();
	}
	GET(insertion_point, current_insertion);
	GET(selection_start, current_start);
	GET(selection_end, current_end);
	if (changed) AnnounceSelectionChanged();
}

void TextSelectionController::SetInsertionPoint(int position) {
	changing = true;
	if (insertion_point != position) {
		insertion_point = position;
		if (use_stc && ctrl_stc) {
			ctrl_stc->SetCurrentPos(position);
			ctrl_stc->SetAnchor(position);
		}
		else if (ctrl_text) {
			long text_position = 0;
			long last_position = ctrl_text->GetLastPosition();
			for (; text_position < last_position; ++text_position) {
				if (ctrl_text->GetRange(0, text_position).utf8_str().length() >= static_cast<size_t>(std::max(0, position)))
					break;
			}
			ctrl_text->SetInsertionPoint(text_position);
		}
	}
	changing = false;
	AnnounceSelectionChanged();
}

void TextSelectionController::SetSelection(int start, int end) {
	changing = true;
	if (selection_start != start || selection_end != end) {
		selection_start = start;
		selection_end = end;
		if (use_stc && ctrl_stc)
			ctrl_stc->SetSelection(start, end);
		else if (ctrl_text) {
			long text_start = -1, text_end = -1;
			long last_position = ctrl_text->GetLastPosition();
			size_t byte_start = static_cast<size_t>(std::max(0, start));
			size_t byte_end = static_cast<size_t>(std::max(0, end));
			for (long pos = 0; pos < last_position; ++pos) {
				size_t byte_pos = ctrl_text->GetRange(0, pos).utf8_str().length();
				if (text_start == -1 && byte_pos >= byte_start)
					text_start = pos;
				if (text_end == -1 && byte_pos >= byte_end)
					text_end = pos;
				if (text_start != -1 && text_end != -1)
					break;
			}
			if (text_start == -1) text_start = last_position;
			if (text_end == -1) text_end = last_position;
			ctrl_text->SetSelection(text_start, text_end);
		}
	}
	changing = false;
	AnnounceSelectionChanged();
}


void TextSelectionController::CommitStagedChanges() {
	if (has_staged_selection) {
		if (staged_selection_start <= staged_selection_end) {
			SetSelection(staged_selection_start, staged_selection_end);
		} else {
			// commit some crimes to get this to work in all cases
			SetInsertionPoint(staged_selection_end == 0 ? staged_selection_start : 0);
			SetSelection(staged_selection_start, staged_selection_start);
			SetInsertionPoint(staged_selection_end);
		}
		has_staged_selection = false;
	}
}