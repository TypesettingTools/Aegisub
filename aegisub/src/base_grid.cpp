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
#include <cmath>
#include <iterator>
#include <numeric>

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/kbdstate.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#endif

#include "base_grid.h"

#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "include/aegisub/menu.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "frame_main.h"
#include "main.h"
#include "utils.h"
#include "video_context.h"
#include "video_slider.h"

enum {
	GRID_SCROLLBAR = 1730,
	MENU_SHOW_COL = 1250 // Needs 15 IDs after this
};

enum RowColor {
	COLOR_DEFAULT = 0,
	COLOR_HEADER,
	COLOR_SELECTION,
	COLOR_COMMENT,
	COLOR_VISIBLE,
	COLOR_SELECTED_COMMENT
};

template<class S1, class S2, class D>
static inline void set_difference(const S1 &src1, const S2 &src2, D &dst) {
	std::set_difference(
		src1.begin(), src1.end(), src2.begin(), src2.end(),
		std::inserter(dst, dst.begin()));
}

BaseGrid::BaseGrid(wxWindow* parent, agi::Context *context, const wxSize& size, long style, const wxString& name)
: wxWindow(parent, -1, wxDefaultPosition, size, style, name)
, lineHeight(1) // non-zero to avoid div by 0
, holding(false)
, scrollBar(new wxScrollBar(this, GRID_SCROLLBAR, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL))
, bmp(0)
, byFrame(false)
, extendRow(-1)
, active_line(0)
, batch_level(0)
, batch_active_line_changed(false)
, seek_listener(context->videoController->AddSeekListener(std::tr1::bind(&BaseGrid::Refresh, this, false, (wxRect*)NULL)))
, context_menu(0)
, yPos(0)
, context(context)
{
	scrollBar->SetScrollbar(0,10,100,10);

	wxBoxSizer *scrollbarpositioner = new wxBoxSizer(wxHORIZONTAL);
	scrollbarpositioner->AddStretchSpacer();
	scrollbarpositioner->Add(scrollBar, 0, wxEXPAND, 0);

	SetSizerAndFit(scrollbarpositioner);

	UpdateStyle();
	OnHighlightVisibleChange(*OPT_GET("Subtitle/Grid/Highlight Subtitles in Frame"));

	OPT_SUB("Subtitle/Grid/Font Face", &BaseGrid::UpdateStyle, this);
	OPT_SUB("Subtitle/Grid/Font Size", &BaseGrid::UpdateStyle, this);
	OPT_SUB("Subtitle/Grid/Highlight Subtitles in Frame", &BaseGrid::OnHighlightVisibleChange, this);
	context->ass->AddCommitListener(&BaseGrid::OnSubtitlesCommit, this);
	context->ass->AddFileOpenListener(&BaseGrid::OnSubtitlesOpen, this);
	context->ass->AddFileSaveListener(&BaseGrid::OnSubtitlesSave, this);

	std::tr1::function<void (agi::OptionValue const&)> Refresh(std::tr1::bind(&BaseGrid::Refresh, this, false, (wxRect*)NULL));
	OPT_SUB("Colour/Subtitle Grid/Active Border", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Background/Background", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Background/Comment", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Background/Inframe", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Background/Selected Comment", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Background/Selection", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Collision", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Header", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Left Column", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Lines", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Selection", Refresh);
	OPT_SUB("Colour/Subtitle Grid/Standard", Refresh);
	OPT_SUB("Subtitle/Grid/Hide Overrides", Refresh);

	Bind(wxEVT_CONTEXT_MENU, &BaseGrid::OnContextMenu, this);
}

BaseGrid::~BaseGrid() {
	ClearMaps();
	delete bmp;
	delete context_menu;
}

BEGIN_EVENT_TABLE(BaseGrid,wxWindow)
	EVT_PAINT(BaseGrid::OnPaint)
	EVT_SIZE(BaseGrid::OnSize)
	EVT_COMMAND_SCROLL(GRID_SCROLLBAR,BaseGrid::OnScroll)
	EVT_MOUSE_EVENTS(BaseGrid::OnMouseEvent)
	EVT_KEY_DOWN(BaseGrid::OnKeyDown)
	EVT_MENU_RANGE(MENU_SHOW_COL,MENU_SHOW_COL+15,BaseGrid::OnShowColMenu)
