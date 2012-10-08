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

#include "config.h"

#include "scintilla_text_selection_controller.h"

#include "scintilla_text_ctrl.h"

ScintillaTextSelectionController::ScintillaTextSelectionController(ScintillaTextCtrl *ctrl)
: ctrl(ctrl)
{
}

void ScintillaTextSelectionController::SetInsertionPoint(int position) {
	ctrl->SetInsertionPoint(ctrl->GetUnicodePosition(position));
}

int ScintillaTextSelectionController::GetInsertionPoint() const {
	return ctrl->GetReverseUnicodePosition(ctrl->GetInsertionPoint());
}

void ScintillaTextSelectionController::SetSelection(int start, int end) {
	ctrl->SetSelectionU(start, end);
}

int ScintillaTextSelectionController::GetSelectionStart() const {
	return ctrl->GetReverseUnicodePosition(ctrl->GetSelectionStart());
}

int ScintillaTextSelectionController::GetSelectionEnd() const {
	return ctrl->GetReverseUnicodePosition(ctrl->GetSelectionEnd());
}
