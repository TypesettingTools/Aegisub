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
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/filefn.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/treebook.h>
#endif

#include <libaegisub/exception.h>

#include "colour_button.h"
#include "compat.h"
#include "libresrc/libresrc.h"
#include "preferences.h"
#include "main.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "video_provider_manager.h"

#include "preferences_base.h"

/// Define make all platform-specific options visible in a single view.
#define SHOW_ALL 1

DEFINE_BASE_EXCEPTION_NOINNER(PreferencesError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceIncorrectType, PreferencesError, "preferences/incorrect_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceNotSupported, PreferencesError, "preferences/not_supported")

#define OPTION_UPDATER(type, evttype, body)                               \
	class type {                                                          \
		const char *name;                                                 \
		Preferences *parent;                                              \
	public:                                                               \
		type(const char *n="",Preferences *p=NULL) : name(n),parent(p) {} \
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

OptionPage::OptionPage(wxTreebook *book, Preferences *parent, wxString name, int style)
: wxScrolled<wxPanel>(book, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
, parent(parent)
{

	if (style & PAGE_SUB) {
		book->AddSubPage(this, name, true);
	} else {
		book->AddPage(this, name, true);
	}

	if (style & PAGE_SCROLL) {
		SetScrollbars(0, 20, 0, 50);
	} else {
		SetScrollbars(0, 0, 0, 0);
	}

	sizer = new wxBoxSizer(wxVERTICAL);
}

OptionPage::~OptionPage() {}

void OptionPage::CellSkip(wxFlexGridSizer *&flex) {
	flex->Add(new wxStaticText(this, wxID_ANY , wxEmptyString), 0, wxALL, 5);
}

void OptionPage::OptionAdd(wxFlexGridSizer *&flex, const wxString &name, const char *opt_name, double min, double max, double inc) {

	const agi::OptionValue *opt = OPT_GET(opt_name);

	int type = opt->GetType();

	switch (type) {

		case agi::OptionValue::Type_Bool: {
			wxCheckBox *cb = new wxCheckBox(this, wxID_ANY, name);
			flex->Add(cb, 1, wxEXPAND, 0);
			cb->SetValue(opt->GetBool());
			cb->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, BoolUpdater(opt_name, parent));
			break;
		}

		case agi::OptionValue::Type_Int: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxSpinCtrl *sc = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt());
			sc->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, IntUpdater(opt_name, parent));
			flex->Add(sc);

			break;
		}
		case agi::OptionValue::Type_Double: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetDouble(), inc);
			scd->Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, DoubleUpdater(opt_name, parent));
			flex->Add(scd);

			break;
		}

		case agi::OptionValue::Type_String: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxTextCtrl *text = new wxTextCtrl(this, wxID_ANY , lagi_wxString(opt->GetString()), wxDefaultPosition, wxDefaultSize);
			flex->Add(text, 1, wxEXPAND);
			text->Bind(wxEVT_COMMAND_TEXT_UPDATED, StringUpdater(opt_name, parent));
			break;
		}

		case agi::OptionValue::Type_Colour: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			ColourButton *cb = new ColourButton(this, wxID_ANY, wxSize(40,10), lagi_wxColour(opt->GetColour()));
			flex->Add(cb);
			cb->Bind(wxEVT_COMMAND_BUTTON_CLICKED, ColourUpdater(opt_name, parent));
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}


void OptionPage::OptionChoice(wxFlexGridSizer *&flex, const wxString &name, const wxArrayString &choices, const char *opt_name) {
	const agi::OptionValue *opt = OPT_GET(opt_name);

	flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
	wxComboBox *cb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY | wxCB_DROPDOWN);
	flex->Add(cb, 1, wxEXPAND, 0);

	switch (opt->GetType()) {
		case agi::OptionValue::Type_Int: {
			int val = opt->GetInt();
			cb->SetValue(choices[val < (int)choices.size() ? val : opt->GetDefaultInt()]);
			cb->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, IntCBUpdater(opt_name, parent));
			break;
		}
		case agi::OptionValue::Type_String: {
			cb->SetValue(lagi_wxString(opt->GetString()));
			cb->Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, StringUpdater(opt_name, parent));
			break;
		}
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


void OptionPage::OptionBrowse(wxFlexGridSizer *&flex, const wxString &name, BrowseType browse_type, const char *opt_name) {
	const agi::OptionValue *opt = OPT_GET(opt_name);

	if (opt->GetType() != agi::OptionValue::Type_String)
		throw PreferenceIncorrectType("Option must be agi::OptionValue::Type_String for BrowseButton.");

	flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);

	wxFlexGridSizer *button_flex = new wxFlexGridSizer(2,5,5);
	button_flex->AddGrowableCol(0,1);
	flex->Add(button_flex, 1, wxEXPAND, 5);

	wxTextCtrl *text = new wxTextCtrl(this, wxID_ANY , opt->GetString(), wxDefaultPosition, wxDefaultSize);
	button_flex->Add(text, 1, wxEXPAND);
	BrowseButton *browse = new BrowseButton(this, wxID_ANY, wxEmptyString, browse_type);
	browse->Bind(text);
	button_flex->Add(browse, 1, wxEXPAND);
	text->Bind(wxEVT_COMMAND_TEXT_UPDATED, StringUpdater(opt_name, parent));
}

void Preferences::SetOption(const char *name, wxAny value) {
	pending_changes[name] = value;
	if (IsEnabled()) applyButton->Enable(true);
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


void Preferences::OnCancel(wxCommandEvent &event) {
	EndModal(0);
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
	wxStdDialogButtonSizer *stdButtonSizer = new wxStdDialogButtonSizer();
	stdButtonSizer->AddButton(new wxButton(this,wxID_OK));
	stdButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	stdButtonSizer->AddButton(applyButton = new wxButton(this,wxID_APPLY));
	stdButtonSizer->Realize();
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *defaultButton = new wxButton(this,2342,_("Restore Defaults"));
	buttonSizer->Add(defaultButton,0,wxEXPAND);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(stdButtonSizer,0,wxEXPAND);


	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book, 1 ,wxEXPAND | wxALL, 5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	SetSizerAndFit(mainSizer);
	this->SetMinSize(wxSize(-1, 500));
	this->SetMaxSize(wxSize(-1, 500));
	CenterOnParent();

	applyButton->Enable(false);
}

Preferences::~Preferences() {
}

BEGIN_EVENT_TABLE(Preferences, wxDialog)
	EVT_BUTTON(wxID_OK, Preferences::OnOK)
	EVT_BUTTON(wxID_CANCEL, Preferences::OnCancel)
	EVT_BUTTON(wxID_APPLY, Preferences::OnApply)
END_EVENT_TABLE()
