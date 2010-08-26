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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file base_grid.cpp
/// @brief Base for subtitle grid in main UI
/// @ingroup main_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#include <iterator>

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/sizer.h>
#endif

#include "base_grid.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "audio_display.h"
#include "compat.h"
#include "frame_main.h"
#include "main.h"
#include "subs_edit_box.h"
#include "utils.h"
#include "video_box.h"
#include "video_context.h"
#include "video_slider.h"

template<class S1, class S2, class D>
static inline void set_difference(const S1 &src1, const S2 &src2, D &dst) {
	std::set_difference(
		src1.begin(), src1.end(), src2.begin(), src2.end(),
		std::inserter(dst, dst.begin()));
}


/// @brief Constructor 
/// @param parent 
/// @param id     
/// @param pos    
/// @param size   
/// @param style  
/// @param name   
///
BaseGrid::BaseGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
: wxWindow(parent, id, pos, size, style, name)
, context(VideoContext::Get())
{
	// Misc variables
	lastRow = -1;
	yPos = 0;
	extendRow = -1;
	bmp = NULL;
	holding = false;
	byFrame = false;
	lineHeight = 1; // non-zero to avoid div by 0
	active_line = 0;

	batch_level = 0;
	batch_active_line_changed = false;

	// Set scrollbar
	scrollBar = new wxScrollBar(this,GRID_SCROLLBAR,wxDefaultPosition,wxDefaultSize,wxSB_VERTICAL);
	scrollBar->SetScrollbar(0,10,100,10);

	wxBoxSizer *scrollbarpositioner = new wxBoxSizer(wxHORIZONTAL);
	scrollbarpositioner->AddStretchSpacer();
	scrollbarpositioner->Add(scrollBar, 0, wxEXPAND, 0);

	SetSizerAndFit(scrollbarpositioner);

	// Set style
	UpdateStyle();

	agi::OptionValue::ChangeListener UpdateStyle(std::tr1::bind(&BaseGrid::UpdateStyle, this));
	OPT_GET("Subtitle/Grid/Font Face")->Subscribe(this, UpdateStyle);
	OPT_GET("Subtitle/Grid/Font Size")->Subscribe(this, UpdateStyle);

	agi::OptionValue::ChangeListener Refresh(std::tr1::bind(&BaseGrid::Refresh, this, false, (wxRect*)NULL));
	OPT_GET("Colour/Subtitle Grid/Active Border")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Background/Background")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Background/Comment")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Background/Inframe")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Background/Selected Comment")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Background/Selection")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Collision")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Header")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Left Column")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Lines")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Selection")->Subscribe(this, Refresh);
	OPT_GET("Colour/Subtitle Grid/Standard")->Subscribe(this, Refresh);
	OPT_GET("Subtitle/Grid/Highlight Subtitles in Frame")->Subscribe(this, Refresh);
}



/// @brief Destructor 
///
BaseGrid::~BaseGrid() {
	ClearMaps();
	delete bmp;
}



/// @brief Update style 
///
void BaseGrid::UpdateStyle() {
	// Set font
	wxString fontname = lagi_wxString(OPT_GET("Subtitle/Grid/Font Face")->GetString());
	if (fontname.IsEmpty()) fontname = _T("Tahoma");
	font.SetFaceName(fontname);
	font.SetPointSize(OPT_GET("Subtitle/Grid/Font Size")->GetInt());
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
	std::vector<bool> column_array;
	OPT_GET("Subtitle/Grid/Column")->GetListBool(column_array);
	assert(column_array.size() == columns);
	for (int i=0;i<columns;i++) showCol[i] = column_array[i];
	SetColumnWidths();

	// Update
	AdjustScrollbar();
	Refresh();
}



/// @brief Clears grid 
///
void BaseGrid::ClearMaps() {
	Selection old_selection(selection);

	index_line_map.clear();
	line_index_map.clear();
	selection.clear();
	yPos = 0;
	AdjustScrollbar();

	AnnounceSelectedSetChanged(Selection(), old_selection);
}

