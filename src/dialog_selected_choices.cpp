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

#include <numeric>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/listbox.h>
#include <wx/sizer.h>

int GetSelectedChoices(wxWindow *parent, wxArrayInt& selections, wxString const& message, wxString const& caption, wxArrayString const& choices) {
	wxMultiChoiceDialog dialog(parent, message, caption, choices);

	auto selAll = new wxButton(&dialog, -1, _("Select &All"));
	selAll->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
		wxArrayInt sel(choices.size(), 0);
		std::iota(sel.begin(), sel.end(), 0);
		dialog.SetSelections(sel);
	});

	auto selNone = new wxButton(&dialog, -1, _("Select &None"));
	selNone->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { dialog.SetSelections(wxArrayInt()); });

	auto buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(selAll, wxSizerFlags(0).Left());
	buttonSizer->Add(selNone, wxSizerFlags(0).Right());

	auto sizer = dialog.GetSizer();
	sizer->Insert(2, buttonSizer, wxSizerFlags(0).Center());
	sizer->Fit(&dialog);

	dialog.SetSelections(selections);

	if (dialog.ShowModal() != wxID_OK) return -1;

	selections = dialog.GetSelections();
	return selections.GetCount();
}
