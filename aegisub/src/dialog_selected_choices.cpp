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

/// @file dialog_selected_choices.cpp
/// @brief wxMultiChoiceDialog with Select All and Select None
/// @ingroup

#include "dialog_selected_choices.h"

#include <numeric>

#ifdef __VISUALC__
#pragma warning(disable:4996)
#endif

SelectedChoicesDialog::SelectedChoicesDialog(wxWindow *parent, wxString const& message, wxString const& caption, wxArrayString const& choices) {
	Create(parent, message, caption, choices);

	wxButton *selAll = new wxButton(this, -1, _("Select &All"));
	wxButton *selNone = new wxButton(this, -1, _("Select &None"));
	selAll->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SelectedChoicesDialog::SelectAll, this);
	selNone->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) { SetSelections(wxArrayInt()); });

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(selAll, wxSizerFlags(0).Left());
	buttonSizer->Add(selNone, wxSizerFlags(0).Right());

	wxSizer *sizer = GetSizer();
	sizer->Insert(2, buttonSizer, wxSizerFlags(0).Center());
	sizer->Fit(this);
}

void SelectedChoicesDialog::SelectAll(wxCommandEvent&) {
	wxArrayInt sel(m_listbox->GetCount(), 1);
	sel[0] = 0;
	std::partial_sum(sel.begin(), sel.end(), sel.begin());
	SetSelections(sel);
}

int GetSelectedChoices(wxWindow *parent, wxArrayInt& selections, wxString const& message, wxString const& caption, wxArrayString const& choices) {
	SelectedChoicesDialog dialog(parent, message, caption, choices);
	dialog.SetSelections(selections);

	if (dialog.ShowModal() != wxID_OK) return -1;

	selections = dialog.GetSelections();
	return selections.GetCount();
}