/// @brief Update maps 
///
void BaseGrid::UpdateMaps(bool preserve_selected_rows) {
	BeginBatch();
	int active_row = line_index_map[active_line];

	std::vector<int> sel_rows;
	if (preserve_selected_rows) {
		sel_rows.reserve(selection.size());
		std::transform(selection.begin(), selection.end(), std::back_inserter(sel_rows),
			std::bind1st(std::mem_fun(&BaseGrid::GetDialogueIndex), this));
	}

	index_line_map.clear();
	line_index_map.clear();

	for (entryIter cur=AssFile::top->Line.begin();cur != AssFile::top->Line.end();cur++) {
		AssDialogue *curdiag = dynamic_cast<AssDialogue*>(*cur);
		if (curdiag) {
			line_index_map[curdiag] = (int)index_line_map.size();
			index_line_map.push_back(curdiag);
		}
	}

	if (preserve_selected_rows) {
		Selection sel;

		// If the file shrank enough that no selected rows are left, select the
		// last row
		if (sel_rows.empty()) {
			sel_rows.push_back(index_line_map.size() - 1);
		}
		else if (sel_rows[0] >= (int)index_line_map.size()) {
			sel_rows[0] = index_line_map.size() - 1;
		}
		for (size_t i = 0; i < sel_rows.size(); i++) {
			if (sel_rows[i] >= (int)index_line_map.size()) break;
			sel.insert(index_line_map[sel_rows[i]]);
		}

		SetSelectedSet(sel);
	}
	else {
		Selection lines;
		std::copy(index_line_map.begin(), index_line_map.end(), std::inserter(lines, lines.begin()));
		Selection new_sel;
		// Remove lines which no longer exist from the selection
		set_intersection(selection.begin(), selection.end(),
			lines.begin(), lines.end(),
			std::inserter(new_sel, new_sel.begin()));

		SetSelectedSet(new_sel);
	}

	// The active line may have ceased to exist; pick a new one if so
	if (line_index_map.find(active_line) == line_index_map.end()) {
		if (active_row < (int)index_line_map.size()) {
			SetActiveLine(index_line_map[active_row]);
		}
		else if (preserve_selected_rows && !selection.empty()) {
			SetActiveLine(index_line_map[sel_rows[0]]);
		}
		else {
			SetActiveLine(index_line_map.back());
		}
	}

	if (selection.empty() && active_line) {
		Selection sel;
		sel.insert(active_line);
		SetSelectedSet(sel);
	}

	EndBatch();

	Refresh(false);
}




/// @brief Begin batch 
///
void BaseGrid::BeginBatch() {
	//Freeze();

	++batch_level;
}



/// @brief End batch 
///
void BaseGrid::EndBatch() {
	--batch_level;
	assert(batch_level >= 0);
	if (batch_level == 0) {
		if (batch_active_line_changed)
			AnnounceActiveLineChanged(active_line);
		batch_active_line_changed = false;
		if (!batch_selection_added.empty() || !batch_selection_removed.empty())
			AnnounceSelectedSetChanged(batch_selection_added, batch_selection_removed);
		batch_selection_added.clear();
		batch_selection_removed.clear();
	}

	//Thaw();
	AdjustScrollbar();
}



/// @brief Makes cell visible 
/// @param row    
/// @param col    
/// @param center 
///
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



/// @brief Select a row 
/// @param row           
/// @param addToSelected 
/// @param select        
///
void BaseGrid::SelectRow(int row, bool addToSelected, bool select) {
	if (row < 0 || (size_t)row >= index_line_map.size()) return;

	AssDialogue *line = index_line_map[row];

	if (!addToSelected) {
		Selection sel;
		if (select) sel.insert(line);
		SetSelectedSet(sel);
	}

	else if (select && selection.find(line) == selection.end()) {
		selection.insert(line);

		Selection added;
		added.insert(line);

		AnnounceSelectedSetChanged(added, Selection());

		int w = 0;
		int h = 0;
		GetClientSize(&w,&h);
		RefreshRect(wxRect(0,(row+1-yPos)*lineHeight,w,lineHeight),false);
	}

	else if (!select && selection.find(line) != selection.end()) {
		selection.erase(line);

		Selection removed;
		removed.insert(line);

		AnnounceSelectedSetChanged(Selection(), removed);

		int w = 0;
		int h = 0;
		GetClientSize(&w,&h);
		RefreshRect(wxRect(0,(row+1-yPos)*lineHeight,w,lineHeight),false);
	}
}



