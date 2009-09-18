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

/// @file audio_karaoke.h
/// @see audio_karaoke.cpp
/// @ingroup audio_ui
///


#pragma once


///////////
// Headers
#ifndef AGI_PRE
#include <vector>

#include <wx/log.h>
#include <wx/menu.h>
#include <wx/window.h>
#endif

#include "ass_karaoke.h"


//////////////
// Prototypes
class AssDialogue;
class AssDialogueBlockOverride;
class AssOverrideTag;
class AssOverrideParameter;
class AudioDisplay;
class AudioBox;
class AudioKaraokeTagMenu;


/// DOCME
struct AudioKaraokeSyllable : AssKaraokeSyllable {

	/// DOCME
	int start_time; // centiseconds

	/// DOCME
	bool selected;

	/// DOCME
	std::vector<int> pending_splits;

	/// DOCME
	int display_w;

	/// DOCME
	int display_x;

	AudioKaraokeSyllable();
	AudioKaraokeSyllable(const AssKaraokeSyllable &base);
};

/// DOCME
typedef std::vector<AudioKaraokeSyllable> AudioKaraokeVector;



/// DOCME
/// @class AudioKaraoke
/// @brief DOCME
///
/// DOCME
class AudioKaraoke : public wxWindow {
	friend class AudioKaraokeTagMenu;
private:

	/// DOCME
	AssDialogue *diag;

	/// DOCME
	AssDialogue *workDiag;

	/// DOCME
	int startClickSyl;

	/// DOCME
	bool must_rebuild;


	/// DOCME
	int split_cursor_syl;

	/// DOCME
	int split_cursor_x;

	void AutoSplit();
	bool ParseDialogue(AssDialogue *diag);

	int GetSylAtX(int x);
	int SplitSyl(unsigned int n);

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnMouse(wxMouseEvent &event);

public:

	/// DOCME
	AudioDisplay *display;

	/// DOCME
	AudioBox *box;


	/// DOCME
	int curSyllable;

	/// DOCME
	int selectionCount;

	/// DOCME
	bool enabled;

	/// DOCME
	bool splitting;

	/// DOCME
	AudioKaraokeVector syllables;

	AudioKaraoke(wxWindow *parent);
	virtual ~AudioKaraoke();

	bool LoadFromDialogue(AssDialogue *diag);
	void Commit();
	void SetSyllable(int n);
	void SetSelection(int start,int end=-1);
	bool SyllableDelta(int n,int delta,int mode);

	void Join();
	void BeginSplit();
	void EndSplit(bool commit=true);

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class AudioKaraokeTagMenu
/// @brief DOCME
///
/// DOCME
class AudioKaraokeTagMenu : public wxMenu {
private:

	/// DOCME
	AudioKaraoke *kara;

	void OnSelectItem(wxCommandEvent &event);
public:
	AudioKaraokeTagMenu(AudioKaraoke *_kara);
	virtual ~AudioKaraokeTagMenu();

	DECLARE_EVENT_TABLE()
};
