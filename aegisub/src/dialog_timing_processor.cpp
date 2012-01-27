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

#include "dialog_timing_processor.h"

#ifndef AGI_PRE
#include <algorithm>
#include <tr1/functional>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valnum.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"
#include "utils.h"
#include "video_context.h"

namespace {
using std::tr1::placeholders::_1;

void set_ctrl_state(wxCommandEvent &evt, wxCheckBox *cb, wxTextCtrl *tc) {
	tc->Enable(cb->IsChecked());
	evt.Skip();
}

wxTextCtrl *make_ctrl(wxWindow *parent, wxSizer *sizer, wxString const& desc, int *value, wxCheckBox *cb, wxString const& tooltip) {
	wxIntegerValidator<int> validator(value);
	validator.SetMin(0);
	wxTextCtrl *ctrl = new wxTextCtrl(parent, -1, "", wxDefaultPosition, wxSize(60,-1), 0, validator);
	ctrl->SetToolTip(tooltip);
	if (!desc.empty())
		sizer->Add(new wxStaticText(parent, -1, desc), wxSizerFlags().Center().Border(wxRIGHT));
	sizer->Add(ctrl, wxSizerFlags().Expand().Border(wxRIGHT));

	ctrl->Enable(cb->IsChecked());
	cb->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, bind(set_ctrl_state, _1, cb, ctrl));

	return ctrl;
}

inline wxTextCtrl *make_ctrl(wxStaticBoxSizer *sizer, wxString const& desc, int *value, wxCheckBox *cb, wxString const& tooltip) {
	return make_ctrl(sizer->GetStaticBox()->GetParent(), sizer, desc, value, cb, tooltip);
}

wxCheckBox *make_check(wxStaticBoxSizer *sizer, wxString const& desc, const char *opt, wxString const& tooltip) {
	wxCheckBox *cb = new wxCheckBox(sizer->GetStaticBox()->GetParent(), -1, desc);
	cb->SetToolTip(tooltip);
	cb->SetValue(OPT_GET(opt)->GetBool());
	sizer->Add(cb, wxSizerFlags().Border(wxRIGHT).Expand());
	return cb;
}
}

