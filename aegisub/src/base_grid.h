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
	int lineHeight;         ///< Height of a line in pixels in the current font
	int lastRow;            ///< ?
	bool holding;           ///< Is a drag selection in process?
	wxFont font;            ///< Current grid font
	wxScrollBar *scrollBar; ///< The grid's scrollbar
	wxBitmap *bmp;          ///< Back buffer which the grid is rendered into
	bool byFrame;           ///< Should times be displayed as frame numbers
	/// Row from which the selection shrinks/grows from when selecting via the
	/// keyboard. Equal to the active row except for when using shift+up/down.
	int extendRow;

	Selection selection;      ///< Currently selected lines
	AssDialogue *active_line; ///< The currently active line or 0 if none
	std::vector<AssDialogue*> index_line_map;  ///< Row number -> dialogue line
	std::map<AssDialogue*,int> line_index_map; ///< Dialogue line -> row number

	/// Selection batch nesting depth; changes are commited only when this
	/// hits zero
	int batch_level;
	/// Has the active line been changed in the current batch?
	bool batch_active_line_changed;
	/// Lines which will be added to the selection when the current batch is
	/// completed; should be disjoint from selection
	Selection batch_selection_added;
	/// Lines which will be removed from the selection when the current batch
	/// is completed; should be a subset of selection
	Selection batch_selection_removed;

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	void DrawImage(wxDC &dc);
	void ScrollTo(int y);

	virtual void OnPopupMenu(bool alternate = false) { }

protected:
	int colWidth[16];      ///< Width in pixels of each column
	agi::Context *context; ///< Current project context

	static const int columns = 10; ///< Total number of columns
	bool showCol[columns]; ///< Column visibility mask

	int yPos;

	void AdjustScrollbar();
	void SetColumnWidths();

	// Re-implement functions from BaseSelectionController to add batching
	void AnnounceActiveLineChanged(AssDialogue *new_line);
	void AnnounceSelectedSetChanged(const Selection &lines_added, const Selection &lines_removed);

public:
	// SelectionController implementation
	void SetActiveLine(AssDialogue *new_line);
	AssDialogue * GetActiveLine() const { return active_line; }
	void SetSelectedSet(const Selection &new_selection);
	void GetSelectedSet(Selection &res) const { res = selection; }
	Selection const& GetSelectedSet() const { return selection; }
	void NextLine();
	void PrevLine();

	void BeginBatch();
	void EndBatch();
	void SetByFrame(bool state);

	bool IsDisplayed(const AssDialogue *line) const;
	void SelectRow(int row, bool addToSelected = false, bool select=true);
	bool IsInSelection(int row) const;
	int GetFirstSelRow() const;
	int GetLastSelRow() const;
	void SelectVisible();
	wxArrayInt GetSelection() const;

	void ClearMaps();
	/// @brief Update the row <-> AssDialogue mappings
	/// @param preserve_selected_rows Try to keep the same rows selected rather
	///                               rather than the same lines
	void UpdateMaps(bool preserve_selected_rows = false);
	void UpdateStyle();

	int GetRows() const { return index_line_map.size(); }
	void MakeCellVisible(int row, int col,bool center=true);

	AssDialogue *GetDialogue(int n) const;
	int GetDialogueIndex(AssDialogue *diag) const;

	BaseGrid(wxWindow* parent, agi::Context *context, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
	~BaseGrid();

	DECLARE_EVENT_TABLE()
};