END_EVENT_TABLE()

void BaseGrid::OnSubtitlesCommit(int type) {
	if (type == AssFile::COMMIT_NEW)
		UpdateMaps(true);
	else if (type & AssFile::COMMIT_ORDER || type & AssFile::COMMIT_DIAG_ADDREM)
		UpdateMaps(false);

	if (type & AssFile::COMMIT_DIAG_META) {
		SetColumnWidths();
		Refresh(false);
		return;
	}
	if (type & AssFile::COMMIT_DIAG_TIME)
		Refresh(false);
		//RefreshRect(wxRect(time_cols_x, 0, time_cols_w, GetClientSize().GetHeight()), false);
	else if (type & AssFile::COMMIT_DIAG_TEXT)
		RefreshRect(wxRect(text_col_x, 0, text_col_w, GetClientSize().GetHeight()), false);
}

void BaseGrid::OnSubtitlesOpen() {
	BeginBatch();
	ClearMaps();
	UpdateMaps();

	if (GetRows()) {
		int row = context->ass->GetScriptInfoAsInt("Active Line");
		if (row < 0 || row >= GetRows())
			row = 0;

		SetActiveLine(GetDialogue(row));
		SelectRow(row);
	}

	ScrollTo(context->ass->GetScriptInfoAsInt("Scroll Position"));

	EndBatch();
	SetColumnWidths();
}

void BaseGrid::OnSubtitlesSave() {
	context->ass->SetScriptInfo("Scroll Position", wxString::Format("%d", yPos));
	context->ass->SetScriptInfo("Active Line", wxString::Format("%d", GetDialogueIndex(active_line)));
}

void BaseGrid::OnShowColMenu(wxCommandEvent &event) {
	int item = event.GetId() - MENU_SHOW_COL;
	showCol[item] = !showCol[item];

	std::vector<bool> map(showCol, showCol + columns);
	OPT_SET("Subtitle/Grid/Column")->SetListBool(map);

	SetColumnWidths();
	Refresh(false);
}

void BaseGrid::OnHighlightVisibleChange(agi::OptionValue const& opt) {
	if (opt.GetBool()) {
		seek_listener.Unblock();
	}
	else {
		seek_listener.Block();
	}
}

void BaseGrid::UpdateStyle() {
	wxString fontname = lagi_wxString(OPT_GET("Subtitle/Grid/Font Face")->GetString());
	if (fontname.empty()) fontname = "Tahoma";
	font.SetFaceName(fontname);
	font.SetPointSize(OPT_GET("Subtitle/Grid/Font Size")->GetInt());
	font.SetWeight(wxFONTWEIGHT_NORMAL);

	// Set line height
	{
		wxClientDC dc(this);
		dc.SetFont(font);
		int fw,fh;
		dc.GetTextExtent("#TWFfgGhH", &fw, &fh, NULL, NULL, &font);
		lineHeight = fh + 4;
	}

	// Set column widths
	std::vector<bool> column_array(OPT_GET("Subtitle/Grid/Column")->GetListBool());
	assert(column_array.size() == (size_t)columns);
	for (int i = 0; i < columns; ++i) showCol[i] = column_array[i];
	SetColumnWidths();

	// Update
	AdjustScrollbar();
	Refresh();
}

void BaseGrid::ClearMaps() {
	Selection old_selection(selection);

	index_line_map.clear();
	line_index_map.clear();
	selection.clear();
	yPos = 0;
	AdjustScrollbar();

	AnnounceSelectedSetChanged(Selection(), old_selection);
}

