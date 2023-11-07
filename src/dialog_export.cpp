// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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

#include "ass_exporter.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "subtitle_format.h"
#include "utils.h"

#include <libaegisub/charset_conv.h>
#include <libaegisub/split.h>

#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace {
class DialogExport {
	wxDialog d;
	agi::Context *c;

	/// The export transform engine
	AssExporter exporter;

	/// The description of the currently selected export filter
	wxTextCtrl *filter_description;

	/// A list of all registered export filters
	wxCheckListBox *filter_list;

	/// A list of available target charsets
	wxChoice *charset_list;

	wxSizer *opt_sizer;

	void OnProcess(wxCommandEvent &);
	void OnChange(wxCommandEvent &);

	/// Set all the checkboxes
	void SetAll(bool new_value);
	/// Update which options sizers are shown
	void RefreshOptions();

public:
	DialogExport(agi::Context *c);
	~DialogExport();
	void ShowModal() { d.ShowModal(); }
};

// Swap the items at idx and idx + 1
void swap(wxCheckListBox *list, int idx, int sel_dir) {
	if (idx < 0 || idx + 1 == (int)list->GetCount()) return;

	list->Freeze();
	wxString tempname = list->GetString(idx);
	bool tempval = list->IsChecked(idx);
	list->SetString(idx, list->GetString(idx + 1));
	list->Check(idx, list->IsChecked(idx + 1));
	list->SetString(idx + 1, tempname);
	list->Check(idx + 1, tempval);
	list->SetSelection(idx + sel_dir);
	list->Thaw();
}

DialogExport::DialogExport(agi::Context *c)
: d(c->parent, -1, _("Export"), wxDefaultPosition, wxSize(200, 100), wxCAPTION | wxCLOSE_BOX)
, c(c)
, exporter(c)
{
	d.SetIcon(GETICON(export_menu_16));
	d.SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	std::vector<std::string> filters = exporter.GetAllFilterNames();
	filter_list = new wxCheckListBox(&d, -1, wxDefaultPosition, wxSize(200, 100), to_wx(filters));
	filter_list->Bind(wxEVT_CHECKLISTBOX, [=,  this](wxCommandEvent&) { RefreshOptions(); });
	filter_list->Bind(wxEVT_LISTBOX, &DialogExport::OnChange, this);

	// Get selected filters
	std::string const& selected = c->ass->Properties.export_filters;
	for (auto token : agi::Split(selected, '|')) {
		auto it = find(begin(filters), end(filters), token);
		if (it != end(filters))
			filter_list->Check(distance(begin(filters), it));
	}

	wxButton *btn_up = new wxButton(&d, -1, _("Move &Up"), wxDefaultPosition, wxSize(90, -1));
	wxButton *btn_down = new wxButton(&d, -1, _("Move &Down"), wxDefaultPosition, wxSize(90, -1));
	wxButton *btn_all = new wxButton(&d, -1, _("Select &All"), wxDefaultPosition, wxSize(80, -1));
	wxButton *btn_none = new wxButton(&d, -1, _("Select &None"), wxDefaultPosition, wxSize(80, -1));

	btn_up->Bind(wxEVT_BUTTON, [=,  this](wxCommandEvent&) { swap(filter_list, filter_list->GetSelection() - 1, 0); });
	btn_down->Bind(wxEVT_BUTTON, [=,  this](wxCommandEvent&) { swap(filter_list, filter_list->GetSelection(), 1); });
	btn_all->Bind(wxEVT_BUTTON, [=,  this](wxCommandEvent&) { SetAll(true); });
	btn_none->Bind(wxEVT_BUTTON, [=,  this](wxCommandEvent&) { SetAll(false); });

	wxSizer *top_buttons = new wxBoxSizer(wxHORIZONTAL);
	top_buttons->Add(btn_up, wxSizerFlags(1).Expand());
	top_buttons->Add(btn_down, wxSizerFlags(1).Expand().Border(wxRIGHT));
	top_buttons->Add(btn_all, wxSizerFlags(1).Expand());
	top_buttons->Add(btn_none, wxSizerFlags(1).Expand());

	filter_description = new wxTextCtrl(&d, -1, "", wxDefaultPosition, wxSize(200, 60), wxTE_MULTILINE | wxTE_READONLY);

	// Charset dropdown list
	wxStaticText *charset_list_label = new wxStaticText(&d, -1, _("Text encoding:"));
	charset_list = new wxChoice(&d, -1, wxDefaultPosition, wxDefaultSize, agi::charset::GetEncodingsList<wxArrayString>());
	wxSizer *charset_list_sizer = new wxBoxSizer(wxHORIZONTAL);
	charset_list_sizer->Add(charset_list_label, wxSizerFlags().Center().Border(wxRIGHT));
	charset_list_sizer->Add(charset_list, wxSizerFlags(1).Expand());
	if (!charset_list->SetStringSelection(to_wx(c->ass->Properties.export_encoding)))
		charset_list->SetStringSelection("Unicode (UTF-8)");

	wxSizer *top_sizer = new wxStaticBoxSizer(wxVERTICAL, &d, _("Filters"));
	top_sizer->Add(filter_list, wxSizerFlags(1).Expand());
	top_sizer->Add(top_buttons, wxSizerFlags(0).Expand());
	top_sizer->Add(filter_description, wxSizerFlags(0).Expand().Border(wxTOP));
	top_sizer->Add(charset_list_sizer, wxSizerFlags(0).Expand().Border(wxTOP));

	auto btn_sizer = d.CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxHELP);
	btn_sizer->GetAffirmativeButton()->SetLabelText(_("Export..."));
	d.Bind(wxEVT_BUTTON, &DialogExport::OnProcess, this, wxID_OK);
	d.Bind(wxEVT_BUTTON, std::bind(&HelpButton::OpenPage, "Export"), wxID_HELP);

	wxSizer *horz_sizer = new wxBoxSizer(wxHORIZONTAL);
	opt_sizer = new wxBoxSizer(wxVERTICAL);
	exporter.DrawSettings(&d, opt_sizer);
	horz_sizer->Add(top_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxRIGHT));
	horz_sizer->Add(opt_sizer, wxSizerFlags(1).Expand().Border(wxALL & ~wxLEFT));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(horz_sizer, wxSizerFlags(1).Expand());
	main_sizer->Add(btn_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxTOP));
	d.SetSizerAndFit(main_sizer);
	RefreshOptions();
	d.CenterOnParent();
}

