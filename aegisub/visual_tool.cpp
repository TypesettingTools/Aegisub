// Copyright (c) 2007, Rodrigo Braz Monteiro
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


//////////////
// Headers
#include <wx/glcanvas.h>
#include <GL/glu.h>
#include <wx/wxprec.h>
#include "visual_tool.h"
#include "video_display.h"
#include "video_provider.h"
#include "vfr.h"
#include "ass_file.h"
#include "ass_time.h"
#include "ass_dialogue.h"
#include "ass_override.h"
#include "ass_style.h"
#include "subs_grid.h"
#include "options.h"
#include "subs_edit_box.h"
#include "export_visible_lines.h"
#include "utils.h"


///////////////
// Constructor
VisualTool::VisualTool(VideoDisplay *par) {
	parent = par;
	colour[0] = wxColour(27,60,114);
	colour[1] = wxColour(166,247,177);
	colour[2] = wxColour(255,255,255);
	colour[3] = wxColour(187,0,0);

	holding = false;
	curDiag = NULL;
}


//////////////
// Destructor
VisualTool::~VisualTool() {
}


///////////////
// Mouse event
void VisualTool::OnMouseEvent (wxMouseEvent &event) {
	// General variables
	mouseX = event.GetX();
	mouseY = event.GetY();
	parent->ConvertMouseCoords(mouseX,mouseY);
	parent->GetClientSize(&w,&h);
	VideoContext::Get()->GetScriptSize(sw,sh);
	frame_n = VideoContext::Get()->GetFrameN();
	SubtitlesGrid *grid = VideoContext::Get()->grid;
	bool realTime = Options.AsBool(_T("Video Visual Realtime"));

	// Mouse leaving control
	if (event.Leaving()) {
		mouseX = -1;
		mouseY = -1;
	}

	// Transformed mouse x/y
	mx = mouseX * sw / w;
	my = mouseY * sh / h;

	// Clicks
	leftClick = event.ButtonDown(wxMOUSE_BTN_LEFT);
	leftDClick = event.LeftDClick();
	shiftDown = event.m_shiftDown;
	ctrlDown = event.m_controlDown;
	altDown = event.m_altDown;

	// Hold
	if (CanHold()) {
		// Start holding
		if (!holding && event.LeftIsDown()) {
			// Get a dialogue
			curDiag = GetActiveDialogueLine();
			if (curDiag) {
				// Initialize Drag
				InitializeHold();

				// Set flags
				holding = true;
				parent->CaptureMouse();
				if (realTime) AssLimitToVisibleFilter::SetFrame(frame_n);
			}
		}

		if (holding) {
			// Holding
			if (event.LeftIsDown()) {
				// Update drag
				UpdateHold();

				if (realTime) {
					// Commit
					CommitHold();
					grid->editBox->CommitText(true);
					grid->CommitChanges(false,true);
				}
			}

			// Release
			else {
				// Disable limiting
				if (realTime) AssLimitToVisibleFilter::SetFrame(-1);

				// Commit
				CommitHold();
				grid->editBox->CommitText();
				grid->ass->FlagAsModified(_("visual typesetting"));
				grid->CommitChanges(false,true);

				// Clean up
				holding = false;
				curDiag = NULL;
				parent->ReleaseMouse();
				parent->SetFocus();
			}
		}
	}

	// Update
	Update();
}


////////////////////////////
// Get active dialogue line
AssDialogue* VisualTool::GetActiveDialogueLine() {
	SubtitlesGrid *grid = VideoContext::Get()->grid;
	AssDialogue *diag = grid->GetDialogue(grid->editBox->linen);

	// Check if it's within range
	if (diag) {
		int f1 = VFR_Output.GetFrameAtTime(diag->Start.GetMS(),true);
		int f2 = VFR_Output.GetFrameAtTime(diag->End.GetMS(),false);

		// Invisible
		if (f1 > frame_n || f2 < frame_n) return NULL;
	}

	return diag;
}


