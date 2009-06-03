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


////////////
// Includes
#include "config.h"

#include "base_grid.h"
#include "utils.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "options.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "frame_main.h"
#include "video_box.h"
#include "video_slider.h"
#include "video_context.h"
#include "audio_display.h"


///////////////
// Constructor
BaseGrid::BaseGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxWindow(parent, id, pos, size, style, name)
{
	// Misc variables
	lastRow = -1;
	yPos = 0;
	extendRow = -1;
	bmp = NULL;
	holding = false;
	byFrame = false;

	// Set scrollbar
	scrollBar = new wxScrollBar(this,GRID_SCROLLBAR,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	scrollBar->SetScrollbar(0,10,100,10);

	// Set style
	UpdateStyle();
}


//////////////
// Destructor
BaseGrid::~BaseGrid() {
	delete bmp;
}


////////////////
// Update style
void BaseGrid::UpdateStyle() {
	// Set font
	wxString fontname = Options.AsText(_T("Grid Font Face"));
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

	// Set column widths
	for (int i=0;i<10;i++) showCol[i] = Options.AsBool(_T("Grid show column ") + AegiIntegerToString(i));
	SetColumnWidths();

	// Update
	AdjustScrollbar();
	Refresh();
}


///////////////
// Clears grid
void BaseGrid::Clear () {
	diagMap.clear();
	diagPtrMap.clear();
	selMap.clear();
	yPos = 0;
	AdjustScrollbar();
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
void BaseGrid::MakeCellVisible(int row, int col,bool center) {
	// Update last row selection
	lastRow = row;

	// Get size
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	bool forceCenter = !center;

	// Get min and max visible
	int minVis = yPos+1;
	int maxVis = yPos+h/lineHeight-3;

	// Make visible
	if (forceCenter || row < minVis || row > maxVis) {
		if (center) {
			ScrollTo(row - h/lineHeight/2 + 1);
		}

		else {
			if (row < minVis) ScrollTo(row - 1);
			if (row > maxVis) ScrollTo(row - h/lineHeight + 3);
		}
	}
}


////////////////
// Select a row
void BaseGrid::SelectRow(int row, bool addToSelected, bool select) {
	// Sanity checking
	if (row >= GetRows()) row = GetRows()-1;
	else if (row < 0) row = 0;

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


/////////////////////////
// Selects visible lines
void BaseGrid::SelectVisible() {
	int rows = GetRows();
	bool selectedOne = false;
	for (int i=0;i<rows;i++) {
		if (IsDisplayed(GetDialogue(i))) {
			if (!selectedOne) {
				SelectRow(i,false);
				MakeCellVisible(i,0);
				selectedOne = true;
			}
			else {
				SelectRow(i,true);
			}
		}
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
	if (row >= GetRows() || row < 0) return false;
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


///////////////////////////
// Gets first selected row
int BaseGrid::GetFirstSelRow() {
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			return i;
		}
	}
	return -1;
}


/////////////////////////////////////////////////////
// Gets last selected row from first block selection
int BaseGrid::GetLastSelRow() {
	int frow = GetFirstSelRow();
	while (IsInSelection(frow)) {
		frow++;
	}
	return frow-1;
}


//////////////////////////
// Gets all selected rows
wxArrayInt BaseGrid::GetSelection(bool *cont) {
	// Prepare
	int nrows = GetRows();
	int last = -1;
	bool continuous = true;
	wxArrayInt selections;

	// Scan
	for (int i=0;i<nrows;i++) {
		if (selMap[i]) {
			selections.Add(i);
			if (last != -1 && i != last+1) continuous = false;
			last = i;
		}
	}

	// Return
	if (cont) *cont = continuous;
	return selections;
}


//////////////////////
// Get number of rows
int BaseGrid::GetRows() const {
	return diagMap.size();
}


///////////////
// Event table
BEGIN_EVENT_TABLE(BaseGrid,wxWindow)
	EVT_PAINT(BaseGrid::OnPaint)
	EVT_SIZE(BaseGrid::OnSize)
	EVT_COMMAND_SCROLL(GRID_SCROLLBAR,BaseGrid::OnScroll)
	EVT_MOUSE_EVENTS(BaseGrid::OnMouseEvent)
	EVT_KEY_DOWN(BaseGrid::OnKeyPress)
END_EVENT_TABLE()


///////////////
// Paint event
void BaseGrid::OnPaint (wxPaintEvent &event) {
	// Prepare
	wxPaintDC dc(this);
	bool direct = false;

	if (direct) {
		DrawImage(dc);
	}

	else {
		// Get size and pos
		int w = 0;
		int h = 0;
		GetClientSize(&w,&h);
		w -= scrollBar->GetSize().GetWidth();

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
	// Get size and pos
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	w -= scrollBar->GetSize().GetWidth();

	// Set font
	dc.SetFont(font);

	// Clear background
	dc.SetBackground(wxBrush(Options.AsColour(_T("Grid Background"))));
	dc.Clear();

	// Draw labels
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Options.AsColour(_T("Grid left column"))));
	dc.DrawRectangle(0,lineHeight,colWidth[0],h-lineHeight);

	// Visible lines
	int drawPerScreen = h/lineHeight + 1;
	int nDraw = MID(0,drawPerScreen,GetRows()-yPos);
	int maxH = (nDraw+1) * lineHeight;

	// Row colors
	std::vector<wxBrush> rowColors;
	std::vector<wxColor> foreColors;
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid Background"))));					// 0 = Standard
	foreColors.push_back(Options.AsColour(_T("Grid standard foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid Header"))));						// 1 = Header
	foreColors.push_back(Options.AsColour(_T("Grid standard foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid selection background"))));		// 2 = Selected
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid comment background"))));			// 3 = Commented
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid inframe background"))));			// 4 = Video Highlighted
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));
	rowColors.push_back(wxBrush(Options.AsColour(_T("Grid selected comment background"))));	// 5 = Commented & selected
	foreColors.push_back(Options.AsColour(_T("Grid selection foreground")));

	// First grid row
	bool drawGrid = true;
	if (drawGrid) {
		dc.SetPen(wxPen(Options.AsColour(_T("Grid lines"))));
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
		curDiag = (curRow>=0) ? GetDialogue(curRow) : NULL;
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
				strings.Add(wxString::Format(_T("%i"),VFR_Output.GetFrameAtTime(curDiag->Start.GetMS(),true)));
				strings.Add(wxString::Format(_T("%i"),VFR_Output.GetFrameAtTime(curDiag->End.GetMS(),false)));
			}
			else {
				strings.Add(curDiag->Start.GetASSFormated());
				strings.Add(curDiag->End.GetASSFormated());
			}
			strings.Add(curDiag->Style);
			strings.Add(curDiag->Actor);
			strings.Add(curDiag->Effect);
			strings.Add(curDiag->GetMarginString(0));
			strings.Add(curDiag->GetMarginString(1));
			strings.Add(curDiag->GetMarginString(2));

			// Set text
			int mode = Options.AsInt(_T("Grid Hide Overrides"));
			wxString value = _T("");

			// Hidden overrides
			if (mode == 1 || mode == 2) {
				wxString replaceWith = Options.AsText(_T("Grid hide overrides char"));
				int textlen = curDiag->Text.Length();
				int depth = 0;
				wxChar curChar;
				for (int i=0;i<textlen;i++) {
					curChar = curDiag->Text[i];
					if (curChar == _T('{')) depth = 1;
					else if (curChar == _T('}')) {
						depth--;
						if (depth == 0 && mode == 1) value += replaceWith;
						if (depth < 0) depth = 0;
					}
					else if (depth != 1) value += curChar;
				}
			}

			// Show overrides
			else value = curDiag->Text;

			// Cap length and set text
			if (value.Length() > 512) value = value.Left(512) + _T("...");
			strings.Add(value);

			// Set color
			curColor = 0;
			bool inSel = IsInSelection(curRow,0);
			if (inSel && curDiag->Comment) curColor = 5;
			else if (inSel) curColor = 2;
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
		if (collides) dc.SetTextForeground(Options.AsColour(_T("Grid collision foreground")));
		else {
			dc.SetTextForeground(foreColors[curColor]);
		}

		// Draw text
		wxRect cur;
		bool isCenter;
		for (int j=0;j<11;j++) {
			// Check width
			if (colWidth[j] == 0) continue;

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
			dc.SetPen(wxPen(Options.AsColour(_T("Grid lines"))));
			dc.DrawLine(0,dy+lineHeight,w,dy+lineHeight);
			dc.SetPen(*wxTRANSPARENT_PEN);
		}
	}

	// Draw grid columns
	dx = 0;
	if (drawGrid) {
		dc.SetPen(wxPen(Options.AsColour(_T("Grid lines"))));
		for (int i=0;i<10;i++) {
			dx += colWidth[i];
			dc.DrawLine(dx,0,dx,maxH);
		}
		dc.DrawLine(0,0,0,maxH);
		dc.DrawLine(w-1,0,w-1,maxH);
	}

	// Draw currently active line border
	dc.SetPen(wxPen(Options.AsColour(_T("Grid Active border"))));
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dy = (editBox->linen+1-yPos) * lineHeight;
	dc.DrawRectangle(0,dy,w,lineHeight+1);
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
#ifdef __APPLE__
	bool ctrl = event.m_metaDown;