DialogTimingProcessor::DialogTimingProcessor(agi::Context *c)
: wxDialog(c->parent, -1, _("Timing Post-Processor"))
, c(c)
{
	using std::tr1::bind;

	SetIcon(BitmapToIcon(GETIMAGE(timing_processor_toolbutton_24)));

	// Read options
	leadIn = OPT_GET("Audio/Lead/IN")->GetInt();
	leadOut = OPT_GET("Audio/Lead/OUT")->GetInt();
	beforeStart = OPT_GET("Tool/Timing Post Processor/Threshold/Key Start Before")->GetInt();
	beforeEnd = OPT_GET("Tool/Timing Post Processor/Threshold/Key End Before")->GetInt();
	afterStart = OPT_GET("Tool/Timing Post Processor/Threshold/Key Start After")->GetInt();
	afterEnd = OPT_GET("Tool/Timing Post Processor/Threshold/Key End After")->GetInt();
	adjDistance = OPT_GET("Tool/Timing Post Processor/Threshold/Adjacent")->GetInt();

	// Styles box
	wxSizer *LeftSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Apply to styles"));
	StyleList = new wxCheckListBox(this, -1, wxDefaultPosition, wxSize(150,150), c->ass->GetStyles());
	StyleList->SetToolTip(_("Select styles to process. Unchecked ones will be ignored."));

	wxButton *all = new wxButton(this,-1,_("&All"));
	all->SetToolTip(_("Select all styles."));

	wxButton *none = new wxButton(this,-1,_("&None"));
	none->SetToolTip(_("Deselect all styles."));

	// Options box
	wxStaticBoxSizer *optionsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	onlySelection = new wxCheckBox(this,-1,_("Affect &selection only"));
	onlySelection->SetValue(OPT_GET("Tool/Timing Post Processor/Only Selection")->GetBool());
	optionsSizer->Add(onlySelection,1,wxALL,0);

	// Lead-in/out box
	wxStaticBoxSizer *LeadSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Lead-in/Lead-out"));

	hasLeadIn = make_check(LeadSizer, _("Add lead &in:"),
		"Tool/Timing Post Processor/Enable/Lead/IN",
		_("Enable adding of lead-ins to lines."));
	make_ctrl(LeadSizer, "", &leadIn, hasLeadIn, _("Lead in to be added, in milliseconds."));

	hasLeadOut = make_check(LeadSizer, _("Add lead &out:"),
		"Tool/Timing Post Processor/Enable/Lead/OUT",
		_("Enable adding of lead-outs to lines."));
	make_ctrl(LeadSizer, "", &leadOut, hasLeadOut, _("Lead out to be added, in milliseconds."));

	LeadSizer->AddStretchSpacer(1);

	// Adjacent subs sizer
	wxStaticBoxSizer *AdjacentSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Make adjacent subtitles continuous"));
	adjsEnable = make_check(AdjacentSizer, _("&Enable"),
		"Tool/Timing Post Processor/Enable/Adjacent",
		_("Enable snapping of subtitles together if they are within a certain distance of each other."));

	make_ctrl(AdjacentSizer, _("Threshold:"), &adjDistance, adjsEnable,
		_("Maximum difference between start and end time for two subtitles to be made continuous, in milliseconds."));

	adjacentBias = new wxSlider(this, -1, mid<int>(0, OPT_GET("Tool/Timing Post Processor/Adjacent Bias")->GetDouble() * 100, 100), 0, 100, wxDefaultPosition, wxSize(-1,20));
	adjacentBias->SetToolTip(_("Sets how to set the adjoining of lines. If set totally to left, it will extend start time of the second line; if totally to right, it will extend the end time of the first line."));

	AdjacentSizer->Add(new wxStaticText(this, -1, _("Bias: Start <- ")), wxSizerFlags().Center());
	AdjacentSizer->Add(adjacentBias, wxSizerFlags(1).Expand().Center());
	AdjacentSizer->Add(new wxStaticText(this, -1, _(" -> End")), wxSizerFlags().Center());

	// Keyframes sizer
	wxStaticBoxSizer *KeyframesSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Keyframe snapping"));
	wxSizer *KeyframesFlexSizer = new wxFlexGridSizer(2,5,5,0);

	keysEnable = new wxCheckBox(this, -1, _("E&nable"));
	keysEnable->SetToolTip(_("Enable snapping of subtitles to nearest keyframe, if distance is within threshold."));
	keysEnable->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Keyframe")->GetBool());
	KeyframesFlexSizer->Add(keysEnable,0,wxRIGHT|wxEXPAND,10);

	// Keyframes are only available if timecodes are loaded
	bool keysAvailable = c->videoController->KeyFramesLoaded() && c->videoController->TimecodesLoaded();
	if (!keysAvailable) {
		keysEnable->SetValue(false);
		keysEnable->Enable(false);
	}

	make_ctrl(this, KeyframesFlexSizer, _("Starts before thres.:"), &beforeStart, keysEnable,
		_("Threshold for 'before start' distance, that is, how many milliseconds a subtitle must start before a keyframe to snap to it."));

	make_ctrl(this, KeyframesFlexSizer, _("Starts after thres.:"), &afterStart, keysEnable,
		_("Threshold for 'after start' distance, that is, how many milliseconds a subtitle must start after a keyframe to snap to it."));

	KeyframesFlexSizer->AddStretchSpacer(1);

	make_ctrl(this, KeyframesFlexSizer, _("Ends before thres.:"), &beforeEnd, keysEnable,
		_("Threshold for 'before end' distance, that is, how many milliseconds a subtitle must end before a keyframe to snap to it."));

	make_ctrl(this, KeyframesFlexSizer, _("Ends after thres.:"), &afterEnd, keysEnable,
		_("Threshold for 'after end' distance, that is, how many milliseconds a subtitle must end after a keyframe to snap to it."));

	KeyframesSizer->Add(KeyframesFlexSizer,0,wxEXPAND);
	KeyframesSizer->AddStretchSpacer(1);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	ApplyButton = ButtonSizer->GetAffirmativeButton();
	ButtonSizer->GetHelpButton()->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&HelpButton::OpenPage, "Timing Processor"));

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
	LeftSizer->Add(StyleList, wxSizerFlags(1).Border(wxBOTTOM));
	LeftSizer->Add(StyleButtonsSizer, wxSizerFlags().Expand());

	// Top Sizer
	wxSizer *TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(LeftSizer,0,wxRIGHT|wxEXPAND,5);
	TopSizer->Add(RightSizer,1,wxALL|wxEXPAND,0);

	// Main Sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(TopSizer,1,wxALL|wxEXPAND,5);
	SetSizerAndFit(MainSizer);
	CenterOnParent();

	Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, bind(&DialogTimingProcessor::UpdateControls, this));
	Bind(wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, bind(&DialogTimingProcessor::UpdateControls, this));
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogTimingProcessor::OnApply, this, wxID_OK);
	all->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogTimingProcessor::CheckAll, this, true));
	none->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&DialogTimingProcessor::CheckAll, this, false));

	CheckAll(true);
}

