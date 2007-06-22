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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#ifndef AUDIO_KARAOKE_H
#define AUDIO_KARAOKE_H


///////////
// Headers
#include <wx/wxprec.h>
#include <vector>


//////////////
// Prototypes
class AssDialogue;
class AssDialogueBlockOverride;
class AssOverrideTag;
class AssOverrideParameter;
class AudioDisplay;
class AudioBox;
class AudioKaraokeTagMenu;


//////////////////
// Syllable class
class KaraokeSyllable {
public:
	int length;
	int position;
	int display_w;
	int display_x;
	wxString contents;
	wxString tag;
	bool selected;

	AssOverrideParameter *original_tagdata;

	std::vector<int> pending_splits;

	KaraokeSyllable();
};


////////////
// Typedefs
typedef std::vector<KaraokeSyllable> SylVector;


/////////
// Class
class AudioKaraoke : public wxWindow {
	friend class AudioKaraokeTagMenu;
private:
	AssDialogue *diag;
	AssDialogue *workDiag;
	int startClickSyl;
	bool must_rebuild;

	int split_cursor_syl;
	int split_cursor_x;

	AssOverrideTag *GetKaraokeLength(AssDialogueBlockOverride *block);
	wxString GetSyllableTag(AssDialogueBlockOverride *block,int n);
	void AutoSplit();
	bool ParseDialogue(AssDialogue *diag);

	int GetSylAtX(int x);
	int SplitSyl(unsigned int n);

	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnMouse(wxMouseEvent &event);

public:
	AudioDisplay *display;
	AudioBox *box;

	int curSyllable;
	int selectionCount;
	bool enabled;
	bool splitting;
	SylVector syllables;

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


///////////////
// Helper menu
class AudioKaraokeTagMenu : public wxMenu {
private:
	AudioKaraoke *kara;

	void OnSelectItem(wxCommandEvent &event);
public:
	AudioKaraokeTagMenu(AudioKaraoke *_kara);
	virtual ~AudioKaraokeTagMenu();

	DECLARE_EVENT_TABLE()
};


#endif
