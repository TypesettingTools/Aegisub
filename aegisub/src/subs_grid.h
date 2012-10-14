// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file subs_grid.h
/// @see subs_grid.cpp
/// @ingroup main_ui
///

#ifndef AGI_PRE
#include <vector>

#include <wx/grid.h>
#include <wx/wx.h>
#endif

#include "base_grid.h"

/// DOCME
/// @class SubtitlesGrid
/// @brief DOCME
///
/// DOCME
class SubtitlesGrid: public BaseGrid {
public:
	SubtitlesGrid(wxWindow *parent, agi::Context *context, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);

	/// @brief Adjoins selected lines, setting each line's start time to the previous line's end time
	/// @param n1       First line to adjoin
	/// @param n2       Last line to adjoin
	/// @param setStart Set the start times (rather than end times)
	void AdjoinLines(int first,int last,bool setStart);

	void InsertLine(AssDialogue *line,int position,bool insertAfter,bool update=true);
	/// @brief Delete selected lines
	/// @param target       Lines to delete
	/// @param flagModified Commit the file afterwards
	void DeleteLines(wxArrayInt lines, bool flagModified=true);

	/// @brief Copy to clipboard
	/// @param target Lines to copy
	void CopyLines(wxArrayInt lines);
	/// @brief Cut to clipboard
	/// @param target Lines to cut
	void CutLines(wxArrayInt lines);

	void RecombineLines();

	/// Retrieve a list of selected lines in the actual ASS file (i.e. not as displayed in the grid but as represented in the file)
	std::vector<int> GetAbsoluteSelection() const;
	/// @brief Update list of selected lines from absolute selection
	/// @param selection Sorted list of selections
	void SetSelectionFromAbsolute(std::vector<int> &selection);
};
