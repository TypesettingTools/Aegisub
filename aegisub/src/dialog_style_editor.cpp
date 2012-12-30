// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file dialog_style_editor.cpp
/// @brief Style Editor dialogue box
/// @ingroup style_editor
///

#include "config.h"

#include <algorithm>

#include <wx/bmpbuttn.h>
#include <wx/fontenum.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "ass_style_storage.h"
#include "colour_button.h"
#include "compat.h"
#include "dialog_style_editor.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "include/aegisub/subtitles_provider.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "persist_location.h"
#include "selection_controller.h"
#include "subs_grid.h"
#include "subs_preview.h"
#include "utils.h"
#include "validators.h"

#include <libaegisub/of_type_adaptor.h>

/// Style rename helper that walks a file searching for a style and optionally
/// updating references to it
class StyleRenamer {
	agi::Context *c;
	bool found_any;
	bool do_replace;
	std::string source_name;
	std::string new_name;

	/// Process a single override parameter to check if it's \r with this style name
	static void ProcessTag(std::string const& tag, AssOverrideParameter* param, void *userData) {
		StyleRenamer *self = static_cast<StyleRenamer*>(userData);
		if (tag == "\\r" && param->GetType() == VARDATA_TEXT && param->Get<std::string>() == self->source_name) {
			if (self->do_replace)
				param->Set(self->new_name);
			else
				self->found_any = true;
		}
	}

	void Walk(bool replace) {
		found_any = false;
		do_replace = replace;

		wxString wx_old(to_wx(source_name));
		wxString wx_new(to_wx(new_name));

		for (auto diag : c->ass->Line | agi::of_type<AssDialogue>()) {
			if (diag->Style == wx_old) {
				if (replace)
					diag->Style = wx_new;
				else
					found_any = true;
			}

			boost::ptr_vector<AssDialogueBlock> blocks(diag->ParseTags());
			for (auto block : blocks | agi::of_type<AssDialogueBlockOverride>())
				block->ProcessParameters(&StyleRenamer::ProcessTag, this);
			if (replace)
				diag->UpdateText(blocks);

			if (found_any) return;
		}
	}

public:
	StyleRenamer(agi::Context *c, std::string const& source_name, std::string const& new_name)
	: c(c)
	, found_any(false)
	, do_replace(false)
	, source_name(source_name)
	, new_name(new_name)
	{
	}

	/// Check if there are any uses of the original style name in the file
	bool NeedsReplace() {
		Walk(false);
		return found_any;
	}

	/// Replace all uses of the original style name with the new one
	void Replace() {
		Walk(true);
	}
};

static void add_with_label(wxSizer *sizer, wxWindow *parent, wxString const& label, wxWindow *ctrl) {
	sizer->Add(new wxStaticText(parent, -1, label), wxSizerFlags().Center().Right().Border(wxLEFT | wxRIGHT));
	sizer->Add(ctrl, wxSizerFlags(1).Left().Expand());
}

static wxSpinCtrl *spin_ctrl(wxWindow *parent, float value, int max_value) {
	return new wxSpinCtrl(parent, -1, AegiFloatToString(value), wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 0, max_value, value);
}

static wxTextCtrl *num_text_ctrl(wxWindow *parent, double value, wxSize size = wxSize(70, 20)) {
	return new wxTextCtrl(parent, -1, "", wxDefaultPosition, size, 0, NumValidator(value));
}

