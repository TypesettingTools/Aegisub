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

/// @file dialog_kara_timing_copy.h
/// @see dialog_kara_timing_copy.cpp
/// @ingroup tools_ui kara_timing_copy
///

#include <list>
#include <vector>

#include <wx/dialog.h>

namespace agi { struct Context; }
class AssDialogue;
class AssEntry;
class AssFile;
class KaraokeLineMatchDisplay;
class wxComboBox;
class wxCheckBox;

class DialogKanjiTimer : public wxDialog {
	AssFile *subs;

	KaraokeLineMatchDisplay *display;

	wxComboBox *SourceStyle, *DestStyle;
	wxCheckBox *Interpolate;

	std::vector<std::pair<AssDialogue*, wxString> > LinesToChange;

	AssEntry *currentSourceLine;
	AssEntry *currentDestinationLine;

	void OnClose(wxCommandEvent &event);
	void OnStart(wxCommandEvent &event);
	void OnLink(wxCommandEvent &event);
	void OnUnlink(wxCommandEvent &event);
	void OnSkipSource(wxCommandEvent &event);
	void OnSkipDest(wxCommandEvent &event);
	void OnGoBack(wxCommandEvent &event);
	void OnAccept(wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	void ResetForNewLine();
	void TryAutoMatch();

	AssEntry *FindNextStyleMatch(AssEntry *search_from, const wxString &search_style);
	AssEntry *FindPrevStyleMatch(AssEntry *search_from, const wxString &search_style);

public:
	DialogKanjiTimer(agi::Context *context);
	DECLARE_EVENT_TABLE()
};
