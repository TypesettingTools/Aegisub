// Copyright (c) 2011 Niels Martin Hansen <nielsm@aegisub.org>
// Copyright (c) 2012 Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file dialog_export_ebu3264.cpp
/// @see dialog_export_ebu3264.h
/// @ingroup subtitle_io export

#include "dialog_export_ebu3264.h"

#include "compat.h"
#include "format.h"
#include "options.h"

#include <libaegisub/charset_conv.h>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>

namespace {
	const boost::regex timecode_regex("([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2}):([[:digit:]]{2})");

	/// Validator for SMPTE timecodes
	class TimecodeValidator final : public wxValidator {
		EbuTimecode *value;

		wxTextCtrl *GetCtrl() const { return dynamic_cast<wxTextCtrl*>(GetWindow()); }

		bool TransferToWindow() override {
			wxTextCtrl *ctrl = GetCtrl();
			if (!ctrl) return false;
			ctrl->SetValue(fmt_wx("%02d:%02d:%02d:%02d", value->h, value->m, value->s, value->f));
			return true;
		}

		bool TransferFromWindow() override {
			wxTextCtrl *ctrl = GetCtrl();
			if (!ctrl) return false;

			std::string str = from_wx(ctrl->GetValue());
			boost::smatch result;
			if (!regex_match(str, result, timecode_regex))
				return false;

			value->h = boost::lexical_cast<int>(result.str(1));
			value->m = boost::lexical_cast<int>(result.str(2));
			value->s = boost::lexical_cast<int>(result.str(3));
			value->f = boost::lexical_cast<int>(result.str(4));

			return true;
		}

		bool Validate(wxWindow *) override {
			wxTextCtrl *ctrl = GetCtrl();
			if (!ctrl) return false;

			if (!regex_match(from_wx(ctrl->GetValue()), timecode_regex)) {
				wxMessageBox(_("Time code offset in incorrect format. Ensure it is entered as four groups of two digits separated by colons."), _("EBU STL export"), wxICON_EXCLAMATION);
				return false;
			}
			return true;
		}

		wxObject *Clone() const override { return new TimecodeValidator(*this); }

	public:
		TimecodeValidator(EbuTimecode *target) : value(target) { assert(target); }
		TimecodeValidator(TimecodeValidator const& other) : wxValidator(other), value(other.value) { }
	};

} // namespace {