DialogStyleEditor::DialogStyleEditor(wxWindow *parent, AssStyle *style, agi::Context *c, AssStyleStorage *store, std::string const& new_name)
: wxDialog (parent, -1, _("Style Editor"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
, c(c)
, is_new(false)
, style(style)
, store(store)
{
	if (new_name.size()) {
		is_new = true;
		style = this->style = new AssStyle(*style);
		style->name = new_name;
	}
	else if (!style) {
		is_new = true;
		style = this->style = new AssStyle;
	}

	work.reset(new AssStyle(*style));

	SetIcon(GETICON(style_toolbutton_16));

	// Prepare control values
	wxString EncodingValue = AegiIntegerToString(style->encoding);
	wxString alignValues[9] = { "7", "8", "9", "4", "5", "6", "1", "2", "3" };
	wxArrayString fontList = wxFontEnumerator::GetFacenames();
	fontList.Sort();

	// Encoding options
	wxArrayString encodingStrings;
	AssStyle::GetEncodings(encodingStrings);

	// Create sizers
	wxSizer *NameSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Style Name"));
	wxSizer *FontSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Font"));
	wxSizer *ColorsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Colors"));
	wxSizer *MarginSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Margins"));
	wxSizer *OutlineBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Outline"));
	wxSizer *MiscBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Miscellaneous"));
	wxSizer *PreviewBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Preview"));

	// Create controls
	StyleName = new wxTextCtrl(this, -1, to_wx(style->name));
	FontName = new wxComboBox(this, -1, to_wx(style->font), wxDefaultPosition, wxSize(150, -1), 0, 0, wxCB_DROPDOWN);
	FontSize =  num_text_ctrl(this, style->fontsize, wxSize(50, -1));
	BoxBold = new wxCheckBox(this, -1, _("&Bold"));
	BoxItalic = new wxCheckBox(this, -1, _("&Italic"));
	BoxUnderline = new wxCheckBox(this, -1, _("&Underline"));
	BoxStrikeout = new wxCheckBox(this, -1, _("&Strikeout"));
	colorButton[0] = new ColourButton(this, -1, wxSize(55, 16), style->primary);
	colorButton[1] = new ColourButton(this, -1, wxSize(55, 16), style->secondary);
	colorButton[2] = new ColourButton(this, -1, wxSize(55, 16), style->outline);
	colorButton[3] = new ColourButton(this, -1, wxSize(55, 16), style->shadow);
	colorAlpha[0] = spin_ctrl(this, style->primary.a, 255);
	colorAlpha[1] = spin_ctrl(this, style->secondary.a, 255);
	colorAlpha[2] = spin_ctrl(this, style->outline.a, 255);
	colorAlpha[3] = spin_ctrl(this, style->shadow.a, 255);
	for (int i = 0; i < 3; i++)
		margin[i] = spin_ctrl(this, style->Margin[i], 9999);
	Alignment = new wxRadioBox(this, -1, _("Alignment"), wxDefaultPosition, wxDefaultSize, 9, alignValues, 3, wxRA_SPECIFY_COLS);
	Outline = num_text_ctrl(this, style->outline_w, wxSize(50, -1));
	Shadow = num_text_ctrl(this, style->shadow_w, wxSize(50, -1));
	OutlineType = new wxCheckBox(this, -1, _("&Opaque box"));
	ScaleX = num_text_ctrl(this, style->scalex);
	ScaleY = num_text_ctrl(this, style->scaley);
	Angle = num_text_ctrl(this, style->angle);
	Spacing = num_text_ctrl(this, style->spacing);
	Encoding = new wxComboBox(this, -1, "", wxDefaultPosition, wxDefaultSize, encodingStrings, wxCB_READONLY);

	// Set control tooltips
	StyleName->SetToolTip(_("Style name"));
	FontName->SetToolTip(_("Font face"));
	FontSize->SetToolTip(_("Font size"));
	colorButton[0]->SetToolTip(_("Choose primary color"));
	colorButton[1]->SetToolTip(_("Choose secondary color"));
	colorButton[2]->SetToolTip(_("Choose outline color"));
	colorButton[3]->SetToolTip(_("Choose shadow color"));
	for (int i=0;i<4;i++) colorAlpha[i]->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)"));
	margin[0]->SetToolTip(_("Distance from left edge, in pixels"));
	margin[1]->SetToolTip(_("Distance from right edge, in pixels"));
	margin[2]->SetToolTip(_("Distance from top/bottom edge, in pixels"));
	OutlineType->SetToolTip(_("When selected, display an opaque box behind the subtitles instead of an outline around the text"));
	Outline->SetToolTip(_("Outline width, in pixels"));
	Shadow->SetToolTip(_("Shadow distance, in pixels"));
	ScaleX->SetToolTip(_("Scale X, in percentage"));
	ScaleY->SetToolTip(_("Scale Y, in percentage"));
	Angle->SetToolTip(_("Angle to rotate in Z axis, in degrees"));
	Encoding->SetToolTip(_("Encoding, only useful in unicode if the font doesn't have the proper unicode mapping"));
	Spacing->SetToolTip(_("Character spacing, in pixels"));
	Alignment->SetToolTip(_("Alignment in screen, in numpad style"));

	// Set up controls
	BoxBold->SetValue(style->bold);
	BoxItalic->SetValue(style->italic);
	BoxUnderline->SetValue(style->underline);
	BoxStrikeout->SetValue(style->strikeout);
	OutlineType->SetValue(style->borderstyle == 3);
	Alignment->SetSelection(AlignToControl(style->alignment));
	// Fill font face list box
	FontName->Freeze();
	FontName->Append(fontList);
	FontName->SetValue(to_wx(style->font));
	FontName->Thaw();

	// Set encoding value
	bool found = false;
	for (size_t i=0;i<encodingStrings.Count();i++) {
		if (encodingStrings[i].StartsWith(EncodingValue)) {
			Encoding->Select(i);
			found = true;
			break;
		}
	}
	if (!found) Encoding->Select(0);

	// Style name sizer
	NameSizer->Add(StyleName, 1, wxALL, 0);

	// Font sizer
	wxSizer *FontSizerTop = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *FontSizerBottom = new wxBoxSizer(wxHORIZONTAL);
	FontSizerTop->Add(FontName, 1, wxALL, 0);
	FontSizerTop->Add(FontSize, 0, wxLEFT, 5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizerBottom->Add(BoxBold, 0, 0, 0);
	FontSizerBottom->Add(BoxItalic, 0, wxLEFT, 5);
	FontSizerBottom->Add(BoxUnderline, 0, wxLEFT, 5);
	FontSizerBottom->Add(BoxStrikeout, 0, wxLEFT, 5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizer->Add(FontSizerTop, 1, wxALL | wxEXPAND, 0);
	FontSizer->Add(FontSizerBottom, 1, wxTOP | wxEXPAND, 5);

	// Colors sizer
	wxSizer *ColorSizer[4];
	wxString colorLabels[] = { _("Primary"), _("Secondary"), _("Outline"), _("Shadow") };
	ColorsSizer->AddStretchSpacer(1);
	for (int i=0;i<4;i++) {
		ColorSizer[i] = new wxBoxSizer(wxVERTICAL);
		ColorSizer[i]->Add(new wxStaticText(this, -1, colorLabels[i]), 0, wxBOTTOM | wxALIGN_CENTER, 5);
		ColorSizer[i]->Add(colorButton[i], 0, wxBOTTOM | wxALIGN_CENTER, 5);
		ColorSizer[i]->Add(colorAlpha[i], 0, wxALIGN_CENTER, 0);
		ColorsSizer->Add(ColorSizer[i], 0, wxLEFT, i?5:0);
	}
	ColorsSizer->AddStretchSpacer(1);

	// Margins
	wxString marginLabels[] = { _("Left"), _("Right"), _("Vert") };
	MarginSizer->AddStretchSpacer(1);
	wxSizer *marginSubSizer[3];
	for (int i=0;i<3;i++) {
		marginSubSizer[i] = new wxBoxSizer(wxVERTICAL);
		marginSubSizer[i]->AddStretchSpacer(1);
		marginSubSizer[i]->Add(new wxStaticText(this, -1, marginLabels[i]), 0, wxCENTER, 0);
		marginSubSizer[i]->Add(margin[i], 0, wxTOP | wxCENTER, 5);
		marginSubSizer[i]->AddStretchSpacer(1);
		MarginSizer->Add(marginSubSizer[i], 0, wxEXPAND | wxLEFT, i?5:0);
	}
	MarginSizer->AddStretchSpacer(1);

	// Margins+Alignment
	wxSizer *MarginAlign = new wxBoxSizer(wxHORIZONTAL);
	MarginAlign->Add(MarginSizer, 1, wxLEFT | wxEXPAND, 0);
	MarginAlign->Add(Alignment, 0, wxLEFT | wxEXPAND, 5);

	// Outline
	add_with_label(OutlineBox, this, _("Outline:"), Outline);
	add_with_label(OutlineBox, this, _("Shadow:"), Shadow);
	OutlineBox->Add(OutlineType, 0, wxLEFT | wxALIGN_CENTER, 5);

	// Misc
	wxFlexGridSizer *MiscBoxTop = new wxFlexGridSizer(2, 4, 5, 5);
	add_with_label(MiscBoxTop, this, _("Scale X%:"), ScaleX);
	add_with_label(MiscBoxTop, this, _("Scale Y%:"), ScaleY);
	add_with_label(MiscBoxTop, this, _("Rotation:"), Angle);
	add_with_label(MiscBoxTop, this, _("Spacing:"), Spacing);

	wxSizer *MiscBoxBottom = new wxBoxSizer(wxHORIZONTAL);
	add_with_label(MiscBoxBottom, this, _("Encoding:"), Encoding);

	MiscBox->Add(MiscBoxTop, wxSizerFlags().Expand().Center());
	MiscBox->Add(MiscBoxBottom, wxSizerFlags().Expand().Center().Border(wxTOP));

	// Preview
	SubsPreview = nullptr;
	PreviewText = nullptr;
	ColourButton *previewButton = 0;
	if (!SubtitlesProviderFactory::GetClasses().empty()) {
		PreviewText = new wxTextCtrl(this, -1, to_wx(OPT_GET("Tool/Style Editor/Preview Text")->GetString()));
		previewButton = new ColourButton(this, -1, wxSize(45, 16), OPT_GET("Colour/Style Editor/Background/Preview")->GetColor());
		SubsPreview = new SubtitlesPreview(this, wxSize(100, 60), wxSUNKEN_BORDER, OPT_GET("Colour/Style Editor/Background/Preview")->GetColor());

		SubsPreview->SetToolTip(_("Preview of current style"));
		SubsPreview->SetStyle(*style);
		SubsPreview->SetText(PreviewText->GetValue());
		PreviewText->SetToolTip(_("Text to be used for the preview"));
		previewButton->SetToolTip(_("Color of preview background"));

		wxSizer *PreviewBottomSizer = new wxBoxSizer(wxHORIZONTAL);
		PreviewBottomSizer->Add(PreviewText, 1, wxEXPAND | wxRIGHT, 5);
		PreviewBottomSizer->Add(previewButton, 0, wxEXPAND, 0);
		PreviewBox->Add(SubsPreview, 1, wxEXPAND | wxBOTTOM, 5);
		PreviewBox->Add(PreviewBottomSizer, 0, wxEXPAND | wxBOTTOM, 0);
	}
	else {
		wxStaticText *NoSP = new wxStaticText(this, -1, _("No subtitle providers available. Cannot preview subs."));
		PreviewBox->AddStretchSpacer();
		PreviewBox->Add(NoSP, 1, wxEXPAND|wxLEFT|wxRIGHT, 8);
		PreviewBox->AddStretchSpacer();
	}

	// Buttons
	wxStdDialogButtonSizer *ButtonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL | wxAPPLY | wxHELP);

	// Left side sizer
	wxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);
	LeftSizer->Add(NameSizer, 0, wxBOTTOM | wxEXPAND, 5);
	LeftSizer->Add(FontSizer, 0, wxBOTTOM | wxEXPAND, 5);
	LeftSizer->Add(ColorsSizer, 0, wxBOTTOM | wxEXPAND, 5);
	LeftSizer->Add(MarginAlign, 0, wxBOTTOM | wxEXPAND, 0);

	// Right side sizer
	wxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
	RightSizer->Add(OutlineBox, wxSizerFlags().Expand().Border(wxBOTTOM));
	RightSizer->Add(MiscBox, wxSizerFlags().Expand().Border(wxBOTTOM));
	RightSizer->Add(PreviewBox, wxSizerFlags(1).Expand());

	// Controls Sizer
	wxSizer *ControlSizer = new wxBoxSizer(wxHORIZONTAL);
	ControlSizer->Add(LeftSizer, 0, wxEXPAND, 0);
	ControlSizer->Add(RightSizer, 1, wxLEFT | wxEXPAND, 5);

	// General Layout
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(ControlSizer, 1, wxALL | wxALIGN_CENTER | wxEXPAND, 5);
	MainSizer->Add(ButtonSizer, 0, wxBOTTOM | wxEXPAND, 5);

	SetSizerAndFit(MainSizer);

	// Force the style name text field to scroll based on its final size, rather
	// than its initial size
	StyleName->SetInsertionPoint(0);
	StyleName->SetInsertionPoint(-1);

	persist.reset(new PersistLocation(this, "Tool/Style Editor", true));

	Bind(wxEVT_CHILD_FOCUS, &DialogStyleEditor::OnChildFocus, this);

	if (PreviewText) {
		Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &DialogStyleEditor::OnCommandPreviewUpdate, this);
		Bind(wxEVT_COMMAND_COMBOBOX_SELECTED, &DialogStyleEditor::OnCommandPreviewUpdate, this);
		Bind(wxEVT_COMMAND_SPINCTRL_UPDATED, &DialogStyleEditor::OnCommandPreviewUpdate, this);

		previewButton->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &DialogStyleEditor::OnPreviewColourChange, this);
		FontName->Bind(wxEVT_COMMAND_TEXT_ENTER, &DialogStyleEditor::OnCommandPreviewUpdate, this);
		PreviewText->Bind(wxEVT_COMMAND_TEXT_UPDATED, &DialogStyleEditor::OnPreviewTextChange, this);
	}

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogStyleEditor::Apply, this, true, true), wxID_OK);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogStyleEditor::Apply, this, true, false), wxID_APPLY);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogStyleEditor::Apply, this, false, true), wxID_CANCEL);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&HelpButton::OpenPage, "Style Editor"), wxID_HELP);

	for (int i = 0; i < 4; ++i)
		colorButton[i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, std::bind(&DialogStyleEditor::OnSetColor, this, i + 1, std::placeholders::_1));
}

