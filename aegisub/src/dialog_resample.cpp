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

/// @file dialog_resample.cpp
/// @brief Resample Resolution dialogue box and logic
/// @ingroup tools_ui
///


///////////
// Headers
#include "config.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "dialog_resample.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"


/// @brief Constructor 
/// @param parent 
/// @param _grid  
///
DialogResample::DialogResample(wxWindow *parent, SubtitlesGrid *_grid)
: wxDialog (parent,-1,_("Resample resolution"),wxDefaultPosition)
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(resample_toolbutton_24)));

	// Variables
	AssFile *subs = AssFile::top;
	grid = _grid;

	// Margins
	MarginSymmetrical = NULL;	// Do not remove this
	wxSizer *MarginBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Margin offset"));
	wxSizer *MarginSizer = new wxGridSizer(3,3,5,5);
	MarginTop = new wxTextCtrl(this,TEXT_MARGIN_T,_T("0"),wxDefaultPosition,wxSize(50,-1),0);
	MarginLeft = new wxTextCtrl(this,TEXT_MARGIN_L,_T("0"),wxDefaultPosition,wxSize(50,-1),0);
	MarginSymmetrical = new wxCheckBox(this,CHECK_SYMMETRICAL,_("Symmetrical"));
	MarginRight = new wxTextCtrl(this,TEXT_MARGIN_R,_T("0"),wxDefaultPosition,wxSize(50,-1),0);
	MarginBottom = new wxTextCtrl(this,TEXT_MARGIN_B,_T("0"),wxDefaultPosition,wxSize(50,-1),0);
	MarginSizer->AddSpacer(1);
	MarginSizer->Add(MarginTop,1,wxEXPAND);
	MarginSizer->AddSpacer(1);
	MarginSizer->Add(MarginLeft,1,wxEXPAND);
	MarginSizer->Add(MarginSymmetrical,1,wxEXPAND);
	MarginSizer->Add(MarginRight,1,wxEXPAND);
	MarginSizer->AddSpacer(1);
	MarginSizer->Add(MarginBottom,1,wxEXPAND);
	MarginSizer->AddSpacer(1);
	MarginBoxSizer->Add(MarginSizer,1,wxALIGN_CENTER|wxBOTTOM,5);
	MarginSymmetrical->SetValue(true);
	MarginRight->Enable(false);
	MarginBottom->Enable(false);
	
	// Resolution
	wxSizer *ResBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Resolution"));
	wxSizer *ResSizer = new wxBoxSizer(wxHORIZONTAL);
	int sw,sh;
	subs->GetResolution(sw,sh);
	ResXValue = wxString::Format(_T("%i"),sw);
	ResYValue = wxString::Format(_T("%i"),sh);
	ResX = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(&ResXValue));
	ResY = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(&ResYValue));
	wxStaticText *ResText = new wxStaticText(this,-1,_("x"));
	wxButton *FromVideo = new wxButton(this,BUTTON_DEST_FROM_VIDEO,_("From video"));
	if (!VideoContext::Get()->IsLoaded()) FromVideo->Enable(false);
	ResSizer->Add(ResX,1,wxRIGHT,5);
	ResSizer->Add(ResText,0,wxALIGN_CENTER | wxRIGHT,5);
	ResSizer->Add(ResY,1,wxRIGHT,5);
	ResSizer->Add(FromVideo,1,0,0);
	Anamorphic = new wxCheckBox(this,CHECK_ANAMORPHIC,_("Change aspect ratio"));
	ResBoxSizer->Add(ResSizer,1,wxEXPAND|wxBOTTOM,5);
	ResBoxSizer->Add(Anamorphic,0,0,0);

	// Button sizer
	wxStdDialogButtonSizer *ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizer->AddButton(new wxButton(this,wxID_OK));
	ButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	ButtonSizer->AddButton(new HelpButton(this,_T("Resample")));
	ButtonSizer->Realize();

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(MarginBoxSizer,1,wxEXPAND|wxALL,5);
	MainSizer->Add(ResBoxSizer,0,wxEXPAND|wxALL,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	CenterOnParent();
	instance = this;
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogResample,wxDialog)
	EVT_BUTTON(wxID_OK,DialogResample::OnResample)
	EVT_BUTTON(BUTTON_DEST_FROM_VIDEO,DialogResample::OnGetDestRes)
	EVT_CHECKBOX(CHECK_SYMMETRICAL,DialogResample::OnSymmetrical)
	EVT_TEXT(TEXT_MARGIN_T,DialogResample::OnMarginChange)
	EVT_TEXT(TEXT_MARGIN_L,DialogResample::OnMarginChange)
	EVT_TEXT(TEXT_MARGIN_R,DialogResample::OnMarginChange)
	EVT_TEXT(TEXT_MARGIN_B,DialogResample::OnMarginChange)