void BaseGrid::UpdateMaps(bool preserve_selected_rows) {
	BeginBatch();
	int active_row = line_index_map[active_line];

	std::vector<int> sel_rows;
	if (preserve_selected_rows) {
		sel_rows.reserve(selection.size());
		transform(selection.begin(), selection.end(), back_inserter(sel_rows),
			bind1st(std::mem_fun(&BaseGrid::GetDialogueIndex), this));
	}

	index_line_map.clear();
	line_index_map.clear();

	for (entryIter cur = context->ass->Line.begin(); cur != context->ass->Line.end(); ++cur) {
		if (AssDialogue *curdiag = dynamic_cast<AssDialogue*>(*cur)) {
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
		copy(index_line_map.begin(), index_line_map.end(), inserter(lines, lines.begin()));
		Selection new_sel;
		// Remove lines which no longer exist from the selection
		set_intersection(selection.begin(), selection.end(),
			lines.begin(), lines.end(),
			inserter(new_sel, new_sel.begin()));

		SetSelectedSet(new_sel);
	}

	// Force a reannounce of the active line if it hasn't changed, as it isn't
	// safe to touch the active line while processing a commit event which would
	// cause this function to be called
	AssDialogue *line = active_line;
	active_line = 0;

	// The active line may have ceased to exist; pick a new one if so
	if (line_index_map.size() && line_index_map.find(line) == line_index_map.end()) {
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
	else {
		SetActiveLine(line);
	}

	if (selection.empty() && active_line) {
		Selection sel;
		sel.insert(active_line);
		SetSelectedSet(sel);
	}

	EndBatch();

	SetColumnWidths();

	Refresh(false);
}

void BaseGrid::BeginBatch() {
	++batch_level;
}

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

	AdjustScrollbar();
}

void BaseGrid::MakeCellVisible(int row, int col, bool center) {
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);

	// Get min and max visible
	int minVis = yPos+1;
	int maxVis = yPos+h/lineHeight-3;

	// Make visible
	if (!center || row < minVis || row > maxVis) {
		if (center) {
			ScrollTo(row - h/lineHeight/2 + 1);
		}
		else {
			if (row < minVis) ScrollTo(row - 1);
			if (row > maxVis) ScrollTo(row - h/lineHeight + 3);
		}
	}
}

void BaseGrid::SelectRow(int row, bool addToSelected, bool select) {
	if (row < 0 || (size_t)row >= index_line_map.size()) return;

	AssDialogue *line = index_line_map[row];

	if (!addToSelected) {
		Selection sel;
		if (select) sel.insert(line);
		SetSelectedSet(sel);
		return;
	}

	if (select && selection.find(line) == selection.end()) {
		selection.insert(line);

		Selection added;
		added.insert(line);

		AnnounceSelectedSetChanged(added, Selection());
	}
	else if (!select && selection.find(line) != selection.end()) {
		selection.erase(line);

		Selection removed;
		removed.insert(line);

		AnnounceSelectedSetChanged(Selection(), removed);
	}

	int w, h;
	GetClientSize(&w, &h);
	RefreshRect(wxRect(0, (row + 1 - yPos) * lineHeight, w, lineHeight), false);
}

int BaseGrid::GetFirstSelRow() const {
	if (selection.empty()) return -1;

	Selection::const_iterator it = selection.begin();

	int index = GetDialogueIndex(*it);
	for (; it != selection.end(); ++it) {
		int other_index = GetDialogueIndex(*it);
		if (other_index < index) index = other_index;
	}

	return index;
}

wxArrayInt BaseGrid::GetSelection() const {
	wxArrayInt res;
	res.reserve(selection.size());
	transform(selection.begin(), selection.end(), std::back_inserter(res),
		bind(&BaseGrid::GetDialogueIndex, this, std::tr1::placeholders::_1));
	std::sort(res.begin(), res.end());
	return res;
}


