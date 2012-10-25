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

/// @file dialog_selection.h
/// @see dialog_selection.cpp
/// @ingroup secondary_ui
///

#ifndef AGI_PRE
#include <wx/dialog.h>
#endif

namespace agi { struct Context; }
class AssDialogue;
class wxCheckBox;
class wxRadioBox;
class wxRadioButton;
class wxTextCtrl;

/// DOCME
/// @class DialogSelection
/// @brief DOCME
///
/// DOCME
class DialogSelection : public wxDialog {
	agi::Context *con; ///< Project context

	wxTextCtrl *match_text; ///< Text to search for
	wxCheckBox *case_sensitive; ///< Should the search be case-sensitive
	wxCheckBox *apply_to_dialogue; ///< Select/deselect uncommented lines
	wxCheckBox *apply_to_comments; ///< Select/deselect commented lines
	wxRadioButton *select_unmatching_lines; ///< Select lines which don't match instead
	wxRadioBox *selection_change_type; ///< What sort of action to take on the selection
	wxRadioBox *dialogue_field; ///< Which dialogue field to look at
	wxRadioBox *match_mode;

	void Process(wxCommandEvent&);

	/// Dialogue/Comment check handler to ensure at least one is always checked
	/// @param chk The checkbox to check if both are clear
	void OnDialogueCheckbox(wxCheckBox *chk);

public:
	DialogSelection(agi::Context *c);
	~DialogSelection();
};