END_EVENT_TABLE()



/// @brief Resample tags 
/// @param name     
/// @param n        
/// @param curParam 
/// @param _curDiag 
///
void DialogResample::ResampleTags (wxString name,int n,AssOverrideParameter *curParam,void *_curDiag) {
	instance->DoResampleTags(name,n,curParam,_curDiag);
}

/// @brief DOCME
/// @param name     
/// @param n        
/// @param curParam 
/// @param _curDiag 
/// @return 
///
void DialogResample::DoResampleTags (wxString name,int n,AssOverrideParameter *curParam,void *_curDiag) {
	double resizer = 1.0;
	bool isX = false;
	bool isY = false;

	switch (curParam->classification) {
		case PARCLASS_ABSOLUTE_SIZE:
			resizer = r;
			break;
			
		case PARCLASS_ABSOLUTE_POS_X:
			resizer = rx;
			isX = true;
			break;

		case PARCLASS_ABSOLUTE_POS_Y:
			resizer = ry;
			isY = true;
			break;

		case PARCLASS_RELATIVE_SIZE_X:
			resizer = ar;
			break;

		case PARCLASS_RELATIVE_SIZE_Y:
			//resizer = ry;
			break;

		case PARCLASS_DRAWING:
			{
				AssDialogueBlockDrawing block;
				block.text = curParam->AsText();
				block.TransformCoords(m[0],m[2],rx,ry);
				curParam->SetText(block.GetText());
			}
			return;

		default:
			return;
	}

	VariableDataType curType = curParam->GetType();
	if (curType == VARDATA_FLOAT) {
		float par = curParam->AsFloat();
		if (isX) par += m[0];
		if (isY) par += m[2];
		curParam->SetFloat(par * resizer);
	}
	if (curType == VARDATA_INT) {
		int par = curParam->AsInt();
		if (isX) par += m[0];
		if (isY) par += m[2];
		curParam->SetInt(int(double(par) * resizer + 0.5));
	}
}



