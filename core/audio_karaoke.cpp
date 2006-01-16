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


///////////
// Headers
#include <wx/tokenzr.h>
#include "audio_karaoke.h"
#include "audio_display.h"
#include "audio_box.h"
#include "ass_dialogue.h"
#include "ass_override.h"


////////////////////////
// Syllable constructor
KaraokeSyllable::KaraokeSyllable() {
	length = 0;
	position = 0;
	display_w = 0;
	display_x = 0;
	selected = false;
}


///////////////
// Constructor
AudioKaraoke::AudioKaraoke(wxWindow *parent)
: wxWindow (parent,-1,wxDefaultPosition,wxSize(10,5),wxTAB_TRAVERSAL|wxBORDER_SUNKEN)
{
	enabled = false;
	curSyllable = 0;
	diag = NULL;
}


//////////////////////
// Load from dialogue
bool AudioKaraoke::LoadFromDialogue(AssDialogue *_diag) {
	// Set dialogue
	diag = _diag;
	if (!diag) {
		Refresh(false);
		return false;
	}

	// Split
	bool hasKar = ParseDialogue(diag);

	// No karaoke, autosplit
	if (!hasKar) {
		AutoSplit();
	}

	// Done
	SetSelection(curSyllable);
	Refresh(false);
	return !hasKar;
}


///////////////////////////////
// Calculate length of karaoke
int AudioKaraoke::GetKaraokeLength(AssDialogueBlockOverride *block) {
	AssOverrideTag *tag;
	size_t n = block->Tags.size();
	int len = -1;
	for (size_t i=0;i<n;i++) {
		tag = block->Tags.at(i);
		if (tag->Name == _T("\\k") || tag->Name == _T("\\K") || tag->Name == _T("\\kf") || tag->Name == _T("\\ko")) {
			len = tag->Params.at(0)->AsInt();
		}
	}
	return len;
}


////////////////////////////
// Gets tag of nth syllable
wxString AudioKaraoke::GetSyllableTag(AssDialogueBlockOverride *block,int n) {
	return block->Tags.at(n)->Name;
}


////////////////////
// Writes line back
void AudioKaraoke::Commit() {
	wxString finalText = _T("");
	KaraokeSyllable *syl;
	size_t n = syllables.size();
	for (size_t i=0;i<n;i++) {
		syl = &syllables.at(i);
		finalText += wxString::Format(_T("{\\k%i}"),syl->length) + syl->contents;
	}
	diag->Text = finalText;
	diag->ParseASSTags();
}


//////////////////
// Autosplit line
void AudioKaraoke::AutoSplit() {
	// Get lengths
	int timelen = (diag->End.GetMS() - diag->Start.GetMS())/10;
	int letterlen = diag->Text.Length();
	int round = letterlen / 2;
	int curlen;
	int acumLen = 0;
	wxString newText;

	// Parse words
	wxStringTokenizer tkz(diag->Text,_T(" "),wxTOKEN_RET_DELIMS);
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken();
		curlen = (token.Length() * timelen + round) / letterlen;
		acumLen += curlen;
		if (acumLen > timelen) {
			curlen -= acumLen - timelen;
			acumLen = timelen;
		}
		newText += wxString::Format(_T("{\\k%i}"),curlen) + token;
	}

	// Load
	AssDialogue newDiag(diag->data);
	newDiag.Text = newText;
	newDiag.ParseASSTags();
	ParseDialogue(&newDiag);
}


//////////////////////////////////
// Parses text to extract karaoke
bool AudioKaraoke::ParseDialogue(AssDialogue *curDiag) {
	// Wipe
	syllables.clear();

	// Prepare syllable data
	AssDialogueBlock *block;
	AssDialogueBlockOverride *override;
	AssDialogueBlockPlain *plain;
	KaraokeSyllable temp;
	temp.contents = _T("");
	int pos = 0;
	temp.length = 0;
	temp.position = 0;
	size_t n = curDiag->Blocks.size();
	bool foundOne = false;
	bool foundBlock = false;

	// Load syllable data
	for (size_t i=0;i<n;i++) {
		block = curDiag->Blocks.at(i);
		override = AssDialogueBlock::GetAsOverride(block);
		if (override) {
			int len = GetKaraokeLength(override);
			if (len != -1) {
				if (foundOne) syllables.push_back(temp);
				foundOne = true;
				foundBlock = true;
				pos += temp.length;
				temp.length = len;
				temp.position = pos;
				temp.contents = _T("");
			}
		}
		else {
			plain = AssDialogueBlock::GetAsPlain(block);
			temp.contents += plain->text;
			if (plain->text != _T("")) foundOne = true;
		}
	}

	// Empty?
	if (curDiag->Text.IsEmpty()) {
		temp.length = (curDiag->End.GetMS() - curDiag->Start.GetMS())/10;
		temp.contents = curDiag->Text;
		temp.position = 0;
		foundBlock = true;
	}

	// Last syllable
	if (foundBlock) syllables.push_back(temp);
	return foundBlock;
}


