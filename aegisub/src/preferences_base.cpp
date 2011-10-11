// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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
// $Id$

/// @file preferences_base.cpp
/// @brief Base preferences dialogue classes
/// @ingroup configuration_ui


#ifndef AGI_PRE
#include <wx/any.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/event.h>
#include <wx/filefn.h>
#include <wx/fontdlg.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/treebook.h>
#include <wx/treebook.h>
#endif

#include <libaegisub/exception.h>

#include "preferences_base.h"

#include "colour_button.h"
#include "compat.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "preferences.h"
#include "standard_paths.h"
#include "video_provider_manager.h"

DEFINE_BASE_EXCEPTION_NOINNER(PreferencesError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceIncorrectType, PreferencesError, "preferences/incorrect_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceNotSupported, PreferencesError, "preferences/not_supported")

#define OPTION_UPDATER(type, evttype, body)                               \
	class type {                                                          \
		std::string name;                                                 \
		Preferences *parent;                                              \
	public:                                                               \
		type(std::string const& n, Preferences *p) : name(n),parent(p) {} \
		void operator()(evttype& evt) { parent->SetOption(name, body); }  \
	}

OPTION_UPDATER(StringUpdater, wxCommandEvent, STD_STR(evt.GetString()));
OPTION_UPDATER(IntUpdater, wxSpinEvent, evt.GetInt());
OPTION_UPDATER(IntCBUpdater, wxCommandEvent, evt.GetInt());
OPTION_UPDATER(DoubleUpdater, wxSpinEvent, evt.GetInt());
OPTION_UPDATER(BoolUpdater, wxCommandEvent, !!evt.GetInt());
class ColourUpdater {
	const char *name;
	Preferences *parent;
public:
	ColourUpdater(const char *n = "", Preferences *p = NULL) : name(n), parent(p) { }
	void operator()(wxCommandEvent& evt) {
		ColourButton *btn = static_cast<ColourButton*>(evt.GetClientData());
		if (btn) {
			parent->SetOption(name, STD_STR(btn->GetColour().GetAsString(wxC2S_CSS_SYNTAX)));
		}
		else {
			evt.Skip();
		}
	}
};

static void browse_button(wxTextCtrl *ctrl) {
	wxString def = StandardPaths::DecodePathMaybeRelative(ctrl->GetValue(), "?user/");
	wxDirDialog dlg(0, _("Please choose the folder:"), def);
	if (dlg.ShowModal() == wxID_OK) {
		wxString dir = StandardPaths::EncodePath(dlg.GetPath());
		if (!dir.empty())
			ctrl->SetValue(dir);
	}
}

static void font_button(Preferences *parent, wxTextCtrl *name, wxSpinCtrl *size) {
	wxFont font;
	font.SetFaceName(name->GetValue());
	font.SetPointSize(size->GetValue());
	font = wxGetFontFromUser(parent, font);
	if (font.IsOk()) {
		name->SetValue(font.GetFaceName());
		size->SetValue(wxString::Format("%d", font.GetPointSize()));
	}
}

OptionPage::OptionPage(wxTreebook *book, Preferences *parent, wxString name, int style)
: wxScrolled<wxPanel>(book, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
, parent(parent)
{
	if (style & PAGE_SUB)
		book->AddSubPage(this, name, true);
	else
		book->AddPage(this, name, true);

	if (style & PAGE_SCROLL)
		SetScrollbars(0, 20, 0, 50);
	else
		SetScrollbars(0, 0, 0, 0);

	sizer = new wxBoxSizer(wxVERTICAL);
}

template<class T>
void OptionPage::Add(wxSizer *sizer, wxString const& label, T *control) {
	sizer->Add(new wxStaticText(this, -1, label), 1, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
	sizer->Add(control);
}

void OptionPage::CellSkip(wxFlexGridSizer *flex) {
	flex->Add(new wxStaticText(this, -1, ""), wxSizerFlags().Border());
}

void OptionPage::OptionAdd(wxFlexGridSizer *flex, const wxString &name, const char *opt_name, double min, double max, double inc) {
	const agi::OptionValue *opt = OPT_GET(opt_name);

	switch (opt->GetType()) {
		case agi::OptionValue::Type_Bool: {
			wxCheckBox *cb = new wxCheckBox(this, -1, name);
			flex->Add(cb, 1, wxEXPAND, 0);
			cb->SetValue(opt->GetBool());
			cb->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, BoolUpdater(opt_name, parent));
			break;
		}

		case agi::OptionValue::Type_Int: {
			wxSpinCtrl *sc = new wxSpinCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt());
			sc->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, IntUpdater(opt_name, parent));
			Add(flex, name, sc);
			break;
		}

		case agi::OptionValue::Type_Double: {
			wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetDouble(), inc);
			scd->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, DoubleUpdater(opt_name, parent));
			Add(flex, name, scd);
			break;
		}

		case agi::OptionValue::Type_String: {
			wxTextCtrl *text = new wxTextCtrl(this, -1 , lagi_wxString(opt->GetString()));
			text->Bind(wxEVT_COMMAND_TEXT_UPDATED, StringUpdater(opt_name, parent));
			Add(flex, name, text);
			break;
		}

		case agi::OptionValue::Type_Colour: {
			ColourButton *cb = new ColourButton(this, -1, wxSize(40,10), lagi_wxColour(opt->GetColour()));
			cb->Bind(wxEVT_COMMAND_BUTTON_CLICKED, ColourUpdater(opt_name, parent));
			Add(flex, name, cb);
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}

