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

/// @file preferences_base.cpp
/// @brief Base preferences dialogue classes
/// @ingroup configuration_ui

#include "preferences_base.h"

#include "colour_button.h"
#include "compat.h"
#include "include/aegisub/audio_player.h"
#include "include/aegisub/audio_provider.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "preferences.h"
#include "video_provider_manager.h"

#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dirdlg.h>
#include <wx/event.h>
#include <wx/filefn.h>
#include <wx/fontdlg.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/treebook.h>

#define OPTION_UPDATER(type, evttype, opt, body)                            \
	class type {                                                            \
		std::string name;                                                   \
		Preferences *parent;                                                \
	public:                                                                 \
		type(std::string const& n, Preferences *p) : name(n), parent(p) { } \
		void operator()(evttype& evt) {                                     \
			evt.Skip();                                                     \
			parent->SetOption(agi::util::make_unique<agi::opt>(name, body));\
		}                                                                   \
	}

OPTION_UPDATER(StringUpdater, wxCommandEvent, OptionValueString, from_wx(evt.GetString()));
OPTION_UPDATER(IntUpdater, wxSpinEvent, OptionValueInt, evt.GetInt());
OPTION_UPDATER(IntCBUpdater, wxCommandEvent, OptionValueInt, evt.GetInt());
OPTION_UPDATER(DoubleUpdater, wxSpinEvent, OptionValueDouble, evt.GetInt());
OPTION_UPDATER(BoolUpdater, wxCommandEvent, OptionValueBool, !!evt.GetInt());
OPTION_UPDATER(ColourUpdater, wxThreadEvent, OptionValueColor, evt.GetPayload<agi::Color>());

static void browse_button(wxTextCtrl *ctrl) {
	wxDirDialog dlg(nullptr, _("Please choose the folder:"), config::path->Decode(from_wx(ctrl->GetValue())).wstring());
	if (dlg.ShowModal() == wxID_OK) {
		wxString dir = dlg.GetPath();
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
		size->SetValue(font.GetPointSize());
		// wxGTK doesn't generate wxEVT_SPINCTRL from SetValue
		wxSpinEvent evt(wxEVT_SPINCTRL);
		evt.SetInt(font.GetPointSize());
		size->ProcessWindowEvent(evt);
	}
}

OptionPage::OptionPage(wxTreebook *book, Preferences *parent, wxString name, int style)
: wxScrolled<wxPanel>(book, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
, sizer(new wxBoxSizer(wxVERTICAL))
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
	DisableKeyboardScrolling();
}

template<class T>
void OptionPage::Add(wxSizer *sizer, wxString const& label, T *control) {
	sizer->Add(new wxStaticText(this, -1, label), 1, wxALIGN_CENTRE_VERTICAL);
	sizer->Add(control, wxSizerFlags().Expand());
}

void OptionPage::CellSkip(wxFlexGridSizer *flex) {
	flex->AddStretchSpacer();
}