/// @brief Selects visible lines 
///
void BaseGrid::SelectVisible() {
	Selection new_selection;

	int rows = GetRows();
	bool selectedOne = false;
	for (int i=0;i<rows;i++) {
		AssDialogue *diag = GetDialogue(i);
		if (IsDisplayed(diag)) {
			if (!selectedOne) {
				MakeCellVisible(i,0);
				selectedOne = true;
			}
			new_selection.insert(diag);
		}
	}

	SetSelectedSet(new_selection);
}



/// @brief Unselects all cells 
///
void BaseGrid::ClearSelection() {
	Selection old_selection(selection.begin(), selection.end());

	selection.clear();

	AnnounceSelectedSetChanged(Selection(), old_selection);
}



/// @brief Is cell in selection? 
/// @param row 
/// @param col 
/// @return 
///
bool BaseGrid::IsInSelection(int row, int) const {
	if ((size_t)row >= index_line_map.size() || row < 0) return false;

	return selection.find(index_line_map[row]) != selection.end();
}



/// @brief Number of selected rows 
/// @return 
///
int BaseGrid::GetNumberSelection() const {
	return selection.size();
}



/// @brief Gets first selected row index
/// @return Row index of first selected row, -1 if no selection
///
int BaseGrid::GetFirstSelRow() const {
	Selection::const_iterator it = selection.begin();

	if (it == selection.end()) return -1;

	int index = GetDialogueIndex(*it);

	for (; it != selection.end(); ++it) {
		int other_index = GetDialogueIndex(*it);
		if (other_index < index) index = other_index;
	}

	return index;
}



/// @brief Gets last selected row from first block selection 
/// @return 
///
int BaseGrid::GetLastSelRow() const {
	int frow = GetFirstSelRow();
	while (IsInSelection(frow)) {
		frow++;
	}
	return frow-1;
}



/// @brief Gets all selected rows 
/// @param[out] cont Is the selection contiguous, i.e. free from holes
/// @return Array with indices of selected lines
///
wxArrayInt BaseGrid::GetSelection(bool *cont) const {
	int last = -1;
	bool continuous = true;

	std::set<int> sel_row_indices;

	for (Selection::const_iterator it = selection.begin(); it != selection.end(); ++it) {
		sel_row_indices.insert(GetDialogueIndex(*it));
	}

	// Iterating the int set yields a sorted list
	wxArrayInt res;
	for (std::set<int>::iterator it = sel_row_indices.begin(); it != sel_row_indices.end(); ++it) {
		res.Add(*it);
		if (last != -1 && *it != last+1) continuous = false;
		last = *it;
	}

	if (cont) *cont = continuous;
	return res;
}