////////////////
// Set syllable
void AudioKaraoke::SetSyllable(int n) {
	curSyllable = n;
	startClickSyl = n;
	SetSelection(n);
	Refresh(false);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(AudioKaraoke,wxWindow)
	EVT_PAINT(AudioKaraoke::OnPaint)
	EVT_SIZE(AudioKaraoke::OnSize)
	EVT_MOUSE_EVENTS(AudioKaraoke::OnMouse)
END_EVENT_TABLE()


///////////////
// Paint event
void AudioKaraoke::OnPaint(wxPaintEvent &event) {
	// Get dimensions
	int w,h;
	GetClientSize(&w,&h);

	// Start Paint
	wxPaintDC dc(this);
	dc.BeginDrawing();

	// Draw background
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0,0,w,h);

	// Set syllable font
	wxFont curFont(9,wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Verdana"),wxFONTENCODING_SYSTEM);
	dc.SetFont(curFont);
	dc.SetPen(wxPen(wxColour(0,0,0)));

	// Draw syllables
	if (enabled) {
		wxString temptext;
		size_t syln = syllables.size();
		int dx = 0;
		int tw,th;
		int delta;
		int dlen;
		for (size_t i=0;i<syln;i++) {
			// Calculate text length
			temptext = syllables.at(i).contents;
			temptext.Trim(true);
			temptext.Trim(false);
			GetTextExtent(temptext,&tw,&th,NULL,NULL,&curFont);
			delta = 0;
			if (tw < 10) delta = 10 - tw;
			dlen = tw + 8 + delta;

			// Draw border
			if (syllables.at(i).selected) {
				dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
				dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
			}
			else {
				dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT)));
				dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
			}
			dc.DrawRectangle(dx,0,dlen,h);

			// Draw text
			dc.DrawText(temptext,dx+(delta/2)+4,(h-th)/2);

			// Set syllable data
			syllables.at(i).display_x = dx;
			syllables.at(i).display_w = dlen;

			// Increment dx
			dx += dlen;
		}
	}

	// End paint
	dc.EndDrawing();
	event.Skip();
}


//////////////
// Size event
void AudioKaraoke::OnSize(wxSizeEvent &event) {
	Refresh(false);
}


///////////////
// Mouse event
void AudioKaraoke::OnMouse(wxMouseEvent &event) {
	// Get coordinates
	int x = event.GetX();
	int y = event.GetY();
	bool shift = event.m_shiftDown;

	// Left button down
	if (event.LeftDown()) {
		int syl = GetSylAtX(x);

		if (syl != -1) {
			if (shift) {
				SetSelection(syl,startClickSyl);
				Refresh(false);
			}

			else {
				SetSelection(syl);
				startClickSyl = syl;
				curSyllable = syl;
				Refresh(false);
				display->Update();
			}
		}
	}
}


//////////////////////////////
// Get Syllable at position X
int AudioKaraoke::GetSylAtX(int x) {
	int dx,dw;
	size_t syln = syllables.size();
	for (size_t i=0;i<syln;i++) {
		dx = syllables.at(i).display_x; 
		dw = syllables.at(i).display_w; 
		if (x >= dx && x < dx+dw) {
			return i;
		}
	}
	return -1;
}


/////////////////
// Set selection
void AudioKaraoke::SetSelection(int start,int end) {
	// Default end
	if (end == -1) end = start;

	// Get min/max range
	size_t min = start;
	size_t max = end;
	if (max < min) {
		size_t temp = max;
		max = min;
		min = temp;
	}

	// Set values
	bool state;
	size_t syls = syllables.size();
	int sels = 0;
	for (size_t i=0;i<syls;i++) {
		state = (i >= min && i <= max);
		syllables.at(i).selected = state;
		if (state) sels++;
	}

	// Set box buttons
	box->SetKaraokeButtons(sels > 1,sels > 0);
}