////////////////////////
// Get position of line
void VisualTool::GetLinePosition(AssDialogue *diag,int &x, int &y) {
	int orgx=0,orgy=0;
	GetLinePosition(diag,x,y,orgx,orgy);
}
void VisualTool::GetLinePosition(AssDialogue *diag,int &x, int &y, int &orgx, int &orgy) {
	// No dialogue
	if (!diag) {
		x = -1;
		y = -1;
		return;
	}

	// Default values
	int margin[4];
	for (int i=0;i<4;i++) margin[i] = diag->Margin[i];
	int align = 2;

	// Get style
	AssStyle *style = VideoContext::Get()->grid->ass->GetStyle(diag->Style);
	if (style) {
		align = style->alignment;
		for (int i=0;i<4;i++) {
			if (margin[i] == 0) margin[i] = style->Margin[i];
		}
	}

	// Script size
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);

	// Process margins
	margin[3] = margin[2];
	margin[1] = sw - margin[1];
	margin[3] = sh - margin[3];

	// Position
	bool posSet = false;
	bool orgSet = false;
	int posx = -1;
	int posy = -1;

	// Overrides processing
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	for (size_t i=0;i<blockn;i++) {
		override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(i));
		if (override) {
			for (size_t j=0;j<override->Tags.size();j++) {
				tag = override->Tags.at(j);

				// Position
				if ((tag->Name == _T("\\pos") || tag->Name == _("\\move")) && tag->Params.size() >= 2) {
					if (!posSet) {
						posx = tag->Params[0]->AsInt();
						posy = tag->Params[1]->AsInt();
						posSet = true;
					}
				}

				// Alignment
				else if ((tag->Name == _T("\\an") || tag->Name == _T("\\a")) && tag->Params.size() >= 1) {
					align = tag->Params[0]->AsInt();
					if (tag->Name == _T("\\a")) {
						switch(align) {
							case 1: align = 1; break;
							case 2: align = 2; break;
							case 3: align = 3; break;
							case 5: align = 7; break;
							case 6: align = 8; break;
							case 7: align = 9; break;
							case 9: align = 4; break;
							case 10: align = 5; break;
							case 11: align = 6; break;
							default: align = 2; break;
						}
					}
				}

				// Origin
				else if (!orgSet && tag->Name == _T("\\org") && tag->Params.size() >= 2) {
					orgx = tag->Params[0]->AsInt();
					orgy = tag->Params[1]->AsInt();
					orgSet = true;
				}
			}
		}
	}
	diag->ClearBlocks();

	// Got position
	if (posSet) {
		x = posx;
		y = posy;
		if (!orgSet) {
			orgx = x;
			orgy = y;
		}
		return;
	}

	// Alignment type
	int hor = (align - 1) % 3;
	int vert = (align - 1) / 3;

	// Calculate positions
	if (hor == 0) x = margin[0];
	else if (hor == 1) x = (margin[0] + margin[1])/2;
	else if (hor == 2) x = margin[1];
	if (vert == 0) y = margin[3];
	else if (vert == 1) y = (margin[2] + margin[3])/2;
	else if (vert == 2) y = margin[2];

	// No origin?
	if (!orgSet) {
		orgx = x;
		orgy = y;
	}
}


///////////////////////
// Get line's rotation
void VisualTool::GetLineRotation(AssDialogue *diag,float &rx,float &ry,float &rz) {
	// Default values
	rx = ry = rz = 0.0f;

	// Prepare overrides
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	if (blockn == 0) {
		diag->ClearBlocks();
		return;
	}

	// Process override
	override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(0));
	if (override) {
		for (size_t j=0;j<override->Tags.size();j++) {
			tag = override->Tags.at(j);
			if (tag->Name == _T("\\frx") && tag->Params.size() == 1) {
				rx = tag->Params[0]->AsFloat();
			}
			if (tag->Name == _T("\\fry") && tag->Params.size() == 1) {
				ry = tag->Params[0]->AsFloat();
			}
			if ((tag->Name == _T("\\frz") || tag->Name == _T("\fr")) && tag->Params.size() == 1) {
				rz = tag->Params[0]->AsFloat();
			}
		}
	}
	diag->ClearBlocks();
}


////////////////////
// Get line's scale
void VisualTool::GetLineScale(AssDialogue *diag,float &scalX,float &scalY) {
	// Default values
	scalX = scalY = 100.0f;

	// Prepare overrides
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	if (blockn == 0) {
		diag->ClearBlocks();
		return;
	}

	// Process override
	override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(0));
	if (override) {
		for (size_t j=0;j<override->Tags.size();j++) {
			tag = override->Tags.at(j);
			if (tag->Name == _T("\\fscx") && tag->Params.size() == 1) {
				scalX = tag->Params[0]->AsFloat();
			}
			if (tag->Name == _T("\\fscy") && tag->Params.size() == 1) {
				scalY = tag->Params[0]->AsFloat();
			}
		}
	}
	diag->ClearBlocks();
}


///////////////////
// Get line's clip
void VisualTool::GetLineClip(AssDialogue *diag,int &x1,int &y1,int &x2,int &y2) {
	// Default values
	x1 = y1 = 0;
	int sw,sh;
	VideoContext::Get()->GetScriptSize(sw,sh);
	x2 = sw-1;
	y2 = sh-1;

	// Prepare overrides
	diag->ParseASSTags();
	AssDialogueBlockOverride *override;
	AssOverrideTag *tag;
	size_t blockn = diag->Blocks.size();
	if (blockn == 0) {
		diag->ClearBlocks();
		return;
	}

	// Process override
	override = AssDialogueBlock::GetAsOverride(diag->Blocks.at(0));
	if (override) {
		for (size_t j=0;j<override->Tags.size();j++) {
			tag = override->Tags.at(j);
			if (tag->Name == _T("\\clip") && tag->Params.size() == 4) {
				x1 = tag->Params[0]->AsInt();
				y1 = tag->Params[1]->AsInt();
				x2 = tag->Params[2]->AsInt();
				y2 = tag->Params[3]->AsInt();
			}
		}
	}
	diag->ClearBlocks();
}


////////////////
// Set override
void VisualTool::SetOverride(wxString tag,wxString value) {
	VideoContext::Get()->grid->editBox->SetOverride(tag,value,0,false);
}
