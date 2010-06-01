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


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "export_framerate.h"
#include "vfr.h"


/// @brief Constructor 
///
AssTransformFramerateFilter::AssTransformFramerateFilter() {
	initialized = false;
}



/// @brief Init 
/// @return 
///
void AssTransformFramerateFilter::Init() {
	if (initialized) return;
	initialized = true;
	autoExporter = true;
	Register(_("Transform Framerate"),1000);
	description = _("Transform subtitle times, including those in override tags, from an input framerate to an output framerate.\n\nThis is useful for converting regular time subtitles to VFRaC time subtitles for hardsubbing.\nIt can also be used to convert subtitles to a different speed video, such as NTSC to PAL speedup.");
	Input = NULL;
	Output = NULL;
}



/// @brief Process 
/// @param subs          
/// @param export_dialog 
///
void AssTransformFramerateFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog) {
	// Transform frame rate
	if (Input->IsLoaded() && Output->IsLoaded()) {
		if (Input->GetFrameRateType() == VFR || Output->GetFrameRateType() == VFR || Output->GetAverage() != Input->GetAverage()) {
			TransformFrameRate(subs);
		}
	}
}



/// @brief Get dialog 
/// @param parent 
/// @return 
///
wxWindow *AssTransformFramerateFilter::GetConfigDialogWindow(wxWindow *parent) {
	wxWindow *base = new wxPanel(parent, -1);

	// Input sizer
	wxSizer *InputSizer = new wxBoxSizer(wxHORIZONTAL);
	wxString initialInput;
	wxButton *FromVideo = new wxButton(base,Get_Input_From_Video,_("From Video"));
	if (VFR_Input.IsLoaded()) initialInput = wxString::Format(_T("%2.3f"),VFR_Input.GetAverage());
	else {
		initialInput = _T("23.976");
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
	if (VFR_Output.GetFrameRateType() != VFR) {
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



/// @brief Load settings 
/// @param IsDefault 
///
void AssTransformFramerateFilter::LoadSettings(bool IsDefault) {
	if (IsDefault) {
		Input = &VFR_Input;
		Output = &VFR_Output;
	}
	else {
		double temp;
		InputFramerate->GetValue().ToDouble(&temp);
		t1.SetCFR(temp);
		Input = &t1;
		if (RadioOutputCFR->GetValue()) {
			OutputFramerate->GetValue().ToDouble(&temp);
			t2.SetCFR(temp);
			Output = &t2;
		}
		else Output = &VFR_Output;

		// Reverse
		if (Reverse->IsChecked()) {
			FrameRate *temp = Output;
			Output = Input;
			Input = temp;
		}
	}
}



/// @brief Transform framerate in tags 
/// @param name     
/// @param n        
/// @param curParam 
/// @param curData  
/// @return 
///
void AssTransformFramerateFilter::TransformTimeTags (wxString name,int n,AssOverrideParameter *curParam,void *curData) {
	// Only modify anything if this is a number
	VariableDataType type = curParam->GetType();
	if (type != VARDATA_INT && type != VARDATA_FLOAT) return;

	// Setup
	LineData *lineData = (LineData*) curData;
	AssDialogue *curDiag = lineData->line;;
	bool start = true;
	bool karaoke = false;
	int mult = 1;
	int value;
	switch (curParam->classification) {
		case PARCLASS_RELATIVE_TIME_START:
			break;
		case PARCLASS_RELATIVE_TIME_END:
			start = false;
			break;
		case PARCLASS_KARAOKE:
			karaoke = true;
			mult = 10;
			break;
		default:
			return;
	}

	// Parameter value
	int parVal = curParam->AsInt() * mult;

	// Karaoke preprocess
	int curKarPos = 0;
	if (karaoke) {
		if (name == _T("\\k")) {
			curKarPos = lineData->k;
			lineData->k += parVal/10;
		}
		else if (name == _T("\\K") || name == _T("\\kf")) {
			curKarPos = lineData->kf;
			lineData->kf += parVal/10;
		}
		else if (name == _T("\\ko")) {
			curKarPos = lineData->ko;
			lineData->ko += parVal/10;
		}
		else throw wxString::Format(_T("Unknown karaoke tag! '%s'"), name.c_str());
		curKarPos *= 10;
		parVal += curKarPos;
	}

	// Start time
	if (start) {
		int newStart = instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(curDiag->Start.GetMS()));
		int absTime = curDiag->Start.GetMS() + parVal;
		value = instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(absTime)) - newStart;
		
		// An end time of 0 is actually the end time of the line, so ensure nonzero is never converted to 0
		// Needed in the start case as well as the end one due to \t, whose end time needs the start time
		// behavior
		if (value == 0 && parVal != 0) value = 1;
	}

	// End time
	else {
		int newEnd = instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(curDiag->End.GetMS()));
		int absTime = curDiag->End.GetMS() - parVal;
		value = newEnd - instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(absTime));
		if (value == 0 && parVal != 0) value = 1;
	}

	// Karaoke postprocess
	if (karaoke) {
		int post = instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(curDiag->Start.GetMS() + curKarPos));
		int start = instance.Input->GetTimeAtFrame(instance.Output->GetFrameAtTime(curDiag->Start.GetMS()));
		curKarPos = post-start;
		value -= curKarPos;
	}

	curParam->SetInt(value/mult);
}



/// @brief Transform framerate 
/// @param subs 
///
void AssTransformFramerateFilter::TransformFrameRate(AssFile *subs) {
	int n=0;

	// Run through
	using std::list;
	AssEntry *curEntry;
	AssDialogue *curDialogue;
	for (entryIter cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		curEntry = *cur;
		// why the christ was this ever done to begin with?
		// yes, let's framerate compensate the start timestamp and then use the changed value to
		// compensate it AGAIN 20 lines down? I DO NOT GET IT
		// -Fluff
		//curEntry->Start.SetMS(Input->GetTimeAtFrame(Output->GetFrameAtTime(curEntry->GetStartMS(),true),true));
		curDialogue = dynamic_cast<AssDialogue*>(curEntry);

		// Update dialogue entries
		if (curDialogue) {
			// Line data
			LineData data;
			data.line = curDialogue;
			data.k = 0;
			data.kf = 0;
			data.ko = 0;

			// Process stuff
			curDialogue->ParseASSTags();
			curDialogue->ProcessParameters(TransformTimeTags,&data);
			curDialogue->Start.SetMS(Input->GetTimeAtFrame(Output->GetFrameAtTime(curDialogue->Start.GetMS(),true),true));
			curDialogue->End.SetMS(Input->GetTimeAtFrame(Output->GetFrameAtTime(curDialogue->End.GetMS(),false),false));
			curDialogue->UpdateText();
			curDialogue->UpdateData();
			curDialogue->ClearBlocks();
			n++;
		}
	}
}



/// DOCME
AssTransformFramerateFilter AssTransformFramerateFilter::instance;


