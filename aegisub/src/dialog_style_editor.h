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

/// @file dialog_style_editor.h
/// @see dialog_style_editor.cpp
/// @ingroup style_editor
///


#pragma once


////////////
// Includes
#ifndef AGI_PRE
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#endif

#include "colour_button.h"


//////////////
// Prototypes
class AssStyle;
class SubtitlesGrid;
class SubtitlesPreview;
class AssStyleStorage;



/// DOCME
/// @class DialogStyleEditor
/// @brief DOCME
///
/// DOCME
class DialogStyleEditor : public wxDialog {
private:

	/// DOCME
	bool isLocal;

	/// DOCME
	AssStyle *style;

	/// DOCME
	AssStyle *work;

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	AssStyleStorage *store;


	/// DOCME
	wxString FontSizeValue;

	/// DOCME
	wxString AlignmentValue;

	/// DOCME
	wxString OutlineValue;

	/// DOCME
	wxString ShadowValue;

	/// DOCME
	wxString ScaleXValue;

	/// DOCME
	wxString ScaleYValue;

	/// DOCME
	wxString AngleValue;

	/// DOCME
	wxString EncodingValue;

	/// DOCME
	wxString SpacingValue;


	/// DOCME
	wxTextCtrl *StyleName;

	/// DOCME
	wxComboBox *FontName;

	/// DOCME
	wxTextCtrl *FontSize;

	/// DOCME
	wxCheckBox *BoxBold;

	/// DOCME
	wxCheckBox *BoxItalic;

	/// DOCME
	wxCheckBox *BoxUnderline;

	/// DOCME
	wxCheckBox *BoxStrikeout;

	/// DOCME
	ColourButton *colorButton[4];

	/// DOCME
	wxSpinCtrl *colorAlpha[4];

	/// DOCME
	wxSpinCtrl *margin[4];

	/// DOCME
	wxRadioBox *Alignment;

	/// DOCME
	wxTextCtrl *Outline;

	/// DOCME
	wxTextCtrl *Shadow;

	/// DOCME
	wxCheckBox *OutlineType;

	/// DOCME
	wxTextCtrl *ScaleX;

	/// DOCME
	wxTextCtrl *ScaleY;

	/// DOCME
	wxTextCtrl *Angle;

	/// DOCME
	wxComboBox *Encoding;

	/// DOCME
	wxTextCtrl *Spacing;

	/// DOCME
	wxTextCtrl *PreviewText;

	/// DOCME
	SubtitlesPreview *SubsPreview;

	/// DOCME
	ColourButton *previewButton;

	/// DOCME
	wxSizer *MainSizer;


	/// DOCME
	static wxRect saved_position;

	/// DOCME
	static bool use_saved_position;
	void SavePosition();
	void LoadPosition();

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

	/// @brief DOCME
	/// @param event 
	///
	void OnSpinPreviewUpdate (wxSpinEvent &event) { OnCommandPreviewUpdate(event); }
	void OnPreviewTextChange (wxCommandEvent &event);
	void OnPreviewColourChange (wxCommandEvent &event);

public:
	DialogStyleEditor(wxWindow *parent,AssStyle *style,SubtitlesGrid *grid,bool local,AssStyleStorage *store);
	~DialogStyleEditor();

	void Apply (bool apply,bool close);
	void OnSetColor (int n);

	DECLARE_EVENT_TABLE()
};