void DialogTimingProcessor::CheckAll(bool value) {
	size_t count = StyleList->GetCount();
	for (size_t i = 0; i < count; ++i)
		StyleList->Check(i, value);
	UpdateControls();
}

void DialogTimingProcessor::UpdateControls() {
	// Only enable the OK button if it'll actually do something
	bool any_checked = false;
	size_t len = StyleList->GetCount();
	for (size_t i = 0; i < len; ++i) {
		if (StyleList->IsChecked(i)) {
			any_checked = true;
			break;
		}
	}
	ApplyButton->Enable(any_checked && (hasLeadIn->IsChecked() || hasLeadOut->IsChecked() || keysEnable->IsChecked() || adjsEnable->IsChecked()));
}

void DialogTimingProcessor::OnApply(wxCommandEvent &) {
	TransferDataFromWindow();
	// Save settings
	OPT_SET("Audio/Lead/IN")->SetInt(leadIn);
	OPT_SET("Audio/Lead/OUT")->SetInt(leadOut);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key Start Before")->SetInt(beforeStart);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key Start After")->SetInt(afterStart);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key End Before")->SetInt(beforeEnd);
	OPT_SET("Tool/Timing Post Processor/Threshold/Key End After")->SetInt(afterEnd);
	OPT_SET("Tool/Timing Post Processor/Threshold/Adjacent")->SetInt(adjDistance);
	OPT_SET("Tool/Timing Post Processor/Adjacent Bias")->SetDouble(adjacentBias->GetValue() / 100.0);
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/IN")->SetBool(hasLeadIn->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/OUT")->SetBool(hasLeadOut->IsChecked());
	if (keysEnable->IsEnabled()) OPT_SET("Tool/Timing Post Processor/Enable/Keyframe")->SetBool(keysEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Adjacent")->SetBool(adjsEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Only Selection")->SetBool(onlySelection->IsChecked());

	Process();
	EndModal(0);
}

static bool bad_line(std::set<wxString> *styles, AssDialogue *d) {
	return !d || d->Comment || styles->find(d->Style) == styles->end();
}

std::vector<AssDialogue*> DialogTimingProcessor::SortDialogues() {
	std::set<wxString> styles;
	for (size_t i = 0; i < StyleList->GetCount(); ++i) {
		if (StyleList->IsChecked(i))
			styles.insert(StyleList->GetString(i));
	}

	std::vector<AssDialogue*> sorted;
	sorted.reserve(c->ass->Line.size());

	if (onlySelection->IsChecked()) {
		SelectionController<AssDialogue>::Selection sel = c->selectionController->GetSelectedSet();
		remove_copy_if(sel.begin(), sel.end(), back_inserter(sorted),
			bind(bad_line, &styles, _1));
	}
	else {
		transform(c->ass->Line.begin(), c->ass->Line.end(), back_inserter(sorted), cast<AssDialogue*>());
		sorted.erase(remove_if(sorted.begin(), sorted.end(), bind(bad_line, &styles, _1)), sorted.end());
	}

	// Check if rows are valid
	for (size_t i = 0; i < sorted.size(); ++i) {
		if (sorted[i]->Start > sorted[i]->End) {
			wxMessageBox(
				wxString::Format(_("One of the lines in the file (%i) has negative duration. Aborting."), i),
				_("Invalid script"),
				wxICON_ERROR|wxOK);
			sorted.clear();
			break;
		}
	}

	sort(sorted.begin(), sorted.end(), AssFile::CompStart);
	return sorted;
}

