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

/// @file base_grid.h
/// @see base_grid.cpp
/// @ingroup main_ui
///


#pragma once

////////////
// Includes
#ifndef AGI_PRE
#include <list>
#include <vector>

#include <wx/grid.h>
#include <wx/scrolbar.h>
#endif

#include "selection_controller.h"


//////////////
// Prototypes
class AssEntry;
class AssDialogue;
class SubsEditBox;
class FrameMain;

/// DOCME
typedef std::list<AssEntry*>::iterator entryIter;


typedef SelectionController<AssDialogue> SubtitleSelectionController;
typedef SelectionListener<AssDialogue> SubtitleSelectionListener;


/// DOCME
/// @class BaseGrid
/// @brief DOCME
///
/// DOCME
class BaseGrid : public wxWindow, public BaseSelectionController<AssDialogue> {
private:

	/// DOCME
	int lineHeight;

	/// DOCME
	int colWidth[16];

	/// DOCME
	int lastRow;

	/// DOCME
	int extendRow;

	/// DOCME
	bool holding;

	/// DOCME
	wxFont font;

	/// DOCME
	wxScrollBar *scrollBar;

	/// DOCME
	wxBitmap *bmp;

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);

	void DrawImage(wxDC &dc);

protected:

	/// DOCME
	FrameMain *parentFrame;

	/// DOCME
	static const int columns = 10;
	bool showCol[columns];


	/// @brief DOCME
	/// @param alternate
	///
	virtual void OnPopupMenu(bool alternate=false) {}
	void ScrollTo(int y);

	/// DOCME
	int yPos;

	/// DOCME
	std::vector<int> selMap;

public:
	// SelectionController implementation
	virtual void SetActiveLine(AssDialogue *new_line) { }
	virtual AssDialogue * GetActiveLine() const { return 0; }
	virtual void SetSelectedSet(const Selection &new_selection) { }
	virtual void GetSelectedSet(Selection &selection) const;
	virtual void NextLine() { }
	virtual void PrevLine() { }

public:

	/// DOCME
	SubsEditBox *editBox;


	/// DOCME
	bool byFrame;

	/// DOCME
	std::vector<entryIter> diagMap;

	/// DOCME
	std::vector<AssDialogue *> diagPtrMap;

	void AdjustScrollbar();
	void SetColumnWidths();
	void BeginBatch();
	void EndBatch();
	void SetByFrame (bool state);

	void SelectRow(int row, bool addToSelected = false, bool select=true);
	void ClearSelection();
	bool IsInSelection(int row, int col=0) const;
	static bool IsDisplayed(AssDialogue *line);
	int GetNumberSelection() const;
	int GetFirstSelRow() const;
	int GetLastSelRow() const;
	void SelectVisible();
	wxArrayInt GetSelection(bool *continuous=NULL) const;

	void Clear();
	void UpdateMaps();
	void UpdateStyle();

	int GetRows() const;
	wxArrayInt GetRangeArray(int n1,int n2) const;
	void MakeCellVisible(int row, int col,bool center=true);

	AssDialogue *GetDialogue(int n) const;

	BaseGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
	~BaseGrid();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	GRID_SCROLLBAR = 1730
};


