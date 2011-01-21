// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file export_framerate.cpp
/// @brief Transform Framerate export filter
/// @ingroup export
///

#include "config.h"

#ifndef AGI_PRE
#include <utility>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "export_framerate.h"
#include "utils.h"
#include "video_context.h"

AssTransformFramerateFilter::AssTransformFramerateFilter()
: AssExportFilter(_("Transform Framerate"), _("Transform subtitle times, including those in override tags, from an input framerate to an output framerate.\n\nThis is useful for converting regular time subtitles to VFRaC time subtitles for hardsubbing.\nIt can also be used to convert subtitles to a different speed video, such as NTSC to PAL speedup."), 1000)
, Input(0)
, Output(0)
{
}

void AssTransformFramerateFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog) {
	TransformFrameRate(subs);
}

wxWindow *AssTransformFramerateFilter::GetConfigDialogWindow(wxWindow *parent) {
	wxWindow *base = new wxPanel(parent, -1);

	LoadSettings(true);

	// Input sizer
	wxSizer *InputSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString initialInput;
	wxButton *FromVideo = new wxButton(base,-1,_("From Video"));
	if (Input->IsLoaded()) {
		initialInput = wxString::Format("%2.3f",Input->FPS());
		FromVideo->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AssTransformFramerateFilter::OnFpsFromVideo, this);
	}
	else {
		initialInput = "23.976";
		FromVideo->Enable(false);
	}
	InputFramerate = new wxTextCtrl(base,-1,initialInput,wxDefaultPosition,wxSize(60,20));
	InputSizer->Add(InputFramerate,0,wxEXPAND | wxLEFT,5);
	InputSizer->Add(FromVideo,0,wxEXPAND | wxLEFT,5);
	InputSizer->AddStretchSpacer(1);

	// Output sizers
	wxSizer *OutputSizerTop = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *OutputSizerBottom = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *OutputSizer = new wxBoxSizer(wxVERTICAL);

	// Output top line
	RadioOutputVFR = new wxRadioButton(base,-1,_("Variable"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	OutputSizerTop->Add(RadioOutputVFR,0,wxEXPAND,0);

	// Output bottom line
	RadioOutputCFR = new wxRadioButton(base,-1,_("Constant: "));
	wxString initialOutput = initialInput;
	if (!Output->IsVFR()) {
		RadioOutputVFR->Enable(false);
		RadioOutputCFR->SetValue(true);
	}
	OutputFramerate = new wxTextCtrl(base,-1,initialOutput,wxDefaultPosition,wxSize(60,20));
	OutputSizerBottom->Add(RadioOutputCFR,0,wxEXPAND,0);
	OutputSizerBottom->Add(OutputFramerate,0,wxEXPAND | wxLEFT,5);
	OutputSizerBottom->AddStretchSpacer(1);

	// Reverse checkbox
	Reverse = new wxCheckBox(base,-1,_("Reverse transformation"));

	// Output final
	OutputSizer->Add(OutputSizerTop,0,wxLEFT,5);
	OutputSizer->Add(OutputSizerBottom,0,wxLEFT,5);

	// Main window
	wxSizer *MainSizer = new wxFlexGridSizer(3,2,5,10);
	MainSizer->Add(new wxStaticText(base,-1,_("Input framerate: ")),0,wxEXPAND | wxALIGN_CENTER_VERTICAL,0);
	MainSizer->Add(InputSizer,0,wxEXPAND,0);
	MainSizer->Add(new wxStaticText(base,-1,_("Output: ")),0,wxALIGN_CENTER_VERTICAL,0);
	MainSizer->Add(OutputSizer,0,wxEXPAND,0);
	MainSizer->Add(Reverse,0,wxTOP|wxEXPAND,5);

	// Window
	base->SetSizerAndFit(MainSizer);
	return base;
}

void AssTransformFramerateFilter::OnFpsFromVideo(wxCommandEvent &) {
	InputFramerate->SetValue(wxString::Format("%g", VideoContext::Get()->FPS().FPS()));
}

void AssTransformFramerateFilter::LoadSettings(bool IsDefault) {
	if (IsDefault) {
		Input = &VideoContext::Get()->VFR_Input;
		Output = &VideoContext::Get()->VFR_Output;
	}
	else {
		double temp;
		InputFramerate->GetValue().ToDouble(&temp);
		t1 = temp;
		Input = &t1;
		if (RadioOutputCFR->GetValue()) {
			OutputFramerate->GetValue().ToDouble(&temp);
			t2 = temp;
			Output = &t2;
		}
		else Output = &VideoContext::Get()->VFR_Output;

		if (Reverse->IsChecked()) {
			std::swap(Input, Output);
		}
	}
}

/// Truncate a time to centisecond precision
int FORCEINLINE trunc_cs(int time) {
	return (time / 10) * 10;
}

void AssTransformFramerateFilter::TransformTimeTags(wxString name,int n,AssOverrideParameter *curParam,void *curData) {
	VariableDataType type = curParam->GetType();
	if (type != VARDATA_INT && type != VARDATA_FLOAT) return;

	AssTransformFramerateFilter *instance = static_cast<AssTransformFramerateFilter*>(curData);
	AssDialogue *curDiag = instance->line;

	int parVal = curParam->Get<int>();

	switch (curParam->classification) {
		case PARCLASS_RELATIVE_TIME_START: {
			int value = instance->ConvertTime(trunc_cs(curDiag->Start.GetMS()) + parVal) - instance->newStart;

			// An end time of 0 is actually the end time of the line, so ensure
			// nonzero is never converted to 0
			// Needed here rather than the end case because start/end here mean
			// which end of the line the time is relative to, not whether it's
			// the start or end time (compare \move and \fad)
			if (value == 0 && parVal != 0) value = 1;
			curParam->Set(value);
			break;
		}
		case PARCLASS_RELATIVE_TIME_END:
			curParam->Set(instance->newEnd - instance->ConvertTime(trunc_cs(curDiag->End.GetMS()) - parVal));
			break;
		case PARCLASS_KARAOKE: {
			int start = curDiag->Start.GetMS() / 10 + instance->oldK + parVal;
			int value = (instance->ConvertTime(start * 10) - instance->newStart) / 10 - instance->newK;
			instance->oldK += parVal;
			instance->newK += value;
			curParam->Set(value);
			break;
		}
		default:
			return;
	}
}

void AssTransformFramerateFilter::TransformFrameRate(AssFile *subs) {
	if (!Input->IsLoaded() || !Output->IsLoaded()) return;
	for (entryIter cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		AssDialogue *curDialogue = dynamic_cast<AssDialogue*>(*cur);

		if (curDialogue) {
			line = curDialogue;
			newK = 0;
			oldK = 0;
			newStart = trunc_cs(ConvertTime(curDialogue->Start.GetMS()));
			newEnd = trunc_cs(ConvertTime(curDialogue->End.GetMS()) + 9);

			// Process stuff
			curDialogue->ParseASSTags();
			curDialogue->ProcessParameters(TransformTimeTags, this);
			curDialogue->Start.SetMS(newStart);
			curDialogue->End.SetMS(newEnd);
			curDialogue->UpdateText();
			curDialogue->ClearBlocks();
		}
	}
}

int AssTransformFramerateFilter::ConvertTime(int time) {
	int frame = Output->FrameAtTime(time);
	int frameStart = Output->TimeAtFrame(frame);
	int frameEnd = Output->TimeAtFrame(frame + 1);
	int frameDur = frameEnd - frameStart;
	double dist = double(time - frameStart) / frameDur;

	int newStart = Input->TimeAtFrame(frame);
	int newEnd = Input->TimeAtFrame(frame + 1);
	int newDur = newEnd - newStart;

	return newStart + newDur * dist;
}
