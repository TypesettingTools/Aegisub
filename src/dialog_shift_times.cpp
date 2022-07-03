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

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "dialog_manager.h"
#include "format.h"
#include "include/aegisub/context.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "project.h"
#include "selection_controller.h"
#include "subs_controller.h"
#include "timeedit_ctrl.h"

#include <libaegisub/ass/time.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/signal.h>
#include <libaegisub/vfr.h>

#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/reader.h>
#include <libaegisub/cajun/writer.h>

#include <boost/filesystem/path.hpp>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

namespace {
class DialogShiftTimes final : public wxDialog {
	agi::Context *context;

	agi::fs::path history_filename;
	json::Array history;
	agi::vfr::Framerate fps;
	agi::signal::Connection timecodes_loaded_slot;
	agi::signal::Connection selected_set_changed_slot;

	TimeEdit *shift_time;
	wxTextCtrl *shift_frames;
	wxRadioButton *shift_by_time;
	wxRadioButton *shift_by_frames;
	wxRadioButton *shift_forward;
	wxRadioButton *shift_backward;
	wxRadioBox *selection_mode;
	wxRadioBox *time_fields;
	wxListBox *history_box;

	void SaveHistory(json::Array shifted_blocks);
	void LoadHistory();
	void Process(wxCommandEvent&);
	int Shift(int initial_time, int shift, bool by_time, agi::vfr::Time type);

	void OnClear(wxCommandEvent&);
	void OnByTime(wxCommandEvent&);
	void OnByFrames(wxCommandEvent&);
	void OnHistoryClick(wxCommandEvent&);

