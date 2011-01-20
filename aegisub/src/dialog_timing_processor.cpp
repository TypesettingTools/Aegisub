// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file dialog_timing_processor.cpp
/// @brief Timing Post-processor dialogue box and logic
/// @ingroup tools_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "dialog_timing_processor.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_grid.h"
#include "utils.h"
#include "validators.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"

/// Window IDs
enum {
	CHECK_ENABLE_LEADIN = 1850,
	CHECK_ENABLE_LEADOUT,
	CHECK_ENABLE_KEYFRAME,
	CHECK_ENABLE_ADJASCENT,
	BUTTON_SELECT_ALL,
	BUTTON_SELECT_NONE,
	TIMING_STYLE_LIST
};

DialogTimingProcessor::DialogTimingProcessor(agi::Context *c)
: wxDialog(c->parent,-1,_("Timing Post-Processor"),wxDefaultPosition,wxSize(400,250),wxDEFAULT_DIALOG_STYLE)
, c(c)
{
	SetIcon(BitmapToIcon(GETIMAGE(timing_processor_toolbutton_24)));

	// Set variables
	wxString leadInTime(wxString::Format("%d", OPT_GET("Audio/Lead/IN")->GetInt()));
	wxString leadOutTime(wxString::Format("%d", OPT_GET("Audio/Lead/OUT")->GetInt()));
	wxString thresStartBefore(wxString::Format("%d", OPT_GET("Tool/Timing Post Processor/Threshold/Key Start Before")->GetInt()));
	wxString thresStartAfter(wxString::Format("%d", OPT_GET("Tool/Timing Post Processor/Threshold/Key Start After")->GetInt()));
	wxString thresEndBefore(wxString::Format("%d", OPT_GET("Tool/Timing Post Processor/Threshold/Key End Before")->GetInt()));
	wxString thresEndAfter(wxString::Format("%d", OPT_GET("Tool/Timing Post Processor/Threshold/Key End After")->GetInt()));
	wxString adjsThresTime(wxString::Format("%d", OPT_GET("Tool/Timing Post Processor/Threshold/Adjacent")->GetInt()));

// Styles box
	wxSizer *LeftSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Apply to styles"));
	wxArrayString styles = c->ass->GetStyles();
	StyleList = new wxCheckListBox(this,TIMING_STYLE_LIST,wxDefaultPosition,wxSize(150,150),styles);
	StyleList->SetToolTip(_("Select styles to process. Unchecked ones will be ignored."));
	wxButton *all = new wxButton(this,BUTTON_SELECT_ALL,_("All"));
	all->SetToolTip(_("Select all styles."));
	wxButton *none = new wxButton(this,BUTTON_SELECT_NONE,_("None"));
	none->SetToolTip(_("Deselect all styles."));

	// Options box
	wxSizer *optionsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	onlySelection = new wxCheckBox(this,-1,_("Affect selection only"));
	onlySelection->SetValue(OPT_GET("Tool/Timing Post Processor/Only Selection")->GetBool());
	optionsSizer->Add(onlySelection,1,wxALL,0);

	// Lead-in/out box
	wxSizer *LeadSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Lead-in/Lead-out"));
	hasLeadIn = new wxCheckBox(this,CHECK_ENABLE_LEADIN,_("Add lead in:"));
	hasLeadIn->SetToolTip(_("Enable adding of lead-ins to lines."));
	hasLeadIn->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Lead/IN")->GetBool());
	leadIn = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(80,-1),0,NumValidator(leadInTime));
	leadIn->SetToolTip(_("Lead in to be added, in milliseconds."));
	hasLeadOut = new wxCheckBox(this,CHECK_ENABLE_LEADOUT,_("Add lead out:"));
	hasLeadOut->SetToolTip(_("Enable adding of lead-outs to lines."));
	hasLeadOut->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Lead/OUT")->GetBool());
	leadOut = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(80,-1),0,NumValidator(leadOutTime));
	leadOut->SetToolTip(_("Lead out to be added, in milliseconds."));
	LeadSizer->Add(hasLeadIn,0,wxRIGHT|wxEXPAND,5);
	LeadSizer->Add(leadIn,0,wxRIGHT|wxEXPAND,5);
	LeadSizer->Add(hasLeadOut,0,wxRIGHT|wxEXPAND,5);
	LeadSizer->Add(leadOut,0,wxRIGHT|wxEXPAND,0);
	LeadSizer->AddStretchSpacer(1);

	// Adjacent subs sizer
	wxSizer *AdjacentSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Make adjacent subtitles continuous"));
	adjsEnable = new wxCheckBox(this,CHECK_ENABLE_ADJASCENT,_("Enable"));
	adjsEnable->SetToolTip(_("Enable snapping of subtitles together if they are within a certain distance of each other."));
	adjsEnable->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Adjacent")->GetBool());
	wxStaticText *adjsThresText = new wxStaticText(this,-1,_("Threshold:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	adjacentThres = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(adjsThresTime));
	adjacentThres->SetToolTip(_("Maximum difference between start and end time for two subtitles to be made continuous, in milliseconds."));
	adjacentBias = new wxSlider(this,-1,mid(0,int(OPT_GET("Tool/Timing Post Processor/Adjacent Bias")->GetDouble()*100),100),0,100,wxDefaultPosition,wxSize(-1,20));
	adjacentBias->SetToolTip(_("Sets how to set the adjoining of lines. If set totally to left, it will extend start time of the second line; if totally to right, it will extend the end time of the first line."));
	AdjacentSizer->Add(adjsEnable,0,wxRIGHT|wxEXPAND,10);
	AdjacentSizer->Add(adjsThresText,0,wxRIGHT|wxALIGN_CENTER,5);
	AdjacentSizer->Add(adjacentThres,0,wxRIGHT|wxEXPAND,5);
	AdjacentSizer->Add(new wxStaticText(this,-1,_("Bias: Start <- "),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE),0,wxALIGN_CENTER,0);
	AdjacentSizer->Add(adjacentBias,1,wxEXPAND,0);
	AdjacentSizer->Add(new wxStaticText(this,-1,_(" -> End"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE),0,wxALIGN_CENTER,0);

	// Keyframes sizer
	KeyframesSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Keyframe snapping"));
	wxSizer *KeyframesFlexSizer = new wxFlexGridSizer(2,5,5,0);
	keysEnable = new wxCheckBox(this,CHECK_ENABLE_KEYFRAME,_("Enable"));
	keysEnable->SetToolTip(_("Enable snapping of subtitles to nearest keyframe, if distance is within threshold."));
	keysEnable->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Keyframe")->GetBool());
	wxStaticText *textStartBefore = new wxStaticText(this,-1,_("Starts before thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysStartBefore = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(thresStartBefore));
	keysStartBefore->SetToolTip(_("Threshold for 'before start' distance, that is, how many frames a subtitle must start before a keyframe to snap to it."));
	wxStaticText *textStartAfter = new wxStaticText(this,-1,_("Starts after thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysStartAfter = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(thresStartAfter));
	keysStartAfter->SetToolTip(_("Threshold for 'after start' distance, that is, how many frames a subtitle must start after a keyframe to snap to it."));
	wxStaticText *textEndBefore = new wxStaticText(this,-1,_("Ends before thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysEndBefore = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(thresEndBefore));
	keysEndBefore->SetToolTip(_("Threshold for 'before end' distance, that is, how many frames a subtitle must end before a keyframe to snap to it."));
	wxStaticText *textEndAfter = new wxStaticText(this,-1,_("Ends after thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysEndAfter = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(thresEndAfter));
	keysEndAfter->SetToolTip(_("Threshold for 'after end' distance, that is, how many frames a subtitle must end after a keyframe to snap to it."));
	KeyframesFlexSizer->Add(keysEnable,0,wxRIGHT|wxEXPAND,10);
	KeyframesFlexSizer->Add(textStartBefore,0,wxRIGHT|wxALIGN_CENTER,5);
	KeyframesFlexSizer->Add(keysStartBefore,0,wxRIGHT|wxEXPAND,5);
	KeyframesFlexSizer->Add(textStartAfter,0,wxRIGHT|wxALIGN_CENTER,5);
	KeyframesFlexSizer->Add(keysStartAfter,0,wxRIGHT|wxEXPAND,0);
	KeyframesFlexSizer->AddStretchSpacer(1);
	KeyframesFlexSizer->Add(textEndBefore,0,wxRIGHT|wxALIGN_CENTER,5);
	KeyframesFlexSizer->Add(keysEndBefore,0,wxRIGHT|wxEXPAND,5);
	KeyframesFlexSizer->Add(textEndAfter,0,wxRIGHT|wxALIGN_CENTER,5);
	KeyframesFlexSizer->Add(keysEndAfter,0,wxRIGHT|wxEXPAND,0);
	KeyframesSizer->Add(KeyframesFlexSizer,0,wxEXPAND);
	KeyframesSizer->AddStretchSpacer(1);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ApplyButton = new wxButton(this,wxID_OK);
	ButtonSizer->AddButton(ApplyButton);
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Timing Processor")));
	ButtonSizer->Realize();

	// Right Sizer
	wxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
	RightSizer->Add(optionsSizer,0,wxBOTTOM|wxEXPAND,5);
	RightSizer->Add(LeadSizer,0,wxBOTTOM|wxEXPAND,5);
	RightSizer->Add(AdjacentSizer,0,wxBOTTOM|wxEXPAND,5);
	RightSizer->Add(KeyframesSizer,0,wxBOTTOM|wxEXPAND,5);
	RightSizer->AddStretchSpacer(1);
	RightSizer->Add(ButtonSizer,0,wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND,0);

	// Style buttons sizer
	wxSizer *StyleButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	StyleButtonsSizer->Add(all,1,0,0);
	StyleButtonsSizer->Add(none,1,0,0);

	// Left sizer
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		StyleList->Check(i);
	}
	LeftSizer->Add(StyleList,1,wxBOTTOM|wxEXPAND,0);
	LeftSizer->Add(StyleButtonsSizer,0,wxEXPAND,0);

	// Top Sizer
	wxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(LeftSizer,0,wxRIGHT|wxEXPAND,5);
	TopSizer->Add(RightSizer,1,wxALL|wxEXPAND,0);

	// Main Sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,1,wxALL|wxEXPAND,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);

	CenterOnParent();

	UpdateControls();
}

/// @brief Update controls 
///
void DialogTimingProcessor::UpdateControls() {
	// Boxes
	leadIn->Enable(hasLeadIn->IsChecked());
	leadOut->Enable(hasLeadOut->IsChecked());
	adjacentThres->Enable(adjsEnable->IsChecked());
	adjacentBias->Enable(adjsEnable->IsChecked());

	// Keyframes are only available if timecodes are loaded
	bool keysAvailable = VideoContext::Get()->KeyFramesLoaded();
	bool enableKeys = keysEnable->IsChecked() && keysAvailable;
	keysStartBefore->Enable(enableKeys);
	keysStartAfter->Enable(enableKeys);
	keysEndBefore->Enable(enableKeys);
	keysEndAfter->Enable(enableKeys);
	if (!keysAvailable) {
		keysEnable->SetValue(false);
		keysEnable->Enable(false);
	}

	// Apply button
	int checked = 0;
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		if (StyleList->IsChecked(i)) checked++;
	}
	ApplyButton->Enable(checked && (hasLeadIn->IsChecked() | hasLeadOut->IsChecked() | keysEnable->IsChecked() | adjsEnable->IsChecked()));
}

BEGIN_EVENT_TABLE(DialogTimingProcessor,wxDialog)
	EVT_CHECKBOX(CHECK_ENABLE_LEADIN,DialogTimingProcessor::OnCheckBox)
	EVT_CHECKBOX(CHECK_ENABLE_LEADOUT,DialogTimingProcessor::OnCheckBox)
	EVT_CHECKBOX(CHECK_ENABLE_KEYFRAME,DialogTimingProcessor::OnCheckBox)
	EVT_CHECKBOX(CHECK_ENABLE_ADJASCENT,DialogTimingProcessor::OnCheckBox)
	EVT_CHECKLISTBOX(TIMING_STYLE_LIST,DialogTimingProcessor::OnCheckBox)
	EVT_BUTTON(wxID_OK,DialogTimingProcessor::OnApply)
	EVT_BUTTON(BUTTON_SELECT_ALL,DialogTimingProcessor::OnSelectAll)
	EVT_BUTTON(BUTTON_SELECT_NONE,DialogTimingProcessor::OnSelectNone)
END_EVENT_TABLE()

void DialogTimingProcessor::OnCheckBox(wxCommandEvent &) {
	UpdateControls();
}

void DialogTimingProcessor::OnSelectAll(wxCommandEvent &) {
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		StyleList->Check(i);
	}
	UpdateControls();
}

void DialogTimingProcessor::OnSelectNone(wxCommandEvent &) {
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		StyleList->Check(i,false);
	}
	UpdateControls();
}

void DialogTimingProcessor::OnApply(wxCommandEvent &) {
	// Save settings
	long temp = 0;
	leadIn->GetValue().ToLong(&temp);
	OPT_SET("Audio/Lead/IN")->SetInt(temp);
	leadOut->GetValue().ToLong(&temp);
	OPT_SET("Audio/Lead/OUT")->SetInt(temp);
	keysStartBefore->GetValue().ToLong(&temp);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key Start Before")->SetInt(temp);
	keysStartAfter->GetValue().ToLong(&temp);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key Start After")->SetInt(temp);
	keysEndBefore->GetValue().ToLong(&temp);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key End Before")->SetInt(temp);
	keysEndAfter->GetValue().ToLong(&temp);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key End After")->SetInt(temp);
	adjacentThres->GetValue().ToLong(&temp);
	OPT_SET("Tool/Timing Post Processor/Threshold/Adjacent")->SetInt(temp);
	OPT_SET("Tool/Timing Post Processor/Adjacent Bias")->SetDouble(adjacentBias->GetValue() / 100.0);
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/IN")->SetBool(hasLeadIn->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/OUT")->SetBool(hasLeadOut->IsChecked());
	if (keysEnable->IsEnabled()) OPT_SET("Tool/Timing Post Processor/Enable/Keyframe")->SetBool(keysEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Adjacent")->SetBool(adjsEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Only Selection")->SetBool(onlySelection->IsChecked());

	// Check if rows are valid
	for (entryIter cur = c->ass->Line.begin(); cur != c->ass->Line.end(); ++cur) {
		if (AssDialogue *tempDiag = dynamic_cast<AssDialogue*>(*cur)) {
			if (tempDiag->Start.GetMS() > tempDiag->End.GetMS()) {
				wxMessageBox(
					wxString::Format(
						_("One of the lines in the file (%i) has negative duration. Aborting."),
						std::distance(c->ass->Line.begin(), cur)),
					_("Invalid script"),
					wxICON_ERROR|wxOK);
				EndModal(0);
				return;
			}
		}
	}

	Process();
	EndModal(0);
}

int DialogTimingProcessor::GetClosestKeyFrame(int frame) {
	std::vector<int>::iterator pos = lower_bound(KeyFrames.begin(), KeyFrames.end(), frame);
	if (distance(pos, KeyFrames.end()) < 2) return KeyFrames.back();
	return frame - *pos < *(pos + 1) - frame ? *pos : *(pos + 1);
}

static bool bad_line(std::set<wxString> *styles, AssDialogue *d) {
	return !d || d->Comment || styles->find(d->Style) == styles->end();
}

void DialogTimingProcessor::SortDialogues() {
	std::set<wxString> styles;
	for (size_t i = 0; i < StyleList->GetCount(); ++i) {
		if (StyleList->IsChecked(i)) {
			styles.insert(StyleList->GetString(i));
		}
	}

	Sorted.clear();
	Sorted.reserve(c->ass->Line.size());
	if (onlySelection->IsChecked()) {
		SelectionController<AssDialogue>::Selection sel = c->selectionController->GetSelectedSet();
		remove_copy_if(sel.begin(), sel.end(), back_inserter(Sorted),
			bind(bad_line, &styles, std::tr1::placeholders::_1));
	}
	else {
		std::vector<AssDialogue*> tmp(c->ass->Line.size());
		transform(c->ass->Line.begin(), c->ass->Line.end(), back_inserter(tmp), cast<AssDialogue*>());
		remove_copy_if(tmp.begin(), tmp.end(), back_inserter(Sorted),
			bind(bad_line, &styles, std::tr1::placeholders::_1));
	}
	sort(Sorted.begin(), Sorted.end(), AssFile::CompStart);
}

/// @brief Actually process subtitles 
///
void DialogTimingProcessor::Process() {
	SortDialogues();
	int rows = Sorted.size();

	// Options
	long inVal = 0;
	long outVal = 0;
	leadIn->GetValue().ToLong(&inVal);
	leadOut->GetValue().ToLong(&outVal);
	bool addIn = hasLeadIn->IsChecked() && inVal;
	bool addOut = hasLeadOut->IsChecked() && outVal;

	// Add lead-in/out
	if (addIn || addOut) {
		for (int i=0;i<rows;i++) {
			AssDialogue *cur = Sorted[i];

			// Compare to every previous line (yay for O(n^2)!) to see if it's OK to add lead-in
			if (inVal) {
				int startLead = cur->Start.GetMS() - inVal;
				for (int j=0;j<i;j++) {
					AssDialogue *comp = Sorted[j];

					// Check if they don't already collide (ignore it in that case)
					if (cur->CollidesWith(comp)) continue;

					// Get comparison times
					startLead = std::max(startLead, comp->End.GetMS());
				}
				cur->Start.SetMS(startLead);
			}

			// Compare to every line to see how far can lead-out be extended
			if (outVal) {
				int endLead = cur->End.GetMS() + outVal;
				for (int j=i+1;j<rows;j++) {
					AssDialogue *comp = Sorted[j];

					// Check if they don't already collide (ignore it in that case)
					if (cur->CollidesWith(comp)) continue;

					// Get comparison times
					endLead = std::min(endLead, comp->Start.GetMS());
				}
				cur->End.SetMS(endLead);
			}
		}
	}

	// Make adjacent
	if (adjsEnable->IsChecked()) {
		AssDialogue *prev = Sorted.front();
		
		long adjsThres = 0;
		adjacentThres->GetValue().ToLong(&adjsThres);

		float bias = adjacentBias->GetValue() / 100.0;

		for (int i=1; i < rows;i++) {
			AssDialogue *cur = Sorted[i];

			// Check if they don't collide
			if (cur->CollidesWith(prev)) continue;

			// Compare distance
			int curStart = cur->Start.GetMS();
			int prevEnd = prev->End.GetMS();
			int dist = curStart-prevEnd;
			if (dist > 0 && dist < adjsThres) {
				int setPos = prevEnd+int(dist*bias);
				cur->Start.SetMS(setPos);
				prev->End.SetMS(setPos);
			}

			prev = cur;
		}
	}

	// Keyframe snapping
	if (keysEnable->IsChecked()) {
		KeyFrames = c->videoController->GetKeyFrames();
		KeyFrames.push_back(c->videoController->GetLength() - 1);

		long beforeStart = 0;
		long afterStart = 0;
		long beforeEnd = 0;
		long afterEnd = 0;
		keysStartBefore->GetValue().ToLong(&beforeStart);
		keysStartAfter->GetValue().ToLong(&afterStart);
		keysEndBefore->GetValue().ToLong(&beforeEnd);
		keysEndAfter->GetValue().ToLong(&afterEnd);
		
		for (int i=0;i<rows;i++) {
			AssDialogue *cur = Sorted[i];

			// Get start/end frames
			int startF = c->videoController->FrameAtTime(cur->Start.GetMS(),agi::vfr::START);
			int endF = c->videoController->FrameAtTime(cur->End.GetMS(),agi::vfr::END);

			// Get closest for start
			int closest = GetClosestKeyFrame(startF);
			if ((closest > startF && closest-startF <= beforeStart) || (closest < startF && startF-closest <= afterStart)) {
				cur->Start.SetMS(c->videoController->TimeAtFrame(closest,agi::vfr::START));
			}

			// Get closest for end
			closest = GetClosestKeyFrame(endF)-1;
			if ((closest > endF && closest-endF <= beforeEnd) || (closest < endF && endF-closest <= afterEnd)) {
				cur->End.SetMS(c->videoController->TimeAtFrame(closest,agi::vfr::END));
			}
		}
	}

	// Update grid
	c->ass->Commit(_("timing processor"), AssFile::COMMIT_TIMES);
}
