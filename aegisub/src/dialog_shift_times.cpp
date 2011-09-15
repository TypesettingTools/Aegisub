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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_shift_times.cpp
/// @brief Shift Times dialogue box and logic
/// @ingroup secondary_ui
///

#include "config.h"

#include "dialog_shift_times.h"

#ifndef AGI_PRE
#include <algorithm>
#include <vector>

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/listbox.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#endif

#include <libaegisub/access.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>
#include <libaegisub/scoped_ptr.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "standard_paths.h"
#include "timeedit_ctrl.h"
#include "utils.h"
#include "video_context.h"

DialogShiftTimes::DialogShiftTimes(agi::Context *context)
: wxDialog(context->parent, -1, _("Shift Times"))
, context(context)
, history_filename(STD_STR(StandardPaths::DecodePath("?user/shift_history.txt")))
, timecodes_loaded_slot(context->videoController->AddTimecodesListener(&DialogShiftTimes::OnTimecodesLoaded, this))
{
	SetIcon(BitmapToIcon(GETIMAGE(shift_times_toolbutton_24)));

	// Create controls
	shift_by_time = new wxRadioButton(this, -1, _("Time: "), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	shift_by_time->SetToolTip(_("Shift by time"));
	shift_by_time->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &DialogShiftTimes::OnByTime, this);

	shift_by_frames = new wxRadioButton(this, -1 , _("Frames: "));
	shift_by_frames->SetToolTip(_("Shift by frames"));
	shift_by_frames->Bind(wxEVT_COMMAND_RADIOBUTTON_SELECTED, &DialogShiftTimes::OnByFrames, this);

	shift_time = new TimeEdit(this, -1);
	shift_time->SetToolTip(_("Enter time in h:mm:ss.cs notation"));

	shift_frames = new wxTextCtrl(this, -1);
	shift_frames->SetToolTip(_("Enter number of frames to shift by"));

	shift_forward = new wxRadioButton(this, -1, _("Forward"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	shift_forward->SetToolTip(_("Shifts subs forward, making them appear later. Use if they are appearing too soon."));

	shift_backward = new wxRadioButton(this, -1, _("Backward"));
	shift_backward->SetToolTip(_("Shifts subs backward, making them appear earlier. Use if they are appearing too late."));

	wxString selection_mode_vals[] = { _("All rows"), _("Selected rows"), _("Selection onward") };
	selection_mode = new wxRadioBox(this, -1, _("Affect"), wxDefaultPosition, wxDefaultSize, 3, selection_mode_vals, 1);

	wxString time_field_vals[] = { _("Start and End times"), _("Start times only"), _("End times only") };
	time_fields = new wxRadioBox(this, -1, _("Times"), wxDefaultPosition, wxDefaultSize, 3, time_field_vals, 1);

	history = new wxListBox(this, -1, wxDefaultPosition, wxSize(350, 100), 0, NULL, wxLB_HSCROLL);

	wxButton *clear_button = new wxButton(this, -1, _("Clear"));
	clear_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogShiftTimes::OnClear, this);

	// Set initial control states
	OnTimecodesLoaded(context->videoController->FPS());
	OnSelectedSetChanged(Selection(), Selection());
	LoadHistory();

	shift_time->SetTime(OPT_GET("Tool/Shift Times/Time")->GetInt());
	*shift_frames << (int)OPT_GET("Tool/Shift Times/Frames")->GetInt();
	shift_frames->Disable();
	shift_by_frames->SetValue(!OPT_GET("Tool/Shift Times/ByTime")->GetBool() && shift_by_frames->IsEnabled());
	time_fields->SetSelection(OPT_GET("Tool/Shift Times/Type")->GetInt());
	selection_mode->SetSelection(OPT_GET("Tool/Shift Times/Affect")->GetInt());
	shift_backward->SetValue(OPT_GET("Tool/Shift Times/Direction")->GetBool());

	// Position controls
	wxSizer *shift_amount_sizer = new wxFlexGridSizer(2, 2, 5, 5);
	shift_amount_sizer->Add(shift_by_time, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));
	shift_amount_sizer->Add(shift_time, wxSizerFlags(1));
	shift_amount_sizer->Add(shift_by_frames, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));
	shift_amount_sizer->Add(shift_frames, wxSizerFlags(1));

	wxSizer *shift_direction_sizer = new wxBoxSizer(wxHORIZONTAL);
	shift_direction_sizer->Add(shift_forward, wxSizerFlags(1).Expand());
	shift_direction_sizer->Add(shift_backward, wxSizerFlags(1).Expand().Border(wxLEFT));

	wxSizer *shift_by_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Shift by"));
	shift_by_sizer->Add(shift_amount_sizer, wxSizerFlags().Expand());
	shift_by_sizer->Add(shift_direction_sizer, wxSizerFlags().Expand().Border(wxTOP));

	wxSizer *left_sizer = new wxBoxSizer(wxVERTICAL);
	left_sizer->Add(shift_by_sizer, wxSizerFlags().Expand().Border(wxBOTTOM));
	left_sizer->Add(selection_mode, wxSizerFlags().Expand().Border(wxBOTTOM));
	left_sizer->Add(time_fields, wxSizerFlags().Expand());

	wxSizer *history_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("History"));
	history_sizer->Add(history, wxSizerFlags(1).Expand());
	history_sizer->Add(clear_button, wxSizerFlags().Expand().Border(wxTOP));

	wxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	top_sizer->Add(left_sizer, wxSizerFlags().Border(wxALL & ~wxRIGHT).Expand());
	top_sizer->Add(history_sizer, wxSizerFlags().Border().Expand());

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(top_sizer, wxSizerFlags().Border(wxALL & ~wxBOTTOM));
	main_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL | wxHELP), wxSizerFlags().Right().Border());
	SetSizerAndFit(main_sizer);
	CenterOnParent();

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogShiftTimes::Process, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogShiftTimes::OnClose, this, wxID_CANCEL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(&HelpButton::OpenPage, "Shift Times"), wxID_HELP);
	context->selectionController->AddSelectionListener(this);
}

