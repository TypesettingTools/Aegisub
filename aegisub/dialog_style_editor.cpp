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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


////////////
// Includes
#include <wx/fontdlg.h>
#include <wx/colordlg.h>
#include "dialog_style_editor.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_file.h"
#include "ass_override.h"
#include "validators.h"
#include "subs_grid.h"
#include "utils.h"
#include "dialog_colorpicker.h"
#include "colour_button.h"


///////////////
// Constructor
DialogStyleEditor::DialogStyleEditor (wxWindow *parent, AssStyle *_style, SubtitlesGrid *_grid)
: wxDialog (parent,-1,_("Style Editor"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE,_T("DialogStyleEditor"))
{
	// Set styles
	grid = _grid;
	style = _style;
	work = new AssStyle;
	*work = *style;

	// Style name
	StyleName = new wxTextCtrl(this,-1,style->name);
	wxSizer *NameSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Style name"));
	NameSizer->Add(StyleName,1,wxALL,0);

	// Font
	FontName = new wxTextCtrl(this,-1,style->font,wxDefaultPosition,wxSize(150,20));
	FontSizeValue = FloatToString(style->fontsize);
	FontSize = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(30,20),0,wxTextValidator(wxFILTER_NUMERIC,&FontSizeValue));
	FontName->SetToolTip(_("Font face"));
	FontSize->SetToolTip(_("Font size"));
	wxButton *FontButton = new wxButton(this,BUTTON_STYLE_FONT,_("Choose"));
	wxSizer *FontSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Font"));
	wxSizer *FontSizerTop = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *FontSizerBottom = new wxBoxSizer(wxHORIZONTAL);
	BoxBold = new wxCheckBox(this,CHECKBOX_STYLE_BOLD,_("Bold"));
	BoxItalic = new wxCheckBox(this,CHECKBOX_STYLE_ITALIC,_("Italic"));
	BoxUnderline = new wxCheckBox(this,CHECKBOX_STYLE_UNDERLINE,_("Underline"));
	BoxStrikeout = new wxCheckBox(this,CHECKBOX_STYLE_STRIKEOUT,_("Strikeout"));
	BoxBold->SetValue(style->bold);
	BoxItalic->SetValue(style->italic);
	BoxUnderline->SetValue(style->underline);
	BoxStrikeout->SetValue(style->strikeout);
	FontSizerTop->Add(FontName,1,wxALL,0);
	FontSizerTop->Add(FontSize,0,wxLEFT,5);
	FontSizerTop->Add(FontButton,0,wxLEFT,5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizerBottom->Add(BoxBold,0,0,0);
	FontSizerBottom->Add(BoxItalic,0,wxLEFT,5);
	FontSizerBottom->Add(BoxUnderline,0,wxLEFT,5);
	FontSizerBottom->Add(BoxStrikeout,0,wxLEFT,5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizer->Add(FontSizerTop,1,wxALL | wxEXPAND,0);
	FontSizer->Add(FontSizerBottom,1,wxTOP | wxEXPAND,5);

	// Colors
	wxSizer *ColorsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Colors"));
	wxSizer *ColorSizer1 = new wxBoxSizer(wxVERTICAL);
	wxSizer *ColorSizer2 = new wxBoxSizer(wxVERTICAL);
	wxSizer *ColorSizer3 = new wxBoxSizer(wxVERTICAL);
	wxSizer *ColorSizer4 = new wxBoxSizer(wxVERTICAL);
	ColorsSizer->AddStretchSpacer(1);
	ColorsSizer->Add(ColorSizer1,0,0,0);
	ColorsSizer->Add(ColorSizer2,0,wxLEFT,5);
	ColorsSizer->Add(ColorSizer3,0,wxLEFT,5);
	ColorsSizer->Add(ColorSizer4,0,wxLEFT,5);
	ColorsSizer->AddStretchSpacer(1);
	colorButton[0] = new ColourButton(this,BUTTON_COLOR_1,wxSize(45,16),style->primary.GetWXColor());
	colorButton[1] = new ColourButton(this,BUTTON_COLOR_2,wxSize(45,16),style->secondary.GetWXColor());
	colorButton[2] = new ColourButton(this,BUTTON_COLOR_3,wxSize(45,16),style->outline.GetWXColor());
	colorButton[3] = new ColourButton(this,BUTTON_COLOR_4,wxSize(45,16),style->shadow.GetWXColor());
	colorButton[0]->SetToolTip(_("Click to choose primary color"));
	colorButton[1]->SetToolTip(_("Click to choose secondary color"));
	colorButton[2]->SetToolTip(_("Click to choose outline color"));
	colorButton[3]->SetToolTip(_("Click to choose shadow color"));
	ColorAlpha1Value = wxString::Format(_T("%i"),style->primary.a);
	ColorAlpha1 = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&ColorAlpha1Value));
	ColorAlpha2Value = wxString::Format(_T("%i"),style->secondary.a);
	ColorAlpha2 = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&ColorAlpha2Value));
	ColorAlpha3Value = wxString::Format(_T("%i"),style->outline.a);
	ColorAlpha3 = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&ColorAlpha3Value));
	ColorAlpha4Value = wxString::Format(_T("%i"),style->shadow.a);
	ColorAlpha4 = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&ColorAlpha4Value));
	ColorAlpha1->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)"));
	ColorAlpha2->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)"));
	ColorAlpha3->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)"));
	ColorAlpha4->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)"));
	ColorSizer1->Add(new wxStaticText(this,-1,_("Primary")),0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer2->Add(new wxStaticText(this,-1,_("Secondary")),0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer3->Add(new wxStaticText(this,-1,_("Outline")),0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer4->Add(new wxStaticText(this,-1,_("Shadow")),0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer1->Add(colorButton[0],0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer2->Add(colorButton[1],0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer3->Add(colorButton[2],0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer4->Add(colorButton[3],0,wxBOTTOM | wxALIGN_CENTER,5);
	ColorSizer1->Add(ColorAlpha1,0,wxALIGN_CENTER,0);
	ColorSizer2->Add(ColorAlpha2,0,wxALIGN_CENTER,0);
	ColorSizer3->Add(ColorAlpha3,0,wxALIGN_CENTER,0);
	ColorSizer4->Add(ColorAlpha4,0,wxALIGN_CENTER,0);

	// Margins
	wxSizer *MarginSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Margins"));
	wxSizer *MarginSizerL = new wxBoxSizer(wxVERTICAL);
	wxSizer *MarginSizerR = new wxBoxSizer(wxVERTICAL);
	wxSizer *MarginSizerV = new wxBoxSizer(wxVERTICAL);
	MarginLValue = style->GetMarginString(0);
	MarginL = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&MarginLValue));
	MarginRValue = style->GetMarginString(1);
	MarginR = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&MarginRValue));
	MarginVValue = style->GetMarginString(2);
	MarginV = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&MarginVValue));
	MarginL->SetToolTip(_("Distance from left edge, in pixels"));
	MarginR->SetToolTip(_("Distance from right edge, in pixels"));
	MarginV->SetToolTip(_("Distance from top/bottom edge, in pixels"));
	MarginSizerL->AddStretchSpacer(1);
	MarginSizerL->Add(new wxStaticText(this,-1,_("Left")),0,wxCENTER,0);
	MarginSizerL->Add(MarginL,0,wxTOP | wxCENTER,5);
	MarginSizerL->AddStretchSpacer(1);
	MarginSizerR->AddStretchSpacer(1);
	MarginSizerR->Add(new wxStaticText(this,-1,_("Right")),0,wxCENTER,0);
	MarginSizerR->Add(MarginR,0,wxTOP | wxCENTER,5);
	MarginSizerR->AddStretchSpacer(1);
	MarginSizerV->AddStretchSpacer(1);
	MarginSizerV->Add(new wxStaticText(this,-1,_("Vert")),0,wxCENTER,0);
	MarginSizerV->Add(MarginV,0,wxTOP | wxCENTER,5);
	MarginSizerV->AddStretchSpacer(1);
	MarginSizer->AddStretchSpacer(1);
	MarginSizer->Add(MarginSizerL,0,wxEXPAND,0);
	MarginSizer->Add(MarginSizerR,0,wxEXPAND | wxLEFT,5);
	MarginSizer->Add(MarginSizerV,0,wxEXPAND | wxLEFT,5);
	MarginSizer->AddStretchSpacer(1);

	// Alignment
	wxString blah[9] = { _T("7"),_T("8"),_T("9"),_T("4"),_T("5"),_T("6"),_T("1"),_T("2"),_T("3") };
	Alignment = new wxRadioBox(this, RADIO_ALIGNMENT, _("Alignment"), wxDefaultPosition, wxDefaultSize, 9, blah, 3, wxRA_SPECIFY_COLS);
	Alignment->SetToolTip(_("Alignment in screen, in numpad style"));
	Alignment->SetSelection(AlignToControl(style->alignment));

	// Margins+Alignment
	wxSizer *MarginAlign = new wxBoxSizer(wxHORIZONTAL);
	MarginAlign->Add(MarginSizer,1,wxLEFT | wxEXPAND,0);
	MarginAlign->Add(Alignment,0,wxLEFT | wxEXPAND,5);

	// Outline
	wxSizer *OutlineBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Outline"));
	OutlineType = new wxCheckBox(this,-1,_("Opaque box"));
	OutlineValue = FloatToString(style->outline_w);
	ShadowValue = FloatToString(style->shadow_w);
	Outline = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,wxTextValidator(wxFILTER_NUMERIC,&OutlineValue));
	Shadow = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,wxTextValidator(wxFILTER_NUMERIC,&ShadowValue));
	OutlineType->SetToolTip(_("Checking this will display an opaque box instead of outline"));
	Outline->SetToolTip(_("Outline width, in pixels"));
	Shadow->SetToolTip(_("Shadow distance, in pixels"));
	OutlineBox->AddStretchSpacer(1);
	OutlineBox->Add(new wxStaticText(this,-1,_("Outline:")),0,wxALIGN_CENTER,0);
	OutlineBox->Add(Outline,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(new wxStaticText(this,-1,_("Shadow:")),0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(Shadow,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(OutlineType,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineType->SetValue(style->borderstyle == 3);
	OutlineBox->AddStretchSpacer(1);

	// Encoding options
	wxArrayString encodingStrings;
	encodingStrings.Add(wxString(_T("0 - ")) + _("ANSI"));
	encodingStrings.Add(wxString(_T("1 - ")) + _("Default"));
	encodingStrings.Add(wxString(_T("2 - ")) + _("Symbol"));
	encodingStrings.Add(wxString(_T("77 - ")) + _("Mac"));
	encodingStrings.Add(wxString(_T("128 - ")) + _("Shift_JIS"));
	encodingStrings.Add(wxString(_T("129 - ")) + _("Hangeul"));
	encodingStrings.Add(wxString(_T("130 - ")) + _("Johab"));
	encodingStrings.Add(wxString(_T("134 - ")) + _("GB2312"));
	encodingStrings.Add(wxString(_T("136 - ")) + _("Chinese BIG5"));
	encodingStrings.Add(wxString(_T("161 - ")) + _("Greek"));
	encodingStrings.Add(wxString(_T("162 - ")) + _("Turkish"));
	encodingStrings.Add(wxString(_T("163 - ")) + _("Vietnamese"));
	encodingStrings.Add(wxString(_T("177 - ")) + _("Hebrew"));
	encodingStrings.Add(wxString(_T("178 - ")) + _("Arabic"));
	encodingStrings.Add(wxString(_T("186 - ")) + _("Baltic"));
	encodingStrings.Add(wxString(_T("204 - ")) + _("Russian"));
	encodingStrings.Add(wxString(_T("222 - ")) + _("Thai"));
	encodingStrings.Add(wxString(_T("238 - ")) + _("East European"));
	encodingStrings.Add(wxString(_T("255 - ")) + _("OEM"));

	// Misc
	wxSizer *MiscBox = new wxStaticBoxSizer(wxVERTICAL,this,_("Miscelaneous"));
	wxSizer *MiscBoxTop = new wxFlexGridSizer(2,4,5,5);
	wxSizer *MiscBoxBottom = new wxBoxSizer(wxHORIZONTAL);
	ScaleXValue = FloatToString(style->scalex);
	ScaleYValue = FloatToString(style->scaley);
	AngleValue = FloatToString(style->angle);
	EncodingValue = IntegerToString(style->encoding);
	SpacingValue = FloatToString(style->spacing);
	ScaleX = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition, wxSize(70,20),0,wxTextValidator(wxFILTER_NUMERIC,&ScaleXValue));
	ScaleY = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition, wxSize(70,20),0,wxTextValidator(wxFILTER_NUMERIC,&ScaleYValue));
	Angle = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition, wxSize(40,20),0,wxTextValidator(wxFILTER_NUMERIC,&AngleValue));
	Encoding = new wxComboBox(this,-1,_T(""),wxDefaultPosition, wxDefaultSize, encodingStrings,wxCB_READONLY);
	Spacing = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(40,20),0,wxTextValidator(wxFILTER_NUMERIC,&SpacingValue));
	ScaleX->SetToolTip(_("Scale X, in percentage"));
	ScaleY->SetToolTip(_("Scale Y, in percentage"));
	Angle->SetToolTip(_("Angle to rotate in Z axis, in degrees"));
	Encoding->SetToolTip(_("Encoding, only useful in unicode if the font doesn't have the proper unicode mapping."));
	Spacing->SetToolTip(_("Character spacing, in pixels"));
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Scale X%:")),1,wxALIGN_CENTER,0);
	MiscBoxTop->Add(ScaleX,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Scale Y%:")),1,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxTop->Add(ScaleY,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Angle:")),1,wxALIGN_CENTER,0);
	MiscBoxTop->Add(Angle,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Spacing:")),1,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxTop->Add(Spacing,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxBottom->Add(new wxStaticText(this,-1,_("Encoding:")),0,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxBottom->Add(Encoding,1,wxLEFT | wxALIGN_CENTER,5);
	MiscBox->Add(MiscBoxTop,1,wxEXPAND | wxALIGN_CENTER,0);
	MiscBox->Add(MiscBoxBottom,0,wxEXPAND | wxTOP | wxALIGN_CENTER,5);

	// Set encoding value
	int encLen = EncodingValue.Length();
	bool found = false;
	for (size_t i=0;i<encodingStrings.Count();i++) {
		if (encodingStrings[i].Left(encLen) == EncodingValue) {
			Encoding->Select(i);
			found = true;
			break;
		}
	}
	if (!found) Encoding->Select(0);

	// Buttons
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifndef __WXMAC__
	ButtonSizer->Add(new wxButton(this, wxID_OK),0,wxRIGHT,5);
	ButtonSizer->Add(new wxButton(this, wxID_CANCEL),0,wxRIGHT,5);
	ButtonSizer->Add(new wxButton(this, wxID_APPLY),0,wxRIGHT,5);
#else
	wxButton *okButton = new wxButton(this, wxID_OK);
	ButtonSizer->Add(new wxButton(this, wxID_APPLY),0,wxRIGHT,5);
	ButtonSizer->Add(new wxButton(this, wxID_CANCEL),0,wxRIGHT,5);
	ButtonSizer->Add(okButton,0,wxRIGHT,5);
	okButton->SetDefault();
#endif

	// General Layout
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(NameSizer,0,wxALL | wxEXPAND,5);
	MainSizer->Add(FontSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ColorsSizer,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(MarginAlign,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(OutlineBox,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(MiscBox,0,wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxBOTTOM | wxALIGN_CENTER | wxEXPAND,5);

	// Set sizer
	SetSizer(MainSizer);
	MainSizer->SetSizeHints(this);
	CenterOnParent();
}


//////////////
// Destructor
DialogStyleEditor::~DialogStyleEditor () {
	delete work;
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogStyleEditor, wxDialog)
	EVT_BUTTON(wxID_APPLY, DialogStyleEditor::OnApply)
	EVT_BUTTON(wxID_OK, DialogStyleEditor::OnOK)
	EVT_BUTTON(wxID_CANCEL, DialogStyleEditor::OnCancel)
	EVT_BUTTON(BUTTON_STYLE_FONT, DialogStyleEditor::OnChooseFont)
	EVT_BUTTON(BUTTON_COLOR_1, DialogStyleEditor::OnSetColor1)
	EVT_BUTTON(BUTTON_COLOR_2, DialogStyleEditor::OnSetColor2)
	EVT_BUTTON(BUTTON_COLOR_3, DialogStyleEditor::OnSetColor3)
	EVT_BUTTON(BUTTON_COLOR_4, DialogStyleEditor::OnSetColor4)
END_EVENT_TABLE()


/////////////////////
// Event redirectors
void DialogStyleEditor::OnApply (wxCommandEvent &event) { Apply(true,false); }
void DialogStyleEditor::OnOK (wxCommandEvent &event) { Apply(true,true); }
void DialogStyleEditor::OnCancel (wxCommandEvent &event) { Apply(false,true); }
void DialogStyleEditor::OnSetColor1 (wxCommandEvent &event) { OnSetColor(1); }
void DialogStyleEditor::OnSetColor2 (wxCommandEvent &event) { OnSetColor(2); }
void DialogStyleEditor::OnSetColor3 (wxCommandEvent &event) { OnSetColor(3); }
void DialogStyleEditor::OnSetColor4 (wxCommandEvent &event) { OnSetColor(4); }


/////////////////
// Replace Style
void ReplaceStyle(wxString tag,int n,AssOverrideParameter* param,void *userData) {
	wxArrayString strings = *((wxArrayString*)userData);
	if (tag == _T("\\r")) {
		if (param->GetType() == VARDATA_TEXT) {
			if (param->AsText() == strings[0]) {
				param->SetText(strings[1]);
			}
		}
	}
}


//////////
// Events
void DialogStyleEditor::Apply (bool apply,bool close) {
	// Apply
	if (apply) {
		// Style name
		wxString newStyleName = StyleName->GetValue();

		// Check if style name is unique
		wxArrayString styles = grid->ass->GetStyles();
		for (unsigned int i=0;i<styles.Count();i++) {
			if (styles[i] == newStyleName) {
				if (grid->ass->GetStyle(styles[i]) != style) {
					int answer = wxMessageBox(_T("There is already a style with this name. Proceed anyway?"),_T("Style name conflict."),wxYES_NO);
					if (answer == wxNO) return;
				}
			}
		}

		// Style name change
		if (work->name != newStyleName) {
			if (!work->name.StartsWith(_("Copy of "))) {
				// See if user wants to update style name through script
				int answer = wxNO;
				if (work->name != _T("Default")) answer = wxMessageBox(_T("Do you want to change all instances of this style in the script to this new name?"),_T("Update script?"),wxYES_NO | wxCANCEL);

				// Cancel
				if (answer == wxCANCEL) return;

				// Update
				if (answer == wxYES) {
					int n = grid->GetRows();
					wxArrayString strings;
					strings.Add(work->name);
					strings.Add(newStyleName);
					for (int i=0;i<n;i++) {
						AssDialogue *curDiag = grid->GetDialogue(i);
						if (curDiag->Style == work->name) curDiag->Style = newStyleName;
						curDiag->ParseASSTags();
						curDiag->ProcessParameters(ReplaceStyle,&strings);
						curDiag->UpdateText();
						curDiag->UpdateData();
						curDiag->ClearBlocks();
					}
				}
			}

			// Change name
			work->name = newStyleName;
		}

		// Update scale
		ScaleX->GetValue().ToDouble(&(work->scalex));
		ScaleY->GetValue().ToDouble(&(work->scaley));

		// Update encoding
		long templ = 0;
		wxString enc = Encoding->GetValue();
		enc.Left(enc.Find(_T("-"))-1).ToLong(&templ);
		work->encoding = templ;

		// Angle and spacing
		Angle->GetValue().ToDouble(&(work->angle));
		Spacing->GetValue().ToDouble(&(work->spacing));

		// Outline type
		if(OutlineType->IsChecked()) work->borderstyle = 3;
		else work->borderstyle = 1;

		// Shadow and outline
		Shadow->GetValue().ToDouble(&(work->shadow_w));
		Outline->GetValue().ToDouble(&(work->outline_w));

		// Alignment
		work->alignment = ControlToAlign(Alignment->GetSelection());

		// Margins
		work->SetMarginString(MarginL->GetValue(),0);
		work->SetMarginString(MarginR->GetValue(),1);
		work->SetMarginString(MarginV->GetValue(),2);

		// Color alphas
		ColorAlpha1->GetValue().ToLong(&templ);
		work->primary.a = templ;
		ColorAlpha2->GetValue().ToLong(&templ);
		work->secondary.a = templ;
		ColorAlpha3->GetValue().ToLong(&templ);
		work->outline.a = templ;
		ColorAlpha4->GetValue().ToLong(&templ);
		work->shadow.a = templ;

		// Bold/italic/underline/strikeout
		work->bold = BoxBold->IsChecked();
		work->italic = BoxItalic->IsChecked();
		work->underline = BoxUnderline->IsChecked();
		work->strikeout = BoxStrikeout->IsChecked();

		// Font and its size
		work->font = FontName->GetValue();
		FontSize->GetValue().ToDouble(&(work->fontsize));

		// Copy
		*style = *work;
		style->UpdateData();
		AssFile::top->FlagAsModified(_("style change"));
		grid->CommitChanges();

		// Exit
		if (close) EndModal(1);
	}

	// Close
	else {
		if (close) EndModal(0);
	}
}


///////////////////
// Choose font box
void DialogStyleEditor::OnChooseFont (wxCommandEvent &event) {
	wxFont oldfont (work->fontsize, wxFONTFAMILY_DEFAULT, (work->italic?wxFONTSTYLE_ITALIC:wxFONTSTYLE_NORMAL), (work->bold?wxFONTWEIGHT_BOLD:wxFONTWEIGHT_NORMAL), work->underline, work->font, wxFONTENCODING_DEFAULT);
	wxFont newfont = wxGetFontFromUser(this,oldfont);
	if (newfont.Ok()) {
		FontName->SetValue(newfont.GetFaceName());
		FontSize->SetValue(wxString::Format(_T("%i"),newfont.GetPointSize()));
		BoxBold->SetValue(newfont.GetWeight() == wxFONTWEIGHT_BOLD);
		BoxItalic->SetValue(newfont.GetStyle() == wxFONTSTYLE_ITALIC);
		BoxUnderline->SetValue(newfont.GetUnderlined());
		work->font = newfont.GetFaceName();
		work->fontsize = newfont.GetPointSize();
		work->bold = (newfont.GetWeight() == wxFONTWEIGHT_BOLD);
		work->italic = (newfont.GetStyle() == wxFONTSTYLE_ITALIC);
		work->underline = newfont.GetUnderlined();

		// Comic sans warning
		if (newfont.GetFaceName() == _T("Comic Sans MS")) {
			wxMessageBox(_("You have chosen to use the \"Comic Sans\" font. As the programmer and a typesetter,\nI must urge you to reconsider. Comic Sans is the most abused font in the history\nof computing, so please avoid using it unless it's REALLY suitable. Thanks."), _("Warning"), wxICON_EXCLAMATION | wxOK);
		}
	}
}


////////////////////////////////////////////////
// Sets color for one of the four color buttons
void DialogStyleEditor::OnSetColor (int n) {
	AssColor *modify;
	switch (n) {
		case 1: modify = &work->primary; break;
		case 2: modify = &work->secondary; break;
		case 3: modify = &work->outline; break;
		case 4: modify = &work->shadow; break;
		default: throw _T("Never gets here");
	}
	modify->SetWXColor(colorButton[n-1]->GetColour());
}


///////////////////////////////////////
// Converts control value to alignment
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


///////////////////////////////////////
// Converts alignment value to control
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