//////////////////
// Join syllables
void AudioKaraoke::Join() {
	// Variables
	bool gotOne = false;
	size_t syls = syllables.size();
	KaraokeSyllable *curSyl;
	int first = 0;

	// Loop
	for (size_t i=0;i<syls;i++) {
		curSyl = &syllables.at(i);
		if (curSyl->selected) {
			if (!gotOne) {
				gotOne = true;
				first = i;
			}
			else {
				syllables.at(i-1).length += curSyl->length;
				syllables.at(i-1).contents += curSyl->contents;
				syllables.erase(syllables.begin()+i);
				i--;
				syls--;
			}
		}
	}

	// Set selection
	curSyllable = first;

	// Update
	display->NeedCommit = true;
	display->Update();
	Refresh(false);
}


///////////////////
// Split syllables
void AudioKaraoke::Split() {
	// Variables
	bool hasSplit = false;

	// Loop
	size_t syls = syllables.size();
	for (size_t i=0;i<syls;i++) {
		if (syllables.at(i).selected) {
			int split = SplitSyl(i);
			if (split > 0) {
				syls += split;
				i += split;
				hasSplit = true;
			}
			if (split == -1) break;
		}
	}

	// Update
	if (hasSplit) {
		display->NeedCommit = true;
		display->Update();
		Refresh(false);
	}
}


////////////////////
// Split a syllable
int AudioKaraoke::SplitSyl (int n) {
	// Get split text
	KaraokeSyllable *curSyl = &syllables.at(n);
	wxString result = wxGetTextFromUser(_("Enter pipes (\"|\") to split:"), _("Split syllable"), curSyl->contents);
	if (result.IsEmpty()) return -1;

	// Prepare parsing
	const int splits = result.Freq(_T('|'));
	const int totalCharLen = curSyl->contents.Length() + splits + 1;
	const int charRound = totalCharLen / 2;
	const int totalTimeLen = curSyl->length;
	int curpos = curSyl->position;
	int curlen = 0;
	wxStringTokenizer tkn(result,_T("|"),wxTOKEN_RET_EMPTY_ALL);
	bool isFirst = true;

	// Parse
	for (int curn=n;tkn.HasMoreTokens();curn++) {
		// Prepare syllable
		if (!isFirst) {
			KaraokeSyllable temp;
			syllables.insert(syllables.begin()+curn,temp);
			temp.selected = true;
		}
		curSyl = &syllables.at(curn);

		// Set text
		wxString token = tkn.GetNextToken();
		curSyl->contents = token;

		// Set position
		int len = (totalTimeLen * (token.Length() + 1) + charRound) / totalCharLen;
		curlen += len;
		if (curlen > totalTimeLen) {
			len -= totalTimeLen - curlen;
			curlen = totalTimeLen;
		}
		curSyl->length = len;
		curSyl->position = curpos;
		curpos += len;

		// Done
		isFirst = false;
	}

	// Return splits
	return splits;
}


//////////////////////////////////
// Apply delta length to syllable
bool AudioKaraoke::SyllableDelta(int n,int delta,int mode) {
	// Get syllable and next
	KaraokeSyllable *curSyl=NULL,*nextSyl=NULL;
	curSyl = &syllables.at(n);
	int nkar = syllables.size();
	if (n < nkar-1) {
		nextSyl = &syllables.at(n+1);
	}

	// Get variables
	int len = curSyl->length;

	// Cap delta
	int minLen = 0;
	if (len + delta < minLen) delta = minLen-len;
	if (mode == 0 && nextSyl && (nextSyl->length - delta) < minLen) delta = nextSyl->length - minLen;

	// Apply
	if (delta != 0) {
		curSyl->length += delta;

		// Normal mode
		if (mode == 0 && nextSyl) {
			nextSyl->length -= delta;
			nextSyl->position += delta;
		}

		// Shift mode
		if (mode == 1) {
			for (int i=n+1;i<nkar;i++) {
				syllables.at(i).position += delta;
			}
		}

		// Flag update
		return true;
	}
	return false;
}