DialogShiftTimes::~DialogShiftTimes() {
	context->selectionController->RemoveSelectionListener(this);
}

void DialogShiftTimes::OnTimecodesLoaded(agi::vfr::Framerate const& new_fps) {
	fps = new_fps;
	if (fps.IsLoaded()) {
		shift_by_frames->Enable();
	}
	else {
		shift_by_time->SetValue(true);
		shift_by_frames->Disable();
		shift_time->Enable();
		shift_frames->Disable();
	}
}

void DialogShiftTimes::OnSelectedSetChanged(Selection const&, Selection const&) {
	if (context->selectionController->GetSelectedSet().empty()) {
		selection_mode->Enable(1, false);
		selection_mode->Enable(2, false);
		selection_mode->SetSelection(0);
	}
	else {
		selection_mode->Enable(1, true);
		selection_mode->Enable(2, true);
	}
}


void DialogShiftTimes::OnClear(wxCommandEvent &) {
	wxRemoveFile(lagi_wxString(history_filename));
	history->Clear();
}

void DialogShiftTimes::OnClose(wxCommandEvent &) {
	long shift;
	shift_frames->GetValue().ToLong(&shift);

	OPT_SET("Tool/Shift Times/Time")->SetInt(shift_time->time.GetMS());
	OPT_SET("Tool/Shift Times/Frames")->SetInt(shift);
	OPT_SET("Tool/Shift Times/ByTime")->SetBool(shift_by_time->GetValue());
	OPT_SET("Tool/Shift Times/Type")->SetInt(time_fields->GetSelection());
	OPT_SET("Tool/Shift Times/Affect")->SetInt(selection_mode->GetSelection());
	OPT_SET("Tool/Shift Times/Direction")->SetBool(shift_backward->GetValue());

	Destroy();
}

void DialogShiftTimes::OnByTime(wxCommandEvent &) {
	shift_time->Enable(true);
	shift_frames->Enable(false);
}

void DialogShiftTimes::OnByFrames(wxCommandEvent &) {
	shift_time->Enable(false);
	shift_frames->Enable(true);
}