static int get_closest_kf(std::vector<int> const& kf, int frame) {
	std::vector<int>::const_iterator pos = upper_bound(kf.begin(), kf.end(), frame);
	// Return last keyframe if this is after the last one
	if (pos == kf.end()) return kf.back();
	// *pos is greater than frame, and *(pos - 1) is less than or equal to frame
	return (pos == kf.begin() || *pos - frame < frame - *(pos - 1)) ? *pos : *(pos - 1);
}

template<class Iter, class Field>
static int safe_time(Iter begin, Iter end, AssDialogue *comp, int initial, Field field, int const& (*cmp)(int const&, int const&)) {
	// Compare to every previous line (yay for O(n^2)!) to see if it's OK to add lead-in
	for (; begin != end; ++begin) {
		// If the line doesn't already collide with this line, extend it only
		// to the edge of the line
		if (!comp->CollidesWith(*begin))
			initial = cmp(initial, (*begin)->*field);
	}
	return initial;
}

void DialogTimingProcessor::Process() {
	std::vector<AssDialogue*> sorted = SortDialogues();
	if (sorted.empty()) return;

	// Options
	bool addIn = hasLeadIn->IsChecked() && leadIn;
	bool addOut = hasLeadOut->IsChecked() && leadOut;

	// Add lead-in/out
	if (addIn || addOut) {
		for (size_t i = 0; i < sorted.size(); ++i) {
			AssDialogue *cur = sorted[i];
			if (addIn)
				cur->Start = safe_time(sorted.rend() - i, sorted.rend(), cur, cur->Start - leadIn, &AssDialogue::End, &std::max<int>);

			if (addOut)
				cur->End = safe_time(sorted.begin() + i + 1, sorted.end(), cur, cur->End + leadOut, &AssDialogue::Start, &std::min<int>);
		}
	}

	// Make adjacent
	if (adjsEnable->IsChecked()) {
		double bias = adjacentBias->GetValue() / 100.0;

		for (size_t i = 1; i < sorted.size(); ++i) {
			AssDialogue *prev = sorted[i - 1];
			AssDialogue *cur = sorted[i];

			// Check if they don't collide
			if (cur->CollidesWith(prev)) continue;

			// Compare distance
			int dist = cur->Start - prev->End;
			if (dist > 0 && dist <= adjDistance) {
				int setPos = prev->End + int(dist * bias);
				cur->Start = setPos;
				prev->End = setPos;
			}
		}
	}

	// Keyframe snapping
	if (keysEnable->IsChecked()) {
		std::vector<int> kf = c->videoController->GetKeyFrames();
		if (c->videoController->IsLoaded())
			kf.push_back(c->videoController->GetLength() - 1);

		for (size_t i = 0; i < sorted.size(); ++i) {
			AssDialogue *cur = sorted[i];

			// Get start/end frames
			int startF = c->videoController->FrameAtTime(cur->Start, agi::vfr::START);
			int endF = c->videoController->FrameAtTime(cur->End, agi::vfr::END);

			// Get closest for start
			int closest = get_closest_kf(kf, startF);
			int time = c->videoController->TimeAtFrame(closest, agi::vfr::START);
			if ((closest > startF && time - cur->Start <= beforeStart) || (closest < startF && cur->Start - time <= afterStart)) {
				cur->Start = time;
			}

			// Get closest for end
			closest = get_closest_kf(kf, endF) - 1;
			time = c->videoController->TimeAtFrame(closest, agi::vfr::END);
			if ((closest > endF && time - cur->End <= beforeEnd) || (closest < endF && cur->End - time <= afterEnd)) {
				cur->End = time;
			}
		}
	}

	c->ass->Commit(_("timing processor"), AssFile::COMMIT_DIAG_TIME);
}