int ShowEbuExportConfigurationDialog(wxWindow *owner, EbuExportSettings &s) {
	wxDialog d(owner, -1, _("Export to EBU STL format"));

	wxStaticBoxSizer *text_formatting_sizer = new wxStaticBoxSizer(wxVERTICAL, &d, _("Text formatting"));
	wxStaticBoxSizer *timecode_control_sizer = new wxStaticBoxSizer(wxVERTICAL, &d, _("Time codes"));
	wxStaticBoxSizer *display_standard_sizer = new wxStaticBoxSizer(wxVERTICAL, &d, _("Display standard"));

	wxWindow *text_formatting_box = text_formatting_sizer->GetStaticBox();
	wxWindow *timecode_control_box = timecode_control_sizer->GetStaticBox();
	wxWindow *display_standard_box = display_standard_sizer->GetStaticBox();

	wxString tv_standards[] = {
		_("23.976 fps (non-standard, STL24.01)"),
		_("24 fps (non-standard, STL24.01)"),
		_("25 fps (STL25.01)"),
		_("29.97 fps (non-dropframe, STL30.01)"),
		_("29.97 fps (dropframe, STL30.01)"),
		_("30 fps (STL30.01)")
	};
	wxRadioBox *tv_standard_box = new wxRadioBox(&d, -1, _("TV standard"), wxDefaultPosition, wxDefaultSize, 6, tv_standards, 0, wxRA_SPECIFY_ROWS);

	wxTextCtrl *timecode_offset_entry = new wxTextCtrl(timecode_control_box, -1, "00:00:00:00");
	timecode_offset_entry->SetInitialSize(timecode_offset_entry->GetSizeFromText(timecode_offset_entry->GetValue()));
	wxCheckBox *inclusive_end_times_check = new wxCheckBox(timecode_control_box, -1, _("Out-times are inclusive"));

	wxString text_encodings[] = {
		_("ISO 6937-2 (Latin/Western Europe)"),
		_("ISO 8859-5 (Cyrillic)"),
		_("ISO 8859-6 (Arabic)"),
		_("ISO 8859-7 (Greek)"),
		_("ISO 8859-8 (Hebrew)"),
		_("UTF-8 Unicode (non-standard)")
	};
	wxRadioBox *text_encoding_box = new wxRadioBox(&d, -1, _("Text encoding"), wxDefaultPosition, wxDefaultSize, 6, text_encodings, 0, wxRA_SPECIFY_ROWS);

	wxString wrap_modes[] = {
		_("Automatically wrap long lines (ASS)"),
		_("Automatically wrap long lines (Balanced)"),
		_("Abort if any lines are too long"),
		_("Skip lines that are too long")
	};

	wxSpinCtrl *max_line_length_ctrl = new wxSpinCtrl(text_formatting_box, -1, wxString(), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 99);
	wxComboBox *wrap_mode_ctrl = new wxComboBox(text_formatting_box, -1, wrap_modes[0], wxDefaultPosition, wxDefaultSize, 4, wrap_modes, wxCB_DROPDOWN | wxCB_READONLY);
	wxCheckBox *translate_alignments_check = new wxCheckBox(text_formatting_box, -1, _("Translate alignments"));

	wxString display_standards[] = {
		_("Open subtitles"),
		_("Level-1 teletext"),
		_("Level-2 teletext")
	};

	wxComboBox *display_standard_ctrl = new wxComboBox(display_standard_box, -1, "", wxDefaultPosition, wxDefaultSize, 2, display_standards, wxCB_DROPDOWN | wxCB_READONLY);

	wxSizer *max_line_length_labelled = new wxBoxSizer(wxHORIZONTAL);
	max_line_length_labelled->Add(new wxStaticText(text_formatting_box, -1, _("Max. line length:")), wxSizerFlags(1).Center().Border(wxRIGHT, 12));
	max_line_length_labelled->Add(max_line_length_ctrl);

	wxSizer *timecode_offset_labelled = new wxBoxSizer(wxHORIZONTAL);
	timecode_offset_labelled->Add(new wxStaticText(timecode_control_box, -1, _("Time code offset:")), wxSizerFlags(1).Center().Border(wxRIGHT, 12));
	timecode_offset_labelled->Add(timecode_offset_entry);

	text_formatting_sizer->Add(max_line_length_labelled, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));
	text_formatting_sizer->Add(wrap_mode_ctrl, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));
	text_formatting_sizer->Add(translate_alignments_check, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));

	timecode_control_sizer->Add(timecode_offset_labelled, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));
	timecode_control_sizer->Add(inclusive_end_times_check, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));

	display_standard_sizer->Add(display_standard_ctrl, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 6));

	wxSizer *left_column = new wxBoxSizer(wxVERTICAL);
	left_column->Add(tv_standard_box, wxSizerFlags().Expand().Border(wxBOTTOM, 6));
	left_column->Add(timecode_control_sizer, wxSizerFlags().Expand().Border(wxBOTTOM, 6));
	left_column->Add(display_standard_sizer, wxSizerFlags().Expand());

	wxSizer *right_column = new wxBoxSizer(wxVERTICAL);
	right_column->Add(text_encoding_box, wxSizerFlags().Expand().Border(wxBOTTOM, 6));
	right_column->Add(text_formatting_sizer, wxSizerFlags().Expand());

	wxSizer *vertical_split_sizer = new wxBoxSizer(wxHORIZONTAL);
	vertical_split_sizer->Add(left_column, wxSizerFlags().Border(wxRIGHT, 6));
	vertical_split_sizer->Add(right_column);

	wxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	// Developers are requested to leave &d message in! Intentionally not translatable.
	wxStaticText *sponsor_label = new wxStaticText(&d, -1, "EBU STL format writing sponsored by Bandai");
	sponsor_label->Enable(false);
	buttons_sizer->Add(sponsor_label, wxSizerFlags(1).Bottom());
	buttons_sizer->Add(d.CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Border(wxLEFT, 6));

	wxSizer *main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(vertical_split_sizer, wxSizerFlags().Expand().Border(wxALL, 12));
	main_sizer->Add(buttons_sizer, wxSizerFlags().Expand().Border(wxALL & ~wxTOP, 12));

	d.SetSizerAndFit(main_sizer);
	d.CenterOnParent();

	// set up validators to move data in and out
	tv_standard_box->SetValidator(wxGenericValidator((int*)&s.tv_standard));
	text_encoding_box->SetValidator(wxGenericValidator((int*)&s.text_encoding));
	translate_alignments_check->SetValidator(wxGenericValidator(&s.translate_alignments));
	max_line_length_ctrl->SetValidator(wxGenericValidator(&s.max_line_length));
	wrap_mode_ctrl->SetValidator(wxGenericValidator((int*)&s.line_wrapping_mode));
	inclusive_end_times_check->SetValidator(wxGenericValidator(&s.inclusive_end_times));
	timecode_offset_entry->SetValidator(TimecodeValidator(&s.timecode_offset));
	display_standard_ctrl->SetValidator(wxGenericValidator((int*)&s.display_standard));

	return d.ShowModal();
}

