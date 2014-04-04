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

/// @file dialog_timing_processor.cpp
/// @brief Timing Post-processor dialogue box and logic
/// @ingroup tools_ui
///

#include "config.h"

#include "dialog_timing_processor.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "selection_controller.h"
#include "utils.h"
#include "video_context.h"

#include <libaegisub/address_of_adaptor.h>

#include <algorithm>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <functional>

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

using namespace boost::adaptors;

namespace {
wxTextCtrl *make_ctrl(wxWindow *parent, wxSizer *sizer, wxString const& desc, int *value, wxCheckBox *cb, wxString const& tooltip) {
	wxIntegerValidator<int> validator(value);
	validator.SetMin(0);
	wxTextCtrl *ctrl = new wxTextCtrl(parent, -1, "", wxDefaultPosition, wxSize(60,-1), 0, validator);
	ctrl->SetToolTip(tooltip);
	if (!desc.empty())
		sizer->Add(new wxStaticText(parent, -1, desc), wxSizerFlags().Center().Border(wxRIGHT));
	sizer->Add(ctrl, wxSizerFlags().Expand().Border(wxRIGHT));

	ctrl->Enable(cb->IsChecked());
	cb->Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& evt) {
		ctrl->Enable(cb->IsChecked());
		evt.Skip();
	});

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
	using std::bind;

	SetIcon(GETICON(timing_processor_toolbutton_16));

	// Read options
	leadIn = OPT_GET("Audio/Lead/IN")->GetInt();
	leadOut = OPT_GET("Audio/Lead/OUT")->GetInt();
	beforeStart = OPT_GET("Tool/Timing Post Processor/Threshold/Key Start Before")->GetInt();
	beforeEnd = OPT_GET("Tool/Timing Post Processor/Threshold/Key End Before")->GetInt();
	afterStart = OPT_GET("Tool/Timing Post Processor/Threshold/Key Start After")->GetInt();
	afterEnd = OPT_GET("Tool/Timing Post Processor/Threshold/Key End After")->GetInt();
	adjGap = OPT_GET("Tool/Timing Post Processor/Threshold/Adjacent Gap")->GetInt();
	adjOverlap = OPT_GET("Tool/Timing Post Processor/Threshold/Adjacent Overlap")->GetInt();

	// Styles box
	wxSizer *LeftSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Apply to styles"));
	StyleList = new wxCheckListBox(this, -1, wxDefaultPosition, wxSize(150,150), to_wx(c->ass->GetStyles()));
	StyleList->SetToolTip(_("Select styles to process. Unchecked ones will be ignored."));

	wxButton *all = new wxButton(this,-1,_("&All"));
	all->SetToolTip(_("Select all styles"));

	wxButton *none = new wxButton(this,-1,_("&None"));
	none->SetToolTip(_("Deselect all styles"));

	// Options box
	wxStaticBoxSizer *optionsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Options"));
	onlySelection = new wxCheckBox(this,-1,_("Affect &selection only"));
	onlySelection->SetValue(OPT_GET("Tool/Timing Post Processor/Only Selection")->GetBool());
	optionsSizer->Add(onlySelection,1,wxALL,0);

	// Lead-in/out box
	wxStaticBoxSizer *LeadSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Lead-in/Lead-out"));

	hasLeadIn = make_check(LeadSizer, _("Add lead &in:"),
		"Tool/Timing Post Processor/Enable/Lead/IN",
		_("Enable adding of lead-ins to lines"));
	make_ctrl(LeadSizer, "", &leadIn, hasLeadIn, _("Lead in to be added, in milliseconds"));

	hasLeadOut = make_check(LeadSizer, _("Add lead &out:"),
		"Tool/Timing Post Processor/Enable/Lead/OUT",
		_("Enable adding of lead-outs to lines"));
	make_ctrl(LeadSizer, "", &leadOut, hasLeadOut, _("Lead out to be added, in milliseconds"));

	LeadSizer->AddStretchSpacer(1);

	// Adjacent subs sizer
	wxStaticBoxSizer *AdjacentSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Make adjacent subtitles continuous"));
	adjsEnable = make_check(AdjacentSizer, _("&Enable"),
		"Tool/Timing Post Processor/Enable/Adjacent",
		_("Enable snapping of subtitles together if they are within a certain distance of each other"));

	wxSizer *adjBoxes = new wxBoxSizer(wxHORIZONTAL);
	make_ctrl(this, adjBoxes, _("Max gap:"), &adjGap, adjsEnable,
		_("Maximum difference between start and end time for two subtitles to be made continuous, in milliseconds"));
	make_ctrl(this, adjBoxes, _("Max overlap:"), &adjOverlap, adjsEnable,
		_("Maximum overlap between the end and start time for two subtitles to be made continuous, in milliseconds"));

	adjacentBias = new wxSlider(this, -1, mid<int>(0, OPT_GET("Tool/Timing Post Processor/Adjacent Bias")->GetDouble() * 100, 100), 0, 100, wxDefaultPosition, wxSize(-1,20));
	adjacentBias->SetToolTip(_("Sets how to set the adjoining of lines. If set totally to left, it will extend or shrink start time of the second line; if totally to right, it will extend or shrink the end time of the first line."));

	wxSizer *adjSliderSizer = new wxBoxSizer(wxHORIZONTAL);
	adjSliderSizer->Add(new wxStaticText(this, -1, _("Bias: Start <- ")), wxSizerFlags().Center());
	adjSliderSizer->Add(adjacentBias, wxSizerFlags(1).Center());
	adjSliderSizer->Add(new wxStaticText(this, -1, _(" -> End")), wxSizerFlags().Center());

	wxSizer *adjRightSizer = new wxBoxSizer(wxVERTICAL);
	adjRightSizer->Add(adjBoxes, wxSizerFlags().Expand());
	adjRightSizer->Add(adjSliderSizer, wxSizerFlags().Expand().Border(wxTOP));
	AdjacentSizer->Add(adjRightSizer);

	// Keyframes sizer
	wxStaticBoxSizer *KeyframesSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Keyframe snapping"));
	wxSizer *KeyframesFlexSizer = new wxFlexGridSizer(2,5,5,0);

	keysEnable = new wxCheckBox(this, -1, _("E&nable"));
	keysEnable->SetToolTip(_("Enable snapping of subtitles to nearest keyframe, if distance is within threshold"));
	keysEnable->SetValue(OPT_GET("Tool/Timing Post Processor/Enable/Keyframe")->GetBool());
	KeyframesFlexSizer->Add(keysEnable,0,wxRIGHT|wxEXPAND,10);

	// Keyframes are only available if timecodes are loaded
	bool keysAvailable = c->videoController->KeyFramesLoaded() && c->videoController->TimecodesLoaded();
	if (!keysAvailable) {
		keysEnable->SetValue(false);
		keysEnable->Enable(false);
	}

	make_ctrl(this, KeyframesFlexSizer, _("Starts before thres.:"), &beforeStart, keysEnable,
		_("Threshold for 'before start' distance, that is, how many milliseconds a subtitle must start before a keyframe to snap to it"));

	make_ctrl(this, KeyframesFlexSizer, _("Starts after thres.:"), &afterStart, keysEnable,
		_("Threshold for 'after start' distance, that is, how many milliseconds a subtitle must start after a keyframe to snap to it"));

	KeyframesFlexSizer->AddStretchSpacer(1);

	make_ctrl(this, KeyframesFlexSizer, _("Ends before thres.:"), &beforeEnd, keysEnable,
		_("Threshold for 'before end' distance, that is, how many milliseconds a subtitle must end before a keyframe to snap to it"));

	make_ctrl(this, KeyframesFlexSizer, _("Ends after thres.:"), &afterEnd, keysEnable,
		_("Threshold for 'after end' distance, that is, how many milliseconds a subtitle must end after a keyframe to snap to it"));

	KeyframesSizer->Add(KeyframesFlexSizer,0,wxEXPAND);
	KeyframesSizer->AddStretchSpacer(1);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	ApplyButton = ButtonSizer->GetAffirmativeButton();
	ButtonSizer->GetHelpButton()->Bind(wxEVT_BUTTON, bind(&HelpButton::OpenPage, "Timing Processor"));

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

	Bind(wxEVT_CHECKBOX, bind(&DialogTimingProcessor::UpdateControls, this));
	Bind(wxEVT_CHECKLISTBOX, bind(&DialogTimingProcessor::UpdateControls, this));
	Bind(wxEVT_BUTTON, &DialogTimingProcessor::OnApply, this, wxID_OK);
	all->Bind(wxEVT_BUTTON, bind(&DialogTimingProcessor::CheckAll, this, true));
	none->Bind(wxEVT_BUTTON, bind(&DialogTimingProcessor::CheckAll, this, false));

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
	for (size_t i = 0; !any_checked && i < len; ++i)
		any_checked = StyleList->IsChecked(i);
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
	OPT_SET("Tool/Timing Post Processor/Threshold/Adjacent Gap")->SetInt(adjGap);
	OPT_SET("Tool/Timing Post Processor/Threshold/Adjacent Overlap")->SetInt(adjOverlap);
	OPT_SET("Tool/Timing Post Processor/Adjacent Bias")->SetDouble(adjacentBias->GetValue() / 100.0);
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/IN")->SetBool(hasLeadIn->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Lead/OUT")->SetBool(hasLeadOut->IsChecked());
	if (keysEnable->IsEnabled()) OPT_SET("Tool/Timing Post Processor/Enable/Keyframe")->SetBool(keysEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Enable/Adjacent")->SetBool(adjsEnable->IsChecked());
	OPT_SET("Tool/Timing Post Processor/Only Selection")->SetBool(onlySelection->IsChecked());

	Process();
	EndModal(0);
}

