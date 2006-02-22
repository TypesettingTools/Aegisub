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
#include <wx/wx.h>
#include <wx/grid.h>
#include <list>
#include <vector>


//////////////
// Prototypes
class AssEntry;
class AssDialogue;
class SubsEditBox;
class FrameMain;
class VideoDisplay;
typedef std::list<AssEntry*>::iterator entryIter;


///////////////////
// Base grid class
class BaseGrid : public wxWindow {
private:
	int lineHeight;
	int colWidth[16];
	int lastRow;
	int extendRow;
	bool holding;
	wxFont font;
	wxScrollBar *scrollBar;
	wxBitmap *bmp;

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnScroll(wxScrollEvent &event);
	void OnMouseEvent(wxMouseEvent &event);
	void OnKeyPress(wxKeyEvent &event);

	void DrawImage(wxDC &dc);

protected:
	FrameMain *parentFrame;
	virtual void OnPopupMenu() {}
	void AdjustScrollbar();
	void ScrollTo(int y);
	int yPos;

public:
	SubsEditBox *editBox;
	VideoDisplay *video;

	bool byFrame;
	std::vector<entryIter> diagMap;
	std::vector<AssDialogue *> diagPtrMap;
	std::vector<bool> selMap;

	void SetColumnWidths();
	void BeginBatch();
	void EndBatch();
	void SetByFrame (bool state);

	void SelectRow(int row, bool addToSelected = false, bool select=true);
	void ClearSelection();
	bool IsInSelection(int row, int col) const;
	bool IsDisplayed(AssDialogue *line);
	int GetNumberSelection();
	int GetFirstSelRow();
	void SelectVisible();
	wxArrayInt GetSelection(bool *continuous=NULL);

	void Clear();
	void UpdateMaps();

	int GetRows() const;
	wxArrayInt GetRangeArray(int n1,int n2);
	void MakeCellVisible(int row, int col,bool center=true);

	AssDialogue *GetDialogue(int n);

	BaseGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
	~BaseGrid();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {
	GRID_SCROLLBAR = 1730
};