#else
	bool ctrl = event.m_controlDown;
#endif

	// Row that mouse is over
	bool click = event.ButtonDown(wxMOUSE_BTN_LEFT);
	bool dclick = event.LeftDClick();
	int row = event.GetY()/lineHeight + yPos - 1;
	bool headerClick = row < yPos;
	if (holding && !click) {
		row = MID(0,row,GetRows()-1);
	}
	bool validRow = row >= 0 && row < GetRows();
	if (!validRow) row = -1;

	// Get focus
	if (event.ButtonDown()) {
		if (Options.AsBool(_T("Grid Allow Focus"))) {
			SetFocus();
		}
	}

	// Click type
	bool startedHolding = false;
	if (click && !holding && validRow) {
		holding = true;
		startedHolding = true;
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
		if (delta) {
			ScrollTo(yPos+delta*3);
			if (startedHolding) {
				holding = false;
				ReleaseMouse();
			}
		}
	}

	// Click
	if ((click || holding || dclick) && validRow) {
		// Disable extending
		extendRow = -1;

		// Toggle selected
		if (click && ctrl && !shift && !alt) {
			SelectRow(row,true,!IsInSelection(row,0));
			parentFrame->UpdateToolbar();
			return;
		}

		// Normal click
		if ((click || dclick) && !shift && !ctrl && !alt) {
			if (editBox->linen != row) editBox->SetToLine(row);
			if (dclick) VideoContext::Get()->JumpToFrame(VFR_Output.GetFrameAtTime(GetDialogue(row)->Start.GetMS(),true));
			SelectRow(row,false);
			parentFrame->UpdateToolbar();
			lastRow = row;
			return;
		}

		// Keep selection
		if (click && !shift && !ctrl && alt) {
			editBox->SetToLine(row);
			return;
		}

		// Block select
		if ((click && shift && !ctrl && !alt) || (holding && !ctrl && !alt && !shift)) {
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
		OnPopupMenu(headerClick);
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

	// Set yPos
	yPos = MID(0,yPos,rows - drawPerScreen);

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
		startLen = fw + 10;
		endLen = fw + 10;
	}

	// O(n) widths
	bool showMargin[3];
	showMargin[0] = showMargin[1] = showMargin[2] = false;
	bool showLayer = false;
	int styleLen = 0;
	int actorLen = 0;
	int effectLen = 0;
	int maxLayer = 0;
	int maxStart = 0;
	int maxEnd = 0;
	AssDialogue *curDiag;
	for (int i=0;i<GetRows();i++) {
		curDiag = GetDialogue(i);
		if (curDiag) {
			// Layer
			if (curDiag->Layer > maxLayer) {
				maxLayer = curDiag->Layer;
				showLayer = true;
			}

			// Actor
			if (!curDiag->Actor.IsEmpty()) {
				dc.GetTextExtent(curDiag->Actor, &fw, &fh, NULL, NULL, &font);
				if (fw > actorLen) actorLen = fw;
			}

			// Style
			if (!curDiag->Style.IsEmpty()) {
				dc.GetTextExtent(curDiag->Style, &fw, &fh, NULL, NULL, &font);
				if (fw > styleLen) styleLen = fw;
			}

			// Effect
			if (!curDiag->Effect.IsEmpty()) {
				dc.GetTextExtent(curDiag->Effect, &fw, &fh, NULL, NULL, &font);
				if (fw > effectLen) effectLen = fw;
			}

			// Margins
			for (int j=0;j<3;j++) {
				if (curDiag->Margin[j] != 0) showMargin[j] = true;
			}

			// Times
			if (byFrame) {
				int tmp = VFR_Output.GetFrameAtTime(curDiag->Start.GetMS(),true);
				if (tmp > maxStart) maxStart = tmp;
				tmp = VFR_Output.GetFrameAtTime(curDiag->End.GetMS(),true);
				if (tmp > maxEnd) maxEnd = tmp;
			}
		}
	}

	// Finish layer
	dc.GetTextExtent(wxString::Format(_T("%i"),maxLayer), &fw, &fh, NULL, NULL, &font);
	int layerLen = fw + 10;

	// Finish times
	if (byFrame) {
		dc.GetTextExtent(wxString::Format(_T("%i"),maxStart), &fw, &fh, NULL, NULL, &font);
		startLen = fw + 10;
		dc.GetTextExtent(wxString::Format(_T("%i"),maxEnd), &fw, &fh, NULL, NULL, &font);
		endLen = fw + 10;
	}

	// Style length
	if (false && AssFile::top) {
		AssStyle *curStyle;
		for (entryIter curIter=AssFile::top->Line.begin();curIter!=AssFile::top->Line.end();curIter++) {
			curStyle = AssEntry::GetAsStyle(*curIter);
			if (curStyle) {
				dc.GetTextExtent(curStyle->name, &fw, &fh, NULL, NULL, &font);
				if (fw > styleLen) styleLen = fw;
			}
		}
	}

	// Finish actor/effect/style
	if (actorLen) actorLen += 10;
	if (effectLen) effectLen += 10;
	if (styleLen) styleLen += 10;

	// Set column widths
	colWidth[0] = labelLen;
	colWidth[1] = showLayer ? layerLen : 0;
	colWidth[2] = startLen;
	colWidth[3] = endLen;
	colWidth[4] = styleLen;
	colWidth[5] = actorLen;
	colWidth[6] = effectLen;
	for (int i=0;i<3;i++) colWidth[i+7] = showMargin[i] ? marginLen : 0;

	// Hide columns
	for (int i=0;i<10;i++) {
		if (showCol[i] == false) colWidth[i] = 0;
	}

	// Set size of last
	int total = 0;
	for (int i=0;i<10;i++) total+= colWidth[i];
	colWidth[10] = w-total;
}