void DialogShiftTimes::SaveHistory(std::vector<std::pair<int, int> > const& shifted_blocks) {
	wxString filename = wxFileName(context->ass->filename).GetFullName();
	int fields = time_fields->GetSelection();

	wxString new_line = wxString::Format("%s, %s %s, %s, ",
		filename.empty() ? _("unsaved") : filename,
		shift_by_time->GetValue() ? shift_time->GetValue() : shift_frames->GetValue() + _(" frames"),
		shift_backward->GetValue() ? _("backward") : _("forward"),
		fields == 0 ? _("s+e") : fields == 1 ? _("s") : _("e"));

	int sel_mode = selection_mode->GetSelection();
	if (sel_mode == 0)
		new_line += _("all");
	else if (sel_mode == 2)
		new_line += wxString::Format(_("from %d onward"), shifted_blocks.front().first);
	else {
		new_line += _("sel ");
		for (size_t i = 0; i < shifted_blocks.size(); ++i) {
			std::pair<int, int> const& b = shifted_blocks[i];
			wxString term = i == shifted_blocks.size() - 1 ? "" : ";";
			if (b.first == b.second)
				new_line += wxString::Format("%d%s", b.first, term);
			else
				new_line += wxString::Format("%d-%d%s", b.first, b.second, term);
		}
	}

	try {
		agi::io::Save file(history_filename);

		for (size_t i = 0; i < history->GetCount(); ++i)
			file.Get() << history->GetString(i).utf8_str() << std::endl;
		file.Get() << new_line.utf8_str() << std::endl;
	}
	catch (agi::acs::AcsError const& e) {
		LOG_E("dialog_shift_times/save_history") << "Cannot save shift times history: " << e.GetChainedMessage();
	}
}

void DialogShiftTimes::LoadHistory() {
	history->Clear();
	history->Freeze();

	try {
		agi::scoped_ptr<std::istream> file(agi::io::Open(history_filename));
		std::string buffer;
		while(!file->eof()) {
			getline(*file, buffer);
			if (buffer.size())
				history->Insert(lagi_wxString(buffer), 0);
		}
	}
	catch (agi::acs::AcsError const& e) {
		LOG_E("dialog_shift_times/save_history") << "Cannot load shift times history: " << e.GetChainedMessage();
	}
	catch (...) {
		history->Thaw();
		throw;
	}

	history->Thaw();
}

void DialogShiftTimes::Process(wxCommandEvent &) {
	int mode = selection_mode->GetSelection();
	int type = time_fields->GetSelection();
	bool reverse = shift_backward->GetValue();
	bool by_time = shift_by_time->GetValue();

	bool start = type != 2;
	bool end = type != 1;

	Selection sel = context->selectionController->GetSelectedSet();

	long shift;
	if (by_time) {
		shift = shift_time->time.GetMS();
		if (shift == 0) {
			Close();
			return;
		}
	}
	else
		shift_frames->GetValue().ToLong(&shift);

	if (reverse)
		shift = -shift;

	// Track which rows were shifted for the log
	int row_number = 0;
	int block_start = 0;
	std::vector<std::pair<int, int> > shifted_blocks;

	for (entryIter it = context->ass->Line.begin(); it != context->ass->Line.end(); ++it) {
		AssDialogue *line = dynamic_cast<AssDialogue*>(*it);
		if (!line) continue;
		++row_number;

		if (!sel.count(line)) {
			if (block_start) {
				shifted_blocks.push_back(std::make_pair(block_start, row_number - 1));
				block_start = 0;
			}
			if (mode == 1) continue;
			if (mode == 2 && shifted_blocks.empty()) continue;
		}
		else if (!block_start)
			block_start = row_number;

		if (start)
			line->Start.SetMS(Shift(line->Start.GetMS(), shift, by_time, agi::vfr::START));
		if (end)
			line->End.SetMS(Shift(line->End.GetMS(), shift, by_time, agi::vfr::END));
	}

	context->ass->Commit(_("shifting"), AssFile::COMMIT_DIAG_TIME);

	if (block_start)
		shifted_blocks.push_back(std::make_pair(block_start, row_number - 1));

	SaveHistory(shifted_blocks);
	Close();
}

int DialogShiftTimes::Shift(int initial_time, int shift, bool by_time, agi::vfr::Time type) {
	if (by_time)
		return initial_time + shift;
	else
		return fps.TimeAtFrame(shift + fps.FrameAtTime(initial_time, type), type);
}