agi::vfr::Framerate EbuExportSettings::GetFramerate() const {
	switch (tv_standard) {
		case STL24:     return agi::vfr::Framerate(24, 1);
		case STL25:     return agi::vfr::Framerate(25, 1);
		case STL30:     return agi::vfr::Framerate(30, 1);
		case STL23:     return agi::vfr::Framerate(24000, 1001, false);
		case STL29:     return agi::vfr::Framerate(30000, 1001, false);
		case STL29drop: return agi::vfr::Framerate(30000, 1001);
		default:        return agi::vfr::Framerate(25, 1);
	}
}

std::unique_ptr<agi::charset::IconvWrapper> EbuExportSettings::GetTextEncoder() const {
	const char *encoding = "ISO-8859-1";
	switch (text_encoding) {
		case iso6937_2: encoding = "ISO-6937-2"; break;
		case iso8859_5: encoding = "ISO-8859-5"; break;
		case iso8859_6: encoding = "ISO-8859-6"; break;
		case iso8859_7: encoding = "ISO-8859-7"; break;
		case iso8859_8: encoding = "ISO-8859-8"; break;
		case utf8:      encoding = "utf-8"; break;
	}
	return std::make_unique<agi::charset::IconvWrapper>("utf-8", encoding);
}

EbuExportSettings::EbuExportSettings(std::string const& prefix)
: prefix(prefix)
, tv_standard((TvStandard)OPT_GET(prefix + "/TV Standard")->GetInt())
, text_encoding((TextEncoding)OPT_GET(prefix + "/Text Encoding")->GetInt())
, max_line_length(OPT_GET(prefix + "/Max Line Length")->GetInt())
, line_wrapping_mode((LineWrappingMode)OPT_GET(prefix + "/Line Wrapping Mode")->GetInt())
, translate_alignments(OPT_GET(prefix + "/Translate Alignments")->GetBool())
, inclusive_end_times(OPT_GET(prefix + "/Inclusive End Times")->GetBool())
, display_standard((DisplayStandard)OPT_GET(prefix + "/Display Standard")->GetInt())
{
	timecode_offset.h = OPT_GET(prefix + "/Timecode Offset/H")->GetInt();
	timecode_offset.m = OPT_GET(prefix + "/Timecode Offset/M")->GetInt();
	timecode_offset.s = OPT_GET(prefix + "/Timecode Offset/S")->GetInt();
	timecode_offset.f = OPT_GET(prefix + "/Timecode Offset/F")->GetInt();
}

void EbuExportSettings::Save() const {
	OPT_SET(prefix + "/TV Standard")->SetInt(tv_standard);
	OPT_SET(prefix + "/Text Encoding")->SetInt(text_encoding);
	OPT_SET(prefix + "/Max Line Length")->SetInt(max_line_length);
	OPT_SET(prefix + "/Line Wrapping Mode")->SetInt(line_wrapping_mode);
	OPT_SET(prefix + "/Translate Alignments")->SetBool(translate_alignments);
	OPT_SET(prefix + "/Inclusive End Times")->SetBool(inclusive_end_times);
	OPT_SET(prefix + "/Display Standard")->SetInt(display_standard);
	OPT_SET(prefix + "/Timecode Offset/H")->SetInt(timecode_offset.h);
	OPT_SET(prefix + "/Timecode Offset/M")->SetInt(timecode_offset.m);
	OPT_SET(prefix + "/Timecode Offset/S")->SetInt(timecode_offset.s);
	OPT_SET(prefix + "/Timecode Offset/F")->SetInt(timecode_offset.f);
}