//////////////////////////
// Gets dialogue from map
AssDialogue *BaseGrid::GetDialogue(int n) {
	try {
		if (n < 0) return NULL;
		AssEntry *e = *diagMap.at(n);
		if (e->GetType() != ENTRY_DIALOGUE) return NULL;
		return AssEntry::GetAsDialogue(e);
	}
	catch (...) {
		return NULL;
	}
}


////////////////////////////////////
// Check if line is being displayed
bool BaseGrid::IsDisplayed(AssDialogue *line) {
	if (!VideoContext::Get()->IsLoaded()) return false;
	int f1 = VFR_Output.GetFrameAtTime(line->Start.GetMS(),true);
	int f2 = VFR_Output.GetFrameAtTime(line->End.GetMS(),false);
	if (f1 <= VideoContext::Get()->GetFrameN() && f2 >= VideoContext::Get()->GetFrameN()) return true;
	return false;
}


///////////////
// Update maps
void BaseGrid::UpdateMaps() {
	// Store old
	int len = diagMap.size();
	std::vector<AssDialogue *> tmpDiagPtrMap;
	std::vector<bool> tmpSelMap;
	for (int i=0;i<len;i++) {
		tmpDiagPtrMap.push_back(diagPtrMap[i]);
		tmpSelMap.push_back(selMap[i]);
	}

	// Clear old
	diagPtrMap.clear();
	diagMap.clear();
	selMap.clear();
	
	// Re-generate lines
	int n = 0;
	AssDialogue *curdiag;
	for (entryIter cur=AssFile::top->Line.begin();cur != AssFile::top->Line.end();cur++) {
		curdiag = AssEntry::GetAsDialogue(*cur);
		if (curdiag) {
			// Find old pos
			bool sel = false;
			for (int i=0;i<len;i++) {
				if (tmpDiagPtrMap[i] == curdiag) {
					sel = tmpSelMap[i];
					break;
				}
			}

			// Add new
			diagMap.push_back(cur);
			diagPtrMap.push_back(curdiag);
			selMap.push_back(sel);

			n++;
		}
	}

	// Refresh
	Refresh(false);
}