void OptionPage::OptionChoice(wxFlexGridSizer *flex, const wxString &name, const wxArrayString &choices, const char *opt_name) {
	const agi::OptionValue *opt = OPT_GET(opt_name);

	wxComboBox *cb = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY | wxCB_DROPDOWN);
	Add(flex, name, cb);

	switch (opt->GetType()) {
		case agi::OptionValue::Type_Int: {
			int val = opt->GetInt();
			cb->SetValue(choices[val < (int)choices.size() ? val : opt->GetDefaultInt()]);
			cb->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, IntCBUpdater(opt_name, parent));
			break;
		}
		case agi::OptionValue::Type_String:
			cb->SetValue(lagi_wxString(opt->GetString()));
			cb->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, StringUpdater(opt_name, parent));
			break;

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}

wxFlexGridSizer* OptionPage::PageSizer(wxString name) {
	wxSizer *tmp_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, name);
	sizer->Add(tmp_sizer, 0,wxEXPAND, 5);
	wxFlexGridSizer *flex = new wxFlexGridSizer(2,5,5);
	flex->AddGrowableCol(0,1);
	tmp_sizer->Add(flex, 1, wxEXPAND, 5);
	sizer->AddSpacer(8);
	return flex;
}

void OptionPage::OptionBrowse(wxFlexGridSizer *flex, const wxString &name, const char *opt_name) {
	const agi::OptionValue *opt = OPT_GET(opt_name);

	if (opt->GetType() != agi::OptionValue::Type_String)
		throw PreferenceIncorrectType("Option must be agi::OptionValue::Type_String for BrowseButton.");

	wxTextCtrl *text = new wxTextCtrl(this, -1 , opt->GetString());
	text->Bind(wxEVT_COMMAND_TEXT_UPDATED, StringUpdater(opt_name, parent));

	wxButton *browse = new wxButton(this, -1, _("Browse..."));
	browse->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(browse_button, text));

	wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(text, wxSizerFlags(1).Expand());
	button_sizer->Add(browse, wxSizerFlags().Expand());

	Add(flex, name, button_sizer);
}

