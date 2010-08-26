// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file preferences.h
/// @brief Preferences dialogue
/// @see preferences.cpp
/// @ingroup configuration_ui

#ifndef AGI_PRE
#include <map>

#include <wx/any.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/treebook.h>
#include <wx/listctrl.h>
#endif

class Preferences: public wxDialog {
	wxTreebook *book;
	wxButton *applyButton;

	std::map<std::string, wxAny> pending_changes;

	void OnOK(wxCommandEvent &event);
	void OnCancel(wxCommandEvent &event);
	void OnApply(wxCommandEvent &event);

public:
	Preferences(wxWindow *parent);
	~Preferences();

	void SetOption(const char *name, wxAny value);

	DECLARE_EVENT_TABLE()
};
