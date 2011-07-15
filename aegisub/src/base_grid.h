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

#ifndef AGI_PRE
#include <list>
#include <map>
#include <vector>

#include <wx/grid.h>
#include <wx/scrolbar.h>
#endif

#include "selection_controller.h"

namespace agi { struct Context; }
class AssEntry;
class AssDialogue;
class SubsEditBox;

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
	/// DOCME
	int lineHeight;

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
	void OnKeyDown(wxKeyEvent &event);

	void DrawImage(wxDC &dc);

	Selection selection;
	AssDialogue *active_line;
	std::vector<AssDialogue*> index_line_map;
	std::map<AssDialogue*,int> line_index_map;

	int batch_level;
	bool batch_active_line_changed;
	Selection batch_selection_added;
	Selection batch_selection_removed;

protected:

	/// DOCME
	int colWidth[16];

	agi::Context *context;

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

	// Re-implement functions from BaseSelectionController to add batching
	void AnnounceActiveLineChanged(AssDialogue *new_line);
	void AnnounceSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed);

public:
	// SelectionController implementation
	virtual void SetActiveLine(AssDialogue *new_line);
	virtual AssDialogue * GetActiveLine() const { return active_line; }
	virtual void SetSelectedSet(const Selection &new_selection);
	virtual void GetSelectedSet(Selection &res) const { res = selection; }
	virtual Selection const& GetSelectedSet() const { return selection; }
	virtual void NextLine();
	virtual void PrevLine();

public:
	/// DOCME
	bool byFrame;

	void AdjustScrollbar();
	void SetColumnWidths();
	void BeginBatch();
	void EndBatch();
	void SetByFrame (bool state);

	void SelectRow(int row, bool addToSelected = false, bool select=true);
	void ClearSelection();
	bool IsInSelection(int row, int col=0) const;
	bool IsDisplayed(const AssDialogue *line) const;
	int GetNumberSelection() const;
	int GetFirstSelRow() const;
	int GetLastSelRow() const;
	void SelectVisible();
	wxArrayInt GetSelection(bool *continuous=NULL) const;

	void ClearMaps();
	/// @brief Update the row <-> AssDialogue mappings
	/// @param preserve_selected_rows Try to keep the same rows selected rather
	///                               rather than the same lines
	void UpdateMaps(bool preserve_selected_rows = false);
	void UpdateStyle();

	int GetRows() const;
	wxArrayInt GetRangeArray(int n1,int n2) const;
	void MakeCellVisible(int row, int col,bool center=true);

	AssDialogue *GetDialogue(int n) const;
	int GetDialogueIndex(AssDialogue *diag) const;

	BaseGrid(wxWindow* parent, agi::Context *context, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
	~BaseGrid();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	GRID_SCROLLBAR = 1730
};


