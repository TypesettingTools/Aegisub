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

/// @@file main.h
/// @see main.cpp

#ifndef R_PRECOMP
#include <wx/frame.h>
#include <wx/event.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#endif

#include "view.h"

/// @brief Reporter
class Reporter : public wxApp {
public:
    virtual bool OnInit();

private:
	Report *r;
};

IMPLEMENT_APP(Reporter)

/// @brief Main frame.
class mFrame : public wxFrame {
public:
	mFrame(const wxString& window_title);
	void Submit(wxCommandEvent& event);
	void Cancel(wxCommandEvent& event);
	void ReportView(wxCommandEvent& event);
	void SetReport(Report *report) { r = report; }

private:
	Report *r;
	DECLARE_EVENT_TABLE()
};


BEGIN_EVENT_TABLE(mFrame, wxFrame)
	EVT_BUTTON(wxID_OK, mFrame::Submit)
	EVT_BUTTON(wxID_CANCEL, mFrame::Cancel)
	EVT_BUTTON(42, mFrame::ReportView)
END_EVENT_TABLE()