DialogExport::~DialogExport() {
	c->ass->Properties.export_filters.clear();
	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (filter_list->IsChecked(i)) {
			if (!c->ass->Properties.export_filters.empty())
				c->ass->Properties.export_filters += "|";
			c->ass->Properties.export_filters += from_wx(filter_list->GetString(i));
		}
	}
}

void DialogExport::OnProcess(wxCommandEvent &) {
	if (!d.TransferDataFromWindow()) return;

	auto filename = SaveFileSelector(_("Export subtitles file"), "", "", "", SubtitleFormat::GetWildcards(1), &d);
	if (filename.empty()) return;

	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (filter_list->IsChecked(i))
			exporter.AddFilter(from_wx(filter_list->GetString(i)));
	}

	try {
		wxBusyCursor busy;
		c->ass->Properties.export_encoding = from_wx(charset_list->GetStringSelection());
		exporter.Export(filename, from_wx(charset_list->GetStringSelection()), &d);
	}
	catch (agi::UserCancelException const&) { }
	catch (agi::Exception const& err) {
		wxMessageBox(to_wx(err.GetMessage()), "Error exporting subtitles", wxOK | wxICON_ERROR | wxCENTER, &d);
	}
	catch (std::exception const& err) {
		wxMessageBox(to_wx(err.what()), "Error exporting subtitles", wxOK | wxICON_ERROR | wxCENTER, &d);
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error exporting subtitles", wxOK | wxICON_ERROR | wxCENTER, &d);
	}

	d.EndModal(0);
}

void DialogExport::OnChange(wxCommandEvent &) {
	wxString sel = filter_list->GetStringSelection();
	if (!sel.empty())
		filter_description->SetValue(to_wx(exporter.GetDescription(from_wx(sel))));
}

void DialogExport::SetAll(bool new_value) {
	filter_list->Freeze();
	for (size_t i = 0; i < filter_list->GetCount(); ++i)
		filter_list->Check(i, new_value);
	filter_list->Thaw();

	RefreshOptions();
}

void DialogExport::RefreshOptions() {
	for (size_t i = 0; i < filter_list->GetCount(); ++i) {
		if (wxSizer *sizer = exporter.GetSettingsSizer(from_wx(filter_list->GetString(i))))
			opt_sizer->Show(sizer, filter_list->IsChecked(i), true);
	}
	d.Layout();
	d.GetSizer()->Fit(&d);
}
}

void ShowExportDialog(agi::Context *c) {
	DialogExport(c).ShowModal();
}