	void OnSelectedSetChanged();
	void OnTimecodesLoaded(agi::vfr::Framerate const& new_fps);

public:
	DialogShiftTimes(agi::Context *context);
	~DialogShiftTimes();
};

static wxString get_history_string(json::Object &obj) {
	wxString filename = to_wx(obj["filename"]);
	if (filename.empty())
		filename = _("unsaved");

	wxString shift_amount(to_wx(obj["amount"]));
	if (!obj["is by time"])
		shift_amount = fmt_tl("%s frames", shift_amount);

	wxString shift_direction = obj["is backward"] ? _("backward") : _("forward");

	int64_t time_field = obj["fields"];
	wxString fields =
		time_field == 0 ? _("s+e") :
		time_field == 1 ? _("s")   :
		                  _("e")   ;

	json::Array& sel = obj["selection"];
	wxString lines;

	int64_t sel_mode = obj["mode"];
	if (sel_mode == 0)
		lines = _("all");
	else if (sel_mode == 2) {
		if (!sel.empty())
			lines = fmt_tl("from %d onward", (int64_t)static_cast<json::Object&>(sel.front())["start"]);
	}
	else {
		lines += _("sel ");
		for (auto it = sel.begin(); it != sel.end(); ++it) {
			json::Object& range = *it;
			int beg = (int64_t)range["start"];
			int end = (int64_t)range["end"];
			if (beg == end)
				lines += std::to_wstring(beg);
			else
				lines += fmt_wx("%d-%d", beg, end);
			if (it + 1 != sel.end())
				lines += ";";
		}
	}

	return fmt_wx("%s, %s %s, %s, %s", filename, shift_amount, shift_direction, fields, lines);
}

DialogShiftTimes::DialogShiftTimes(agi::Context *context)
: wxDialog(context->parent, -1, _("Shift Times"))
, context(context)
, history_filename(config::path->Decode("?user/shift_history.json"))
, timecodes_loaded_slot(context->project->AddTimecodesListener(&DialogShiftTimes::OnTimecodesLoaded, this))
, selected_set_changed_slot(context->selectionController->AddSelectionListener(&DialogShiftTimes::OnSelectedSetChanged, this))
{
	SetIcon(GETICON(shift_times_toolbutton_16));

	// Create controls
	shift_by_time = new wxRadioButton(this, -1, _("&Time: "), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	shift_by_time->SetToolTip(_("Shift by time"));
	shift_by_time->Bind(wxEVT_RADIOBUTTON, &DialogShiftTimes::OnByTime, this);

	shift_by_frames = new wxRadioButton(this, -1 , _("&Frames: "));
	shift_by_frames->SetToolTip(_("Shift by frames"));
	shift_by_frames->Bind(wxEVT_RADIOBUTTON, &DialogShiftTimes::OnByFrames, this);

	shift_time = new TimeEdit(this, -1, context);
	shift_time->SetToolTip(_("Enter time in h:mm:ss.cs notation"));

	shift_frames = new wxTextCtrl(this, -1);
	shift_frames->SetToolTip(_("Enter number of frames to shift by"));

	shift_forward = new wxRadioButton(this, -1, _("For&ward"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	shift_forward->SetToolTip(_("Shifts subs forward, making them appear later. Use if they are appearing too soon."));

	shift_backward = new wxRadioButton(this, -1, _("&Backward"));
	shift_backward->SetToolTip(_("Shifts subs backward, making them appear earlier. Use if they are appearing too late."));

	wxString selection_mode_vals[] = { _("&All rows"), _("Selected &rows"), _("Selection &onward") };
	selection_mode = new wxRadioBox(this, -1, _("Affect"), wxDefaultPosition, wxDefaultSize, 3, selection_mode_vals, 1);

	wxString time_field_vals[] = { _("Start a&nd End times"), _("&Start times only"), _("&End times only") };
	time_fields = new wxRadioBox(this, -1, _("Times"), wxDefaultPosition, wxDefaultSize, 3, time_field_vals, 1);

	history_box = new wxListBox(this, -1, wxDefaultPosition, wxSize(350, 100), 0, nullptr, wxLB_HSCROLL);

	wxButton *clear_button = new wxButton(this, -1, _("&Clear"));
	clear_button->Bind(wxEVT_BUTTON, &DialogShiftTimes::OnClear, this);

	// Set initial control states
	OnTimecodesLoaded(context->project->Timecodes());
	OnSelectedSetChanged();
	LoadHistory();

	shift_time->SetTime(OPT_GET("Tool/Shift Times/Time")->GetInt());
	*shift_frames << (int)OPT_GET("Tool/Shift Times/Frames")->GetInt();
	shift_by_frames->SetValue(!OPT_GET("Tool/Shift Times/ByTime")->GetBool() && shift_by_frames->IsEnabled());
	time_fields->SetSelection(OPT_GET("Tool/Shift Times/Type")->GetInt());
	selection_mode->SetSelection(OPT_GET("Tool/Shift Times/Affect")->GetInt());
	shift_backward->SetValue(OPT_GET("Tool/Shift Times/Direction")->GetBool());

	if (shift_by_frames->GetValue())
		shift_time->Disable();
	else
		shift_frames->Disable();

	// Position controls
	wxFlexGridSizer* shift_amount_sizer = new wxFlexGridSizer(2, 2, 5, 5);
	shift_amount_sizer->AddGrowableCol(1, 1);
	shift_amount_sizer->Add(shift_by_time, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));
	shift_amount_sizer->Add(shift_time, wxSizerFlags().Expand());
	shift_amount_sizer->Add(shift_by_frames, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL));
	shift_amount_sizer->Add(shift_frames, wxSizerFlags().Expand());

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

	wxSizer *history_sizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Load from history"));
	history_sizer->Add(history_box, wxSizerFlags(1).Expand());
	history_sizer->Add(clear_button, wxSizerFlags().Expand().Border(wxTOP));

	wxSizer *top_sizer = new wxBoxSizer(wxHORIZONTAL);
	top_sizer->Add(left_sizer, wxSizerFlags().Border(wxALL & ~wxRIGHT).Expand());
	top_sizer->Add(history_sizer, wxSizerFlags().Border().Expand());

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(top_sizer, wxSizerFlags().Border(wxALL & ~wxBOTTOM));
	main_sizer->Add(CreateButtonSizer(wxOK | wxCANCEL | wxHELP), wxSizerFlags().Right().Border());
	SetSizerAndFit(main_sizer);
	CenterOnParent();

	Bind(wxEVT_BUTTON, &DialogShiftTimes::Process, this, wxID_OK);
	Bind(wxEVT_BUTTON, std::bind(&HelpButton::OpenPage, "Shift Times"), wxID_HELP);
	shift_time->Bind(wxEVT_TEXT_ENTER, &DialogShiftTimes::Process, this);
	history_box->Bind(wxEVT_LISTBOX_DCLICK, &DialogShiftTimes::OnHistoryClick, this);
}

DialogShiftTimes::~DialogShiftTimes() {
	long shift;
	shift_frames->GetValue().ToLong(&shift);

	OPT_SET("Tool/Shift Times/Time")->SetInt(shift_time->GetTime());
	OPT_SET("Tool/Shift Times/Frames")->SetInt(shift);
	OPT_SET("Tool/Shift Times/ByTime")->SetBool(shift_by_time->GetValue());
	OPT_SET("Tool/Shift Times/Type")->SetInt(time_fields->GetSelection());
	OPT_SET("Tool/Shift Times/Affect")->SetInt(selection_mode->GetSelection());
	OPT_SET("Tool/Shift Times/Direction")->SetBool(shift_backward->GetValue());
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

void DialogShiftTimes::OnSelectedSetChanged() {
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
	agi::fs::Remove(history_filename);
	history_box->Clear();
	history.clear();
}

void DialogShiftTimes::OnByTime(wxCommandEvent &) {
	shift_time->Enable(true);
	shift_frames->Enable(false);
}

void DialogShiftTimes::OnByFrames(wxCommandEvent &) {
	shift_time->Enable(false);
	shift_frames->Enable(true);
}

void DialogShiftTimes::OnHistoryClick(wxCommandEvent &evt) {
	size_t entry = evt.GetInt();
	if (entry >= history.size()) return;

	json::Object& obj = history[entry];
	if (obj["is by time"]) {
		shift_time->SetTime(agi::Time((std::string)obj["amount"]));
		shift_by_time->SetValue(true);
		OnByTime(evt);
	}
	else {
		shift_frames->SetValue(to_wx(obj["amount"]));
		if (shift_by_frames->IsEnabled()) {
			shift_by_frames->SetValue(true);
			OnByFrames(evt);
		}
	}

	if (obj["is backward"])
		shift_backward->SetValue(true);
	else
		shift_forward->SetValue(true);

	selection_mode->SetSelection((int64_t)obj["mode"]);
	time_fields->SetSelection((int64_t)obj["fields"]);
}

void DialogShiftTimes::SaveHistory(json::Array shifted_blocks) {
	json::Object new_entry;
	new_entry["filename"] = context->subsController->Filename().filename().string();
	new_entry["is by time"] = shift_by_time->GetValue();
	new_entry["is backward"] = shift_backward->GetValue();
	new_entry["amount"] = from_wx(shift_by_time->GetValue() ? shift_time->GetValue() : shift_frames->GetValue());
	new_entry["fields"] = time_fields->GetSelection();
	new_entry["mode"] = selection_mode->GetSelection();
	new_entry["selection"] = std::move(shifted_blocks);

	history.insert(history.begin(), std::move(new_entry));
	if (history.size() > 50)
		history.resize(50);

	try {
		agi::JsonWriter::Write(history, agi::io::Save(history_filename).Get());
	}
	catch (agi::fs::FileSystemError const& e) {
		LOG_E("dialog_shift_times/save_history") << "Cannot save shift times history: " << e.GetMessage();
	}
}

void DialogShiftTimes::LoadHistory() {
	history_box->Clear();
	history_box->Freeze();

	try {
		json::UnknownElement root;
		json::Reader::Read(root, *agi::io::Open(history_filename));
		history = std::move(static_cast<json::Array&>(root));

		for (auto& history_entry : history)
			history_box->Append(get_history_string(history_entry));
	}
	catch (agi::fs::FileSystemError const& e) {
		LOG_D("dialog_shift_times/load_history") << "Cannot load shift times history: " << e.GetMessage();
	}
	catch (json::Exception const& e) {
		LOG_D("dialog_shift_times/load_history") << "Cannot load shift times history: " << e.what();
	}
	catch (...) {
		history_box->Thaw();
		throw;
	}

	history_box->Thaw();
}

void DialogShiftTimes::Process(wxCommandEvent &) {
	int mode = selection_mode->GetSelection();
	int type = time_fields->GetSelection();
	bool reverse = shift_backward->GetValue();
	bool by_time = shift_by_time->GetValue();

	bool start = type != 2;
	bool end = type != 1;

	auto const& sel = context->selectionController->GetSelectedSet();

	long shift;
	if (by_time) {
		shift = shift_time->GetTime();
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
	int block_start = 0;
	json::Array shifted_blocks;

	for (auto& line : context->ass->Events) {
		if (!sel.count(&line)) {
			if (block_start) {
				json::Object block;
				block["start"] = block_start;
				block["end"] = line.Row;
				shifted_blocks.push_back(std::move(block));
				block_start = 0;
			}
			if (mode == 1) continue;
			if (mode == 2 && shifted_blocks.empty()) continue;
		}
		else if (!block_start)
			block_start = line.Row + 1;

		if (start)
			line.Start = Shift(line.Start, shift, by_time, agi::vfr::START);
		if (end)
			line.End = Shift(line.End, shift, by_time, agi::vfr::END);
	}

	context->ass->Commit(_("shifting"), AssFile::COMMIT_DIAG_TIME);

	if (block_start) {
		json::Object block;
		block["start"] = block_start;
		block["end"] = context->ass->Events.back().Row + 1;
		shifted_blocks.push_back(std::move(block));
	}

	SaveHistory(std::move(shifted_blocks));
	Close();
}

int DialogShiftTimes::Shift(int initial_time, int shift, bool by_time, agi::vfr::Time type) {
	if (by_time)
		return initial_time + shift;
	else
		return fps.TimeAtFrame(shift + fps.FrameAtTime(initial_time, type), type);
}
}

void ShowShiftTimesDialog(agi::Context *c) {
	c->dialog->Show<DialogShiftTimes>(c);
}