std::vector<AssDialogue*> DialogTimingProcessor::SortDialogues() {
	std::set<std::string> styles;
	for (size_t i = 0; i < StyleList->GetCount(); ++i) {
		if (StyleList->IsChecked(i))
			styles.insert(from_wx(StyleList->GetString(i)));
	}

	std::vector<AssDialogue*> sorted;

	auto valid_line = [&](const AssDialogue *d) { return !d->Comment && styles.count(d->Style); };
	if (onlySelection->IsChecked())
		boost::copy(c->selectionController->GetSelectedSet() | filtered(valid_line),
		    back_inserter(sorted));
	else {
		sorted.reserve(c->ass->Events.size());
		boost::push_back(sorted, c->ass->Events | agi::address_of | filtered(valid_line));
	}

	// Check if rows are valid
	for (auto diag : sorted) {
		if (diag->Start > diag->End) {
			int line = std::distance(c->ass->Events.begin(), c->ass->iterator_to(*diag));
			wxMessageBox(
				wxString::Format(_("One of the lines in the file (%i) has negative duration. Aborting."), line),
				_("Invalid script"),
				wxOK | wxICON_ERROR | wxCENTER);
			sorted.clear();
			break;
		}
	}

	boost::sort(sorted, [](const AssDialogue *a, const AssDialogue *b) {
		return a->Start < b->Start;
	});
	return sorted;
}