/////////////
// Key press
void BaseGrid::OnKeyPress(wxKeyEvent &event) {
	// Get size
	int w,h;
	GetClientSize(&w,&h);

	// Get scan code
	int key = event.GetKeyCode();
#ifdef __APPLE__
	bool ctrl = event.m_metaDown;
#else
	bool ctrl = event.m_controlDown;
#endif
	bool alt = event.m_altDown;
	bool shift = event.m_shiftDown;

	// Left/right, forward to seek bar if video is loaded
	if (key == WXK_LEFT || key == WXK_RIGHT) {
		if (VideoContext::Get()->IsLoaded()) {
			parentFrame->videoBox->videoSlider->SetFocus();
			parentFrame->videoBox->videoSlider->AddPendingEvent(event);
			return;
		}
		event.Skip();
		return;
	}

	// Select all
	if (key == 'A' && ctrl && !alt && !shift) {
		int rows = GetRows();
		for (int i=0;i<rows;i++) SelectRow(i,true);
	}

	// Up/down
	int dir = 0;
	int step = 1;
	if (key == WXK_UP) dir = -1;
	if (key == WXK_DOWN) dir = 1;
	if (key == WXK_PAGEUP) {
		dir = -1;
		step = h/lineHeight - 2;
	}
	if (key == WXK_PAGEDOWN) {
		dir = 1;
		step = h/lineHeight - 2;
	}
	if (key == WXK_HOME) {
		dir = -1;
		step = GetRows();
	}
	if (key == WXK_END) {
		dir = 1;
		step = GetRows();
	}

	// Moving
	if (dir) {
		// Move selection
		if (!ctrl && !shift && !alt) {
			// Move to extent first
			int curLine = editBox->linen;
			if (extendRow != -1) {
				curLine = extendRow;
				extendRow = -1;
			}

			int next = MID(0,curLine+dir*step,GetRows()-1);
			editBox->SetToLine(next);
			SelectRow(next);
			MakeCellVisible(next,0,false);
			return;
		}

		// Move active only
		if (alt && !shift && !ctrl) {
			extendRow = -1;
			int next = MID(0,editBox->linen+dir*step,GetRows()-1);
			editBox->SetToLine(next);
			Refresh(false);
			MakeCellVisible(next,0,false);
			return;
		}

		// Shift-selection
		if (shift && !ctrl && !alt) {
			// Find end
			if (extendRow == -1) extendRow = editBox->linen;
			extendRow = MID(0,extendRow+dir*step,GetRows()-1);

			// Set range
			int i1 = editBox->linen;
			int i2 = extendRow;
			if (i2 < i1) {
				int aux = i1;
				i1 = i2;
				i2 = aux;
			}

			// Select range
			ClearSelection();
			for (int i=i1;i<=i2;i++) {
				SelectRow(i,true);
			}

			MakeCellVisible(extendRow,0,false);
			return;
		}
	}

	// Other events, send to audio display
	if (VideoContext::Get()->audio->loaded) {
		VideoContext::Get()->audio->AddPendingEvent(event);
	}
	else event.Skip();
}


////////////////////////////////
// Sets display by frame or not
void BaseGrid::SetByFrame (bool state) {
	// Check if it's already the same
	if (byFrame == state) return;
	byFrame = state;
	SetColumnWidths();
	Refresh(false);
}


///////////////////////////////////////////////
// Generates an array covering inclusive range
wxArrayInt BaseGrid::GetRangeArray(int n1,int n2) {
	// Swap if in wrong order
	if (n2 < n1) {
		int aux = n1;
		n1 = n2;
		n2 = aux;
	}

	// Generate array
	wxArrayInt target;
	for (int i=n1;i<=n2;i++) {
		target.Add(i);
	}
	return target;
}
