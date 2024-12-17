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

#include "dialog_progress.h"

#include "compat.h"
#include "utils.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/exception.h>
#include <libaegisub/util_osx.h>

#include <atomic>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#ifdef _MSC_VER
#include <shobjidl.h>
#endif

using agi::dispatch::Main;

namespace {
	void set_taskbar_progress(int progress) {
#ifdef _MSC_VER
		int major, minor;
		wxGetOsVersion(&major, &minor);
		if (major < 6 || (major == 6 && minor < 1)) return;

		ITaskbarList3 *taskbar;
		auto hr = ::CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER,
			__uuidof(ITaskbarList3), (LPVOID *)&taskbar);
		if (FAILED(hr)) return;

		hr = taskbar->HrInit();
		if (FAILED(hr)) {
			taskbar->Release();
			return;
		}

		auto hwnd = wxTheApp->GetTopWindow()->GetHWND();
		if (progress == 0 || progress == 100)
			taskbar->SetProgressState(hwnd, TBPF_NOPROGRESS);
		else if (progress == -1)
			taskbar->SetProgressState(hwnd, TBPF_INDETERMINATE);
		else
			taskbar->SetProgressValue(hwnd, progress, 100);

		taskbar->Release();
#endif
	}
}

class DialogProgressSink final : public agi::ProgressSink {
	DialogProgress *dialog;
	std::atomic<bool> cancelled{false};
	int progress = 0;

public:
	DialogProgressSink(DialogProgress *dialog) : dialog(dialog) { }

	void SetTitle(std::string const& title) override {
		Main().Async([=, this]{ dialog->title->SetLabelText(to_wx(title)); });
	}

	void SetMessage(std::string const& msg) override {
		Main().Async([=, this]{ dialog->text->SetLabelText(to_wx(msg)); });
	}

	void SetProgress(int64_t cur, int64_t max) override {
		int new_progress = mid<int>(0, double(cur) / max * 300, 300);
		if (new_progress != progress) {
			progress = new_progress;
			Main().Async([=, this]{ dialog->SetProgress(new_progress); });
		}
	}

	void Log(std::string const& str) override {
		Main().Async([=, this]{ dialog->pending_log += to_wx(str); });
	}

	bool IsCancelled() override {
		return cancelled;
	}

	void Cancel() {
		cancelled = true;
	}

	void SetIndeterminate() override {
		Main().Async([=, this]{ dialog->pulse_timer.Start(1000); });
	}
};

DialogProgress::DialogProgress(wxWindow *parent, wxString const& title_text, wxString const& message)
: wxDialog(parent, -1, title_text, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED)
, pulse_timer(GetEventHandler())
{
	title = new wxStaticText(this, -1, title_text, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
	gauge = new wxGauge(this, -1, 300, wxDefaultPosition, wxSize(300,20));
	text = new wxStaticText(this, -1, message, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
	cancel_button = new wxButton(this, wxID_CANCEL);
	log_output = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxSize(600, 240), wxTE_MULTILINE | wxTE_READONLY);

	// make the title a slightly larger font
	wxFont title_font = title->GetFont();
	int fontsize = title_font.GetPointSize();
	title_font.SetPointSize(fontsize * 1.375);
	title_font.SetWeight(wxFONTWEIGHT_BOLD);
	title->SetFont(title_font);

	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(title, wxSizerFlags().Expand());
	sizer->Add(gauge, wxSizerFlags(1).Expand().Border());
	sizer->Add(text, wxSizerFlags().Expand());
	sizer->Add(cancel_button, wxSizerFlags().Center().Border());
	sizer->Add(log_output, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	sizer->Hide(log_output);

	SetSizerAndFit(sizer);
	CenterOnParent();

	Bind(wxEVT_SHOW, &DialogProgress::OnShow, this);
	Bind(wxEVT_TIMER, [=, this](wxTimerEvent&) { gauge->Pulse(); });
}

void DialogProgress::Run(std::function<void(agi::ProgressSink*)> task) {
	DialogProgressSink ps(this);
	this->ps = &ps;

	auto current_title = from_wx(title->GetLabelText());
	agi::dispatch::Background().Async([=, this]{
		agi::osx::AppNapDisabler app_nap_disabler(current_title);
		try {
			task(this->ps);
		}
		catch (agi::Exception const& e) {
			this->ps->Log(e.GetMessage());
		}

		Main().Async([this]{
			pulse_timer.Stop();
			Unbind(wxEVT_IDLE, &DialogProgress::OnIdle, this);

			// Unbind the cancel handler so that the default behavior happens (i.e. the
			// dialog is closed) as there's no longer a task to cancel
			Unbind(wxEVT_BUTTON, &DialogProgress::OnCancel, this, wxID_CANCEL);

			// If it ran to completion and there is debug output, leave the window open
			// so the user can read the debug output and switch the cancel button to a
			// close button
			bool cancelled = this->ps->IsCancelled();
			if (cancelled || (log_output->IsEmpty() && !pending_log))
				EndModal(!cancelled);
			else {
				if (!pending_log.empty()) {
					wxIdleEvent evt;
					OnIdle(evt);
				}
				cancel_button->SetLabelText(_("Close"));
				gauge->SetValue(300);
			}
			set_taskbar_progress(0);
		});
	});

	if (!ShowModal())
		throw agi::UserCancelException("Cancelled by user");
}

void DialogProgress::OnShow(wxShowEvent& evt) {
	if (!evt.IsShown()) return;

	// Restore the cancel button in case it was previously switched to a close
	// button
	Bind(wxEVT_BUTTON, &DialogProgress::OnCancel, this, wxID_CANCEL);
	Bind(wxEVT_IDLE, &DialogProgress::OnIdle, this);
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
	if (progress_current > progress_target) {
		progress_current = progress_target;
		gauge->SetValue(progress_current);
		set_taskbar_progress(progress_current / 3);
	}
	else if (progress_current < progress_target) {
		using namespace std::chrono;
		auto now = steady_clock::now();
		int ms = mid<int>(0, duration_cast<milliseconds>(now - progress_anim_start_time).count(), progress_anim_duration);
		int dist = (progress_target - progress_anim_start_value) * ms / progress_anim_duration;
		if (dist) {
			progress_current = progress_anim_start_value + dist;
			gauge->SetValue(progress_current);
			set_taskbar_progress(progress_current / 3);
		}
	}

	if (!pending_log) return;

	if (log_output->IsEmpty()) {
		wxSizer *sizer = GetSizer();
		sizer->Show(log_output);
		Layout();
		sizer->Fit(this);
		CenterOnParent();
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

void DialogProgress::SetProgress(int target) {
	pulse_timer.Stop();

	if (target == progress_target) return;
	using namespace std::chrono;

	progress_anim_start_value = progress_current;
	auto now = steady_clock::now();
	if (progress_target == 0)
		progress_anim_duration = 1000;
	else
		progress_anim_duration = std::max<int>(100, duration_cast<milliseconds>(now - progress_anim_start_time).count() * 11 / 10);
	progress_anim_start_time = now;
	progress_target = target;
}
