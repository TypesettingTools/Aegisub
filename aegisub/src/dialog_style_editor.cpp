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
//
// $Id$

/// @file dialog_style_editor.cpp
/// @brief Style Editor dialogue box
/// @ingroup style_editor
///


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/fontenum.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "ass_style_storage.h"
#include "compat.h"
#include "dialog_style_editor.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "options.h"
#include "subs_grid.h"
#include "subs_preview.h"
#include "subtitles_provider_manager.h"
#include "utils.h"
#include "validators.h"


///////
// IDs
enum {

	/// DOCME
	BUTTON_STYLE_FONT = 1050,

	/// DOCME
	CHECKBOX_STYLE_BOLD,

	/// DOCME
	CHECKBOX_STYLE_ITALIC,

	/// DOCME
	CHECKBOX_STYLE_UNDERLINE,

	/// DOCME
	CHECKBOX_STYLE_STRIKEOUT,

	/// DOCME
	CHECKBOX_OUTLINE,

	/// DOCME
	BUTTON_COLOR_1,

	/// DOCME
	BUTTON_COLOR_2,

	/// DOCME
	BUTTON_COLOR_3,

	/// DOCME
	BUTTON_COLOR_4,

	/// DOCME
	BUTTON_PREVIEW_COLOR,

	/// DOCME
	RADIO_ALIGNMENT,

	/// DOCME
	TEXT_FONT_NAME,

	/// DOCME
	TEXT_FONT_SIZE,

	/// DOCME
	TEXT_ALPHA_1,

	/// DOCME
	TEXT_ALPHA_2,

	/// DOCME
	TEXT_ALPHA_3,

	/// DOCME
	TEXT_ALPHA_4,

	/// DOCME
	TEXT_MARGIN_L,

	/// DOCME
	TEXT_MARGIN_R,

	/// DOCME
	TEXT_MARGIN_V,

	/// DOCME
	TEXT_OUTLINE,

	/// DOCME
	TEXT_SHADOW,

	/// DOCME
	TEXT_SCALE_X,

	/// DOCME
	TEXT_SCALE_Y,

	/// DOCME
	TEXT_ANGLE,

	/// DOCME
	TEXT_SPACING,

	/// DOCME
	TEXT_PREVIEW,

	/// DOCME
	COMBO_ENCODING
};



