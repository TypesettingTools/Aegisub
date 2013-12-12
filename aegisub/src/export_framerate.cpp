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

/// @file export_framerate.cpp
/// @brief Transform Framerate export filter
/// @ingroup export
///

#include "config.h"

#include "export_framerate.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "utils.h"
#include "video_context.h"

#include <libaegisub/of_type_adaptor.h>

#include <utility>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

AssTransformFramerateFilter::AssTransformFramerateFilter()
: AssExportFilter(from_wx(_("Transform Framerate")),
	from_wx(_("Transform subtitle times, including those in override tags, from an input framerate to an output framerate.\n\nThis is useful for converting regular time subtitles to VFRaC time subtitles for hardsubbing.\nIt can also be used to convert subtitles to a different speed video, such as NTSC to PAL speedup.")),
	1000)
{
}

void AssTransformFramerateFilter::ProcessSubs(AssFile *subs, wxWindow *) {
	TransformFrameRate(subs);
}

wxWindow *AssTransformFramerateFilter::GetConfigDialogWindow(wxWindow *parent, agi::Context *c) {
	wxWindow *base = new wxPanel(parent, -1);

	LoadSettings(true, c);

	// Input sizer
	wxSizer *InputSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString initialInput;
	wxButton *FromVideo = new wxButton(base,-1,_("From &video"));
	if (Input->IsLoaded()) {
		initialInput = wxString::Format("%2.3f",Input->FPS());
		FromVideo->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
			InputFramerate->SetValue(wxString::Format("%g", c->videoController->FPS().FPS()));
		});
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
	RadioOutputVFR = new wxRadioButton(base,-1,_("V&ariable"),wxDefaultPosition,wxDefaultSize,wxRB_GROUP);
	OutputSizerTop->Add(RadioOutputVFR,0,wxEXPAND,0);

	// Output bottom line
	RadioOutputCFR = new wxRadioButton(base,-1,_("&Constant: "));
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
	Reverse = new wxCheckBox(base,-1,_("&Reverse transformation"));

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

void AssTransformFramerateFilter::LoadSettings(bool is_default, agi::Context *c) {
	this->c = c;

	if (is_default) {
		Input = &c->videoController->VideoFPS();
		Output = &c->videoController->FPS();
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
		else Output = &c->videoController->FPS();

		if (Reverse->IsChecked()) {
			std::swap(Input, Output);
		}
	}
}

/// Truncate a time to centisecond precision
int FORCEINLINE trunc_cs(int time) {
	return (time / 10) * 10;
}

void AssTransformFramerateFilter::TransformTimeTags(std::string const& name, AssOverrideParameter *curParam, void *curData) {
	VariableDataType type = curParam->GetType();
	if (type != VariableDataType::INT && type != VariableDataType::FLOAT) return;

	AssTransformFramerateFilter *instance = static_cast<AssTransformFramerateFilter*>(curData);
	AssDialogue *curDiag = instance->line;

	int parVal = curParam->Get<int>();

	switch (curParam->classification) {
		case AssParameterClass::RELATIVE_TIME_START: {
			int value = instance->ConvertTime(trunc_cs(curDiag->Start) + parVal) - instance->newStart;

			// An end time of 0 is actually the end time of the line, so ensure
			// nonzero is never converted to 0
			// Needed here rather than the end case because start/end here mean
			// which end of the line the time is relative to, not whether it's
			// the start or end time (compare \move and \fad)
			if (value == 0 && parVal != 0) value = 1;
			curParam->Set(value);
			break;
		}
		case AssParameterClass::RELATIVE_TIME_END:
			curParam->Set(instance->newEnd - instance->ConvertTime(trunc_cs(curDiag->End) - parVal));
			break;
		case AssParameterClass::KARAOKE: {
			int start = curDiag->Start / 10 + instance->oldK + parVal;
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
	for (auto curDialogue : subs->Line | agi::of_type<AssDialogue>()) {
		line = curDialogue;
		newK = 0;
		oldK = 0;
		newStart = trunc_cs(ConvertTime(curDialogue->Start));
		newEnd = trunc_cs(ConvertTime(curDialogue->End) + 9);

		// Process stuff
		boost::ptr_vector<AssDialogueBlock> blocks;
		for (auto block : blocks | agi::of_type<AssDialogueBlockOverride>())
			block->ProcessParameters(TransformTimeTags, this);
		curDialogue->Start = newStart;
		curDialogue->End = newEnd;
		curDialogue->UpdateText(blocks);
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
