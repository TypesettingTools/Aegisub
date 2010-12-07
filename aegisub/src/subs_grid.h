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
#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include <wx/grid.h>
#include <wx/wx.h>
#endif

#include "base_grid.h"

#include <libaegisub/signals.h>

namespace agi { class OptionValue; }

class AssFile;
class AssEntry;
class AssDialogue;
class SubsEditBox;
class FrameMain;

typedef std::list<AssEntry*>::iterator entryIter;

/// DOCME
/// @class SubtitlesGrid
/// @brief DOCME
///
/// DOCME
class SubtitlesGrid: public BaseGrid {
private:
	agi::signal::Connection seekListener;

	void OnPopupMenu(bool alternate=false);
	void OnKeyDown(wxKeyEvent &event);

	void OnSwap(wxCommandEvent &event);
	void OnDuplicate(wxCommandEvent &event);
	void OnDuplicateNextFrame(wxCommandEvent &event);
	void OnJoinConcat(wxCommandEvent &event);
	void OnJoinReplace(wxCommandEvent &event);
	void OnAdjoin(wxCommandEvent &event);
	void OnAdjoin2(wxCommandEvent &event);
	void OnInsertBefore(wxCommandEvent &event);
	void OnInsertAfter(wxCommandEvent &event);
	void OnInsertBeforeVideo(wxCommandEvent &event);
	void OnInsertAfterVideo(wxCommandEvent &event);
	void OnCopyLines(wxCommandEvent &event);
	void OnCutLines(wxCommandEvent &event);
	void OnPasteLines(wxCommandEvent &event);
	void OnDeleteLines(wxCommandEvent &event);
	void OnSetStartToVideo(wxCommandEvent &event);
	void OnSetEndToVideo(wxCommandEvent &event);
	void OnSetVideoToStart(wxCommandEvent &event);
	void OnSetVideoToEnd(wxCommandEvent &event);
	void OnJoinAsKaraoke(wxCommandEvent &event);
	void OnSplitByKaraoke(wxCommandEvent &event);
	void OnRecombine(wxCommandEvent &event);
	void OnAudioClip(wxCommandEvent &event);
	void OnShowColMenu(wxCommandEvent &event);

	void OnHighlightVisibleChange(agi::OptionValue const& opt);

	void OnCommit(int type);

public:
	/// Currently open file
	AssFile *ass;

	SubtitlesGrid(FrameMain* parentFrame,wxWindow *parent, wxWindowID id, AssFile *subs, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxWANTS_CHARS, const wxString& name = wxPanelNameStr);
	~SubtitlesGrid();

	void LoadDefault();

	/// @brief Jump to the start/end time of the current subtitle line
	/// @param start Start vs. End time
	void SetVideoToSubs(bool start);
	/// @brief Set the start/end time of the current subtitle line to the current frame
	/// @param start Start vs. End time
	void SetSubsToVideo(bool start);

	/// @brief Join the selected lines
	/// @param n1     First line to join
	/// @param n2     Last line to join
	/// @param concat Concatenate the lines rather than discarding all past the first
	void JoinLines(int first,int last,bool concat=true);
	/// @brief Join selected lines as karaoke, with their relative times used for syllable lengths
	/// @param n1 First line to join
	/// @param n2 Last line to join
	void JoinAsKaraoke(int first,int last);
	/// @brief Adjoins selected lines, setting each line's start time to the previous line's end time
	/// @param n1       First line to adjoin
	/// @param n2       Last line to adjoin
	/// @param setStart Set the start times (rather than end times)
	void AdjoinLines(int first,int last,bool setStart);
	/// @brief Split line at the given position
	/// @param line Line to split
	/// @param pos Position in line
	/// @param estimateTimes Adjust the times based on the lengths of the halves
	void SplitLine(AssDialogue *line,int splitPosition,bool estimateTimes);
	/// @brief Split a line into as many new lines as there are karaoke syllables, timed as the syllables
	/// @param lineNumber Line to split
	/// @return Were changes made?
	///
	/// DOES NOT FLAG AS MODIFIED OR COMMIT CHANGES
	bool SplitLineByKaraoke(int lineNumber);
	/// @brief Duplicate lines
	/// @param n1        First frame to duplicate
	/// @param n2        Last frame to duplicate
	/// @param nextFrame Set the new lines to start and end on the next frame
	void DuplicateLines(int first,int last,bool nextFrame=false);

	void SwapLines(int line1,int line2);
	/// @brief  Shift line by time
	/// @param n    Line to shift
	/// @param len  ms to shift by
	/// @param type 0: Start + End; 1: Start; 2: End
	void ShiftLineByTime(int lineNumber,int len,int type);
	/// @brief  Shift line by frames
	/// @param n    Line to shift
	/// @param len  frames to shift by
	/// @param type 0: Start + End; 1: Start; 2: End
	void ShiftLineByFrames(int lineNumber,int len,int type);

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
	void PasteLines(int pos,bool over=false);

	/// Retrieve a list of selected lines in the actual ASS file (i.e. not as displayed in the grid but as represented in the file)
	std::vector<int> GetAbsoluteSelection();
	/// @brief Update list of selected lines from absolute selection
	/// @param selection Sorted list of selections
	void SetSelectionFromAbsolute(std::vector<int> &selection);

	DECLARE_EVENT_TABLE()
};

/// Menu event IDs
enum {
	MENU_GRID_START = 1200,
	MENU_INSERT_BEFORE,
	MENU_INSERT_AFTER,
	MENU_INSERT_BEFORE_VIDEO,
	MENU_INSERT_AFTER_VIDEO,
	MENU_SWAP,
	MENU_DUPLICATE,
	MENU_DUPLICATE_NEXT_FRAME,
	MENU_SPLIT_BY_KARAOKE,
	MENU_COPY,
	MENU_PASTE,
	MENU_CUT,
	MENU_DELETE,
	MENU_JOIN_CONCAT,
	MENU_JOIN_REPLACE,
	MENU_ADJOIN,
	MENU_ADJOIN2,
	MENU_JOIN_AS_KARAOKE,
	MENU_RECOMBINE,
	MENU_SET_START_TO_VIDEO,
	MENU_SET_END_TO_VIDEO,
	MENU_SET_VIDEO_TO_START,
	MENU_SET_VIDEO_TO_END,
	MENU_GRID_END,
	MENU_AUDIOCLIP,
	MENU_SHOW_COL = 1250 // Don't put anything after this
};