DialogStyleEditor::~DialogStyleEditor() {
	if (is_new)
		delete style;
}

std::string DialogStyleEditor::GetStyleName() const {
	return style->name;
}

void DialogStyleEditor::Apply(bool apply, bool close) {
	if (apply) {
		std::string new_name = from_wx(StyleName->GetValue());

		// Get list of existing styles
		std::vector<std::string> styles = store ? store->GetNames() : c->ass->GetStyles();

		// Check if style name is unique
		AssStyle *existing = store ? store->GetStyle(new_name) : c->ass->GetStyle(new_name);
		if (existing && existing != style) {
			wxMessageBox(_("There is already a style with this name. Please choose another name."), _("Style name conflict"), wxOK | wxICON_ERROR | wxCENTER);
			return;
		}

		// Style name change
		bool did_rename = false;
		if (work->name != new_name) {
			if (!store && !is_new) {
				StyleRenamer renamer(c, work->name, new_name);
				if (renamer.NeedsReplace()) {
					// See if user wants to update style name through script
					int answer = wxMessageBox(
						_("Do you want to change all instances of this style in the script to this new name?"),
						_("Update script?"),
						wxYES_NO | wxCANCEL);

					if (answer == wxCANCEL) return;

					if (answer == wxYES) {
						did_rename = true;
						renamer.Replace();
					}
				}
			}

			work->name = new_name;
		}

		UpdateWorkStyle();

		*style = *work;
		style->UpdateData();
		if (is_new) {
			if (store)
				store->push_back(style);
			else
				c->ass->InsertLine(style);
			is_new = false;
		}
		if (!store)
			c->ass->Commit(_("style change"), AssFile::COMMIT_STYLES | (did_rename ? AssFile::COMMIT_DIAG_FULL : 0));

		// Update preview
		if (!close && SubsPreview) SubsPreview->SetStyle(*style);
	}

	if (close) {
		EndModal(apply);
		if (PreviewText)
			OPT_SET("Tool/Style Editor/Preview Text")->SetString(from_wx(PreviewText->GetValue()));
	}
}

