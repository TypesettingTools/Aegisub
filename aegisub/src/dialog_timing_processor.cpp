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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

////////////
// Includes
#include "config.h"

#include "dialog_timing_processor.h"
#include "subs_grid.h"
#include "ass_file.h"
#include "options.h"
#include "validators.h"
#include "video_display.h"
#include "video_box.h"
#include "ass_dialogue.h"
#include "ass_time.h"
#include "vfr.h"
#include "utils.h"
#include "help_button.h"


///////////////
// Constructor
DialogTimingProcessor::DialogTimingProcessor(wxWindow *parent,SubtitlesGrid *_grid)
: wxDialog(parent,-1,_("Timing Post-Processor"),wxDefaultPosition,wxSize(400,250),wxDEFAULT_DIALOG_STYLE)
{
	// Set icon
	SetIcon(BitmapToIcon(wxBITMAP(timing_processor_toolbutton)));

	// Set variables
	grid = _grid;
	leadInTime = Options.AsText(_T("Audio lead in"));
	leadOutTime = Options.AsText(_T("Audio lead out"));
	thresStartBefore = Options.AsText(_T("Timing processor key start before thres"));
	thresStartAfter = Options.AsText(_T("Timing processor key start after thres"));
	thresEndBefore = Options.AsText(_T("Timing processor key end before thres"));
	thresEndAfter = Options.AsText(_T("Timing processor key end after thres"));
	adjsThresTime = Options.AsText(_T("Timing processor adjacent thres"));

	// Styles box
	wxSizer *LeftSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Apply to styles"));
	wxArrayString styles = grid->ass->GetStyles();
	StyleList = new wxCheckListBox(this,TIMING_STYLE_LIST,wxDefaultPosition,wxSize(150,150),styles);
	StyleList->SetToolTip(_("Select styles to process. Unchecked ones will be ignored."));
	wxButton *all = new wxButton(this,BUTTON_SELECT_ALL,_("All"));
	all->SetToolTip(_("Select all styles."));
	wxButton *none = new wxButton(this,BUTTON_SELECT_NONE,_("None"));
	none->SetToolTip(_("Deselect all styles."));

	// Options box
	wxSizer *optionsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	onlySelection = new wxCheckBox(this,-1,_("Affect selection only"));
	onlySelection->SetValue(Options.AsBool(_T("Timing processor Only Selection")));
	optionsSizer->Add(onlySelection,1,wxALL,0);

	// Lead-in/out box
	wxSizer *LeadSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Lead-in/Lead-out"));
	hasLeadIn = new wxCheckBox(this,CHECK_ENABLE_LEADIN,_("Add lead in:"));
	hasLeadIn->SetToolTip(_("Enable adding of lead-ins to lines."));
	hasLeadIn->SetValue(Options.AsBool(_T("Timing processor Enable lead-in")));
	leadIn = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(80,-1),0,NumValidator(&leadInTime));
	leadIn->SetToolTip(_("Lead in to be added, in milliseconds."));
	hasLeadOut = new wxCheckBox(this,CHECK_ENABLE_LEADOUT,_("Add lead out:"));
	hasLeadOut->SetToolTip(_("Enable adding of lead-outs to lines."));
	hasLeadOut->SetValue(Options.AsBool(_T("Timing processor Enable lead-out")));
	leadOut = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(80,-1),0,NumValidator(&leadOutTime));
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
	adjsEnable->SetValue(Options.AsBool(_T("Timing processor Enable adjacent")));
	wxStaticText *adjsThresText = new wxStaticText(this,-1,_("Threshold:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	adjacentThres = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(&adjsThresTime));
	adjacentThres->SetToolTip(_("Maximum difference between start and end time for two subtitles to be made continuous, in milliseconds."));
	adjacentBias = new wxSlider(this,-1,MID(0,int(Options.AsFloat(_T("Timing processor adjacent bias"))*100),100),0,100,wxDefaultPosition,wxSize(-1,20));
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
	keysEnable->SetValue(Options.AsBool(_T("Timing processor Enable keyframe")));
	wxStaticText *textStartBefore = new wxStaticText(this,-1,_("Starts before thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysStartBefore = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(&thresStartBefore));
	keysStartBefore->SetToolTip(_("Threshold for 'before start' distance, that is, how many frames a subtitle must start before a keyframe to snap to it."));
	wxStaticText *textStartAfter = new wxStaticText(this,-1,_("Starts after thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysStartAfter = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(&thresStartAfter));
	keysStartAfter->SetToolTip(_("Threshold for 'after start' distance, that is, how many frames a subtitle must start after a keyframe to snap to it."));
	wxStaticText *textEndBefore = new wxStaticText(this,-1,_("Ends before thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysEndBefore = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(&thresEndBefore));
	keysEndBefore->SetToolTip(_("Threshold for 'before end' distance, that is, how many frames a subtitle must end before a keyframe to snap to it."));
	wxStaticText *textEndAfter = new wxStaticText(this,-1,_("Ends after thres.:"),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
	keysEndAfter = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(60,-1),0,NumValidator(&thresEndAfter));
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

	// Update
	UpdateControls();
}


///////////////////
// Update controls
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


///////////////
// Event table
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


////////////////////
// Checkbox clicked
void DialogTimingProcessor::OnCheckBox(wxCommandEvent &event) {
	UpdateControls();
}


/////////////////////
// Select all styles
void DialogTimingProcessor::OnSelectAll(wxCommandEvent &event) {
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		StyleList->Check(i);
	}
	UpdateControls();
}


///////////////////////
// Unselect all styles
void DialogTimingProcessor::OnSelectNone(wxCommandEvent &event) {
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		StyleList->Check(i,false);
	}
	UpdateControls();
}


////////////////////////
// Apply button pressed
void DialogTimingProcessor::OnApply(wxCommandEvent &event) {
	// Save settings
	long temp = 0;
	leadIn->GetValue().ToLong(&temp);
	Options.SetInt(_T("Audio lead in"),temp);
	leadOut->GetValue().ToLong(&temp);
	Options.SetInt(_T("Audio lead out"),temp);
	keysStartBefore->GetValue().ToLong(&temp);
	Options.SetInt(_T("Timing processor key start before thres"),temp);
	keysStartAfter->GetValue().ToLong(&temp);
	Options.SetInt(_T("Timing processor key start after thres"),temp);
	keysEndBefore->GetValue().ToLong(&temp);
	Options.SetInt(_T("Timing processor key end before thres"),temp);
	keysEndAfter->GetValue().ToLong(&temp);
	Options.SetInt(_T("Timing processor key end after thres"),temp);
	adjacentThres->GetValue().ToLong(&temp);
	Options.SetInt(_T("Timing processor adjacent thres"),temp);
	Options.SetFloat(_T("Timing processor adjacent bias"),adjacentBias->GetValue() / 100.0);
	Options.SetBool(_T("Timing processor Enable lead-in"),hasLeadIn->IsChecked());
	Options.SetBool(_T("Timing processor Enable lead-out"),hasLeadOut->IsChecked());
	if (keysEnable->IsEnabled()) Options.SetBool(_T("Timing processor Enable keyframe"),keysEnable->IsChecked());
	Options.SetBool(_T("Timing processor Enable adjacent"),adjsEnable->IsChecked());
	Options.SetBool(_T("Timing processor Only Selection"),onlySelection->IsChecked());
	Options.Save();

	// Check if rows are valid
	bool valid = true;
	AssDialogue *tempDiag;
	int i = 0;
	for (std::list<AssEntry*>::iterator cur=grid->ass->Line.begin();cur!=grid->ass->Line.end();cur++) {
		tempDiag = AssEntry::GetAsDialogue(*cur);
		if (tempDiag) {
			i++;
			if (tempDiag->Start.GetMS() > tempDiag->End.GetMS()) {
				valid = false;
				break;
			}
		}
	}

	// Process
	if (valid) Process();

	// Error message
	else wxMessageBox(wxString::Format(_("One of the lines in the file (%i) has negative duration. Aborting."),i),_("Invalid script"),wxICON_ERROR|wxOK);

	// Close dialogue
	EndModal(0);
}


////////////////////////
// Get closest keyframe
int DialogTimingProcessor::GetClosestKeyFrame(int frame) {
	// Linear dumb search, not very efficient, but it doesn't really matter
	int closest = 0;
	size_t n = KeyFrames.Count();
	for (size_t i=0;i<n;i++) {
		if (abs(KeyFrames[i]-frame) < abs(closest-frame)) {
			closest = KeyFrames[i];
		}
	}
	return closest;
}


////////////////////////////
// Check if style is listed
bool DialogTimingProcessor::StyleOK(wxString styleName) {
	size_t len = StyleList->GetCount();
	for (size_t i=0;i<len;i++) {
		if (StyleList->GetString(i) == styleName && StyleList->IsChecked(i)) return true;
	}
	return false;
}


//////////////////
// Sort dialogues
void DialogTimingProcessor::SortDialogues() {
	// Copy from original to temporary list
	std::list<AssDialogue*> temp;
	AssDialogue *tempDiag;
	int count = grid->GetRows();
	for (int i=0;i<count;i++) {
		tempDiag = grid->GetDialogue(i);
		if (tempDiag && StyleOK(tempDiag->Style) && !tempDiag->Comment) {
			if (!onlySelection->IsChecked() || grid->IsInSelection(i)) {
				tempDiag->FixStartMS();
				temp.push_back(tempDiag);
			}
		}
	}

	// Sort temporary list
	temp.sort(LessByPointedToValue<AssDialogue>());

	// Copy temporary list to final vector
	Sorted.clear();
	for (std::list<AssDialogue*>::iterator cur=temp.begin();cur!=temp.end();cur++) {
		Sorted.push_back(*cur);
	}
}


////////////////////////
// Gets sorted dialogue
AssDialogue *DialogTimingProcessor::GetSortedDialogue(int n) {
	try {
		return Sorted.at(n);
	}
	catch (...) {
		return NULL;
	}
}


//////////////////////////////
// Actually process subtitles
void DialogTimingProcessor::Process() {
	// Sort rows
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
		// Variables
		AssDialogue *cur;
		AssDialogue *comp;
		int start,end;
		int startLead,endLead;
		int compStart,compEnd;

		// For each row
		for (int i=0;i<rows;i++) {
			// Get line and check if it's OK
			cur = GetSortedDialogue(i);

			// Set variables
			start = cur->Start.GetMS();
			end = cur->End.GetMS();
			if (addIn) startLead = start - inVal;
			else startLead = start;
			if (addOut) endLead = end + outVal;
			else endLead = end;

			// Compare to every previous line (yay for O(n^2)!) to see if it's OK to add lead-in
			if (addIn) {
				for (int j=0;j<i;j++) {
					comp = GetSortedDialogue(j);

					// Check if they don't already collide (ignore it in that case)
					if (cur->CollidesWith(comp)) continue;

					// Get comparison times
					compEnd = comp->End.GetMS();

					// Limit lead-in if needed
					if (compEnd > startLead) startLead = compEnd;
				}
			}

			// Compare to every line to see how far can lead-out be extended
			if (addOut) {
				for (int j=i+1;j<rows;j++) {
					comp = GetSortedDialogue(j);

					// Check if they don't already collide (ignore it in that case)
					if (cur->CollidesWith(comp)) continue;

					// Get comparison times
					compStart = comp->Start.GetMS();

					// Limit lead-in if needed
					if (compStart < endLead) endLead = compStart;
				}
			}

			// Set times
			cur->Start.SetMS(startLead);
			cur->End.SetMS(endLead);
			cur->UpdateData();
		}
	}

	// Make adjacent
	if (adjsEnable->IsChecked()) {
		// Variables
		AssDialogue *cur;
		AssDialogue *prev = NULL;
		int curStart,prevEnd;
		long adjsThres = 0;
		int dist;

		// Get threshold
		adjacentThres->GetValue().ToLong(&adjsThres);

		// Get bias
		float bias = adjacentBias->GetValue() / 100.0;

		// For each row
		for (int i=0;i<rows;i++) {
			// Get line and check if it's OK
			cur = GetSortedDialogue(i);

			// Check if previous is OK
			if (!prev) {
				prev = cur;
				continue;
			}

			// Check if they don't collide
			if (cur->CollidesWith(prev)) continue;

			// Compare distance
			curStart = cur->Start.GetMS();
			prevEnd = prev->End.GetMS();
			dist = curStart-prevEnd;
			if (dist > 0 && dist < adjsThres) {
				int setPos = prevEnd+int(dist*bias);
				cur->Start.SetMS(setPos);
				cur->UpdateData();
				prev->End.SetMS(setPos);
				prev->UpdateData();
			}

			// Set previous
			prev = cur;
		}
	}

	// Keyframe snapping
	if (keysEnable->IsChecked()) {
		// Get keyframes
		KeyFrames = VideoContext::Get()->GetKeyFrames();
		KeyFrames.Add(VideoContext::Get()->GetLength()-1);

		// Variables
		int startF,endF;
		int closest;
		bool changed;
		AssDialogue *cur;

		// Get variables
		long beforeStart = 0;
		long afterStart = 0;
		long beforeEnd = 0;
		long afterEnd = 0;
		keysStartBefore->GetValue().ToLong(&beforeStart);
		keysStartAfter->GetValue().ToLong(&afterStart);
		keysEndBefore->GetValue().ToLong(&beforeEnd);
		keysEndAfter->GetValue().ToLong(&afterEnd);
		
		// For each row
		for (int i=0;i<rows;i++) {
			// Get line and check if it's OK
			cur = GetSortedDialogue(i);

			// Get start/end frames
			startF = VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true);
			endF = VFR_Output.GetFrameAtTime(cur->End.GetMS(),false);
			changed = false;

			// Get closest for start
			closest = GetClosestKeyFrame(startF);
			if ((closest > startF && closest-startF <= beforeStart) || (closest < startF && startF-closest <= afterStart)) {
				cur->Start.SetMS(VFR_Output.GetTimeAtFrame(closest,true));
				changed = true;
			}

			// Get closest for end
			closest = GetClosestKeyFrame(endF)-1;
			if ((closest > endF && closest-endF <= beforeEnd) || (closest < endF && endF-closest <= afterEnd)) {
				cur->End.SetMS(VFR_Output.GetTimeAtFrame(closest,false));
				changed = true;
			}

			// Apply changes
			if (changed) {
				cur->UpdateData();
			}
		}
	}

	// Update grid
	grid->ass->FlagAsModified(_("timing processor"));
	grid->CommitChanges();
}
