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


///////////
// Headers
#include "dialog_resample.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_override.h"
#include "subs_grid.h"
#include "validators.h"
#include "video_context.h"
#include "utils.h"


///////////////
// Constructor
DialogResample::DialogResample(wxWindow *parent, SubtitlesGrid *_grid)
: wxDialog (parent,-1,_("Resample resolution"),wxDefaultPosition)
{
	// Set icon
	SetIcon(BitmapToIcon(wxBITMAP(resample_toolbutton)));

	// Variables
	AssFile *subs = AssFile::top;
	grid = _grid;
	
	// Resolution line
	wxSizer *ResBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Resolution"));
	wxSizer *ResSizer = new wxBoxSizer(wxHORIZONTAL);
	int sw,sh;
	subs->GetResolution(sw,sh);
	ResXValue = wxString::Format(_T("%i"),sw);
	ResYValue = wxString::Format(_T("%i"),sh);
	ResX = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(50,20),0,NumValidator(&ResXValue));
	ResY = new wxTextCtrl(this,-1,_T(""),wxDefaultPosition,wxSize(50,20),0,NumValidator(&ResYValue));
	wxStaticText *ResText = new wxStaticText(this,-1,_("x"));
	wxButton *FromVideo = new wxButton(this,BUTTON_DEST_FROM_VIDEO,_("From video"));
	if (!VideoContext::Get()->IsLoaded()) FromVideo->Enable(false);
	ResSizer->Add(ResX,1,wxRIGHT,5);
	ResSizer->Add(ResText,0,wxALIGN_CENTER | wxRIGHT,5);
	ResSizer->Add(ResY,1,wxRIGHT,5);
	ResSizer->Add(FromVideo,1,0,0);

	// Resolution box
	Anamorphic = new wxCheckBox(this,CHECK_ANAMORPHIC,_("Change aspect ratio"));
	ResBoxSizer->Add(ResSizer,1,wxEXPAND|wxBOTTOM,5);
	ResBoxSizer->Add(Anamorphic,0,0,0);

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifndef __WXMAC__
	ButtonSizer->Add(new wxButton(this,BUTTON_RESAMPLE,_("Resample")),0,wxRIGHT,5);
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT,0);
#else
	ButtonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT,5);
	wxButton *resampleButton = new wxButton(this,BUTTON_RESAMPLE,_("Resample"));
	ButtonSizer->Add(resampleButton,0,wxRight,0);
	resampleButton->SetDefault();
#endif

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(ResBoxSizer,1,wxEXPAND|wxALL,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND|wxRIGHT|wxLEFT|wxBOTTOM,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
	CenterOnParent();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogResample,wxDialog)
	EVT_BUTTON(BUTTON_RESAMPLE,DialogResample::OnResample)
	EVT_BUTTON(BUTTON_DEST_FROM_VIDEO,DialogResample::OnGetDestRes)
END_EVENT_TABLE()


/////////////////
// Resample tags
void DialogResample::ResampleTags (wxString name,int n,AssOverrideParameter *curParam,void *_curDiag) {
	double resizer = 1.0;

	switch (curParam->classification) {
		case PARCLASS_ABSOLUTE_SIZE:
			resizer = r;
			break;
			
		case PARCLASS_ABSOLUTE_POS_X:
			resizer = rx;
			break;

		case PARCLASS_ABSOLUTE_POS_Y:
			resizer = ry;
			break;

		case PARCLASS_RELATIVE_SIZE_X:
			resizer = ar;
			break;

		case PARCLASS_RELATIVE_SIZE_Y:
			//resizer = ry;

		default:
			return;
	}

	VariableDataType curType = curParam->GetType();
	if (curType == VARDATA_FLOAT) {
		curParam->SetFloat(curParam->AsFloat() * resizer);
	}
	if (curType == VARDATA_INT) {
		curParam->SetInt(int(double(curParam->AsInt()) * resizer + 0.5));
	}
}


////////////
// Resample
void DialogResample::OnResample (wxCommandEvent &event) {
	// Resolutions
	AssFile *subs = AssFile::top;
	int x1,y1;
	subs->GetResolution(x1,y1);
	long x2 = 0;
	long y2 = 0;
	ResX->GetValue().ToLong(&x2);
	ResY->GetValue().ToLong(&y2);

	// Check for validity
	if (x1 == 0 || x2 == 0 || y1 == 0 || y2 == 0) {
		EndModal(0);
		return;
	}

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
		curDiag = AssEntry::GetAsDialogue(*cur);
		if (curDiag) {
			try {
				// Override tags
				curDiag->ParseASSTags();
				curDiag->ProcessParameters(ResampleTags,curDiag);

				// Drawing tags
				size_t nblocks = curDiag->Blocks.size();
				AssDialogueBlockDrawing *curBlock;
				for (size_t i=0;i<nblocks;i++) {
					curBlock = AssDialogueBlock::GetAsDrawing(curDiag->Blocks.at(i));
					if (curBlock) {
						curBlock->MultiplyCoords(rx,ry);
					}
				}

				// Margins
				for (int i=0;i<2;i++) {
					curDiag->Margin[i] = int(curDiag->Margin[i] * rx + 0.5);
					curDiag->Margin[i+2] = int(curDiag->Margin[i+2] * ry + 0.5);
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
		curStyle = AssEntry::GetAsStyle(*cur);
		if (curStyle) {
			curStyle->fontsize = int(curStyle->fontsize * r + 0.5);
			//curStyle->outline_w *= r;
			//curStyle->shadow_w *= r;
			curStyle->spacing *= rx;
			curStyle->scalex *= ar;
			for (int i=0;i<2;i++) curStyle->Margin[i] = int(curStyle->Margin[i] * rx + 0.5);
			for (int i=2;i<4;i++) curStyle->Margin[i] = int(curStyle->Margin[i] * ry + 0.5);
			curStyle->UpdateData();
		}
	}

	// Change script resolution
	subs->SetScriptInfo(_T("PlayResX"),wxString::Format(_T("%i"),x2));
	subs->SetScriptInfo(_T("PlayResY"),wxString::Format(_T("%i"),y2));

	// Flag as modified
	subs->FlagAsModified(_("resolution resampling"));
	grid->CommitChanges();;
	EndModal(0);
}


/////////////////////////////////////////
// Get destination resolution from video
void DialogResample::OnGetDestRes (wxCommandEvent &event) {
	ResX->SetValue(wxString::Format(_T("%i"),VideoContext::Get()->GetWidth()));
	ResY->SetValue(wxString::Format(_T("%i"),VideoContext::Get()->GetHeight()));
}


////////////////////
// Static variables
double DialogResample::r;
double DialogResample::rx;
double DialogResample::ry;
double DialogResample::ar;