/// @brief Update work style
void DialogStyleEditor::UpdateWorkStyle() {
	work->font = from_wx(FontName->GetValue());
	FontSize->GetValue().ToDouble(&(work->fontsize));

	ScaleX->GetValue().ToDouble(&(work->scalex));
	ScaleY->GetValue().ToDouble(&(work->scaley));

	long templ = 0;
	Encoding->GetValue().BeforeFirst('-').ToLong(&templ);
	work->encoding = templ;

	Angle->GetValue().ToDouble(&(work->angle));
	Spacing->GetValue().ToDouble(&(work->spacing));

	work->borderstyle = OutlineType->IsChecked() ? 3 : 1;

	Shadow->GetValue().ToDouble(&(work->shadow_w));
	Outline->GetValue().ToDouble(&(work->outline_w));

	work->alignment = ControlToAlign(Alignment->GetSelection());

	for (size_t i = 0; i < 3; ++i)
		work->Margin[i] = margin[i]->GetValue();

	work->primary.a = colorAlpha[0]->GetValue();
	work->secondary.a = colorAlpha[1]->GetValue();
	work->outline.a = colorAlpha[2]->GetValue();
	work->shadow.a = colorAlpha[3]->GetValue();

	work->bold = BoxBold->IsChecked();
	work->italic = BoxItalic->IsChecked();
	work->underline = BoxUnderline->IsChecked();
	work->strikeout = BoxStrikeout->IsChecked();
}

