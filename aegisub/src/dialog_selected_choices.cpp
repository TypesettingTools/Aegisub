// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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

	wxButton *selAll = new wxButton(this, -1, _("Select All"));
	wxButton *selNone = new wxButton(this, -1, _("Select None"));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SelectedChoicesDialog::SelectAll, this, selAll->GetId());
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &SelectedChoicesDialog::SelectNone, this, selNone->GetId());

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

void SelectedChoicesDialog::SelectNone(wxCommandEvent&) {
	SetSelections(wxArrayInt());
}

int GetSelectedChoices(wxWindow *parent, wxArrayInt& selections, wxString const& message, wxString const& caption, wxArrayString const& choices) {
	SelectedChoicesDialog dialog(parent, message, caption, choices);
	dialog.SetSelections(selections);

	if (dialog.ShowModal() != wxID_OK) return -1;

	selections = dialog.GetSelections();
	return selections.GetCount();
}
