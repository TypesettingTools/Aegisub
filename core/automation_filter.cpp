// Copyright (c) 2005, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#include "automation_filter.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filename.h>
#include "dialog_progress.h"


AutomationScriptThread::AutomationScriptThread(AutomationScript *a_script, AssFile *a_subs)
: wxThread(wxTHREAD_JOINABLE), script(a_script), subs(a_subs)
{
	Create();
}


wxThread::ExitCode AutomationScriptThread::Entry()
{
	try {
		script->process_lines(subs);
	}
	catch (AutomationError &e) {
		script->OutputDebugString(wxString(_T("Script wrapper: Script produced an exception.")), true);
		script->OutputDebugString(wxString::Format(_T("Message was: %s"), e.message.c_str()), true);
		script->OutputDebugString(wxString(_T("Script wrapper: Output data are probably unchanged or corrupted.")), true);
	}
	return 0;
}



AutomationFilterConfigDialog::AutomationFilterConfigDialog(wxWindow *parent, AutomationScriptConfiguration &config)
: wxPanel(parent, wxID_ANY)
{
	wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 5, 5);

	//wxLogMessage(_T("Now going to create %d controls for automation script"), config.options.size());

	for (std::vector<AutomationScriptConfigurationOption>::iterator opt = config.options.begin(); opt != config.options.end(); opt++) {
		//wxLogMessage(_T("Creating control for kind: %d"), opt->kind);
		if (opt->kind == COK_INVALID)
			continue;

		Control control;
		control.option = &*opt;

		switch (opt->kind) {
			case COK_LABEL:
				control.control = new wxStaticText(this, -1, opt->label);
				break;

			case COK_TEXT:
				control.control = new wxTextCtrl(this, -1, opt->value.stringval);
				break;

			case COK_INT:
				control.control = new wxSpinCtrl(this, -1);
				if (opt->min.isset && opt->max.isset) {
					((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, opt->max.intval);
				} else if (opt->min.isset) {
					((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, 0x7fff);
				} else if (opt->max.isset) {
					((wxSpinCtrl*)control.control)->SetRange(-0x7fff, opt->max.intval);
				} else {
					((wxSpinCtrl*)control.control)->SetRange(-0x7fff, 0x7fff);
				}
				((wxSpinCtrl*)control.control)->SetValue(opt->value.intval);
				break;

			case COK_FLOAT:
				control.control = new wxTextCtrl(this, -1, wxString::Format(_T("%f"), opt->value.floatval));
				break;

			case COK_BOOL:
				control.control = new wxCheckBox(this, -1, opt->label);
				((wxCheckBox*)control.control)->SetValue(opt->value.boolval);
				break;

			case COK_COLOUR:
				// *FIXME* what to do here?
				// just put a stupid edit box for now
				control.control = new wxTextCtrl(this, -1, opt->value.colourval.GetASSFormatted(false));
				break;

			case COK_STYLE:
				control.control = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, AssFile::top->GetStyles());
				((wxChoice*)control.control)->Insert(_T(""), 0);
				break;

		}

		if (opt->kind != COK_LABEL && opt->kind != COK_BOOL) {
			control.label = new wxStaticText(this, -1, opt->label);
			sizer->Add(control.label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
		} else {
			control.label = 0;
			sizer->AddSpacer(0);
		}
		control.control->SetToolTip(opt->hint);
		sizer->Add(control.control, 1, wxEXPAND);

		controls.push_back(control);
	}

	SetSizerAndFit(sizer);
}



AssAutomationFilter::AssAutomationFilter(AutomationScript *a_script)
: script(a_script), dialog(0)
{
	Register(wxString::Format(_T("Automation: %s"), script->name.c_str()), 2000);
	if (script->description.IsEmpty()) {
		Description = wxString::Format(_T("%s\r\n(This Automation script has not provided a description.)"), script->name.c_str());
	} else {
		Description = script->description;
	}
	automation_filter_list.push_back(this);
}


void AssAutomationFilter::Init() {
}


AssAutomationFilter::~AssAutomationFilter()
{
	Unregister();
	automation_filter_list.remove(this);
}


static void progress_reporter(float progress, AutomationScript *script, void *dialog)
{
	((DialogProgress*)dialog)->SetProgress((int)(progress*10), 1000);
}

static void debug_reporter(wxString &str, bool isdebug, AutomationScript *script, void *dialog)
{
	((DialogProgress*)dialog)->SetText(str);
	if (isdebug) {
		wxLogMessage(str);
	}
}


void AssAutomationFilter::ProcessSubs(AssFile *subs)
{
	AutomationScriptThread thread(script, subs);

	// prepare a progress dialog
	DialogProgress *dialog = new DialogProgress(0, script->name, &script->force_cancel, _T(""), 0, 1000);
	dialog->Show();

	// make the script aware of it
	script->progress_reporter = progress_reporter;
	script->progress_target = dialog;
	script->debug_reporter = debug_reporter;
	script->debug_target = dialog;

	// run the script
	thread.Run();
	thread.Wait();

	// make sure the dialog won't be touched again
	script->progress_reporter = 0;
	script->debug_reporter = 0;
	delete dialog;
}


void AssAutomationFilter::LoadSettings(bool IsDefault)
{
	wxString opthname = wxString::Format(_T("Automation Settings %s"), wxFileName(script->filename).GetFullName().c_str());

	// if it's an auto export, just read the serialized settings from the ass file
	// (does nothing if no settings are serialized)
	if (IsDefault) {
		wxString serialized = AssFile::top->GetScriptInfo(opthname);
		script->configuration.unserialize(serialized);
		return;
	}

	// if there's no dialog, we can't do anything
	if (!dialog) return;

	for (std::vector<AutomationFilterConfigDialog::Control>::iterator ctl = dialog->controls.begin(); ctl != dialog->controls.end(); ctl++) {
		switch (ctl->option->kind) {
			case COK_TEXT:
				ctl->option->value.stringval = ((wxTextCtrl*)ctl->control)->GetValue();
				break;

			case COK_INT:
				ctl->option->value.intval = ((wxSpinCtrl*)ctl->control)->GetValue();
				break;

			case COK_FLOAT:
				if (!((wxTextCtrl*)ctl->control)->GetValue().ToDouble(&ctl->option->value.floatval)) {
					wxLogWarning(
						_T("The value entered for field '%s' (%s) could not be converted to a floating-point number. Default value (%f) substituted for the entered value."),
						ctl->option->label.c_str(),
						((wxTextCtrl*)ctl->control)->GetValue().c_str(),
						ctl->option->default_val.floatval);
					ctl->option->value.floatval = ctl->option->default_val.floatval;
				}
				break;

			case COK_BOOL:
				ctl->option->value.boolval = ((wxCheckBox*)ctl->control)->GetValue();
				break;

			case COK_COLOUR:
				// *FIXME* needs to be updated to use a proper color control
				ctl->option->value.colourval.ParseASS(((wxTextCtrl*)ctl->control)->GetValue());
				break;

			case COK_STYLE:
				ctl->option->value.stringval = ((wxChoice*)ctl->control)->GetStringSelection();
				break;
		}
	}

	// serialize the new settings and save them to the file
	AssFile::top->SetScriptInfo(opthname, script->configuration.serialize());
}


wxWindow *AssAutomationFilter::GetConfigDialogWindow(wxWindow *parent)
{
	wxString opthname = wxString::Format(_T("Automation Settings %s"), wxFileName(script->filename).GetFullName().c_str());
	wxString serialized = AssFile::top->GetScriptInfo(opthname);
	script->configuration.unserialize(serialized);

	if (script->configuration.present)
        return dialog = new AutomationFilterConfigDialog(parent, script->configuration);
	else
		return 0;
}


AutomationScript *AssAutomationFilter::GetScript()
{
	return script;
}


const std::list<AssAutomationFilter*>& AssAutomationFilter::GetFilterList()
{
	return AssAutomationFilter::automation_filter_list;
}


// static list of loaded automation filters
std::list<AssAutomationFilter*> AssAutomationFilter::automation_filter_list;

