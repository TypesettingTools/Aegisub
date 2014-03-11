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

/// @file view.h
/// @see view.cpp
/// @ingroup base

#ifndef R_PRECOMP
#include <wx/frame.h>
#include <wx/event.h>
#include <wx/window.h>
#include <wx/dialog.h>
#endif

#include "report.h"

/// @class View
/// @brief View the stored report.
class View: public wxDialog {
public:
	View(wxWindow *frame, Report *r);
	~View() {};

private:
	Report *r;
	std::string *text;
	void CloseDialog(wxCommandEvent& event);
	void Clipboard(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};
