// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
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
// $Id$

/// @@file view.cpp
/// @brief View report in a human readable way.

#ifndef R_PRECOMP
#include <wx/window.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/clipbrd.h>
#endif

#include "view.h"

/// @brief View report dialog.
/// @param frame Parent frame.
/// @param r Report instance.
View::View(wxWindow *frame, Report *r)
: wxDialog (frame, wxID_ANY, _("View Report"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER, _("View Report"))
{
	wxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

	wxSizer *listSizer = new wxBoxSizer(wxHORIZONTAL);
	wxListView *listView = new wxListView(this,wxID_ANY,wxDefaultPosition,wxDefaultSize);

	// Fill the list with the actual report.
	text = new wxString();
	r->Fill(text, listView);

    listSizer->Add(listView, 1, wxEXPAND);
	topSizer->Add(listSizer, 1, wxEXPAND | wxALL, 0);

	wxStdDialogButtonSizer *stdButton = new wxStdDialogButtonSizer();
	stdButton->AddButton(new wxButton(this, wxID_CLOSE, _("Close")));
	stdButton->AddButton(new wxButton(this, wxID_SAVE, _("Copy to clipboard")));
	stdButton->Realize();
	topSizer->Add(stdButton, 0, wxALL | wxCENTRE | wxSHRINK, 5);

	this->SetSizerAndFit(topSizer);
	this->SetSize(wxSize(500,550));
}

/// @brief Close dialog
void View::CloseDialog(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

/// @brief Copy report to clipboard
void View::Clipboard(wxCommandEvent& WXUNUSED(event)) {
	if (wxTheClipboard->Open()) {
#ifdef __UNIX__
		wxTheClipboard->UsePrimarySelection(true);
#endif
		wxTheClipboard->SetData( new wxTextDataObject(*text));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}
}

BEGIN_EVENT_TABLE(View, wxDialog)
    EVT_BUTTON(wxID_CLOSE, View::CloseDialog)
    EVT_BUTTON(wxID_SAVE, View::Clipboard)
END_EVENT_TABLE()