/// @brief Sets color for one of the four color buttons
/// @param n Colour to set
void DialogStyleEditor::OnSetColor(int n, wxCommandEvent& evt) {
	ColourButton *btn = static_cast<ColourButton*>(evt.GetClientData());
	if (!btn) {
		evt.Skip();
		return;
	}

	switch (n) {
		case 1: work->primary = btn->GetColor(); break;
		case 2: work->secondary = btn->GetColor(); break;
		case 3: work->outline = btn->GetColor(); break;
		case 4: work->shadow = btn->GetColor(); break;
		default: throw agi::InternalError("attempted setting colour id outside range", 0);
	}
	if (SubsPreview)
		SubsPreview->SetStyle(*work);
}

void DialogStyleEditor::OnChildFocus(wxChildFocusEvent &event) {
	UpdateWorkStyle();
	if (SubsPreview)
		SubsPreview->SetStyle(*work);
	event.Skip();
}

void DialogStyleEditor::OnPreviewTextChange (wxCommandEvent &event) {
	SubsPreview->SetText(PreviewText->GetValue());
	event.Skip();
}

/// @brief Change colour of preview's background
void DialogStyleEditor::OnPreviewColourChange (wxCommandEvent &evt) {
	ColourButton *btn = static_cast<ColourButton*>(evt.GetClientData());
	if (!btn)
		evt.Skip();
	else {
		SubsPreview->SetColour(btn->GetColor());
		OPT_SET("Colour/Style Editor/Background/Preview")->SetColor(btn->GetColor());
	}
}

/// @brief Command event to update preview
void DialogStyleEditor::OnCommandPreviewUpdate (wxCommandEvent &event) {
	if (!IsShownOnScreen()) return;
	UpdateWorkStyle();
	SubsPreview->SetStyle(*work);
	event.Skip();
}

/// @brief Converts control value to alignment
int DialogStyleEditor::ControlToAlign (int n) {
	switch (n) {
		case 0: return 7;
		case 1: return 8;
		case 2: return 9;
		case 3: return 4;
		case 4: return 5;
		case 5: return 6;
		case 6: return 1;
		case 7: return 2;
		case 8: return 3;
		default: return 2;
	}
}

/// @brief Converts alignment value to control
int DialogStyleEditor::AlignToControl (int n) {
	switch (n) {
		case 7: return 0;
		case 8: return 1;
		case 9: return 2;
		case 4: return 3;
		case 5: return 4;
		case 6: return 5;
		case 1: return 6;
		case 2: return 7;
		case 3: return 8;
		default: return 7;
	}
}