/// @brief Get number of rows 
/// @return 
///
int BaseGrid::GetRows() const {
	return index_line_map.size();
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



/// @brief Paint event 
/// @param event 
///
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



/// @brief Draw image 
/// @param dc 
///
void BaseGrid::DrawImage(wxDC &dc) {
	// Get size and pos
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	w -= scrollBar->GetSize().GetWidth();

	// Set font
	dc.SetFont(font);

	// Clear background
	dc.SetBackground(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Background")->GetColour())));
	dc.Clear();

	// Draw labels
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Left Column")->GetColour())));
	dc.DrawRectangle(0,lineHeight,colWidth[0],h-lineHeight);

	// Visible lines
	int drawPerScreen = h/lineHeight + 1;
	int nDraw = MID(0,drawPerScreen,GetRows()-yPos);
	int maxH = (nDraw+1) * lineHeight;

	// Row colors
	std::vector<wxBrush> rowColors;
	std::vector<wxColor> foreColors;
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Background")->GetColour())));					// 0 = Standard
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Standard")->GetColour()));
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Header")->GetColour())));						// 1 = Header
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Standard")->GetColour()));
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selection")->GetColour())));		// 2 = Selected
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Comment")->GetColour())));			// 3 = Commented
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Inframe")->GetColour())));			// 4 = Video Highlighted
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
	rowColors.push_back(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selected Comment")->GetColour())));	// 5 = Commented & selected
	foreColors.push_back(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));

	// First grid row
	bool drawGrid = true;
	if (drawGrid) {
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Lines")->GetColour())));
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
			AssDialogue *sel = GetActiveLine();
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
				strings.Add(wxString::Format(_T("%i"),context->FrameAtTime(curDiag->Start.GetMS(),agi::vfr::START)));
				strings.Add(wxString::Format(_T("%i"),context->FrameAtTime(curDiag->End.GetMS(),agi::vfr::END)));
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
			int mode = OPT_GET("Subtitle/Grid/Hide Overrides")->GetInt();
			wxString value = _T("");

			// Hidden overrides
			if (mode == 1 || mode == 2) {
				wxString replaceWith = lagi_wxString(OPT_GET("Subtitle/Grid/Hide Overrides Char")->GetString());
				int textlen = curDiag->Text.Length();
				int depth = 0;
				wxChar curChar;
				for (int j=0;j<textlen;j++) {
					curChar = curDiag->Text[j];
					if (curChar == _T('{')) depth = 1;
					else if (curChar == _T('}')) {
						depth--;
						if (depth == 0 && mode == 1) value += replaceWith;
						else if (depth < 0) depth = 0;
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
			else if (OPT_GET("Subtitle/Grid/Highlight Subtitles in Frame")->GetBool() && IsDisplayed(curDiag)) curColor = 4;
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
		if (collides) dc.SetTextForeground(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Collision")->GetColour()));
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
			dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Lines")->GetColour())));
			dc.DrawLine(0,dy+lineHeight,w,dy+lineHeight);
			dc.SetPen(*wxTRANSPARENT_PEN);
		}
	}

	// Draw grid columns
	dx = 0;
	if (drawGrid) {
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Lines")->GetColour())));
		for (int i=0;i<10;i++) {
			dx += colWidth[i];
			dc.DrawLine(dx,0,dx,maxH);
		}
		dc.DrawLine(0,0,0,maxH);
		dc.DrawLine(w-1,0,w-1,maxH);
	}

	// Draw currently active line border
	if (GetActiveLine()) {
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Active Border")->GetColour())));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dy = (line_index_map[GetActiveLine()]+1-yPos) * lineHeight;
		dc.DrawRectangle(0,dy,w,lineHeight+1);
	}
}



/// @brief On size 
/// @param event 
///
void BaseGrid::OnSize(wxSizeEvent &event) {
	AdjustScrollbar();
	SetColumnWidths();
	Refresh(false);
}



/// @brief On scroll 
/// @param event 
///
void BaseGrid::OnScroll(wxScrollEvent &event) {
	int newPos = event.GetPosition();
	if (yPos != newPos) {
		yPos = newPos;
		Refresh(false);
	}
}



/// @brief Mouse events 
/// @param event 
/// @return 
///
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
	AssDialogue *dlg = GetDialogue(row);
	if (!dlg) row = 0;

	// Get focus
	if (event.ButtonDown()) {
		if (OPT_GET("Subtitle/Grid/Focus Allow")->GetBool()) {
			SetFocus();
		}
	}

	// Click type
	bool startedHolding = false;
	if (click && !holding && dlg!=0) {
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
	if ((click || holding || dclick) && dlg!=0) {
		// Disable extending
		extendRow = -1;

		// Toggle selected
		if (click && ctrl && !shift && !alt) {
			bool isSel = IsInSelection(row,0);
			if (isSel && selection.size() == 1) return;
			SelectRow(row,true,!isSel);
			if (dlg == GetActiveLine()) {
				SetActiveLine(GetDialogue(GetFirstSelRow()));
			}
			parentFrame->UpdateToolbar();
			lastRow = row;
			return;
		}

		// Normal click
		if ((click || dclick) && !shift && !ctrl && !alt) {
			SetActiveLine(dlg);
			if (dclick) context->JumpToTime(dlg->Start.GetMS());
			SelectRow(row,false);
			parentFrame->UpdateToolbar();
			lastRow = row;
			return;
		}

		// Keep selection
		if (click && !shift && !ctrl && alt) {
			SetActiveLine(dlg);
			return;
		}

		// Block select
		if ((click && shift && !alt) || (holding && !ctrl && !alt && !shift)) {
			if (lastRow != -1) {
				// Set boundaries
				int i1 = row;
				int i2 = lastRow;
				if (i1 > i2) {
					std::swap(i1, i2);
				}

				// Toggle each
				Selection newsel;
				if (ctrl) newsel = selection;
				for (int i=i1;i<=i2;i++) {
					newsel.insert(GetDialogue(i));
				}
				SetSelectedSet(newsel);

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



/// @brief Scroll to 
/// @param y 
///
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



/// @brief Adjust scrollbar 
///
void BaseGrid::AdjustScrollbar() {
	int w,h,sw,sh;
	GetClientSize(&w,&h);
	int drawPerScreen = h/lineHeight;
	int rows = GetRows();
	bool barToEnable = drawPerScreen < rows+2;

	yPos = MID(0,yPos,rows - drawPerScreen);

	scrollBar->Freeze();
	scrollBar->GetSize(&sw,&sh);
	scrollBar->SetSize(w-sw,0,sw,h);

	if (barToEnable != scrollBar->IsEnabled()) scrollBar->Enable(barToEnable);
	if (barToEnable) {
		scrollBar->SetScrollbar(yPos,drawPerScreen,rows+2,drawPerScreen-2,true);
	}
	scrollBar->Thaw();
}



/// @brief Set column widths 
/// @return 
///
void BaseGrid::SetColumnWidths() {
	if (!IsShownOnScreen()) return;

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
				int tmp = context->FrameAtTime(curDiag->Start.GetMS(),agi::vfr::START);
				if (tmp > maxStart) maxStart = tmp;
				tmp = context->FrameAtTime(curDiag->End.GetMS(),agi::vfr::END);
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
	for (int i=0;i<columns;i++) {
		if (!showCol[i]) colWidth[i] = 0;
	}

	// Set size of last
	int total = 0;
	for (int i=0;i<10;i++) total+= colWidth[i];
	colWidth[10] = w-total;
}



/// @brief Get dialogue by index
/// @param n Index to look up
/// @return Subtitle dialogue line for index, or 0 if invalid index
///
AssDialogue *BaseGrid::GetDialogue(int n) const {
	if (n < 0 || n >= (int)index_line_map.size()) return 0;
	return index_line_map[n];
}



/// @brief Get index by dialogue line
/// @param diag Dialogue line to look up
/// @return Subtitle index for object, or -1 if unknown subtitle
int BaseGrid::GetDialogueIndex(AssDialogue *diag) const {
	std::map<AssDialogue*,int>::const_iterator it = line_index_map.find(diag);
	if (it != line_index_map.end()) return it->second;
	else return -1;
}



/// @brief Check if line is being displayed 
/// @param line 
/// @return 
///
bool BaseGrid::IsDisplayed(AssDialogue *line) {
	VideoContext* con = VideoContext::Get();
	if (!con->IsLoaded()) return false;
	int frame = con->GetFrameN();
	return
		con->FrameAtTime(line->Start.GetMS(),agi::vfr::START) <= frame &&
		con->FrameAtTime(line->End.GetMS(),agi::vfr::END) >= frame;
}

/// @brief Key press 
/// @param event 
/// @return 
///
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
		if (context->IsLoaded()) {
			parentFrame->videoBox->videoSlider->SetFocus();
			parentFrame->videoBox->videoSlider->GetEventHandler()->ProcessEvent(event);
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
			int curLine = GetDialogueIndex(GetActiveLine());
			if (extendRow != -1) {
				curLine = extendRow;
				extendRow = -1;
			}

			int next = MID(0,curLine+dir*step,GetRows()-1);
			SetActiveLine(GetDialogue(next));
			SelectRow(next);
			MakeCellVisible(next,0,false);
			return;
		}

		// Move active only
		if (alt && !shift && !ctrl) {
			extendRow = -1;
			int next = MID(0,GetDialogueIndex(GetActiveLine())+dir*step,GetRows()-1);
			SetActiveLine(GetDialogue(next));
			Refresh(false);
			MakeCellVisible(next,0,false);
			return;
		}

		// Shift-selection
		if (shift && !ctrl && !alt) {
			// Find end
			if (extendRow == -1) GetDialogueIndex(GetActiveLine());
			extendRow = MID(0,extendRow+dir*step,GetRows()-1);

			// Set range
			int i1 = GetDialogueIndex(GetActiveLine());
			int i2 = extendRow;
			if (i2 < i1) {
				std::swap(i1, i2);
			}

			// Select range
			Selection newsel;
			for (int i=i1;i<=i2;i++) {
				newsel.insert(GetDialogue(i));
			}
			SetSelectedSet(newsel);

			MakeCellVisible(extendRow,0,false);
			return;
		}
	}

	// Other events, send to audio display
	if (context->audio->loaded) {
		context->audio->GetEventHandler()->ProcessEvent(event);
	}
	else event.Skip();
}



/// @brief Sets display by frame or not 
/// @param state 
/// @return 
///
void BaseGrid::SetByFrame (bool state) {
	// Check if it's already the same
	if (byFrame == state) return;
	byFrame = state;
	SetColumnWidths();
	Refresh(false);
}



/// @brief Generates an array covering inclusive range 
/// @param n1 
/// @param n2 
///
wxArrayInt BaseGrid::GetRangeArray(int n1,int n2) const {
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



// SelectionController


void BaseGrid::SetSelectedSet(const Selection &new_selection) {
	Selection inserted;
	Selection removed;
	set_difference(new_selection, selection, inserted);
	set_difference(selection, new_selection, removed);
	selection = new_selection;
	AnnounceSelectedSetChanged(inserted, removed);
	Refresh(false);
}


void BaseGrid::SetActiveLine(AssDialogue *new_line) {
	if (new_line != active_line) {
		assert(new_line == 0 || line_index_map.find(new_line) != line_index_map.end());
		active_line = new_line;
		AnnounceActiveLineChanged(active_line);
		Refresh(false);
	}
}


void BaseGrid::PrevLine() {
	int cur_line_i = GetDialogueIndex(GetActiveLine());
	AssDialogue *prev_line = GetDialogue(cur_line_i-1);
	if (prev_line) {
		SetActiveLine(prev_line);
		Selection newsel;
		newsel.insert(prev_line);
		SetSelectedSet(newsel);
		MakeCellVisible(cur_line_i-1, 0, false);
	}
}

void BaseGrid::NextLine() {
	int cur_line_i = GetDialogueIndex(GetActiveLine());
	AssDialogue *next_line = GetDialogue(cur_line_i+1);
	if (next_line) {
		SetActiveLine(next_line);
		Selection newsel;
		newsel.insert(next_line);
		SetSelectedSet(newsel);
		MakeCellVisible(cur_line_i+1, 0, false);
	}
}


void BaseGrid::AnnounceActiveLineChanged(AssDialogue *new_line) {
	if (batch_level > 0)
		batch_active_line_changed = true;
	else
		BaseSelectionController<AssDialogue>::AnnounceActiveLineChanged(new_line);
}

void BaseGrid::AnnounceSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed) {
	if (batch_level > 0) {
		// Remove all previously added lines that are now removed
		Selection temp;
		set_difference(batch_selection_added, lines_removed, temp);
		std::swap(temp, batch_selection_added);
		temp.clear();

		// Remove all previously removed lines that are now added
		set_difference(batch_selection_removed, lines_added, temp);
		std::swap(temp, batch_selection_removed);

		// Add new stuff to batch sets
		batch_selection_added.insert(lines_added.begin(), lines_added.end());
		batch_selection_removed.insert(lines_removed.begin(), lines_removed.end());
	}
	else {
		BaseSelectionController<AssDialogue>::AnnounceSelectedSetChanged(lines_added, lines_removed);
	}
}