wxControl *OptionPage::OptionAdd(wxFlexGridSizer *flex, const wxString &name, const char *opt_name, double min, double max, double inc) {
	parent->AddChangeableOption(opt_name);
	const agi::OptionValue *opt = OPT_GET(opt_name);

	switch (opt->GetType()) {
		case agi::OptionValue::Type_Bool: {
			wxCheckBox *cb = new wxCheckBox(this, -1, name);
			flex->Add(cb, 1, wxEXPAND, 0);
			cb->SetValue(opt->GetBool());
			cb->Bind(wxEVT_CHECKBOX, BoolUpdater(opt_name, parent));
			return cb;
		}

		case agi::OptionValue::Type_Int: {
			wxSpinCtrl *sc = new wxSpinCtrl(this, -1, std::to_wstring((int)opt->GetInt()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetInt());
			sc->Bind(wxEVT_SPINCTRL, IntUpdater(opt_name, parent));
			Add(flex, name, sc);
			return sc;
		}

		case agi::OptionValue::Type_Double: {
			wxSpinCtrlDouble *scd = new wxSpinCtrlDouble(this, -1, wxString::Format("%g", opt->GetDouble()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, opt->GetDouble(), inc);
			scd->Bind(wxEVT_SPINCTRL, DoubleUpdater(opt_name, parent));
			Add(flex, name, scd);
			return scd;
		}

		case agi::OptionValue::Type_String: {
			wxTextCtrl *text = new wxTextCtrl(this, -1 , to_wx(opt->GetString()));
			text->Bind(wxEVT_TEXT, StringUpdater(opt_name, parent));
			Add(flex, name, text);
			return text;
		}

		case agi::OptionValue::Type_Color: {
			auto cb = new ColourButton(this, wxSize(40,10), false, opt->GetColor());
			cb->Bind(EVT_COLOR, ColourUpdater(opt_name, parent));
			Add(flex, name, cb);
			return cb;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}

void OptionPage::OptionChoice(wxFlexGridSizer *flex, const wxString &name, const wxArrayString &choices, const char *opt_name) {
	parent->AddChangeableOption(opt_name);
	const agi::OptionValue *opt = OPT_GET(opt_name);

	wxComboBox *cb = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices, wxCB_READONLY | wxCB_DROPDOWN);
	Add(flex, name, cb);

	switch (opt->GetType()) {
		case agi::OptionValue::Type_Int: {
			int val = opt->GetInt();
			cb->Select(val < (int)choices.size() ? val : 0);
			cb->Bind(wxEVT_COMBOBOX, IntCBUpdater(opt_name, parent));
			break;
		}
		case agi::OptionValue::Type_String: {
			wxString val(to_wx(opt->GetString()));
			if (cb->FindString(val) != wxNOT_FOUND)
				cb->SetStringSelection(val);
			else if (!choices.empty())
				cb->SetSelection(0);
			cb->Bind(wxEVT_COMBOBOX, StringUpdater(opt_name, parent));
			break;
		}

		default:
			throw PreferenceNotSupported("Unsupported type");
	}
}

wxFlexGridSizer* OptionPage::PageSizer(wxString name) {
	wxSizer *tmp_sizer = new wxStaticBoxSizer(wxHORIZONTAL, this, name);
	sizer->Add(tmp_sizer, 0,wxEXPAND, 5);
	auto flex = new wxFlexGridSizer(2,5,5);
	flex->AddGrowableCol(0,1);
	tmp_sizer->Add(flex, 1, wxEXPAND, 5);
	sizer->AddSpacer(8);
	return flex;
}

void OptionPage::OptionBrowse(wxFlexGridSizer *flex, const wxString &name, const char *opt_name, wxControl *enabler, bool do_enable) {
	parent->AddChangeableOption(opt_name);
	const agi::OptionValue *opt = OPT_GET(opt_name);

	if (opt->GetType() != agi::OptionValue::Type_String)
		throw PreferenceIncorrectType("Option must be agi::OptionValue::Type_String for BrowseButton.");

	wxTextCtrl *text = new wxTextCtrl(this, -1 , to_wx(opt->GetString()));
	text->SetMinSize(wxSize(160, -1));
	text->Bind(wxEVT_TEXT, StringUpdater(opt_name, parent));

	wxButton *browse = new wxButton(this, -1, _("Browse..."));
	browse->Bind(wxEVT_BUTTON, std::bind(browse_button, text));

	wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(text, wxSizerFlags(1).Expand());
	button_sizer->Add(browse, wxSizerFlags().Expand());

	Add(flex, name, button_sizer);

	if (enabler) {
		if (do_enable) {
			EnableIfChecked(enabler, text);
			EnableIfChecked(enabler, browse);
		}
		else {
			DisableIfChecked(enabler, text);
			DisableIfChecked(enabler, browse);
		}
	}
}

void OptionPage::OptionFont(wxSizer *sizer, std::string opt_prefix) {
	const agi::OptionValue *face_opt = OPT_GET(opt_prefix + "Font Face");
	const agi::OptionValue *size_opt = OPT_GET(opt_prefix + "Font Size");

	parent->AddChangeableOption(face_opt->GetName());
	parent->AddChangeableOption(size_opt->GetName());

	wxTextCtrl *font_name = new wxTextCtrl(this, -1, to_wx(face_opt->GetString()));
	font_name->SetMinSize(wxSize(160, -1));
	font_name->Bind(wxEVT_TEXT, StringUpdater(face_opt->GetName().c_str(), parent));

	wxSpinCtrl *font_size = new wxSpinCtrl(this, -1, std::to_wstring((int)size_opt->GetInt()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 3, 42, size_opt->GetInt());
	font_size->Bind(wxEVT_SPINCTRL, IntUpdater(size_opt->GetName().c_str(), parent));

	wxButton *pick_btn = new wxButton(this, -1, _("Choose..."));
	pick_btn->Bind(wxEVT_BUTTON, std::bind(font_button, parent, font_name, font_size));

	wxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(font_name, wxSizerFlags(1).Expand());
	button_sizer->Add(pick_btn, wxSizerFlags().Expand());

	Add(sizer, _("Font Face"), button_sizer);
	Add(sizer, _("Font Size"), font_size);
}

struct disabler {
	wxControl *ctrl;
	bool enable;

	disabler(wxControl *ctrl, bool enable) : ctrl(ctrl), enable(enable) { }
	void operator()(wxCommandEvent &evt) {
		ctrl->Enable(!!evt.GetInt() == enable);
		evt.Skip();
	}
};

void OptionPage::EnableIfChecked(wxControl *cbx, wxControl *ctrl) {
	wxCheckBox *cb = dynamic_cast<wxCheckBox*>(cbx);
	if (!cb) return;

	ctrl->Enable(cb->IsChecked());
	cb->Bind(wxEVT_CHECKBOX, disabler(ctrl, true));
}

void OptionPage::DisableIfChecked(wxControl *cbx, wxControl *ctrl) {
	wxCheckBox *cb = dynamic_cast<wxCheckBox*>(cbx);
	if (!cb) return;

	ctrl->Enable(!cb->IsChecked());
	cb->Bind(wxEVT_CHECKBOX, disabler(ctrl, false));
}
