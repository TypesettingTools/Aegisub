// Copyright (c) 2006, Rodrigo Braz Monteiro
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


#pragma once


////////////
// Includes
#include "base_grid.h"
#include "utils.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "options.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "frame_main.h"
#include "video_display.h"


///////////////
// Constructor
BaseGrid::BaseGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxWindow(parent, id, pos, size, style, name)
{
	// Misc variables
	lastRow = -1;
	yPos = 0;
	bmp = NULL;
	holding = false;

	// Set font
	wxString fontname = Options.AsText(_T("Font Face"));
	if (fontname.IsEmpty()) fontname = _T("Tahoma");
	font.SetFaceName(fontname);
	font.SetPointSize(Options.AsInt(_T("Grid font size")));
	font.SetWeight(wxFONTWEIGHT_NORMAL);

	// Set line height
	{
		wxClientDC dc(this);
		dc.SetFont(font);
		int fw,fh;
		dc.GetTextExtent(_T("#TWFfgGhH"), &fw, &fh, NULL, NULL, &font);
		lineHeight = fh+4;
	}

	// Set scrollbar
	scrollBar = new wxScrollBar(this,GRID_SCROLLBAR,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	scrollBar->SetScrollbar(0,10,100,10);
	
	// Set column widths
	SetColumnWidths();
}


//////////////
// Destructor
BaseGrid::~BaseGrid() {
	delete bmp;
}



////////////////
// Select a row
void BaseGrid::SelectRow(int row, bool addToSelected, bool select) {
	if (!addToSelected) ClearSelection();
	try {
		bool cur = selMap.at(row);
		if (select != cur) {
			selMap.at(row) = select;
			
			if (!addToSelected) Refresh(false);

			else {
				int w = 0;
				int h = 0;
				GetClientSize(&w,&h);
				RefreshRect(wxRect(0,(row+1-yPos)*lineHeight,w,lineHeight),false);
			}
		}
	}
	catch (...) {}
}


///////////////
// Begin batch
void BaseGrid::BeginBatch() {
	//Freeze();
}


/////////////
// End batch
void BaseGrid::EndBatch() {
	//Thaw();
	AdjustScrollbar();
}


//////////////////////
// Makes cell visible
void BaseGrid::MakeCellVisible(int row, int col) {
	// Get size
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	bool forceCenter = true;

	// Get min and max visible
	int minVis = yPos+1;
	int maxVis = yPos+h/lineHeight-3;

	// Make visible
	if (forceCenter || row < minVis || row > maxVis) {
		ScrollTo(row - h/lineHeight/2 + 1);
	}
}


///////////////////////
// Unselects all cells
void BaseGrid::ClearSelection() {
	int rows = selMap.size();
	for (int i=0;i<rows;i++) {
		selMap[i] = false;
	}
	Refresh(false);
}


/////////////////////////
// Is cell in selection?
bool BaseGrid::IsInSelection(int row, int col) const {
	(void) col;
	try {
		return selMap.at(row);
	}
	catch (...) {
		return false;
	}
}


///////////////////////////
// Number of selected rows
int BaseGrid::GetNumberSelection() {
	int count = 0;
	int rows = selMap.size();
	for (int i=0;i<rows;i++) {
		if (selMap[i]) count++;
	}
	return count;
}


//////////////////////
// Get number of rows
int BaseGrid::GetRows() const {
	return diagMap.size();
}


/////////////////////
// Auto size columns
void BaseGrid::AutoSizeColumn(int col, bool setAsMin) {
	(void) col;
	(void) setAsMin;
	SetColumnWidths();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(BaseGrid,wxWindow)
	EVT_PAINT(BaseGrid::OnPaint)
	EVT_SIZE(BaseGrid::OnSize)
	EVT_COMMAND_SCROLL(GRID_SCROLLBAR,BaseGrid::OnScroll)
	EVT_MOUSE_EVENTS(BaseGrid::OnMouseEvent)
END_EVENT_TABLE()


///////////////
// Paint event
void BaseGrid::OnPaint (wxPaintEvent &event) {
	// Prepare
	wxPaintDC dc(this);
	bool direct = false;

	if (direct) {
		dc.BeginDrawing();
		DrawImage(dc);
		dc.EndDrawing();
	}

	else {
		// Get size and pos
		int w = 0;
		int h = 0;
		GetClientSize(&w,&h);

		// Prepare bitmap
		if (bmp) {
			if (bmp->GetWidth() < w || bmp->GetHeight() < h) {
				delete bmp;
				bmp = NULL;
			}
		}
		if (!bmp) bmp = new wxBitmap(w,h);

		// Draw bitmap
		wxMemoryDC bmpDC;
		bmpDC.SelectObject(*bmp);
		DrawImage(bmpDC);
		dc.Blit(0,0,w,h,&bmpDC,0,0);
	}
}


//////////////
// Draw image
void BaseGrid::DrawImage(wxDC &dc) {
	dc.BeginDrawing();

	// Get size and pos
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);

	// Set font
	dc.SetFont(font);

	// Clear background
	dc.SetBackground(wxBrush(wxColour(255,255,255)));
	dc.Clear();

	// Draw labels
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(wxColour(196,236,201)));
	dc.DrawRectangle(0,lineHeight,colWidth[0],h-lineHeight);

	// Visible lines
	int drawPerScreen = h/lineHeight + 1;
	int nDraw = MID(0,drawPerScreen,GetRows()-yPos);
	int maxH = (nDraw+1) * lineHeight;

	// Row colors
	std::vector<wxBrush> rowColors;
	std::vector<wxColor> foreColors;
	rowColors.push_back(wxBrush(wxColour(255,255,255)));								// 0 = Standard
	foreColors.push_back(wxColour(0,0,0));
	rowColors.push_back(wxBrush(wxColour(165,207,231)));								// 1 = Header
	foreColors.push_back(wxColour(0,0,0));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid selection background"))));	// 2 = Selected
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid comment background"))));		// 3 = Commented
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid inframe background"))));		// 4 = Video Highlighted
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));

	// First grid row
	bool drawGrid = true;
	if (drawGrid) {
		dc.SetPen(wxPen(wxColour(128,128,128)));
		dc.DrawLine(0,0,w,0);
		dc.SetPen(*wxTRANSPARENT_PEN);
	}

	// Draw rows
	int dx = 0;
	int dy = 0;
	int curColor = 0;
	AssDialogue *curDiag;
	for (int i=0;i<nDraw+1;i++) {
		// Prepare
		int curRow = i+yPos-1;
		curDiag = GetDialogue(curRow);
		dx = 0;
		dy = i*lineHeight;

		// Check for collisions
		bool collides = false;
		if (curDiag) {
			AssDialogue *sel = GetDialogue(editBox->linen);
			if (sel && sel != curDiag) {
				if (curDiag->CollidesWith(sel)) collides = true;
			}
		}

		// Text array
		wxArrayString strings;

		// Header
		if (i == 0) {
			strings.Add(_("#"));
			strings.Add(_("L"));
			strings.Add(_("Start"));
			strings.Add(_("End"));
			strings.Add(_("Style"));
			strings.Add(_("Actor"));
			strings.Add(_("Effect"));
			strings.Add(_("Left"));
			strings.Add(_("Right"));
			strings.Add(_("Vert"));
			strings.Add(_("Text"));
			curColor = 1;
		}

		// Lines
		else if (curDiag) {
			// Set fields
			strings.Add(wxString::Format(_T("%i"),curRow+1));
			strings.Add(wxString::Format(_T("%i"),curDiag->Layer));
			if (byFrame) {
				strings.Add(wxString::Format(_T("%i"),VFR_Output.CorrectFrameAtTime(curDiag->Start.GetMS(),true)));
				strings.Add(wxString::Format(_T("%i"),VFR_Output.CorrectFrameAtTime(curDiag->End.GetMS(),true)));
			}
			else {
				strings.Add(curDiag->Start.GetASSFormated());
				strings.Add(curDiag->End.GetASSFormated());
			}
			strings.Add(curDiag->Style);
			strings.Add(curDiag->Actor);
			strings.Add(curDiag->Effect);
			strings.Add(curDiag->GetMarginString(1));
			strings.Add(curDiag->GetMarginString(2));
			strings.Add(curDiag->GetMarginString(3));

			// Set text
			int mode = Options.AsInt(_T("Grid Hide Overrides"));
			wxString value = _T("");

			// Hidden overrides
			if (mode == 1 || mode == 2) {
				wxString replaceWith = Options.AsText(_T("Grid hide overrides char"));
				curDiag->ParseASSTags();
				size_t n = curDiag->Blocks.size();
				for (size_t i=0;i<n;i++) {
					AssDialogueBlock *block = curDiag->Blocks.at(i);
					AssDialogueBlockPlain *plain = AssDialogueBlock::GetAsPlain(block);
					if (plain) {
						value += plain->GetText();
					}
					else {
						if (mode == 1) {
							value += replaceWith;
						}
					}
				}
			}

			// Show overrides
			else value = curDiag->Text;

			// Cap length and set text
			if (value.Length() > 512) value = value.Left(512) + _T("...");
			strings.Add(value);

			// Set color
			curColor = 0;
			if (IsInSelection(curRow,0)) curColor = 2;
			else if (curDiag->Comment) curColor = 3;
			else if (Options.AsBool(_T("Highlight subs in frame")) && IsDisplayed(curDiag)) curColor = 4;
		}

		else {
			for (int j=0;j<11;j++) strings.Add(_T("?"));
		}

		// Draw row background color
		if (curColor) {
			dc.SetBrush(rowColors[curColor]);
			dc.DrawRectangle((curColor == 1) ? 0 : colWidth[0],i*lineHeight+1,w,lineHeight);
		}

		// Set text color
		if (collides) dc.SetTextForeground(wxColour(255,0,0));
		else {
			dc.SetTextForeground(foreColors[curColor]);
		}

		// Draw text
		wxRect cur;
		bool isCenter;
		for (int j=0;j<11;j++) {
			// Is center?
			isCenter = !(j == 4 || j == 5 || j == 6 || j == 10);

			// Calculate clipping
			cur = wxRect(dx+4,dy,colWidth[j]-6,lineHeight);

			// Set clipping
			dc.DestroyClippingRegion();
			dc.SetClippingRegion(cur);

			// Draw
			dc.DrawLabel(strings[j],cur,isCenter ? wxALIGN_CENTER : (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
			dx += colWidth[j];
		}
		//if (collides) dc.SetPen(wxPen(wxColour(255,0,0)));

		// Draw grid
		dc.DestroyClippingRegion();
		if (drawGrid) {
			dc.SetPen(wxPen(wxColour(128,128,128)));
			dc.DrawLine(0,dy+lineHeight,w,dy+lineHeight);
			dc.SetPen(*wxTRANSPARENT_PEN);
		}
	}

	// Draw grid columns
	dx = 0;
	if (drawGrid) {
		dc.SetPen(wxPen(wxColour(128,128,128)));
		for (int i=0;i<10;i++) {
			dx += colWidth[i];
			dc.DrawLine(dx,0,dx,maxH);
		}
		dc.DrawLine(0,0,0,maxH);
		dc.DrawLine(w-1,0,w-1,h);
	}

	// Draw currently active line border
	dc.SetPen(wxPen(wxColour(255,91,239)));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dy = (editBox->linen+1-yPos) * lineHeight;
	dc.DrawRectangle(0,dy,w,lineHeight+1);

	// Done
	dc.EndDrawing();
}


///////////
// On size
void BaseGrid::OnSize(wxSizeEvent &event) {
	AdjustScrollbar();
	SetColumnWidths();
	Refresh(false);
}


/////////////
// On scroll
void BaseGrid::OnScroll(wxScrollEvent &event) {
	int newPos = event.GetPosition();
	if (yPos != newPos) {
		yPos = newPos;
		Refresh(false);
	}
}


////////////////
// Mouse events
void BaseGrid::OnMouseEvent(wxMouseEvent &event) {
	// Window size
	int w,h;
	GetClientSize(&w,&h);

	// Modifiers
	bool shift = event.m_shiftDown;
	bool alt = event.m_altDown;
	bool ctrl = event.m_controlDown;

	// Row that mouse is over
	bool click = event.ButtonDown(wxMOUSE_BTN_LEFT);
	int row = event.GetY()/lineHeight + yPos - 1;
	if (holding && !click) {
		row = MID(0,row,GetRows()-1);
	}
	bool validRow = row >= 0 && row < GetRows();
	if (!validRow) row = -1;

	// Click type
	if (click && !holding && validRow) {
		holding = true;
		CaptureMouse();
	}
	if (!event.ButtonIsDown(wxMOUSE_BTN_LEFT) && holding) {
		holding = false;
		ReleaseMouse();
	}

	// Scroll to keep visible
	if (holding) {
		// Find direction
		int minVis = yPos+1;
		int maxVis = yPos+h/lineHeight-3;
		int delta = 0;
		if (row < minVis) delta = -1;
		if (row > maxVis) delta = +1;

		// Scroll
		ScrollTo(yPos+delta*3);
	}

	// Click
	if ((click || holding) && validRow) {
		// Toggle selected
		if (click && ctrl && !shift) {
			SelectRow(row,true,!IsInSelection(row,0));
			parentFrame->UpdateToolbar();
			return;
		}

		// Normal click
		if (click && !shift && !ctrl && !alt) {
			editBox->SetToLine(row);
			SelectRow(row,false);
			parentFrame->UpdateToolbar();
			lastRow = row;
			return;
		}

		// Block select
		if ((click && shift && !ctrl) || (holding)) {
			if (lastRow != -1) {
				// Set boundaries
				int i1 = row;
				int i2 = lastRow;
				if (i1 > i2) {
					int aux = i1;
					i1 = i2;
					i2 = aux;
				}

				// Toggle each
				bool notFirst = false;
				for (int i=i1;i<=i2;i++) {
					SelectRow(i,notFirst,true);
					notFirst = true;
				}
				parentFrame->UpdateToolbar();
			}
			return;
		}

		return;
	}

	// Popup
	if (event.ButtonDown(wxMOUSE_BTN_RIGHT)) {
		OnPopupMenu();
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		int step = 3 * event.GetWheelRotation() / event.GetWheelDelta();
		ScrollTo(yPos - step);
		return;
	}

	event.Skip();
}


/////////////
// Scroll to
void BaseGrid::ScrollTo(int y) {
	int w,h;
	GetClientSize(&w,&h);
	int nextY = MID(0,y,GetRows()+2 - h/lineHeight);
	if (yPos != nextY) {
		yPos = nextY;
		if (scrollBar->IsEnabled()) scrollBar->SetThumbPosition(yPos);
		Refresh(false);
	}
}


////////////////////
// Adjust scrollbar
void BaseGrid::AdjustScrollbar() {
	// Variables
	int w,h,sw,sh;
	GetClientSize(&w,&h);
	int drawPerScreen = h/lineHeight;
	int rows = GetRows();
	bool barToEnable = drawPerScreen < rows+2;
	bool barEnabled = scrollBar->IsEnabled();

	// Set size
	scrollBar->Freeze();
	scrollBar->GetSize(&sw,&sh);
	scrollBar->SetSize(w-sw,0,sw,h);

	// Set parameters
	if (barEnabled) {
		scrollBar->SetScrollbar(yPos,drawPerScreen,rows+2,drawPerScreen-2,true);
	}
	if (barToEnable != barEnabled) scrollBar->Enable(barToEnable);
	scrollBar->Thaw();
}


/////////////////////
// Set column widths
void BaseGrid::SetColumnWidths() {
	// Width/height
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);

	// DC for text extents test
	wxClientDC dc(this);
	dc.SetFont(font);
	int fw,fh;
	//dc.GetTextExtent(_T("#TWFfgGhH"), &fw, &fh, NULL, NULL, &font);

	// O(1) widths
	dc.GetTextExtent(_T("0000"), &fw, &fh, NULL, NULL, &font);
	int marginLen = fw + 10;
	dc.GetTextExtent(wxString::Format(_T("%i"),GetRows()), &fw, &fh, NULL, NULL, &font);
	int labelLen = fw + 10;
	int startLen = 0;
	int endLen = 0;
	if (!byFrame) {
		AssTime time;
		dc.GetTextExtent(time.GetASSFormated(), &fw, &fh, NULL, NULL, &font);
		startLen = fw;
		endLen = fw;
	}

	// O(n) widths
	int layerLen = 0;
	int actorLen = 0;
	int effectLen = 0;
	AssDialogue *curDiag;
	for (int i=0;i<GetRows();i++) {
		curDiag = GetDialogue(i);
		if (curDiag) {
			// Layer
			dc.GetTextExtent(wxString::Format(_T("%i"),curDiag->Layer), &fw, &fh, NULL, NULL, &font);
			if (fw > layerLen) layerLen = fw;

			// Actor
			dc.GetTextExtent(curDiag->Actor, &fw, &fh, NULL, NULL, &font);
			if (fw > actorLen) actorLen = fw;

			// Effect
			dc.GetTextExtent(curDiag->Effect, &fw, &fh, NULL, NULL, &font);
			if (fw > effectLen) effectLen = fw;

			// Times
			if (byFrame) {
				dc.GetTextExtent(wxString::Format(_T("%i"),VFR_Output.CorrectFrameAtTime(curDiag->Start.GetMS(),true)), &fw, &fh, NULL, NULL, &font);
				if (fw > startLen) startLen = fw;
				dc.GetTextExtent(wxString::Format(_T("%i"),VFR_Output.CorrectFrameAtTime(curDiag->End.GetMS(),true)), &fw, &fh, NULL, NULL, &font);
				if (fw > endLen) endLen = fw;
			}
		}
	}
	layerLen += 10;
	if (actorLen) actorLen += 10;
	if (effectLen) effectLen += 10;
	startLen += 10;
	endLen += 10;

	// Style length
	int styleLen = 0;
	AssStyle *curStyle;
	if (AssFile::top) {
		for (entryIter curIter=AssFile::top->Line.begin();curIter!=AssFile::top->Line.end();curIter++) {
			curStyle = AssEntry::GetAsStyle(*curIter);
			if (curStyle) {
				dc.GetTextExtent(curStyle->name, &fw, &fh, NULL, NULL, &font);
				if (fw > styleLen) styleLen = fw;
			}
		}
	}
	styleLen += 10;

	// Set column widths
	colWidth[0] = labelLen;
	colWidth[1] = layerLen;
	colWidth[2] = startLen;
	colWidth[3] = endLen;
	colWidth[4] = styleLen;
	colWidth[5] = actorLen;
	colWidth[6] = effectLen;
	colWidth[7] = marginLen;
	colWidth[8] = marginLen;
	colWidth[9] = marginLen;

	// Set size of last
	int total = 0;
	for (int i=0;i<10;i++) total+= colWidth[i];
	colWidth[10] = w-total;
}


//////////////////////////
// Gets dialogue from map
AssDialogue *BaseGrid::GetDialogue(int n) {
	try {
		return AssEntry::GetAsDialogue(*(diagMap.at(n)));
	}
	catch (...) {
		return NULL;
	}
}


////////////////////////////////////
// Check if line is being displayed
bool BaseGrid::IsDisplayed(AssDialogue *line) {
	if (!video->loaded) return false;
	int f1 = VFR_Output.CorrectFrameAtTime(line->Start.GetMS(),true);
	int f2 = VFR_Output.CorrectFrameAtTime(line->End.GetMS(),false);
	if (f1 <= video->frame_n && f2 >= video->frame_n) return true;
	return false;
}
