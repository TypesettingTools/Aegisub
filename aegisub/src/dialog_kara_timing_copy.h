// Copyright (c) 2006-2009, Dan Donovan (Dansolo), Niels Martin Hansen
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

/// @file dialog_kara_timing_copy.h
/// @see dialog_kara_timing_copy.cpp
/// @ingroup tools_ui kara_timing_copy
///




///////////
// Headers
#ifndef AGI_PRE
#include <vector>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/regex.h>
#endif

#include "ass_entry.h"
#include "ass_file.h"
#include "kana_table.h"


//////////////
// Prototypes
class SubtitlesGrid;
class AssOverrideParameter;
class KaraokeLineMatchDisplay;



/// DOCME
/// @class DialogKanjiTimer
/// @brief DOCME
///
/// DOCME
class DialogKanjiTimer : public wxDialog {

	/// DOCME
	SubtitlesGrid *grid;

	/// DOCME
	AssFile *subs;


	/// DOCME
	KaraokeLineMatchDisplay *display;

	/// DOCME

	/// DOCME
	wxComboBox *SourceStyle, *DestStyle;

	/// DOCME
	wxCheckBox *Interpolate;


	/// DOCME
	std::vector<std::pair<entryIter,wxString> > LinesToChange;

	/// DOCME
	entryIter currentSourceLine;

	/// DOCME
	entryIter currentDestinationLine;

	void OnClose(wxCommandEvent &event);
	void OnStart(wxCommandEvent &event);
	void OnLink(wxCommandEvent &event);
	void OnUnlink(wxCommandEvent &event);
	void OnSkipSource(wxCommandEvent &event);
	void OnSkipDest(wxCommandEvent &event);
	void OnGoBack(wxCommandEvent &event);
	void OnAccept(wxCommandEvent &event);
	inline void OnKeyEnter(wxCommandEvent &event);

	void ResetForNewLine();
	void TryAutoMatch();

	entryIter FindNextStyleMatch(entryIter search_from, const wxString &search_style);
	entryIter FindPrevStyleMatch(entryIter search_from, const wxString &search_style);

public:
	DialogKanjiTimer(wxWindow *parent, SubtitlesGrid *grid);
	void OnKeyDown(wxKeyEvent &event);
	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	BUTTON_KTSTART = 2500,

	/// DOCME
	BUTTON_KTLINK,

	/// DOCME
	BUTTON_KTUNLINK,

	/// DOCME
	BUTTON_KTSKIPSOURCE,

	/// DOCME
	BUTTON_KTSKIPDEST,

	/// DOCME
	BUTTON_KTGOBACK,

	/// DOCME
	BUTTON_KTACCEPT,

	/// DOCME
	TEXT_SOURCE,

	/// DOCME
	TEXT_DEST
};