void BaseGrid::OnPaint(wxPaintEvent &) {
	// Get size and pos
	wxSize cs = GetClientSize();
	cs.SetWidth(cs.GetWidth() - scrollBar->GetSize().GetWidth());

	// Prepare bitmap
	if (bmp && (bmp->GetWidth() < cs.GetWidth() || bmp->GetHeight() < cs.GetHeight())) {
		delete bmp;
		bmp = 0;
	}
	if (!bmp) bmp = new wxBitmap(cs);

	// Find which columns need to be repainted
	bool paint_columns[11];
	memset(paint_columns, 0, sizeof paint_columns);
	for (wxRegionIterator region(GetUpdateRegion()); region; ++region)
	{
		wxRect updrect = region.GetRect();
		int x = 0;
		for (size_t i = 0; i < 11; ++i) {
			if (updrect.x < x + colWidth[i] && updrect.x + updrect.width > x)
				paint_columns[i] = true;
			x += colWidth[i];
		}
	}

	// Draw bitmap
	wxMemoryDC bmpDC;
	bmpDC.SelectObject(*bmp);
	DrawImage(bmpDC, paint_columns);

	wxPaintDC dc(this);
	dc.Blit(wxPoint(0, 0), cs, &bmpDC, wxPoint(0, 0));
}

