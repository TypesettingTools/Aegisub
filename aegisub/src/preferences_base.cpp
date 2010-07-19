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
#include "subtitles_provider_manager.h"
#include "video_provider_manager.h"
#include "audio_player_manager.h"
#include "audio_provider_manager.h"

#include "preferences_base.h"

/// Define make all platform-specific options visible in a single view.
#define SHOW_ALL 1

DEFINE_BASE_EXCEPTION_NOINNER(PreferencesError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceIncorrectType, PreferencesError, "preferences/incorrect_type")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PreferenceNotSupported, PreferencesError, "preferences/not_supported")



OptionPage::OptionPage(wxTreebook *book, wxString name, int style):
	wxScrolled<wxPanel>(book, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL) {

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

	agi::OptionValue *opt = OPT_GET(opt_name);

	int type = opt->GetType();

	switch (type) {

		case agi::OptionValue::Type_Bool: {
			wxCheckBox *cb = new wxCheckBox(this, wxID_ANY, name);
			flex->Add(cb, 1, wxEXPAND, 0);
			cb->SetValue(opt->GetBool());
			break;
		}

		case agi::OptionValue::Type_Int:
		case agi::OptionValue::Type_Double: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt(), inc);
			flex->Add(scd);

			break;
		}

		case agi::OptionValue::Type_String: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			wxTextCtrl *text = new wxTextCtrl(this, wxID_ANY , lagi_wxString(opt->GetString()), wxDefaultPosition, wxDefaultSize);
			flex->Add(text, 1, wxEXPAND);
			break;
		}

		case agi::OptionValue::Type_Colour: {
			flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
			flex->Add(new ColourButton(this, wxID_ANY, wxSize(40,10), lagi_wxColour(opt->GetColour())));
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}


void OptionPage::OptionChoice(wxFlexGridSizer *&flex, const wxString &name, const wxArrayString &choices, const char *opt_name) {
	agi::OptionValue *opt = OPT_GET(opt_name);

	int type = opt->GetType();
	wxString selection;

	switch (type) {
		case agi::OptionValue::Type_Int: {
			selection = choices.Item(opt->GetInt());
			break;
		}
		case agi::OptionValue::Type_String: {
			selection.assign(opt->GetString());
			break;
		}
		default:
			throw PreferenceNotSupported("Unsupported type");
	}

	flex->Add(new wxStaticText(this, wxID_ANY, name), 1, wxALIGN_CENTRE_VERTICAL);
	wxComboBox *cb = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY | wxCB_DROPDOWN);
	cb->SetValue(selection);
	flex->Add(cb, 1, wxEXPAND, 0);
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

	agi::OptionValue *opt = OPT_GET(opt_name);

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
}


void Preferences::OnOK(wxCommandEvent &event) {
	EndModal(0);
}


void Preferences::OnApply(wxCommandEvent &event) {
}


void Preferences::OnCancel(wxCommandEvent &event) {
	EndModal(0);
}

Preferences::Preferences(wxWindow *parent): wxDialog(parent, -1, _("Preferences"), wxDefaultPosition, wxSize(-1, 500)) {
//	SetIcon(BitmapToIcon(GETIMAGE(options_button_24)));

	book = new wxTreebook(this, -1, wxDefaultPosition, wxDefaultSize);
	general = new General(book);
	subtitles = new Subtitles(book);
	audio = new Audio(book);
	video = new Video(book);
	interface_ = new Interface(book);
	interface_colours = new Interface_Colours(book);
	interface_hotkeys = new Interface_Hotkeys(book);
	paths = new Paths(book);
	file_associations = new File_Associations(book);
	backup = new Backup(book);
	automation = new Automation(book);
	advanced = new Advanced(book);
	advanced_interface = new Advanced_Interface(book);
	advanced_audio = new Advanced_Audio(book);
	advanced_video = new Advanced_Video(book);

	book->Fit();

	/// @todo Save the last page and start with that page on next launch.
	book->ChangeSelection(0);

	// Bottom Buttons
	wxStdDialogButtonSizer *stdButtonSizer = new wxStdDialogButtonSizer();
	stdButtonSizer->AddButton(new wxButton(this,wxID_OK));
	stdButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	stdButtonSizer->AddButton(new wxButton(this,wxID_APPLY));
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



}

Preferences::~Preferences() {
}


BEGIN_EVENT_TABLE(Preferences, wxDialog)
    EVT_BUTTON(wxID_OK, Preferences::OnOK)
    EVT_BUTTON(wxID_CANCEL, Preferences::OnCancel)
    EVT_BUTTON(wxID_APPLY, Preferences::OnApply)
END_EVENT_TABLE()
