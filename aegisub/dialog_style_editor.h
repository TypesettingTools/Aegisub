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


#ifndef DIALOG_STYLE_EDITOR_H
#define DIALOG_STYLE_EDITOR_H


////////////
// Includes
#include <wx/wxprec.h>
#include <wx/spinctrl.h>
#include "colour_button.h"


//////////////
// Prototypes
class AssStyle;
class SubtitlesGrid;
class SubtitlesPreview;


/////////
// Class
class DialogStyleEditor : public wxDialog {
private:
	AssStyle *style;
	AssStyle *work;
	SubtitlesGrid *grid;

	wxString FontSizeValue;
	wxString AlignmentValue;
	wxString OutlineValue;
	wxString ShadowValue;
	wxString ScaleXValue;
	wxString ScaleYValue;
	wxString AngleValue;
	wxString EncodingValue;
	wxString SpacingValue;

	wxTextCtrl *StyleName;
	wxComboBox *FontName;
	wxTextCtrl *FontSize;
	wxCheckBox *BoxBold;
	wxCheckBox *BoxItalic;
	wxCheckBox *BoxUnderline;
	wxCheckBox *BoxStrikeout;
	ColourButton *colorButton[4];
	wxSpinCtrl *colorAlpha[4];
	wxSpinCtrl *margin[4];
	wxRadioBox *Alignment;
	wxTextCtrl *Outline;
	wxTextCtrl *Shadow;
	wxCheckBox *OutlineType;
	wxTextCtrl *ScaleX;
	wxTextCtrl *ScaleY;
	wxTextCtrl *Angle;
	wxComboBox *Encoding;
	wxTextCtrl *Spacing;
	wxTextCtrl *PreviewText;
	SubtitlesPreview *SubsPreview;
	ColourButton *previewButton;
	wxSizer *MainSizer;

	void SetBitmapColor (int n,wxColour color);
	int AlignToControl (int n);
	int ControlToAlign (int n);
	void UpdateWorkStyle ();

	void OnApply (wxCommandEvent &event);
	void OnCancel (wxCommandEvent &event);
	void OnOK (wxCommandEvent &event);
	void OnChooseFont (wxCommandEvent &event);
	void OnSetColor1 (wxCommandEvent &event);
	void OnSetColor2 (wxCommandEvent &event);
	void OnSetColor3 (wxCommandEvent &event);
	void OnSetColor4 (wxCommandEvent &event);
	void OnChildFocus (wxChildFocusEvent &event);
	void OnCommandPreviewUpdate (wxCommandEvent &event);
	void OnSpinPreviewUpdate (wxSpinEvent &event) { OnCommandPreviewUpdate(event); }
	void OnPreviewTextChange (wxCommandEvent &event);
	void OnPreviewColourChange (wxCommandEvent &event);

public:
	DialogStyleEditor(wxWindow *parent,AssStyle *style,SubtitlesGrid *grid);
	~DialogStyleEditor();

	void Apply (bool apply,bool close);
	void OnSetColor (int n);

	DECLARE_EVENT_TABLE()
};


#endif