static int get_closest_kf(std::vector<int> const& kf, int frame) {
	const auto pos = boost::upper_bound(kf, frame);
	// Return last keyframe if this is after the last one
	if (pos == end(kf)) return kf.back();
	// *pos is greater than frame, and *(pos - 1) is less than or equal to frame
	return (pos == begin(kf) || *pos - frame < frame - *(pos - 1)) ? *pos : *(pos - 1);
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

	// Add lead-in/out
	if (hasLeadIn->IsChecked() && leadIn) {
		for (size_t i = 0; i < sorted.size(); ++i)
			sorted[i]->Start = safe_time(sorted.rend() - i, sorted.rend(),
				sorted[i], sorted[i]->Start - leadIn,
				&AssDialogue::End, &std::max<int>);
	}

	if (hasLeadOut->IsChecked() && leadOut) {
		for (size_t i = 0; i < sorted.size(); ++i)
			sorted[i]->End = safe_time(sorted.begin() + i + 1, sorted.end(),
				sorted[i], sorted[i]->End + leadOut,
				&AssDialogue::Start, &std::min<int>);
	}

	// Make adjacent
	if (adjsEnable->IsChecked()) {
		double bias = adjacentBias->GetValue() / 100.0;

		for (size_t i = 1; i < sorted.size(); ++i) {
			AssDialogue *prev = sorted[i - 1];
			AssDialogue *cur = sorted[i];

			int dist = cur->Start - prev->End;
			if ((dist < 0 && -dist <= adjOverlap) || (dist > 0 && dist <= adjGap)) {
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

		for (AssDialogue *cur : sorted) {
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