void OptionPage::OptionFont(wxSizer *sizer, std::string opt_prefix) {
	const agi::OptionValue *face_opt = OPT_GET(opt_prefix + "Font Face");
	const agi::OptionValue *size_opt = OPT_GET(opt_prefix + "Font Size");

	wxTextCtrl *font_name = new wxTextCtrl(this, -1, face_opt->GetString());
	font_name->Bind(wxEVT_COMMAND_TEXT_UPDATED, StringUpdater(face_opt->GetName().c_str(), parent));

	wxSpinCtrl *font_size = new wxSpinCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 3, 42, size_opt->GetInt());
	font_size->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, IntUpdater(size_opt->GetName().c_str(), parent));

	wxButton *pick_btn = new wxButton(this, -1, _("Choose..."));
	pick_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::tr1::bind(font_button, parent, font_name, font_size));

	wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(font_name, wxSizerFlags(1).Expand());
	button_sizer->Add(pick_btn, wxSizerFlags().Expand());

	Add(sizer, _("Font Face"), button_sizer);
	Add(sizer, _("Font Size"), font_size);
}

void Preferences::SetOption(std::string const& name, wxAny value) {
	pending_changes[name] = value;
	if (IsEnabled())
		applyButton->Enable(true);
}

void Preferences::OnOK(wxCommandEvent &event) {
	OnApply(event);
	EndModal(0);
}

void Preferences::OnApply(wxCommandEvent &event) {
	for (std::map<std::string, wxAny>::iterator cur = pending_changes.begin(); cur != pending_changes.end(); ++cur) {
		agi::OptionValue *opt = OPT_SET(cur->first);
		switch (opt->GetType()) {
			case agi::OptionValue::Type_Bool:
				opt->SetBool(cur->second.As<bool>());
				break;
			case agi::OptionValue::Type_Colour:
				opt->SetColour(cur->second.As<agi::Colour>());
				break;
			case agi::OptionValue::Type_Double:
				opt->SetDouble(cur->second.As<double>());
				break;
			case agi::OptionValue::Type_Int:
				opt->SetInt(cur->second.As<int>());
				break;
			case agi::OptionValue::Type_String:
				opt->SetString(cur->second.As<std::string>());
				break;
			default:
				throw PreferenceNotSupported("Unsupported type");
		}
	}
	pending_changes.clear();
	applyButton->Enable(false);
	config::opt->Flush();
}

static void PageChanged(wxBookCtrlEvent& evt) {
	OPT_SET("Tool/Preferences/Page")->SetInt(evt.GetSelection());
}

Preferences::Preferences(wxWindow *parent): wxDialog(parent, -1, _("Preferences"), wxDefaultPosition, wxSize(-1, 500)) {
//	SetIcon(BitmapToIcon(GETIMAGE(options_button_24)));

	book = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	new General(book, this);
	new Subtitles(book, this);
	new Audio(book, this);
	new Video(book, this);
	new Interface(book, this);
	new Interface_Colours(book, this);
	new Interface_Hotkeys(book, this);
	new Paths(book, this);
	new File_Associations(book, this);
	new Backup(book, this);
	new Automation(book, this);
	new Advanced(book, this);
	new Advanced_Interface(book, this);
	new Advanced_Audio(book, this);
	new Advanced_Video(book, this);

	book->Fit();

	book->ChangeSelection(OPT_GET("Tool/Preferences/Page")->GetInt());
	book->Bind(wxEVT_COMMAND_TREEBOOK_PAGE_CHANGED, &PageChanged);

	// Bottom Buttons
	wxStdDialogButtonSizer *stdButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxAPPLY);
	applyButton = stdButtonSizer->GetApplyButton();
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *defaultButton = new wxButton(this, -1, _("Restore Defaults"));
	buttonSizer->Add(defaultButton, wxSizerFlags(0).Expand());
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(stdButtonSizer, wxSizerFlags(0).Expand());

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, wxSizerFlags(1).Expand().Border());
	mainSizer->Add(buttonSizer, wxSizerFlags(0).Expand().Border(wxALL & ~wxTOP));

	SetSizerAndFit(mainSizer);
	SetMinSize(wxSize(-1, 500));
	SetMaxSize(wxSize(-1, 500));
	CenterOnParent();

	applyButton->Enable(false);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnOK, this, wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Preferences::OnApply, this, wxID_APPLY);
}

Preferences::~Preferences() {
}
