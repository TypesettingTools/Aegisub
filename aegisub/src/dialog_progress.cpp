// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file dialog_progress.cpp
/// @brief Progress-bar dialog box for displaying during long operations
/// @ingroup utility
///

#include "config.h"

#include "dialog_progress.h"

#include "compat.h"
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/exception.h>

#include <mutex>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

using agi::dispatch::Main;

class DialogProgressSink : public agi::ProgressSink {
	DialogProgress *dialog;
	bool cancelled;
	std::mutex cancelled_mutex;

public:
	DialogProgressSink(DialogProgress *dialog)
	: dialog(dialog)
	, cancelled(false)
	{
	}

	void SetTitle(std::string const& title) {
		Main().Async([=]{ dialog->title->SetLabelText(to_wx(title)); });
	}

	void SetMessage(std::string const& msg) {
		Main().Async([=]{ dialog->text->SetLabelText(to_wx(msg)); });
	}

	void SetProgress(int64_t cur, int64_t max) {
		Main().Async([=]{ dialog->gauge->SetValue(mid<int>(0, double(cur) / max * 100, 100)); });
	}

	void Log(std::string const& str) {
		Main().Async([=]{ dialog->pending_log += to_wx(str); });
	}

	bool IsCancelled() {
		std::lock_guard<std::mutex> lock(cancelled_mutex);
		return cancelled;
	}

	void Cancel() {
		std::lock_guard<std::mutex> lock(cancelled_mutex);
		cancelled = true;
	}

	void SetIndeterminate() {
		Main().Async([=]{ dialog->pulse_timer.Start(1000); });
	}
};

DialogProgress::DialogProgress(wxWindow *parent, wxString const& title_text, wxString const& message)
: wxDialog(parent, -1, title_text, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
, pulse_timer(GetEventHandler())
{
	title = new wxStaticText(this, -1, title_text, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
	gauge = new wxGauge(this, -1, 100, wxDefaultPosition, wxSize(300,20));
	text = new wxStaticText(this, -1, message, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
	cancel_button = new wxButton(this, wxID_CANCEL);
	log_output = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(300, 120), wxTE_MULTILINE | wxTE_READONLY);

	// make the title a slightly larger font
	wxFont title_font = title->GetFont();
	int fontsize = title_font.GetPointSize();
	title_font.SetPointSize(fontsize * 1.375);
	title_font.SetWeight(wxFONTWEIGHT_BOLD);
	title->SetFont(title_font);

	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(title, wxSizerFlags().Expand().Center());
	sizer->Add(gauge, wxSizerFlags(1).Expand().Border());
	sizer->Add(text, wxSizerFlags().Expand().Center());
	sizer->Add(cancel_button, wxSizerFlags().Center().Border());
	sizer->Add(log_output, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	sizer->Hide(log_output);

	SetSizerAndFit(sizer);
	CenterOnParent();

	Bind(wxEVT_SHOW, &DialogProgress::OnShow, this);
	Bind(wxEVT_TIMER, [=](wxTimerEvent&) { gauge->Pulse(); });
	Bind(wxEVT_IDLE, &DialogProgress::OnIdle, this);
}

void DialogProgress::Run(std::function<void(agi::ProgressSink*)> task, int priority) {
	DialogProgressSink ps(this);
	this->ps = &ps;

	agi::dispatch::Background().Async([=]{
		try {
			task(this->ps);
		}
		catch (agi::Exception const& e) {
			this->ps->Log(e.GetChainedMessage());
		}

		Main().Async([this]{
			pulse_timer.Stop();

			// Unbind the cancel handler so that the default behavior happens (i.e. the
			// dialog is closed) as there's no longer a task to cancel
			Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProgress::OnCancel, this, wxID_CANCEL);

			// If it ran to completion and there is debug output, leave the window open
			// so the user can read the debug output and switch the cancel button to a
			// close button
			bool cancelled = this->ps->IsCancelled();
			if (cancelled || (log_output->IsEmpty() && !pending_log))
				EndModal(!cancelled);
			else
				cancel_button->SetLabelText(_("Close"));
		});
	});

	if (!ShowModal())
		throw agi::UserCancelException("Cancelled by user");
}

void DialogProgress::OnShow(wxShowEvent&) {
	// Restore the cancel button in case it was previously switched to a close
	// button
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProgress::OnCancel, this, wxID_CANCEL);
	cancel_button->SetLabelText(_("Cancel"));
	cancel_button->Enable();

	wxSizer *sizer = GetSizer();
	if (sizer->IsShown(log_output)) {
		sizer->Hide(log_output);
		Layout();
		sizer->Fit(this);
		log_output->Clear();
	}
}

void DialogProgress::OnIdle(wxIdleEvent&) {
	if (!pending_log) return;

	if (log_output->IsEmpty()) {
		wxSizer *sizer = GetSizer();
		sizer->Show(log_output);
		Layout();
		sizer->Fit(this);
	}

	*log_output << pending_log;
	log_output->SetInsertionPointEnd();
	pending_log.clear();
}

void DialogProgress::OnCancel(wxCommandEvent &) {
	ps->Cancel();
	cancel_button->Enable(false);
	cancel_button->SetLabelText(_("Cancelling..."));
}
