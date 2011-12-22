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
//
// $Id$

/// @file dialog_progress.cpp
/// @brief Progress-bar dialog box for displaying during long operations
/// @ingroup utility
///

#include "config.h"

#include "dialog_progress.h"

#include <libaegisub/exception.h>

#include "compat.h"
#include "utils.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

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
		SafeQueue(EVT_TITLE, lagi_wxString(title));
	}

	void SetMessage(std::string const& msg) {
		SafeQueue(EVT_MESSAGE, lagi_wxString(msg));
	}

	void SetProgress(int cur, int max) {
		SafeQueue(EVT_PROGRESS, int(double(cur) / max * 100));
	}

	void Log(std::string const& str) {
		SafeQueue(EVT_LOG, lagi_wxString(str));
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
	std::tr1::function<void(agi::ProgressSink*)> task;
	agi::ProgressSink *ps;
	wxDialog *dialog;

public:
	TaskRunner(std::tr1::function<void(agi::ProgressSink*)> task, agi::ProgressSink *ps, wxDialog *dialog, int priority)
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
		task(ps);
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
	Bind(wxEVT_TIMER, &DialogProgress::OnPulseTimer, this);

	Bind(EVT_TITLE, &DialogProgress::OnSetTitle, this);
	Bind(EVT_MESSAGE, &DialogProgress::OnSetMessage, this);
	Bind(EVT_PROGRESS, &DialogProgress::OnSetProgress, this);
	Bind(EVT_INDETERMINATE, &DialogProgress::OnSetIndeterminate, this);
	Bind(EVT_COMPLETE, &DialogProgress::OnComplete, this);
	Bind(EVT_LOG, &DialogProgress::OnLog, this);
}

void DialogProgress::Run(std::tr1::function<void(agi::ProgressSink*)> task, int priority) {
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

void DialogProgress::OnSetTitle(wxThreadEvent &evt) {
	title->SetLabelText(evt.GetPayload<wxString>());
}

void DialogProgress::OnSetMessage(wxThreadEvent &evt) {
	text->SetLabelText(evt.GetPayload<wxString>());
}

void DialogProgress::OnSetProgress(wxThreadEvent &evt) {
	gauge->SetValue(mid(0, evt.GetPayload<int>(), 100));
}

void DialogProgress::OnSetIndeterminate(wxThreadEvent &) {
	pulse_timer.Start(1000);
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
	if (cancelled || log_output->IsEmpty())
		EndModal(!cancelled);
	else
		cancel_button->SetLabelText(_("Close"));
}

void DialogProgress::OnLog(wxThreadEvent &evt) {
	if (log_output->IsEmpty()) {
		wxSizer *sizer = GetSizer();
		sizer->Show(log_output);
		Layout();
		sizer->Fit(this);
	}

	*log_output << evt.GetPayload<wxString>();
	log_output->SetInsertionPointEnd();
}

void DialogProgress::OnCancel(wxCommandEvent &) {
	ps->Cancel();
	cancel_button->Enable(false);
	cancel_button->SetLabelText(_("Cancelling..."));
}

void DialogProgress::OnPulseTimer(wxTimerEvent&) {
	gauge->Pulse();
}
