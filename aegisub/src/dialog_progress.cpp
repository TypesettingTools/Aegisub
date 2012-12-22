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

/// @file dialog_progress.cpp
/// @brief Progress-bar dialog box for displaying during long operations
/// @ingroup utility
///

#include "config.h"

#include "dialog_progress.h"

#include <libaegisub/exception.h>

#include "compat.h"
#include "utils.h"

#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

wxDEFINE_EVENT(EVT_TITLE, wxThreadEvent);
wxDEFINE_EVENT(EVT_MESSAGE, wxThreadEvent);
wxDEFINE_EVENT(EVT_PROGRESS, wxThreadEvent);
wxDEFINE_EVENT(EVT_INDETERMINATE, wxThreadEvent);
wxDEFINE_EVENT(EVT_LOG, wxThreadEvent);
wxDEFINE_EVENT(EVT_COMPLETE, wxThreadEvent);

class DialogProgressSink : public agi::ProgressSink {
	DialogProgress *dialog;
	bool cancelled;
	wxMutex cancelled_mutex;

	template<class T>
	void SafeQueue(wxEventType type, T const& value) {
		wxThreadEvent *evt = new wxThreadEvent(type);
		evt->SetPayload(value);
		wxQueueEvent(dialog, evt);
	}

public:
	DialogProgressSink(DialogProgress *dialog)
	: dialog(dialog)
	, cancelled(false)
	{
	}

	void SetTitle(std::string const& title) {
		SafeQueue(EVT_TITLE, to_wx(title));
	}

	void SetMessage(std::string const& msg) {
		SafeQueue(EVT_MESSAGE, to_wx(msg));
	}

	void SetProgress(int64_t cur, int64_t max) {
		SafeQueue(EVT_PROGRESS, int(double(cur) / max * 100));
	}

	void Log(std::string const& str) {
		SafeQueue(EVT_LOG, to_wx(str));
	}

	bool IsCancelled() {
		wxMutexLocker l(cancelled_mutex);
		return cancelled;
	}

	void Cancel() {
		wxMutexLocker l(cancelled_mutex);
		cancelled = true;
	}

	void SetIndeterminate() {
		wxQueueEvent(dialog, new wxThreadEvent(EVT_INDETERMINATE));
	}
};

class TaskRunner : public wxThread {
	std::function<void(agi::ProgressSink*)> task;
	agi::ProgressSink *ps;
	wxDialog *dialog;

public:
	TaskRunner(std::function<void(agi::ProgressSink*)> task, agi::ProgressSink *ps, wxDialog *dialog, int priority)
	: task(task)
	, ps(ps)
	, dialog(dialog)
	{
		Create();
		if (priority != -1)
			SetPriority(priority);
		Run();
	}

	wxThread::ExitCode Entry() {
		try {
			task(ps);
		}
		catch (agi::Exception const& e) {
			ps->Log(e.GetChainedMessage());
		}
		wxQueueEvent(dialog, new wxThreadEvent(EVT_COMPLETE));
		return 0;
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

	Bind(EVT_TITLE, [=](wxThreadEvent& e) { title->SetLabelText(e.GetPayload<wxString>()); });
	Bind(EVT_MESSAGE, [=](wxThreadEvent& e) { text->SetLabelText(e.GetPayload<wxString>()); });
	Bind(EVT_PROGRESS, [=](wxThreadEvent& e) { gauge->SetValue(mid(0, e.GetPayload<int>(), 100)); });
	Bind(EVT_INDETERMINATE, [=](wxThreadEvent &) { pulse_timer.Start(1000); });
	Bind(EVT_COMPLETE, &DialogProgress::OnComplete, this);
	Bind(EVT_LOG, [=](wxThreadEvent& e) { pending_log += e.GetPayload<wxString>(); });
}

void DialogProgress::Run(std::function<void(agi::ProgressSink*)> task, int priority) {
	DialogProgressSink ps(this);
	this->ps = &ps;
	new TaskRunner(task, &ps, this, priority);
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

void DialogProgress::OnComplete(wxThreadEvent &) {
	pulse_timer.Stop();

	// Unbind the cancel handler so that the default behavior happens (i.e. the
	// dialog is closed) as there's no longer a task to cancel
	Unbind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogProgress::OnCancel, this, wxID_CANCEL);

	// If it ran to completion and there is debug output, leave the window open
	// so the user can read the debug output and switch the cancel button to a
	// close button
	bool cancelled = ps->IsCancelled();
	if (cancelled || (log_output->IsEmpty() && !pending_log))
		EndModal(!cancelled);
	else
		cancel_button->SetLabelText(_("Close"));
}

void DialogProgress::OnCancel(wxCommandEvent &) {
	ps->Cancel();
	cancel_button->Enable(false);
	cancel_button->SetLabelText(_("Cancelling..."));
}
