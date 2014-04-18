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

/// @file base_grid.h
/// @see base_grid.cpp
/// @ingroup main_ui
///

#include <libaegisub/signal.h>

#include <array>
#include <memory>
#include <vector>
#include <wx/window.h>

namespace agi {
	struct Context;
	class OptionValue;
}
class AssDialogue;

class BaseGrid final : public wxWindow {
	static const int column_count = 11;

	std::vector<agi::signal::Connection> connections;
	int lineHeight = 1;     ///< Height of a line in pixels in the current font
	bool holding = false;   ///< Is a drag selection in process?
	wxFont font;            ///< Current grid font
	wxScrollBar *scrollBar; ///< The grid's scrollbar
	bool byFrame = false;   ///< Should times be displayed as frame numbers
	wxBrush rowColors[7];   ///< Cached brushes used for row backgrounds

	/// Row from which the selection shrinks/grows from when selecting via the
	/// keyboard, shift-clicking or dragging
	int extendRow = -1;

	std::vector<AssDialogue*> index_line_map;  ///< Row number -> dialogue line

	/// Connection for video seek event. Stored explicitly so that it can be
	/// blocked if the relevant option is disabled
	agi::signal::Connection seek_listener;

	/// Cached grid body context menu
	std::unique_ptr<wxMenu> context_menu;

	void OnContextMenu(wxContextMenuEvent &evt);
	void OnHighlightVisibleChange(agi::OptionValue const& opt);
	void OnKeyDown(wxKeyEvent &event);
	void OnCharHook(wxKeyEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnShowColMenu(wxCommandEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnSubtitlesCommit(int type);
	void OnSubtitlesOpen();
	void OnSubtitlesSave();
	void OnActiveLineChanged(AssDialogue *);

	void DrawImage(wxDC &dc, bool paint_columns[]);
	void GetRowStrings(int row, AssDialogue *line, bool *paint_columns, wxString *strings, bool replace, wxString const& rep_char) const;

	void ScrollTo(int y);

	std::array<int, column_count> colWidth; ///< Width in pixels of each column
	std::array<int, column_count> headerWidth; ///< Width in pixels of each column's header
	std::array<wxString, column_count> headerNames;

	int time_cols_x; ///< Left edge of the times columns
	int time_cols_w; ///< Width of the two times columns
	int text_col_x; ///< Left edge of the text column
	int text_col_w; ///< Width of the text column

	std::array<bool, column_count - 1> showCol; ///< Column visibility mask (Text can't be hidden)

	int yPos = 0;

	void AdjustScrollbar();
	void SetColumnWidths();

	bool IsDisplayed(const AssDialogue *line) const;

	agi::Context *context; ///< Current project context

	void ClearMaps();
	/// @brief Update the row <-> AssDialogue mappings
	void UpdateMaps();
	void UpdateStyle();

	void SelectRow(int row, bool addToSelected = false, bool select=true);

	int GetRows() const { return index_line_map.size(); }
	void MakeRowVisible(int row);

	/// @brief Get dialogue by index
	/// @param n Index to look up
	/// @return Subtitle dialogue line for index, or 0 if invalid index
	AssDialogue *GetDialogue(int n) const;

public:
	BaseGrid(wxWindow* parent, agi::Context *context);
	~BaseGrid();

	void SetByFrame(bool state);

	DECLARE_EVENT_TABLE()
};