void BaseGrid::DrawImage(wxDC &dc, bool paint_columns[]) {
	int w = 0;
	int h = 0;
	GetClientSize(&w,&h);
	w -= scrollBar->GetSize().GetWidth();

	dc.SetFont(font);

	dc.SetBackground(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Background")->GetColour())));
	dc.Clear();

	// Draw labels
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Left Column")->GetColour())));
	dc.DrawRectangle(0,lineHeight,colWidth[0],h-lineHeight);

	// Visible lines
	int drawPerScreen = h/lineHeight + 1;
	int nDraw = mid(0,drawPerScreen,GetRows()-yPos);
	int maxH = (nDraw+1) * lineHeight;

	// Row colors
	wxColour text_standard(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Standard")->GetColour()));
	wxColour text_selection(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Selection")->GetColour()));
	wxColour text_collision(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Collision")->GetColour()));

	wxBrush rowColors[] = {
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Background")->GetColour())),
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Header")->GetColour())),
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selection")->GetColour())),
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Comment")->GetColour())),
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Inframe")->GetColour())),
		wxBrush(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Background/Selected Comment")->GetColour())),
	};

	// First grid row
	wxPen grid_pen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Lines")->GetColour()));
	dc.SetPen(grid_pen);
	dc.DrawLine(0, 0, w, 0);
	dc.SetPen(*wxTRANSPARENT_PEN);

	wxString strings[] = {
		_("#"), _("L"), _("Start"), _("End"), _("Style"), _("Actor"),
		_("Effect"), _("Left"), _("Right"), _("Vert"), _("Text")
	};

	for (int i = 0; i < nDraw + 1; i++) {
		int curRow = i + yPos - 1;
		RowColor curColor = COLOR_DEFAULT;

		// Header
		if (i == 0) {
			curColor = COLOR_HEADER;
			dc.SetTextForeground(text_standard);
		}
		// Lines
		else if (AssDialogue *curDiag = GetDialogue(curRow)) {
			GetRowStrings(curRow, curDiag, paint_columns, strings);

			bool inSel = !!selection.count(curDiag);
			if (inSel && curDiag->Comment)
				curColor = COLOR_SELECTED_COMMENT;
			else if (inSel)
				curColor = COLOR_SELECTION;
			else if (curDiag->Comment)
				curColor = COLOR_COMMENT;
			else if (OPT_GET("Subtitle/Grid/Highlight Subtitles in Frame")->GetBool() && IsDisplayed(curDiag))
				curColor = COLOR_VISIBLE;
			else
				curColor = COLOR_DEFAULT;

			if (active_line != curDiag && curDiag->CollidesWith(active_line))
				dc.SetTextForeground(text_collision);
			else if (inSel)
				dc.SetTextForeground(text_selection);
			else
				dc.SetTextForeground(text_standard);
		}
		else {
			assert(false);
		}

		// Draw row background color
		if (curColor) {
			dc.SetBrush(rowColors[curColor]);
			dc.DrawRectangle((curColor == 1) ? 0 : colWidth[0],i*lineHeight+1,w,lineHeight);
		}

		// Draw text
		int dx = 0;
		int dy = i*lineHeight;
		for (int j = 0; j < 11; j++) {
			if (colWidth[j] == 0) continue;

			if (paint_columns[j]) {
				wxSize ext = dc.GetTextExtent(strings[j]);

				int left = dx + 4;
				int top = dy + (lineHeight - ext.GetHeight()) / 2;

				// Centered columns
				if (!(j == 4 || j == 5 || j == 6 || j == 10)) {
					left += (colWidth[j] - 6 - ext.GetWidth()) / 2;
				}

				dc.DrawText(strings[j], left, top);
			}
			dx += colWidth[j];
		}

		// Draw grid
		dc.DestroyClippingRegion();
		dc.SetPen(grid_pen);
		dc.DrawLine(0,dy+lineHeight,w,dy+lineHeight);
		dc.SetPen(*wxTRANSPARENT_PEN);
	}

	// Draw grid columns
	int dx = 0;
	dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Lines")->GetColour())));
	for (int i=0;i<10;i++) {
		dx += colWidth[i];
		dc.DrawLine(dx,0,dx,maxH);
	}
	dc.DrawLine(0,0,0,maxH);
	dc.DrawLine(w-1,0,w-1,maxH);

	// Draw currently active line border
	if (GetActiveLine()) {
		dc.SetPen(wxPen(lagi_wxColour(OPT_GET("Colour/Subtitle Grid/Active Border")->GetColour())));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		int dy = (line_index_map[GetActiveLine()]+1-yPos) * lineHeight;
		dc.DrawRectangle(0,dy,w,lineHeight+1);
	}
}

void BaseGrid::GetRowStrings(int row, AssDialogue *line, bool *paint_columns, wxString *strings) const {
	if (paint_columns[0]) strings[0] = wxString::Format("%d", row + 1);
	if (paint_columns[1]) strings[1] = wxString::Format("%d", line->Layer);
	if (byFrame) {
		if (paint_columns[2]) strings[2] = wxString::Format("%d", context->videoController->FrameAtTime(line->Start, agi::vfr::START));
		if (paint_columns[3]) strings[3] = wxString::Format("%d", context->videoController->FrameAtTime(line->End, agi::vfr::END));
	}
	else {
		if (paint_columns[2]) strings[2] = line->Start.GetASSFormated();
		if (paint_columns[3]) strings[3] = line->End.GetASSFormated();
	}
	if (paint_columns[4]) strings[4] = line->Style;
	if (paint_columns[5]) strings[5] = line->Actor;
	if (paint_columns[6]) strings[6] = line->Effect;
	if (paint_columns[7]) strings[7] = line->GetMarginString(0);
	if (paint_columns[8]) strings[8] = line->GetMarginString(1);
	if (paint_columns[9]) strings[9] = line->GetMarginString(2);

	if (paint_columns[10]) {
		strings[10].clear();
		int mode = OPT_GET("Subtitle/Grid/Hide Overrides")->GetInt();

		// Hidden overrides
		if (mode == 1 || mode == 2) {
			wxString replaceWith;
			if (mode == 1)
				replaceWith = lagi_wxString(OPT_GET("Subtitle/Grid/Hide Overrides Char")->GetString());

			strings[10].reserve(line->Text.size());
			size_t start = 0, pos;
			while ((pos = line->Text.find('{', start)) != wxString::npos) {
				strings[10] += line->Text.Mid(start, pos - start);
				strings[10] += replaceWith;
				start = line->Text.find('}', pos);
				if (start != wxString::npos) ++start;
			}
			strings[10] += line->Text.Mid(start);
		}

		// Show overrides
		else
			strings[10] = line->Text;

		// Cap length and set text
		if (strings[10].size() > 512)
			strings[10] = strings[10].Left(512) + "...";
	}
}

void BaseGrid::OnSize(wxSizeEvent &) {
	AdjustScrollbar();
	SetColumnWidths();
	Refresh(false);
}

void BaseGrid::OnScroll(wxScrollEvent &event) {
	int newPos = event.GetPosition();
	if (yPos != newPos) {
		yPos = newPos;
		Refresh(false);
	}
}

void BaseGrid::OnMouseEvent(wxMouseEvent &event) {
	int w,h;
	GetClientSize(&w,&h);

	// Modifiers
	bool shift = event.ShiftDown();
	bool alt = event.AltDown();
	bool ctrl = event.CmdDown();

	// Row that mouse is over
	bool click = event.LeftDown();
	bool dclick = event.LeftDClick();
	int row = event.GetY()/lineHeight + yPos - 1;
	if (holding && !click) {
		row = mid(0,row,GetRows()-1);
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
	if (click && !holding && dlg) {
		holding = true;
		startedHolding = true;
		CaptureMouse();
	}
	if (!event.LeftIsDown() && holding) {
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
	if ((click || holding || dclick) && dlg) {
		int old_extend = extendRow;
		SetActiveLine(dlg);

		// Toggle selected
		if (click && ctrl && !shift && !alt) {
			bool isSel = !!selection.count(dlg);
			if (isSel && selection.size() == 1) return;
			SelectRow(row, true, !isSel);
			return;
		}

		// Normal click
		if ((click || dclick) && !shift && !ctrl && !alt) {
			if (dclick) context->videoController->JumpToTime(dlg->Start);
			SelectRow(row, false);
			return;
		}

		// Change active line only
		if (click && !shift && !ctrl && alt) {
			return;
		}

		// Block select
		if ((click && shift && !alt) || (holding && !ctrl && !alt && !shift)) {
			extendRow = old_extend;
			int i1 = row;
			int i2 = extendRow;

			if (i1 > i2)
				std::swap(i1, i2);

			// Toggle each
			Selection newsel;
			if (ctrl) newsel = selection;
			for (int i = i1; i <= i2; i++) {
				newsel.insert(GetDialogue(i));
			}
			SetSelectedSet(newsel);
			return;
		}

		return;
	}

	// Mouse wheel
	if (event.GetWheelRotation() != 0) {
		if (ForwardMouseWheelEvent(this, event)) {
			int step = shift ? h / lineHeight - 2 : 3;
			ScrollTo(yPos - step * event.GetWheelRotation() / event.GetWheelDelta());
		}
		return;
	}

	event.Skip();
}

void BaseGrid::OnContextMenu(wxContextMenuEvent &evt) {
	wxPoint pos = evt.GetPosition();
	if (pos == wxDefaultPosition || ScreenToClient(pos).y > lineHeight) {
		if (!context_menu) context_menu = menu::GetMenu("grid_context", context);
		menu::OpenPopupMenu(context_menu, this);
	}
	else {
		const wxString strings[] = {
			_("Line Number"),
			_("Layer"),
			_("Start"),
			_("End"),
			_("Style"),
			_("Actor"),
			_("Effect"),
			_("Left"),
			_("Right"),
			_("Vert"),
		};

		wxMenu menu;
		for (int i = 0; i < columns; ++i)
			menu.Append(MENU_SHOW_COL + i, strings[i], "", wxITEM_CHECK)->Check(showCol[i]);
		PopupMenu(&menu);
	}
}

void BaseGrid::ScrollTo(int y) {
	int w,h;
	GetClientSize(&w,&h);
	int nextY = mid(0,y,GetRows()+2 - h/lineHeight);
	if (yPos != nextY) {
		yPos = nextY;
		if (scrollBar->IsEnabled()) scrollBar->SetThumbPosition(yPos);
		Refresh(false);
	}
}

void BaseGrid::AdjustScrollbar() {
	int w,h,sw,sh;
	GetClientSize(&w,&h);
	int drawPerScreen = h/lineHeight;
	int rows = GetRows();
	bool barToEnable = drawPerScreen < rows+2;

	yPos = mid(0,yPos,rows - drawPerScreen);

	scrollBar->Freeze();
	scrollBar->GetSize(&sw,&sh);
	scrollBar->SetSize(w-sw,0,sw,h);

	if (barToEnable != scrollBar->IsEnabled()) scrollBar->Enable(barToEnable);
	if (barToEnable) {
		scrollBar->SetScrollbar(yPos,drawPerScreen,rows+2,drawPerScreen-2,true);
	}
	scrollBar->Thaw();
}

void BaseGrid::SetColumnWidths() {
	if (!IsShownOnScreen()) return;

	// Width/height
	int w, h;
	GetClientSize(&w,&h);

	// DC for text extents test
	wxClientDC dc(this);
	dc.SetFont(font);

	// O(1) widths
	int marginLen = dc.GetTextExtent("0000").GetWidth();

	int labelLen = dc.GetTextExtent(wxString::Format("%d", GetRows())).GetWidth();
	int startLen = 0;
	int endLen = 0;
	if (!byFrame)
		startLen = endLen = dc.GetTextExtent(AssTime().GetASSFormated()).GetWidth();

	// O(n) widths
	bool showMargin[3] = { false, false, false };
	int styleLen = 0;
	int actorLen = 0;
	int effectLen = 0;
	int maxLayer = 0;
	int maxStart = 0;
	int maxEnd = 0;
	for (int i = 0; i < GetRows(); i++) {
		AssDialogue *curDiag = GetDialogue(i);

		maxLayer = std::max(maxLayer, curDiag->Layer);
		actorLen = std::max(actorLen, dc.GetTextExtent(curDiag->Actor).GetWidth());
		styleLen = std::max(styleLen, dc.GetTextExtent(curDiag->Style).GetWidth());
		effectLen = std::max(effectLen, dc.GetTextExtent(curDiag->Effect).GetWidth());

		// Margins
		for (int j = 0; j < 3; j++) {
			if (curDiag->Margin[j])
				showMargin[j] = true;
		}

		// Times
		if (byFrame) {
			maxStart = std::max(maxStart, context->videoController->FrameAtTime(curDiag->Start, agi::vfr::START));
			maxEnd = std::max(maxEnd, context->videoController->FrameAtTime(curDiag->End, agi::vfr::END));
		}
	}

	// Finish layer
	int layerLen = maxLayer ? dc.GetTextExtent(wxString::Format("%d", maxLayer)).GetWidth() : 0;

	// Finish times
	if (byFrame) {
		startLen = dc.GetTextExtent(wxString::Format("%d", maxStart)).GetWidth();
		endLen = dc.GetTextExtent(wxString::Format("%d", maxEnd)).GetWidth();
	}

	// Set column widths
	colWidth[0] = labelLen;
	colWidth[1] = layerLen;
	colWidth[2] = startLen;
	colWidth[3] = endLen;
	colWidth[4] = styleLen;
	colWidth[5] = actorLen;
	colWidth[6] = effectLen;
	for (int i = 0; i < 3; i++)
		colWidth[i + 7] = showMargin[i] ? marginLen : 0;
	colWidth[10] = 1;

	// Hide columns
	for (int i = 0; i < columns; i++) {
		if (!showCol[i])
			colWidth[i] = 0;
	}

	wxString col_names[11] = {
		_("#"),
		_("L"),
		_("Start"),
		_("End"),
		_("Style"),
		_("Actor"),
		_("Effect"),
		_("Left"),
		_("Right"),
		_("Vert"),
		_("Text")
	};

	// Ensure every visible column is at least as big as its header
	for (size_t i = 0; i < 11; ++i) {
		if (colWidth[i])
			colWidth[i] = std::max(colWidth[i], dc.GetTextExtent(col_names[i]).GetWidth());
	}

	// Add padding to all non-empty columns
	for (size_t i = 0; i < 10; ++i) {
		if (colWidth[i])
			colWidth[i] += 10;
	}


	// Set size of last
	int total = std::accumulate(colWidth, colWidth + 10, 0);
	colWidth[10] = w - total;

	time_cols_x = colWidth[0] + colWidth[1];
	time_cols_w = colWidth[2] + colWidth[3];
	text_col_x = total;
	text_col_w = colWidth[10];
}

AssDialogue *BaseGrid::GetDialogue(int n) const {
	if (static_cast<size_t>(n) >= index_line_map.size()) return 0;
	return index_line_map[n];
}

int BaseGrid::GetDialogueIndex(AssDialogue *diag) const {
	std::map<AssDialogue*,int>::const_iterator it = line_index_map.find(diag);
	if (it != line_index_map.end()) return it->second;
	return -1;
}

bool BaseGrid::IsDisplayed(const AssDialogue *line) const {
	if (!context->videoController->IsLoaded()) return false;
	int frame = context->videoController->GetFrameN();
	return
		context->videoController->FrameAtTime(line->Start,agi::vfr::START) <= frame &&
		context->videoController->FrameAtTime(line->End,agi::vfr::END) >= frame;
}

void BaseGrid::OnKeyDown(wxKeyEvent &event) {
	if (hotkey::check("Subtitle Grid", context, event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		return;

	int w,h;
	GetClientSize(&w,&h);

	int key = event.GetKeyCode();
	bool ctrl = event.CmdDown();
	bool alt = event.AltDown();
	bool shift = event.ShiftDown();

	int dir = 0;
	int step = 1;
	if (key == WXK_UP) dir = -1;
	else if (key == WXK_DOWN) dir = 1;
	else if (key == WXK_PAGEUP) {
		dir = -1;
		step = h / lineHeight - 2;
	}
	else if (key == WXK_PAGEDOWN) {
		dir = 1;
		step = h / lineHeight - 2;
	}
	else if (key == WXK_HOME) {
		dir = -1;
		step = GetRows();
	}
	else if (key == WXK_END) {
		dir = 1;
		step = GetRows();
	}

	// Moving
	if (dir) {
		int old_extend = extendRow;
		int next = mid(0, GetDialogueIndex(active_line) + dir * step, GetRows() - 1);
		SetActiveLine(GetDialogue(next));

		// Move selection
		if (!ctrl && !shift && !alt) {
			SelectRow(next);
			return;
		}

		// Move active only
		if (alt && !shift && !ctrl) {
			Refresh(false);
			return;
		}

		// Shift-selection
		if (shift && !ctrl && !alt) {
			extendRow = old_extend;
			// Set range
			int begin = next;
			int end = extendRow;
			if (end < begin)
				std::swap(begin, end);

			// Select range
			Selection newsel;
			for (int i = begin; i <= end; i++)
				newsel.insert(GetDialogue(i));

			SetSelectedSet(newsel);

			MakeCellVisible(extendRow, 0, false);
			return;
		}
	}
	else if (!hotkey::check("Audio", context, event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers())) {
		event.Skip();
	}
}

void BaseGrid::SetByFrame(bool state) {
	if (byFrame == state) return;
	byFrame = state;
	SetColumnWidths();
	Refresh(false);
}

void BaseGrid::SetSelectedSet(const Selection &new_selection) {
	Selection inserted, removed;
	set_difference(new_selection, selection, inserted);
	set_difference(selection, new_selection, removed);
	selection = new_selection;
	AnnounceSelectedSetChanged(inserted, removed);
	Refresh(false);
}

void BaseGrid::SetActiveLine(AssDialogue *new_line) {
	if (new_line != active_line) {
		assert(new_line == 0 || line_index_map.count(new_line));
		active_line = new_line;
		AnnounceActiveLineChanged(active_line);
		MakeCellVisible(GetDialogueIndex(active_line), 0, false);
		Refresh(false);
		extendRow = GetDialogueIndex(new_line);
	}
}

void BaseGrid::PrevLine() {
	int cur_line_i = GetDialogueIndex(GetActiveLine());
	if (AssDialogue *prev_line = GetDialogue(cur_line_i-1)) {
		SetActiveLine(prev_line);
		Selection newsel;
		newsel.insert(prev_line);
		SetSelectedSet(newsel);
	}
}

void BaseGrid::NextLine() {
	int cur_line_i = GetDialogueIndex(GetActiveLine());
	if (AssDialogue *next_line = GetDialogue(cur_line_i+1)) {
		SetActiveLine(next_line);
		Selection newsel;
		newsel.insert(next_line);
		SetSelectedSet(newsel);
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
