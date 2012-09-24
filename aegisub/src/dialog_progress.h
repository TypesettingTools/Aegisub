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

/// @file dialog_progress.h
/// @see dialog_progress.cpp
/// @ingroup utility
///

#ifndef AGI_PRE
#include <wx/dialog.h>
#include <wx/timer.h>
#endif

#include <libaegisub/background_runner.h>
#include <libaegisub/scoped_ptr.h>

class DialogProgressSink;
class wxButton;
class wxGauge;
class wxStaticText;
class wxTextCtrl;

/// @class DialogProgress
/// @brief Progress-bar dialog box for displaying during long operations
class DialogProgress : public wxDialog, public agi::BackgroundRunner {
	DialogProgressSink *ps;

	wxStaticText *title;
	wxStaticText *text;
	wxGauge *gauge;
	wxButton *cancel_button;
	wxTextCtrl *log_output;

	wxTimer pulse_timer;

	wxString pending_log;

	void OnSetTitle(wxThreadEvent &evt);
	void OnSetMessage(wxThreadEvent &evt);
	void OnSetProgress(wxThreadEvent &evt);
	void OnSetIndeterminate(wxThreadEvent &evt);
	void OnLog(wxThreadEvent &evt);
	void OnComplete(wxThreadEvent &evt);

	void OnShow(wxShowEvent&);
	void OnCancel(wxCommandEvent &);
	void OnPulseTimer(wxTimerEvent&);
	void OnIdle(wxIdleEvent&);

public:
	/// Constructor
	/// @param parent Parent window of the dialog
	/// @param title Initial title of the dialog
	/// @param message Initial message of the dialog
	DialogProgress(wxWindow *parent, wxString const& title="", wxString const& message="");

	/// BackgroundWorker implementation
	void Run(std::function<void(agi::ProgressSink *)> task, int priority=-1);
};