/// @brief Constructor 
/// @param parent 
/// @param _style 
/// @param _grid  
/// @param local  
/// @param _store 
///
DialogStyleEditor::DialogStyleEditor (wxWindow *parent, AssStyle *_style, SubtitlesGrid *_grid,bool local,AssStyleStorage *_store,bool newStyle)
: wxDialog (parent,-1,_("Style Editor"),wxDefaultPosition,wxDefaultSize,wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,_T("DialogStyleEditor"))
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(style_toolbutton_24)));

	// Set variables
	isLocal = local;
	isNew = newStyle;
	store = _store;

	// Set styles
	grid = _grid;
	style = _style;
	work = new AssStyle;
	*work = *style;

	// Prepare control values
	FontSizeValue = AegiFloatToString(style->fontsize);
	OutlineValue = AegiFloatToString(style->outline_w);
	ShadowValue = AegiFloatToString(style->shadow_w);
	ScaleXValue = AegiFloatToString(style->scalex);
	ScaleYValue = AegiFloatToString(style->scaley);
	AngleValue = AegiFloatToString(style->angle);
	EncodingValue = AegiIntegerToString(style->encoding);
	SpacingValue = AegiFloatToString(style->spacing);
	wxString alignValues[9] = { _T("7"),_T("8"),_T("9"),_T("4"),_T("5"),_T("6"),_T("1"),_T("2"),_T("3") };
	wxArrayString fontList = wxFontEnumerator::GetFacenames();
	fontList.Sort();

	// Encoding options
	wxArrayString encodingStrings;
	AssStyle::GetEncodings(encodingStrings);
	
	// Create sizers
	wxSizer *NameSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Style name"));
	wxSizer *FontSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Font"));
	wxSizer *ColorsSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Colors"));
	wxSizer *MarginSizer = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Margins"));
	wxSizer *OutlineBox = new wxStaticBoxSizer(wxHORIZONTAL,this,_("Outline"));
	wxSizer *MiscBox = new wxStaticBoxSizer(wxVERTICAL,this,_("Miscellaneous"));
	wxSizer *PreviewBox = new wxStaticBoxSizer(wxVERTICAL,this,_("Preview"));

	// Create controls
	StyleName = new wxTextCtrl(this,-1,style->name);
	FontName = new wxComboBox(this,TEXT_FONT_NAME,style->font,wxDefaultPosition,wxSize(150,-1),0,0,wxCB_DROPDOWN);
	FontSize = new wxTextCtrl(this,TEXT_FONT_SIZE,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(&FontSizeValue,true,false));
	BoxBold = new wxCheckBox(this,CHECKBOX_STYLE_BOLD,_("Bold"));
	BoxItalic = new wxCheckBox(this,CHECKBOX_STYLE_ITALIC,_("Italic"));
	BoxUnderline = new wxCheckBox(this,CHECKBOX_STYLE_UNDERLINE,_("Underline"));
	BoxStrikeout = new wxCheckBox(this,CHECKBOX_STYLE_STRIKEOUT,_("Strikeout"));
	colorButton[0] = new ColourButton(this,BUTTON_COLOR_1,wxSize(55,16),style->primary.GetWXColor());
	colorButton[1] = new ColourButton(this,BUTTON_COLOR_2,wxSize(55,16),style->secondary.GetWXColor());
	colorButton[2] = new ColourButton(this,BUTTON_COLOR_3,wxSize(55,16),style->outline.GetWXColor());
	colorButton[3] = new ColourButton(this,BUTTON_COLOR_4,wxSize(55,16),style->shadow.GetWXColor());
	colorAlpha[0] = new wxSpinCtrl(this,TEXT_ALPHA_1,AegiFloatToString(style->primary.a),wxDefaultPosition,wxSize(60,-1),wxSP_ARROW_KEYS,0,255,style->primary.a);
	colorAlpha[1] = new wxSpinCtrl(this,TEXT_ALPHA_2,AegiFloatToString(style->secondary.a),wxDefaultPosition,wxSize(60,-1),wxSP_ARROW_KEYS,0,255,style->secondary.a);
	colorAlpha[2] = new wxSpinCtrl(this,TEXT_ALPHA_3,AegiFloatToString(style->outline.a),wxDefaultPosition,wxSize(60,-1),wxSP_ARROW_KEYS,0,255,style->outline.a);
	colorAlpha[3] = new wxSpinCtrl(this,TEXT_ALPHA_4,AegiFloatToString(style->shadow.a),wxDefaultPosition,wxSize(60,-1),wxSP_ARROW_KEYS,0,255,style->shadow.a);
	for (int i=0;i<3;i++) margin[i] = new wxSpinCtrl(this,TEXT_MARGIN_L+i,AegiFloatToString(style->Margin[i]),wxDefaultPosition,wxSize(60,-1),wxSP_ARROW_KEYS,0,9999,style->Margin[i]);
	margin[3] = 0;
	Alignment = new wxRadioBox(this, RADIO_ALIGNMENT, _("Alignment"), wxDefaultPosition, wxDefaultSize, 9, alignValues, 3, wxRA_SPECIFY_COLS);
	Outline = new wxTextCtrl(this,TEXT_OUTLINE,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&OutlineValue,true,false));
	Shadow = new wxTextCtrl(this,TEXT_SHADOW,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&ShadowValue,true,false));
	OutlineType = new wxCheckBox(this,CHECKBOX_OUTLINE,_("Opaque box"));
	ScaleX = new wxTextCtrl(this,TEXT_SCALE_X,_T(""),wxDefaultPosition, wxSize(70,20),0,NumValidator(&ScaleXValue,true,false));
	ScaleY = new wxTextCtrl(this,TEXT_SCALE_Y,_T(""),wxDefaultPosition, wxSize(70,20),0,NumValidator(&ScaleYValue,true,false));
	Angle = new wxTextCtrl(this,TEXT_ANGLE,_T(""),wxDefaultPosition, wxSize(40,20),0,NumValidator(&AngleValue,true,true));
	Spacing = new wxTextCtrl(this,TEXT_SPACING,_T(""),wxDefaultPosition,wxSize(40,20),0,NumValidator(&SpacingValue,true,true));
	Encoding = new wxComboBox(this,COMBO_ENCODING,_T(""),wxDefaultPosition, wxDefaultSize, encodingStrings,wxCB_READONLY);

	// Set control tooltips
	StyleName->SetToolTip(_("Style name."));
	FontName->SetToolTip(_("Font face."));
	FontSize->SetToolTip(_("Font size."));
	colorButton[0]->SetToolTip(_("Choose primary color."));
	colorButton[1]->SetToolTip(_("Choose secondary color."));
	colorButton[2]->SetToolTip(_("Choose outline color."));
	colorButton[3]->SetToolTip(_("Choose shadow color."));
	for (int i=0;i<4;i++) colorAlpha[i]->SetToolTip(_("Set opacity, from 0 (opaque) to 255 (transparent)."));
	margin[0]->SetToolTip(_("Distance from left edge, in pixels."));
	margin[1]->SetToolTip(_("Distance from right edge, in pixels."));
	margin[2]->SetToolTip(_("Distance from top/bottom edge, in pixels."));
	OutlineType->SetToolTip(_("When selected, display an opaque box behind the subtitles instead of an outline around the text."));
	Outline->SetToolTip(_("Outline width, in pixels."));
	Shadow->SetToolTip(_("Shadow distance, in pixels."));
	ScaleX->SetToolTip(_("Scale X, in percentage."));
	ScaleY->SetToolTip(_("Scale Y, in percentage."));
	Angle->SetToolTip(_("Angle to rotate in Z axis, in degrees."));
	Encoding->SetToolTip(_("Encoding, only useful in unicode if the font doesn't have the proper unicode mapping."));
	Spacing->SetToolTip(_("Character spacing, in pixels."));
	Alignment->SetToolTip(_("Alignment in screen, in numpad style."));

	// Set up controls
	BoxBold->SetValue(style->bold);
	BoxItalic->SetValue(style->italic);
	BoxUnderline->SetValue(style->underline);
	BoxStrikeout->SetValue(style->strikeout);
	OutlineType->SetValue(style->borderstyle == 3);
	Alignment->SetSelection(AlignToControl(style->alignment));
	// Fill font face list box
	FontName->Freeze();
	for (size_t i = 0; i < fontList.size(); i++) {
		FontName->Append(fontList[i]);
	}
	FontName->SetValue(style->font);
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
	NameSizer->Add(StyleName,1,wxALL,0);

	// Font sizer
	wxSizer *FontSizerTop = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *FontSizerBottom = new wxBoxSizer(wxHORIZONTAL);
	FontSizerTop->Add(FontName,1,wxALL,0);
	FontSizerTop->Add(FontSize,0,wxLEFT,5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizerBottom->Add(BoxBold,0,0,0);
	FontSizerBottom->Add(BoxItalic,0,wxLEFT,5);
	FontSizerBottom->Add(BoxUnderline,0,wxLEFT,5);
	FontSizerBottom->Add(BoxStrikeout,0,wxLEFT,5);
	FontSizerBottom->AddStretchSpacer(1);
	FontSizer->Add(FontSizerTop,1,wxALL | wxEXPAND,0);
	FontSizer->Add(FontSizerBottom,1,wxTOP | wxEXPAND,5);

	// Colors sizer
	wxSizer *ColorSizer[4];
	wxString colorLabels[] = { _("Primary"), _("Secondary"), _("Outline"), _("Shadow") };
	ColorsSizer->AddStretchSpacer(1);
	for (int i=0;i<4;i++) {
		ColorSizer[i] = new wxBoxSizer(wxVERTICAL);
		ColorSizer[i]->Add(new wxStaticText(this,-1,colorLabels[i]),0,wxBOTTOM | wxALIGN_CENTER,5);
		ColorSizer[i]->Add(colorButton[i],0,wxBOTTOM | wxALIGN_CENTER,5);
		ColorSizer[i]->Add(colorAlpha[i],0,wxALIGN_CENTER,0);
		ColorsSizer->Add(ColorSizer[i],0,wxLEFT,i?5:0);
	}
	ColorsSizer->AddStretchSpacer(1);

	// Margins
	wxString marginLabels[] = { _("Left"), _("Right"), _("Vert") };
	MarginSizer->AddStretchSpacer(1);
	wxSizer *marginSubSizer[3];
	for (int i=0;i<3;i++) {
		marginSubSizer[i] = new wxBoxSizer(wxVERTICAL);
		marginSubSizer[i]->AddStretchSpacer(1);
		marginSubSizer[i]->Add(new wxStaticText(this,-1,marginLabels[i]),0,wxCENTER,0);
		marginSubSizer[i]->Add(margin[i],0,wxTOP | wxCENTER,5);
		marginSubSizer[i]->AddStretchSpacer(1);
		MarginSizer->Add(marginSubSizer[i],0,wxEXPAND | wxLEFT,i?5:0);
	}
	MarginSizer->AddStretchSpacer(1);

	// Margins+Alignment
	wxSizer *MarginAlign = new wxBoxSizer(wxHORIZONTAL);
	MarginAlign->Add(MarginSizer,1,wxLEFT | wxEXPAND,0);
	MarginAlign->Add(Alignment,0,wxLEFT | wxEXPAND,5);

	// Outline
	OutlineBox->AddStretchSpacer(1);
	OutlineBox->Add(new wxStaticText(this,-1,_("Outline:")),0,wxALIGN_CENTER,0);
	OutlineBox->Add(Outline,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(new wxStaticText(this,-1,_("Shadow:")),0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(Shadow,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->Add(OutlineType,0,wxLEFT | wxALIGN_CENTER,5);
	OutlineBox->AddStretchSpacer(1);

	// Misc
	wxFlexGridSizer *MiscBoxTop = new wxFlexGridSizer(2,4,5,5);
	wxSizer *MiscBoxBottom = new wxBoxSizer(wxHORIZONTAL);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Scale X%:")),1,wxALIGN_CENTER,0);
	MiscBoxTop->Add(ScaleX,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Scale Y%:")),1,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxTop->Add(ScaleY,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Rotation:")),1,wxALIGN_CENTER,0);
	MiscBoxTop->Add(Angle,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->Add(new wxStaticText(this,-1,_("Spacing:")),1,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxTop->Add(Spacing,0,wxLEFT | wxALIGN_CENTER | wxEXPAND,5);
	MiscBoxTop->AddGrowableCol(1,1);
	MiscBoxTop->AddGrowableCol(3,1);
	MiscBoxBottom->Add(new wxStaticText(this,-1,_("Encoding:")),0,wxLEFT | wxALIGN_CENTER,5);
	MiscBoxBottom->Add(Encoding,1,wxLEFT | wxALIGN_CENTER,5);
	MiscBox->Add(MiscBoxTop,0,wxEXPAND | wxALIGN_CENTER,0);
	MiscBox->Add(MiscBoxBottom,1,wxEXPAND | wxTOP | wxALIGN_CENTER,5);

	// Preview
	SubsPreview = NULL;
	PreviewText = NULL;
	if (SubtitlesProviderFactoryManager::ProviderAvailable()) {
		PreviewText = new wxTextCtrl(this,TEXT_PREVIEW,lagi_wxString(OPT_GET("Tool/Style Editor/Preview Text")->GetString()));
		previewButton = new ColourButton(this,BUTTON_PREVIEW_COLOR,wxSize(45,16),lagi_wxColour(OPT_GET("Colour/Style Editor/Background/Preview")->GetColour()));
		SubsPreview = new SubtitlesPreview(this,-1,wxDefaultPosition,wxSize(100,60),wxSUNKEN_BORDER,lagi_wxColour(OPT_GET("Colour/Style Editor/Background/Preview")->GetColour()));
	
		SubsPreview->SetToolTip(_("Preview of current style."));
		SubsPreview->SetStyle(*style);
		SubsPreview->SetText(PreviewText->GetValue());
		PreviewText->SetToolTip(_("Text to be used for the preview."));
		previewButton->SetToolTip(_("Colour of preview background."));

		wxSizer *PreviewBottomSizer = new wxBoxSizer(wxHORIZONTAL);
		PreviewBottomSizer->Add(PreviewText,1,wxEXPAND | wxRIGHT,5);
		PreviewBottomSizer->Add(previewButton,0,wxEXPAND,0);
		PreviewBox->Add(SubsPreview,1,wxEXPAND | wxBOTTOM,5);
		PreviewBox->Add(PreviewBottomSizer,0,wxEXPAND | wxBOTTOM,0);
	}
	else {
		wxStaticText *NoSP = new wxStaticText(this, -1, _("No subtitle providers available. Cannot preview subs."));
		PreviewBox->AddStretchSpacer();
		PreviewBox->Add(NoSP,1,wxEXPAND|wxLEFT|wxRIGHT,8);
		PreviewBox->AddStretchSpacer();
		SubsPreview = NULL;
		PreviewText = NULL;
	}



	// Buttons
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	wxButton *okButton = new wxButton(this, wxID_OK);
	okButton->SetDefault();
	ButtonSizer->AddButton(new wxButton(this, wxID_CANCEL));
	ButtonSizer->AddButton(new wxButton(this, wxID_APPLY));
	ButtonSizer->AddButton(new HelpButton(this, _T("Style Editor")));
	ButtonSizer->AddButton(okButton);
	ButtonSizer->Realize();

	// Left side sizer
	wxSizer *LeftSizer = new wxBoxSizer(wxVERTICAL);
	LeftSizer->Add(NameSizer,0,wxBOTTOM | wxEXPAND,5);
	LeftSizer->Add(FontSizer,0,wxBOTTOM | wxEXPAND,5);
	LeftSizer->Add(ColorsSizer,0,wxBOTTOM | wxEXPAND,5);
	LeftSizer->Add(MarginAlign,0,wxBOTTOM | wxEXPAND,0);

	// Right side sizer
	wxSizer *RightSizer = new wxBoxSizer(wxVERTICAL);
	RightSizer->Add(OutlineBox,0,wxEXPAND | wxBOTTOM,5);
	RightSizer->Add(MiscBox,0,wxEXPAND | wxBOTTOM,5);
	RightSizer->Add(PreviewBox,1,wxEXPAND,0);

	// Controls Sizer
	wxSizer *ControlSizer = new wxBoxSizer(wxHORIZONTAL);
	ControlSizer->Add(LeftSizer,0,wxEXPAND,0);
	ControlSizer->Add(RightSizer,1,wxLEFT | wxEXPAND,5);

	// General Layout
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(ControlSizer,1,wxALL | wxALIGN_CENTER | wxEXPAND,5);
	MainSizer->Add(ButtonSizer,0,wxBOTTOM | wxEXPAND,5);

	// Set sizer
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	LoadPosition();
}

/// @brief Destructor 
///
DialogStyleEditor::~DialogStyleEditor () {
	delete work;
}

///////////////
// Event table
BEGIN_EVENT_TABLE(DialogStyleEditor, wxDialog)
	EVT_BUTTON(wxID_APPLY, DialogStyleEditor::OnApply)
	EVT_BUTTON(wxID_OK, DialogStyleEditor::OnOK)
	EVT_BUTTON(wxID_CANCEL, DialogStyleEditor::OnCancel)
	EVT_BUTTON(BUTTON_COLOR_1, DialogStyleEditor::OnSetColor1)
	EVT_BUTTON(BUTTON_COLOR_2, DialogStyleEditor::OnSetColor2)
	EVT_BUTTON(BUTTON_COLOR_3, DialogStyleEditor::OnSetColor3)
	EVT_BUTTON(BUTTON_COLOR_4, DialogStyleEditor::OnSetColor4)
	EVT_BUTTON(BUTTON_PREVIEW_COLOR, DialogStyleEditor::OnPreviewColourChange)

	EVT_CHILD_FOCUS(DialogStyleEditor::OnChildFocus)
	EVT_TEXT(TEXT_PREVIEW, DialogStyleEditor::OnPreviewTextChange)
	EVT_CHECKBOX(CHECKBOX_STYLE_BOLD, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_CHECKBOX(CHECKBOX_STYLE_ITALIC, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_CHECKBOX(CHECKBOX_STYLE_UNDERLINE, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_CHECKBOX(CHECKBOX_STYLE_STRIKEOUT, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_CHECKBOX(CHECKBOX_OUTLINE, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_COMBOBOX(COMBO_ENCODING, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_COMBOBOX(TEXT_FONT_NAME, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_TEXT_ENTER(TEXT_FONT_NAME, DialogStyleEditor::OnCommandPreviewUpdate)
	EVT_SPINCTRL(TEXT_ALPHA_1, DialogStyleEditor::OnSpinPreviewUpdate)
	EVT_SPINCTRL(TEXT_ALPHA_2, DialogStyleEditor::OnSpinPreviewUpdate)
	EVT_SPINCTRL(TEXT_ALPHA_3, DialogStyleEditor::OnSpinPreviewUpdate)
	EVT_SPINCTRL(TEXT_ALPHA_4, DialogStyleEditor::OnSpinPreviewUpdate)
END_EVENT_TABLE()



/// @brief Event redirectors 
/// @param event 
///
void DialogStyleEditor::OnApply (wxCommandEvent &event) { Apply(true,false); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnOK (wxCommandEvent &event) { SavePosition(); Apply(true,true); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnCancel (wxCommandEvent &event) { SavePosition(); Apply(false,true); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnSetColor1 (wxCommandEvent &event) { OnSetColor(1); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnSetColor2 (wxCommandEvent &event) { OnSetColor(2); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnSetColor3 (wxCommandEvent &event) { OnSetColor(3); }

/// @brief DOCME
/// @param event 
///
void DialogStyleEditor::OnSetColor4 (wxCommandEvent &event) { OnSetColor(4); }

/// @brief Replace Style 
/// @param tag      
/// @param n        
/// @param param    
/// @param userData 
///
void ReplaceStyle(wxString tag,int n,AssOverrideParameter* param,void *userData) {
	wxArrayString strings = *((wxArrayString*)userData);
	if (tag == _T("\\r")) {
		if (param->GetType() == VARDATA_TEXT) {
			if (param->Get<wxString>() == strings[0]) {
				param->Set(strings[1]);
			}
		}
	}
}

/// @brief Events 
/// @param apply 
/// @param close 
/// @return 
///
void DialogStyleEditor::Apply (bool apply,bool close) {
	// Apply
	if (apply) {
		// Style name
		wxString newStyleName = StyleName->GetValue();

		// Get list of existing styles
		wxArrayString styles;
		if (isLocal) styles = grid->ass->GetStyles();
		else if (store) styles = store->GetNames();

		// Check if style name is unique
		for (unsigned int i=0;i<styles.Count();i++) {
			if (newStyleName.CmpNoCase(styles[i]) == 0) {
				if ((isLocal && (grid->ass->GetStyle(styles[i]) != style)) || (!isLocal && (store->GetStyle(styles[i]) != style))) {
					wxMessageBox(_T("There is already a style with this name. Please choose another name."),_T("Style name conflict."),wxICON_ERROR|wxOK);
					return;
				}
			}
		}

		// Style name change
		if (work->name != newStyleName) {
			if (!isNew && isLocal) {
				// See if user wants to update style name through script
				int answer = wxNO;
				if (work->name != _T("Default")) answer = wxMessageBox(_("Do you want to change all instances of this style in the script to this new name?"),_("Update script?"),wxYES_NO | wxCANCEL);

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
						curDiag->ClearBlocks();
					}
				}
			}

			// Change name
			work->name = newStyleName;
		}

		// Update work style
		UpdateWorkStyle();

		// Copy
		*style = *work;
		style->UpdateData();
		if (isLocal) {
			AssFile::top->FlagAsModified(_("style change"));
			grid->CommitChanges();
		}

		// Exit
		if (close) {
			EndModal(1);
			if (PreviewText) OPT_SET("Tool/Style Editor/Preview Text")->SetString(STD_STR(PreviewText->GetValue()));
		}

		// Update preview
		else if (SubsPreview) SubsPreview->SetStyle(*style);
	}

	// Close
	else {
		if (close) {
			EndModal(0);
			if (PreviewText) OPT_SET("Tool/Style Editor/Preview Text")->SetString(STD_STR(PreviewText->GetValue()));
		}
	}
}

/// @brief Update work style 
///
void DialogStyleEditor::UpdateWorkStyle() {
	// Font and its size
	work->font = FontName->GetValue();
	FontSize->GetValue().ToDouble(&(work->fontsize));

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
	for (int i=0;i<3;i++) work->Margin[i] = margin[i]->GetValue();
	work->Margin[3] = margin[2]->GetValue();

	// Color alphas
	work->primary.a = colorAlpha[0]->GetValue();
	work->secondary.a = colorAlpha[1]->GetValue();
	work->outline.a = colorAlpha[2]->GetValue();
	work->shadow.a = colorAlpha[3]->GetValue();

	// Bold/italic/underline/strikeout
	work->bold = BoxBold->IsChecked();
	work->italic = BoxItalic->IsChecked();
	work->underline = BoxUnderline->IsChecked();
	work->strikeout = BoxStrikeout->IsChecked();
}

/// @brief Choose font box 
/// @param event 
///
void DialogStyleEditor::OnChooseFont (wxCommandEvent &event) {
	wxFont oldfont (int(work->fontsize), wxFONTFAMILY_DEFAULT, (work->italic?wxFONTSTYLE_ITALIC:wxFONTSTYLE_NORMAL), (work->bold?wxFONTWEIGHT_BOLD:wxFONTWEIGHT_NORMAL), work->underline, work->font, wxFONTENCODING_DEFAULT);
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
		UpdateWorkStyle();
		if (SubsPreview) SubsPreview->SetStyle(*work);

		// Comic sans warning
		if (newfont.GetFaceName() == _T("Comic Sans MS")) {
			wxMessageBox(_("You have chosen to use the \"Comic Sans\" font. As the programmer and a typesetter,\nI must urge you to reconsider. Comic Sans is the most abused font in the history\nof computing, so please avoid using it unless it's REALLY suitable. Thanks."), _("Warning"), wxICON_EXCLAMATION | wxOK);
		}
	}
}

/// @brief Sets color for one of the four color buttons 
/// @param n 
///
void DialogStyleEditor::OnSetColor (int n) {
	AssColor *modify;
	switch (n) {
		case 1: modify = &work->primary; break;
		case 2: modify = &work->secondary; break;
		case 3: modify = &work->outline; break;
		case 4: modify = &work->shadow; break;
		default: throw _T("Internal error in style editor, attempted setting colour id outside range");
	}
	modify->SetWXColor(colorButton[n-1]->GetColour());
	if (SubsPreview) SubsPreview->SetStyle(*work);
}

/// @brief Child focus change 
/// @param event 
///
void DialogStyleEditor::OnChildFocus (wxChildFocusEvent &event) {
	UpdateWorkStyle();
	if (SubsPreview) SubsPreview->SetStyle(*work);
	event.Skip();
}

/// @brief Preview text changed 
/// @param event 
///
void DialogStyleEditor::OnPreviewTextChange (wxCommandEvent &event) {
	if (PreviewText) {
		if (SubsPreview) SubsPreview->SetText(PreviewText->GetValue());
		event.Skip();
	}
}

/// @brief Change colour of preview's background 
/// @param event 
///
void DialogStyleEditor::OnPreviewColourChange (wxCommandEvent &event) {
	if (SubsPreview) SubsPreview->SetColour(previewButton->GetColour());
	OPT_SET("Colour/Style Editor/Background/Preview")->SetColour(STD_STR(previewButton->GetColour().GetAsString(wxC2S_CSS_SYNTAX)));
}

/// @brief Command event to update preview 
/// @param event 
/// @return 
///
void DialogStyleEditor::OnCommandPreviewUpdate (wxCommandEvent &event) {
	if (!IsShownOnScreen()) return;
	UpdateWorkStyle();
	if (SubsPreview) SubsPreview->SetStyle(*work);
	event.Skip();
}

/// @brief Converts control value to alignment 
/// @param n 
/// @return 
///
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
/// @param n 
/// @return 
///
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

/// @brief Load and save window position for the session 
///
void DialogStyleEditor::SavePosition() {
	use_saved_position = true;
	saved_position = GetRect();
}

/// @brief DOCME
///
void DialogStyleEditor::LoadPosition() {
	if (use_saved_position)
		SetSize(saved_position);
	else
		CentreOnParent();
}

/// DOCME
wxRect DialogStyleEditor::saved_position;

/// DOCME
bool DialogStyleEditor::use_saved_position = false;