/// @brief Resample 
/// @param event 
/// @return 
///
void DialogResample::OnResample (wxCommandEvent &event) {
	// Resolutions
	AssFile *subs = AssFile::top;
	int x1,y1;
	subs->GetResolution(x1,y1);
	long x2 = 0;
	long y2 = 0;
	ResX->GetValue().ToLong(&x2);
	ResY->GetValue().ToLong(&y2);

	// Sanity check
	if (x1 == 0 || y1 == 0) {
		wxMessageBox(_T("Invalid source resolution. This should not happen. Please contact the developers."),_("Error"),wxCENTRE|wxICON_ERROR);
		return;
	}
	if (x2 == 0 || y2 == 0) {
		wxMessageBox(_("Invalid resolution: destination resolution cannot be 0 on either dimension."),_("Error"),wxCENTRE|wxICON_ERROR);
		return;
	}

	// Get margins
	MarginLeft->GetValue().ToLong(&m[0]);
	MarginRight->GetValue().ToLong(&m[1]);
	MarginTop->GetValue().ToLong(&m[2]);
	MarginBottom->GetValue().ToLong(&m[3]);

	// Add margins to original resolution
	x1 += m[0] + m[1];
	x2 += m[2] + m[3];

	// Calculate resamples
	rx = double(x2)/double(x1);
	ry = double(y2)/double(y1);
	r = ry;
	if (Anamorphic->IsChecked()) ar = rx/ry;
	else ar = 1.0;

	// Iterate through subs
	AssStyle *curStyle;
	AssDialogue *curDiag;
	for (entryIter cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		// Apply to dialogues
		curDiag = dynamic_cast<AssDialogue*>(*cur);
		if (curDiag && !(curDiag->Comment && (curDiag->Effect.StartsWith(_T("template")) || curDiag->Effect.StartsWith(_T("code"))))) {
			try {
				// Override tags
				curDiag->ParseASSTags();
				curDiag->ProcessParameters(&DialogResample::ResampleTags,curDiag);

				// Drawing tags
				size_t nblocks = curDiag->Blocks.size();
				AssDialogueBlockDrawing *curBlock;
				for (size_t i=0;i<nblocks;i++) {
					curBlock = AssDialogueBlock::GetAsDrawing(curDiag->Blocks.at(i));
					if (curBlock) {
						curBlock->TransformCoords(m[0],m[2],rx,ry);
					}
				}

				// Margins
				for (int i=0;i<2;i++) {
					curDiag->Margin[i] = int((curDiag->Margin[i]+m[i]) * rx + 0.5);
					curDiag->Margin[i+2] = int((curDiag->Margin[i+2]+m[i+2]) * ry + 0.5);
				}

				// Update
				curDiag->UpdateText();
				curDiag->UpdateData();
				curDiag->ClearBlocks();
				continue;
			}
			catch (const wchar_t *err) {
				wxLogMessage(err);
			}
			catch (wxString err) {
				wxLogMessage(err);
			}
		}

		// Apply to styles
		curStyle = dynamic_cast<AssStyle*>(*cur);
		if (curStyle) {
			curStyle->fontsize = int(curStyle->fontsize * r + 0.5);
			curStyle->outline_w *= r;
			curStyle->shadow_w *= r;
			curStyle->spacing *= rx;
			curStyle->scalex *= ar;
			for (int i=0;i<2;i++) {
				curStyle->Margin[i] = int((curStyle->Margin[i]+m[i]) * rx + 0.5);
				curStyle->Margin[i+2] = int((curStyle->Margin[i+2]+m[i+2]) * ry + 0.5);
			}
			curStyle->UpdateData();
		}
	}

	// Change script resolution
	subs->SetScriptInfo(_T("PlayResX"),wxString::Format(_T("%i"),x2));
	subs->SetScriptInfo(_T("PlayResY"),wxString::Format(_T("%i"),y2));

	// Flag as modified
	subs->FlagAsModified(_("resolution resampling"));
	grid->CommitChanges();
	grid->editBox->Update();
	EndModal(0);
}



/// @brief Get destination resolution from video 
/// @param event 
///
void DialogResample::OnGetDestRes (wxCommandEvent &event) {
	ResX->SetValue(wxString::Format(_T("%i"),VideoContext::Get()->GetWidth()));
	ResY->SetValue(wxString::Format(_T("%i"),VideoContext::Get()->GetHeight()));
}



/// @brief Symmetrical checkbox clicked 
/// @param event 
///
void DialogResample::OnSymmetrical (wxCommandEvent &event) {
	bool state = !MarginSymmetrical->IsChecked();
	MarginRight->Enable(state);
	MarginBottom->Enable(state);
	if (!state) {
		MarginRight->SetValue(MarginLeft->GetValue());
		MarginBottom->SetValue(MarginTop->GetValue());
	}
}



/// @brief Margin value changed 
/// @param event 
/// @return 
///
void DialogResample::OnMarginChange (wxCommandEvent &event) {
	if (!MarginSymmetrical) return;
	bool state = !MarginSymmetrical->IsChecked();
	if (!state && (event.GetEventObject() == MarginLeft || event.GetEventObject() == MarginTop)) {
		MarginRight->SetValue(MarginLeft->GetValue());
		MarginBottom->SetValue(MarginTop->GetValue());
	}
}



/// DOCME
DialogResample *DialogResample::instance = NULL;


