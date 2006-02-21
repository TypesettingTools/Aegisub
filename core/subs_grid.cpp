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


////////////
// Includes
#include "subs_grid.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "video_display.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "frame_main.h"
#include "hotkeys.h"
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>


///////////////
// Event table
BEGIN_EVENT_TABLE(SubtitlesGrid, BaseGrid)
	EVT_KEY_DOWN(SubtitlesGrid::OnKeyDown)
	EVT_MENU(MENU_SWAP,SubtitlesGrid::OnSwap)
	EVT_MENU(MENU_DUPLICATE,SubtitlesGrid::OnDuplicate)
	EVT_MENU(MENU_DUPLICATE_NEXT_FRAME,SubtitlesGrid::OnDuplicateNextFrame)
	EVT_MENU(MENU_JOIN_CONCAT,SubtitlesGrid::OnJoinConcat)
	EVT_MENU(MENU_JOIN_REPLACE,SubtitlesGrid::OnJoinReplace)
	EVT_MENU(MENU_ADJOIN,SubtitlesGrid::OnAdjoin)
	EVT_MENU(MENU_ADJOIN2,SubtitlesGrid::OnAdjoin2)
	EVT_MENU(MENU_INSERT_BEFORE,SubtitlesGrid::OnInsertBefore)
	EVT_MENU(MENU_INSERT_AFTER,SubtitlesGrid::OnInsertAfter)
	EVT_MENU(MENU_INSERT_BEFORE_VIDEO,SubtitlesGrid::OnInsertBeforeVideo)
	EVT_MENU(MENU_INSERT_AFTER_VIDEO,SubtitlesGrid::OnInsertAfterVideo)
	EVT_MENU(MENU_COPY,SubtitlesGrid::OnCopyLines)
	EVT_MENU(MENU_PASTE,SubtitlesGrid::OnPasteLines)
	EVT_MENU(MENU_CUT,SubtitlesGrid::OnCutLines)
	EVT_MENU(MENU_DELETE,SubtitlesGrid::OnDeleteLines)
	EVT_MENU(MENU_SET_START_TO_VIDEO,SubtitlesGrid::OnSetStartToVideo)
	EVT_MENU(MENU_SET_END_TO_VIDEO,SubtitlesGrid::OnSetEndToVideo)
	EVT_MENU(MENU_SET_VIDEO_TO_START,SubtitlesGrid::OnSetVideoToStart)
	EVT_MENU(MENU_SET_VIDEO_TO_END,SubtitlesGrid::OnSetVideoToEnd)
	EVT_MENU(MENU_JOIN_AS_KARAOKE,SubtitlesGrid::OnJoinAsKaraoke)
	EVT_MENU(MENU_1_12_2_RECOMBINE,SubtitlesGrid::On1122Recombine)
	EVT_MENU(MENU_12_2_RECOMBINE,SubtitlesGrid::On122Recombine)
	EVT_MENU(MENU_1_12_RECOMBINE,SubtitlesGrid::On112Recombine)

	EVT_ERASE_BACKGROUND(SubtitlesGrid::OnEraseBackground)
END_EVENT_TABLE()


///////////////
// Constructor
SubtitlesGrid::SubtitlesGrid(FrameMain* parentFr, wxWindow *parent, wxWindowID id, VideoDisplay *_video, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
                        : BaseGrid(parent,id,pos,size,style,name)
{
	// Vars
	changingCol = false;
	byFrame = false;
	ass = NULL;
	video = _video;
	editBox = NULL;
	parentFrame = parentFr;

	// Font size
	int fontSize = Options.AsInt(_T("Grid font size"));
	wxFont font;
	font.SetPointSize(fontSize);
	wxClientDC dc(this);
	dc.SetFont(font);
	int w,h;
	dc.GetTextExtent(_T("#TWFfgGhH"), &w, &h, NULL, NULL, &font);
	RowHeight = h+4;
}


//////////////
// Destructor
SubtitlesGrid::~SubtitlesGrid() {
	wxRemoveFile(tempfile);
	tempfile = _T("");
}


//////////////
// Popup menu
void SubtitlesGrid::OnPopupMenu() {
	// Get selections
	bool continuous;
	wxArrayInt selections = GetSelection(&continuous);
	int sels = selections.Count();

	// Show menu if at least one is selected
	if (sels > 0) {
		wxMenu menu;
		bool state;

		// Insert
		state = (sels == 1);
		menu.Append(MENU_INSERT_BEFORE,_("&Insert (before)"),_T("Inserts a line before current"))->Enable(state);
		menu.Append(MENU_INSERT_AFTER,_("Insert (after)"),_T("Inserts a line after current"))->Enable(state);
		state = (sels == 1 && video && video->loaded);
		menu.Append(MENU_INSERT_BEFORE_VIDEO,_("Insert at video time (before)"),_T("Inserts a line after current, starting at video time"))->Enable(state);
		menu.Append(MENU_INSERT_AFTER_VIDEO,_("Insert at video time (after)"),_T("Inserts a line after current, starting at video time"))->Enable(state);
		menu.AppendSeparator();

		// Video/time sync
		state = (video && video->loaded);
		menu.Append(MENU_SET_VIDEO_TO_START,_("Jump video to start"),_T("Sets current video time to start time"))->Enable(state);
		menu.Append(MENU_SET_VIDEO_TO_END,_("Jump video to end"),_T("Sets current video time to end time"))->Enable(state);
		menu.Append(MENU_SET_START_TO_VIDEO,_("Set start to video"),_T("Sets start times to current video time"))->Enable(state);
		menu.Append(MENU_SET_END_TO_VIDEO,_("Set end to video"),_T("Sets end times to current video time"))->Enable(state);
		menu.AppendSeparator();

		// Duplicate selection
		menu.Append(MENU_DUPLICATE,_("&Duplicate"),_T("Duplicate the selected lines"))->Enable(continuous);
		menu.Append(MENU_DUPLICATE_NEXT_FRAME,_("&Duplicate and shift by 1 frame"),_T("Duplicate lines and shift by one frame"))->Enable(continuous && VFR_Output.loaded);

		// Swaps selection
		state = (sels == 2);
		menu.Append(MENU_SWAP,_("&Swap"),_T("Swaps the two selected lines"))->Enable(state);

		// Join selection
		state = (sels >= 2 && continuous);
		menu.Append(MENU_JOIN_CONCAT,_("&Join (concatenate)"),_T("Joins selected lines in a single one, concatenating text together"))->Enable(state);
		menu.Append(MENU_JOIN_REPLACE,_("Join (keep first)"),_T("Joins selected lines in a single one, keeping text of first and discarding remaining"))->Enable(state);
		menu.Append(MENU_JOIN_AS_KARAOKE,_("Join (as Karaoke)"),_T(""))->Enable(state);

		// Adjoin selection
		menu.Append(MENU_ADJOIN,_("&Make times continuous (change start)"),_T("Changes times of subs so start times begin on previous's end time"))->Enable(state);
		menu.Append(MENU_ADJOIN2,_("&Make times continuous (change end)"),_T("Changes times of subs so end times begin on next's start time"))->Enable(state);
		menu.AppendSeparator();

		// Recombine selection
		state = (sels == 2 && continuous);
		menu.Append(MENU_1_12_RECOMBINE,_("Recombine (1, 1+2) into (1, 2)"),_T("Recombine subtitles when first one is actually first plus second"))->Enable(state);
		menu.Append(MENU_12_2_RECOMBINE,_("Recombine (1+2, 2) into (1, 2)"),_T("Recombine subtitles when second one is actually first plus second"))->Enable(state);
		state = (sels == 3 && continuous);
		menu.Append(MENU_1_12_2_RECOMBINE,_("Recombine (1, 1+2, 2) into (1, 2)"),_T("Recombine subtitles when middle one is actually first plus second"))->Enable(state);
		menu.AppendSeparator();

		// Copy/cut/paste
		menu.Append(MENU_COPY,_("&Copy"),_T("Copies selected lines to clipboard"));
		menu.Append(MENU_CUT,_("C&ut"),_T("Cuts selected lines to clipboard"));
		menu.Append(MENU_PASTE,_("&Paste"),_T("Paste lines from clipboard"));
		menu.AppendSeparator();

		// Delete
		menu.Append(MENU_DELETE,_("Delete"),_T("Delete currently selected lines"));

		PopupMenu(&menu);
	}
}


///////////////////////////
// Process keyboard events
void SubtitlesGrid::OnKeyDown(wxKeyEvent &event) {
	// Get key
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_controlDown,event.m_altDown,event.m_shiftDown);

	// Get selection
	bool continuous = false;
	wxArrayInt sels = GetSelection(&continuous);
	int n_found = sels.Count();
	int n = 0;
	int n2 = 0;
	int nrows = GetRows();
	if (n_found > 0) {
		n = sels[0];
		n2 = sels[n_found-1];
	}

	if (n_found == 1) {
		// Move down
		if (Hotkeys.IsPressed(_T("Grid move row down"))) {
			if (n < nrows-1) {
				SwapLines(n,n+1);
				SelectRow(n+1);
				editBox->SetToLine(n+1);
			}
			return;
		}

		// Move up
		if (Hotkeys.IsPressed(_T("Grid move row up"))) {
			if (n > 0) {
				SwapLines(n-1,n);
				SelectRow(n-1);
				editBox->SetToLine(n-1);
			}
			return;
		}
	}

	if (n_found >= 1) {
		// Copy
		if (Hotkeys.IsPressed(_T("Copy"))) {
			wxCommandEvent dummy;
			OnCopyLines(dummy);
			return;
		}

		// Cut
		if (Hotkeys.IsPressed(_T("Cut"))) {
			wxCommandEvent dummy;
			OnCutLines(dummy);
			return;
		}

		// Paste
		if (Hotkeys.IsPressed(_T("Paste"))) {
			wxCommandEvent dummy;
			OnPasteLines(dummy);
			return;
		}

		// Delete
		if (Hotkeys.IsPressed(_T("Grid delete rows"))) {
			DeleteLines(-1,-1,true);
			return;
		}

		if (continuous) {
			// Duplicate
			if (Hotkeys.IsPressed(_T("Grid duplicate rows"))) {
				DuplicateLines(n,n2,false);
				return;
			}

			// Duplicate and shift
			if (VFR_Output.loaded) {
				if (Hotkeys.IsPressed(_T("Grid duplicate and shift one frame"))) {
					DuplicateLines(n,n2,true);
					return;
				}
			}
		}
	}

	event.Skip();
}


///////////////////////
// Duplicate selection
void SubtitlesGrid::OnDuplicate (wxCommandEvent &WXUNUSED(&event)) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	DuplicateLines(n1,n2);
}


//////////////////////////////////////////////
// Duplicate selection and shift by one frame
void SubtitlesGrid::OnDuplicateNextFrame (wxCommandEvent &WXUNUSED(&event)) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	// Duplicate
	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	DuplicateLines(n1,n2,true);
}


/////////////
// Call swap
void SubtitlesGrid::OnSwap (wxCommandEvent &event) {
	int n1,n2;
	int n_found = 0;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (n_found == 0) n1 = i;
			else if (n_found == 1) n2 = i;
			else throw _T("Too many lines found!");
			n_found++;
		}
	}
	SwapLines(n1,n2);
}


///////////////////////////
// Call join (concatenate)
void SubtitlesGrid::OnJoinConcat (wxCommandEvent &event) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	JoinLines(n1,n2,true);
}


///////////////////////
// Call join (replace)
void SubtitlesGrid::OnJoinReplace (wxCommandEvent &event) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	JoinLines(n1,n2,false);
}


////////////////
// Adjoin lines
void SubtitlesGrid::OnAdjoin (wxCommandEvent &event) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	AdjoinLines(n1,n2,true);
}

void SubtitlesGrid::OnAdjoin2 (wxCommandEvent &event) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	AdjoinLines(n1,n2,false);
}



////////////////////////
// Call join as karaoke
void SubtitlesGrid::OnJoinAsKaraoke (wxCommandEvent &event) {
	int n1 = -1;
	int n2 = -1;
	bool gotfirst = false;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!gotfirst) {
				n1 = i;
				gotfirst = true;
			}
			else n2 = i;
		}
	}

	if (n1 == -1) return;
	if (n2 == -1) n2 = n1;
	JoinAsKaraoke(n1,n2);
}


//////////////////////
// Call insert before
void SubtitlesGrid::OnInsertBefore (wxCommandEvent &event) {
	// Find line
	int n;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			n = i;
			break;
		}
	}

	// Create line to add
	AssDialogue *def = new AssDialogue;
	if (n == 0) {
		def->Start.SetMS(0);
		def->End = GetDialogue(n)->Start;
	}
	else {
		def->Start = GetDialogue(n-1)->End;
		def->End = GetDialogue(n)->Start;
	}
	if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+5000);
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,false);
	SelectRow(n);
	editBox->SetToLine(n);
}


/////////////////////
// Call insert after
void SubtitlesGrid::OnInsertAfter (wxCommandEvent &event) {
	// Find line
	int n;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			n = i;
			break;
		}
	}

	// Create line to add
	AssDialogue *def = new AssDialogue;
	if (n == nrows-1) {
		def->Start = GetDialogue(n)->End;
		def->End = GetDialogue(n)->End;
		def->End.SetMS(def->End.GetMS()+5000);
	}
	else {
		def->Start = GetDialogue(n)->End;
		def->End = GetDialogue(n+1)->Start;
	}
	if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+5000);
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,true);
	SelectRow(n+1);
	editBox->SetToLine(n+1);
}


/////////////////////////////////
// Call insert before with video
void SubtitlesGrid::OnInsertBeforeVideo (wxCommandEvent &event) {
	// Find line
	int n;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			n = i;
			break;
		}
	}

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.CorrectTimeAtFrame(video->frame_n,true);
	def->Start.SetMS(video_ms);
	def->End.SetMS(video_ms+5000);
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,false);
	SelectRow(n);
	editBox->SetToLine(n);
}


////////////////////////////////
// Call insert after with video
void SubtitlesGrid::OnInsertAfterVideo (wxCommandEvent &event) {
	// Find line
	int n;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			n = i;
			break;
		}
	}

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.CorrectTimeAtFrame(video->frame_n,true);
	def->Start.SetMS(video_ms);
	def->End.SetMS(video_ms+5000);
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,true);
	SelectRow(n+1);
	editBox->SetToLine(n+1);
}


///////////////////////////////
// Copy selection to clipboard
void SubtitlesGrid::OnCopyLines (wxCommandEvent &WXUNUSED(&event)) {
	CopyLines();
}


///////////////////////////////
// Cuts selection to clipboard
void SubtitlesGrid::OnCutLines (wxCommandEvent &WXUNUSED(&event)) {
	CopyLines();
	DeleteLines(-1,-1,true);
}


////////////////////////
// Paste from clipboard
void SubtitlesGrid::OnPasteLines (wxCommandEvent &WXUNUSED(&event)) {
	int n;
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			n = i;
			break;
		}
	}

	PasteLines(n);
}


///////////////////////////////
// Copy selection to clipboard
void SubtitlesGrid::OnDeleteLines (wxCommandEvent &WXUNUSED(&event)) {
	DeleteLines(-1,-1,true);
}


//////////////////////////
// Set start to video pos
void SubtitlesGrid::OnSetStartToVideo(wxCommandEvent &event) {
	// Check if it's OK to do it
	if (!VFR_Output.loaded) return;

	// Get new time
	int ms = VFR_Output.CorrectTimeAtFrame(video->frame_n,true);

	// Update selection
	wxArrayInt sel = GetSelection();
	AssDialogue *cur;
	int modified =0;
	for (size_t i=0;i<sel.Count();i++) {
		cur = GetDialogue(sel[i]);
		if (cur) {
			modified++;
			cur->Start.SetMS(ms);
			cur->UpdateData();
			SetRowToLine(sel[i],cur);
		}
	}

	// Commit
	if (modified) {
		ass->FlagAsModified();
		CommitChanges();
		editBox->Update();
	}
}


////////////////////////
// Set end to video pos
void SubtitlesGrid::OnSetEndToVideo(wxCommandEvent &event) {
	// Check if it's OK to do it
	if (!VFR_Output.loaded) return;

	// Get new time
	int ms = VFR_Output.CorrectTimeAtFrame(video->frame_n,false);

	// Update selection
	wxArrayInt sel = GetSelection();
	AssDialogue *cur;
	int modified = 0;
	for (size_t i=0;i<sel.Count();i++) {
		cur = GetDialogue(sel[i]);
		if (cur) {
			cur->End.SetMS(ms);
			cur->UpdateData();
			modified++;
			SetRowToLine(sel[i],cur);
		}
	}

	// Commit
	if (modified) {
		ass->FlagAsModified();
		CommitChanges();
		editBox->Update();
	}
}


//////////////////////////
// Set video pos to start
void SubtitlesGrid::OnSetVideoToStart(wxCommandEvent &event) {
	wxArrayInt sel = GetSelection();
	if (sel.Count() == 0) return;
	AssDialogue *cur = GetDialogue(sel[0]);
	if (cur) video->JumpToFrame(VFR_Output.CorrectFrameAtTime(cur->Start.GetMS(),true));
}


////////////////////////
// Set video pos to end
void SubtitlesGrid::OnSetVideoToEnd(wxCommandEvent &event) {
	wxArrayInt sel = GetSelection();
	if (sel.Count() == 0) return;
	AssDialogue *cur = GetDialogue(sel[0]);
	//if (cur) video->JumpToTime(cur->End.GetMS());
	if (cur) video->JumpToFrame(VFR_Output.CorrectFrameAtTime(cur->End.GetMS(),false));
}


/////////////////////
// 1,1+2,2 Recombine
void SubtitlesGrid::On1122Recombine(wxCommandEvent &event) {
	// Get selection
	bool cont;
	wxArrayInt sel = GetSelection(&cont);
	if (sel.Count() != 3 || !cont) throw _T("Invalid number of selections");
	int n = sel[0];

	// Update
	AssDialogue *n1,*n2,*n3;
	n1 = GetDialogue(n);
	n2 = GetDialogue(n+1);
	n3 = GetDialogue(n+2);
	n1->End = n2->End;
	n3->Start = n2->Start;
	n1->UpdateData();
	n3->UpdateData();

	// Delete middle
	DeleteLines(n+1,n+1,false);
}


///////////////////
// 1+2,2 Recombine
void SubtitlesGrid::On122Recombine(wxCommandEvent &event) {
	// Get selection
	bool cont;
	wxArrayInt sel = GetSelection(&cont);
	if (sel.Count() != 2 || !cont) throw _T("Invalid number of selections");
	int n = sel[0];

	// Update
	AssDialogue *n1,*n2;
	n1 = GetDialogue(n);
	n2 = GetDialogue(n+1);
	n1->Text.Trim(true).Trim(false);
	n2->Text.Trim(true).Trim(false);

	// Check if n2 is a suffix of n1
	if (n1->Text.Right(n2->Text.Length()) == n2->Text) {
		n1->Text = n1->Text.SubString(0, n1->Text.Length() - n2->Text.Length() - 1).Trim(true).Trim(false);
		while (n1->Text.Left(2) == _T("\\N") || n1->Text.Left(2) == _T("\\n"))
			n1->Text = n1->Text.Mid(2);
		while (n1->Text.Right(2) == _T("\\N") || n1->Text.Right(2) == _T("\\n"))
			n1->Text = n1->Text.Mid(0,n1->Text.Length()-2);
		n2->Start = n1->Start;
		//n1->ParseASSTags();
		n1->UpdateData();
		n2->UpdateData();

		// Commit
		SetRowToLine(n,n1);
		SetRowToLine(n+1,n2);
		ass->FlagAsModified();
		CommitChanges();
	} else {
		parentFrame->StatusTimeout(_T("Unable to recombine: Second line is not a suffix of first one."));
	}
}


///////////////////
// 1,1+2 Recombine
void SubtitlesGrid::On112Recombine(wxCommandEvent &event) {
	// Get selection
	bool cont;
	wxArrayInt sel = GetSelection(&cont);
	if (sel.Count() != 2 || !cont) throw _T("Invalid number of selections");
	int n = sel[0];

	// Update
	AssDialogue *n1,*n2;
	n1 = GetDialogue(n);
	n2 = GetDialogue(n+1);
	n1->Text.Trim(true).Trim(false);
	n2->Text.Trim(true).Trim(false);

	// Check if n1 is a prefix of n2 and recombine
	if (n2->Text.Left(n1->Text.Length()) == n1->Text) {
		n2->Text = n2->Text.Mid(n1->Text.Length()).Trim(true).Trim(false);
		while (n2->Text.Left(2) == _T("\\N") || n2->Text.Left(2) == _T("\\n"))
			n2->Text = n2->Text.Mid(2);
		while (n2->Text.Right(2) == _T("\\N") || n2->Text.Right(2) == _T("\\n"))
			n2->Text = n2->Text.Mid(0,n2->Text.Length()-2);
		n1->End = n2->End;
		//n2->ParseASSTags();
		n1->UpdateData();
		n2->UpdateData();

		// Commit
		SetRowToLine(n,n1);
		SetRowToLine(n+1,n2);
		ass->FlagAsModified();
		CommitChanges();
	} else {
		parentFrame->StatusTimeout(_T("Unable to recombine: First line is not a prefix of second one."));
	}
}


//////////////////////////////////////
// Clears grid and sets it to default
void SubtitlesGrid::LoadDefault (AssFile *_ass) {
	if (_ass) {
		ass = _ass;
	}
	ass->LoadDefault();
	LoadFromAss(NULL,false,true);
}


///////////////
// Clears grid
void SubtitlesGrid::Clear () {
	//if (GetNumberRows() > 0) DeleteRows(0,GetNumberRows());
	diagMap.clear();
	diagPtrMap.clear();
	selMap.clear();
	yPos = 0;
	AdjustScrollbar();
}


/////////////////////////////////////
// Read data from ASS file structure
void SubtitlesGrid::LoadFromAss (AssFile *_ass,bool keepSelection,bool dontModify) {
	// Store selected rows
	std::vector<int> srows;
	if (keepSelection) {
		int nrows = GetRows();
		for (int i=0;i<nrows;i++) {
			if (IsInSelection(i,0)) {
				srows.push_back(i);
			}
		}
	}

	// Clear grid and choose subtitles file
	BeginBatch();
	Clear();
	if (_ass) ass = _ass;
	else {
		if (!ass) throw _T("Trying to set subs grid to current ass file, but there is none");
	}

	// Run through subs adding them
	int n = 0;
	AssDialogue *curdiag;
	ready = false;
	for (entryIter cur=ass->Line.begin();cur != ass->Line.end();cur++) {
		curdiag = AssEntry::GetAsDialogue(*cur);
		if (curdiag) {
			//AppendRows(1);
			//SetRowToLine(n,curdiag);
			diagMap.push_back(cur);
			diagPtrMap.push_back(curdiag);
			selMap.push_back(false);
			n++;
		}
	}
	ready = true;

	// Restore selection
	if (keepSelection) {
		for (size_t i=0;i<srows.size();i++) {
			SelectRow(srows.at(i),true);
		}
	}

	// Select first
	else {
		SelectRow(0);
	}

	// Finish setting layout
	AdjustScrollbar();
	SetColumnWidths();
	EndBatch();

	// Commit
	if (!AssFile::Popping) {
		if (dontModify) AssFile::StackPush();
		else ass->FlagAsModified();
	}
	CommitChanges();

	// Set edit box
	if (editBox) {
		int nrows = GetRows();
		int firstsel = -1;
		for (int i=0;i<nrows;i++) {
			if (IsInSelection(i,0)) {
				firstsel = i;
				break;
			}
		}
		editBox->UpdateGlobals();
		if (_ass) editBox->SetToLine(firstsel);
	}
}


/////////////////////////////////////////
// Sets one line to a line from the subs
void SubtitlesGrid::SetRowToLine(int n,AssDialogue *line) {
	Refresh(false);
}


//////////////////
// Sets row color
void SubtitlesGrid::SetRowColour(int n,AssDialogue *line) {
	Refresh(false);
}


//////////////////////
// Update row colours
void SubtitlesGrid::UpdateRowColours() {
	Refresh(false);
}


///////////////////
// Swaps two lines
void SubtitlesGrid::SwapLines(int n1,int n2) {
	// Check bounds and get iterators
	int rows = GetRows();
	if (n1 < 0 || n2 < 0 || n1 >= rows || n2 >= rows) return;
	entryIter src1 = diagMap.at(n1);
	entryIter src2 = diagMap.at(n2);
	
	// Swaps
	iter_swap(src1,src2);

	// Update display
	SetRowToLine(n1,AssEntry::GetAsDialogue(*src1));
	SetRowToLine(n2,AssEntry::GetAsDialogue(*src2));

	// Update mapping
	diagMap[n1] = src1;
	diagMap[n2] = src2;
	ass->FlagAsModified();
	CommitChanges();
}


/////////////////
// Insert a line
void SubtitlesGrid::InsertLine(AssDialogue *line,int n,bool after,bool update) {
	// Check bounds and get iterators
	entryIter pos = diagMap.at(n);
	
	// Insert
	if (after) {
		n++;
		pos++;
	}
	line->UpdateData();
	entryIter newIter = ass->Line.insert(pos,line);
	//InsertRows(n);
	//SetRowToLine(n,line);
	diagMap.insert(diagMap.begin() + n,newIter);
	selMap.insert(selMap.begin() + n,false);

	// Update
	if (update) {
		ass->FlagAsModified();
		CommitChanges();
	}
}


///////////////////////////
// Copy lines to clipboard
void SubtitlesGrid::CopyLines() {
	// Prepare text
	wxString data = _T("");
	AssDialogue *cur;
	int nrows = GetRows();
	bool first = true;
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			if (!first) data += _T("\r\n");
			first = false;
			cur = GetDialogue(i);
			data += cur->data;
		}
	}

	// Send to clipboard
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(data));
		wxTheClipboard->Close();
	}
}


//////////////////////////////
// Paste lines from clipboard
void SubtitlesGrid::PasteLines(int n) {
	// Prepare text
	wxString data = _T("");

	// Read from clipboard
	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
			wxTextDataObject rawdata;
			wxTheClipboard->GetData(rawdata);
			data = rawdata.GetText();
		}
		wxTheClipboard->Close();
	}

	// Check if it actually got anything
	if (!data.empty()) {
		int inserted = 0;
		wxStringTokenizer token (data,_T("\r\n"),wxTOKEN_STRTOK);
		while (token.HasMoreTokens()) {
			wxString curdata = token.GetNextToken();
			curdata.Trim(true);
			curdata.Trim(false);
			try { 
				AssDialogue *curdiag = new AssDialogue(curdata);
				//AssDialogue *curdiag = new AssDialogue;
				//curdiag->data = curdata;
				//curdiag->Parse();
				//curdiag->UpdateData();
				//InsertLine(curdiag,n,true,false);
				InsertLine(curdiag,n+inserted,false,false);
				inserted++;
			}
			catch (...) {
			}
		}

		if (inserted > 0) {
			// Commit
			ass->FlagAsModified();
			CommitChanges();

			// Set selection
			SelectRow(n);
			for (int i=n+1;i<n+inserted;i++) {
				SelectRow(i,true);
			}
			editBox->SetToLine(n+1);
		}
	}
}


/////////////////////////
// Delete selected lines
void SubtitlesGrid::DeleteLines(int n1,int n2,bool sel) {
	// Check if it's wiping file
	int deleted = 0;

	// Range
	if (!sel) {
		// Deallocate lines
		for (int i=n1;i<=n2;i++) {
			delete GetDialogue(i);
		}

		// Remove from AssFile
		if (n1 != n2) ass->Line.erase(diagMap.at(n1),++diagMap.at(n2));
		else ass->Line.erase(diagMap.at(n1));
		deleted = n2-n1+1;
	}

	// Selection
	else {
		int nlines = GetRows();
		for (int i=0;i<nlines;i++) {
			if (IsInSelection(i,0)) {
				delete (AssDialogue*)(*diagMap.at(i));
				ass->Line.erase(diagMap.at(i));
				deleted++;
			}
		}
	}

	// Add default line if file was wiped
	if (GetRows() == deleted) {
		AssDialogue *def = new AssDialogue;
		ass->Line.push_back(def);
	}

	// Update
	UpdateMaps();
	AdjustScrollbar();
	ass->FlagAsModified();
	CommitChanges();
}


////////////////////////
// Joins selected lines
void SubtitlesGrid::JoinLines(int n1,int n2,bool concat) {
	// Initialize
	int min_ms = 0x0FFFFFFF;
	int max_ms = -1;
	wxString finalText = _T("");

	// Collect data
	AssDialogue *cur;
	int start,end;
	bool gotfirst = false;
	for (int i=n1;i<=n2;i++) {
		cur = GetDialogue(i);
		start = cur->Start.GetMS();
		end = cur->End.GetMS();
		if (start < min_ms) min_ms = start;
		if (end > max_ms) max_ms = end;
		if (concat || !gotfirst) {
			if (gotfirst) finalText += _T("\\N");
			gotfirst = true;
			finalText += cur->Text;
		}
	}

	// Apply settings to first line
	cur = GetDialogue(n1);
	cur->Start.SetMS(min_ms);
	cur->End.SetMS(max_ms);
	cur->Text = finalText;
	cur->UpdateData();

	// Delete remaining lines (this will auto commit)
	DeleteLines(n1+1,n2,false);

	// Select new line
	editBox->SetToLine(n1);
	SelectRow(n1);
}


//////////////////////////
// Adjoins selected lines
void SubtitlesGrid::AdjoinLines(int n1,int n2,bool setStart) {
	// Set start
	if (setStart) {
		AssDialogue *prev = GetDialogue(n1);
		AssDialogue *cur;
		for (int i=n1+1;i<=n2;i++) {
			cur = GetDialogue(i);
			if (!cur) return;
			cur->Start = prev->End;
			cur->UpdateData();
			SetRowToLine(i,cur);
			prev = cur;
		}
	}

	// Set end
	else {
		AssDialogue *next;
		AssDialogue *cur = GetDialogue(n1);
		for (int i=n1;i<n2;i++) {
			next = GetDialogue(i+1);
			if (!next) return;
			cur->End = next->Start;
			cur->UpdateData();
			SetRowToLine(i,cur);
			cur = next;
		}
	}

	// Commit
	AssFile::top->FlagAsModified();
	CommitChanges();
}


///////////////////////////////////
// Joins selected lines as karaoke
void SubtitlesGrid::JoinAsKaraoke(int n1,int n2) {
	// Initialize
	wxString finalText = _T("");

	// Collect data
	AssDialogue *cur;
	int start,end;
	int firststart;
	int lastend = -1;
	int len1,len2;
	for (int i=n1;i<=n2;i++) {
		cur = GetDialogue(i);

		// Get times
		start = cur->Start.GetMS();
		end = cur->End.GetMS();

		// Get len
		if (lastend == -1) {
			lastend = start;
			firststart = start;
		}
		len1 = (start - lastend) / 10;
		len2 = (end - lastend) / 10;

		// Create text
		if (len1 != 0) finalText += _T("{\\k") + wxString::Format(_T("%i"),len1) + _T("}");
		finalText += _T("{\\k") + wxString::Format(_T("%i"),len2) + _T("}") + cur->Text;
		lastend = end;
	}

	// Apply settings to first line
	cur = GetDialogue(n1);
	cur->Start.SetMS(firststart);
	cur->End.SetMS(lastend);
	cur->Text = finalText;
	cur->UpdateData();

	// Delete remaining lines (this will auto commit)
	DeleteLines(n1+1,n2,false);

	// Select new line
	editBox->SetToLine(n1);
	SelectRow(n1);
}


///////////////////
// Duplicate lines
void SubtitlesGrid::DuplicateLines(int n1,int n2,bool nextFrame) {
	AssDialogue *cur;
	bool update = false;
	int step=0;
	for (int i=n1;i<=n2;i++) {
		// Create
		if (i == n2) update = true;
		//cur = new AssDialogue(GetDialogue(i+step)->data);
		cur = new AssDialogue(GetDialogue(i)->data);

		// Shift to next frame
		if (nextFrame) {
			int posFrame = VFR_Output.CorrectFrameAtTime(cur->End.GetMS(),false) + 1;
			cur->Start.SetMS(VFR_Output.CorrectTimeAtFrame(posFrame,true));
			cur->End.SetMS(VFR_Output.CorrectTimeAtFrame(posFrame,false));
			cur->UpdateData();
		}

		// Insert
		//InsertLine(cur,n1+step,false,update);
		InsertLine(cur,n2+step,true,update);
		step++;
	}

	// Select new lines
	SelectRow(n1+step,false);
	for (int i=n1+1;i<=n2;i++) {
		SelectRow(i+step,true);
	}
	editBox->SetToLine(n1+step);
}


///////////////////////
// Shifts line by time
// -------------------
// Where type =
//  0: Start + End
//  1: Start
//  2: End
//
void SubtitlesGrid::ShiftLineByTime(int n,int len,int type) {
	AssDialogue *cur = GetDialogue(n);

	// Start
	if (type != 2) cur->Start.SetMS(cur->Start.GetMS() + len);
	// End
	if (type != 1) cur->End.SetMS(cur->End.GetMS() + len);

	// Update data
	cur->UpdateData();
}


////////////////////////
// Shifts line by frame
// -------------------
// Where type =
//  0: Start + End
//  1: Start
//  2: End
//
void SubtitlesGrid::ShiftLineByFrames(int n,int len,int type) {
	AssDialogue *cur = GetDialogue(n);

	// Start
	if (type != 2) cur->Start.SetMS(VFR_Output.CorrectTimeAtFrame(len + VFR_Output.CorrectFrameAtTime(cur->Start.GetMS(),true),true));
	// End
	if (type != 1) cur->End.SetMS(VFR_Output.CorrectTimeAtFrame(len + VFR_Output.CorrectFrameAtTime(cur->End.GetMS(),false),false));

	// Update data
	cur->UpdateData();
}


//////////////
// Split line
void SubtitlesGrid::SplitLine(int n,int pos,int mode) {
	// Split
	AssDialogue *n1,*n2;
	n1 = GetDialogue(n);
	n2 = new AssDialogue(n1->data);
	InsertLine(n2,n,true,false);

	// Modify text
	wxString orig = n1->Text;
	n1->Text = orig.Left(pos);
	n2->Text = orig.Mid(pos);
	//n1->ParseASSTags();
	//n2->ParseASSTags();

	// Modify time
	if (mode == 1) {
		double splitPos = double(pos)/orig.Length();
		int splitTime = (n1->End.GetMS() - n1->Start.GetMS())*splitPos + n1->Start.GetMS();
		n1->End.SetMS(splitTime);
		n2->Start.SetMS(splitTime);
	}

	// Update data
	n1->UpdateData();
	n2->UpdateData();
	SetRowToLine(n,n1);
	SetRowToLine(n+1,n2);

	// Update editbox and audio
	editBox->SetToLine(n);

	// Commit
	ass->FlagAsModified();
	CommitChanges();
}


//////////////////
// Commit changes
// --------------
// This will save the work .ass and make avisynth refresh it
void SubtitlesGrid::CommitChanges(bool force) {
	if (video->loaded || force) {
		// Check if it's playing
		bool playing = false;
		if (video->IsPlaying) {
			playing = true;
			video->Stop();
		}

		// Export
		wxString workfile = GetTempWorkFile();
		ass->Export(workfile);

		if (video->loaded)
			video->RefreshSubtitles();

		// Resume play
		if (playing) video->Play();
	}
	parentFrame->UpdateTitle();
}


///////////////////////////
// Gets first selected row
int SubtitlesGrid::GetFirstSelRow() {
	int nrows = GetRows();
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			return i;
		}
	}
	return -1;
}


//////////////////////////
// Gets all selected rows
wxArrayInt SubtitlesGrid::GetSelection(bool *cont) {
	// Prepare
	int nrows = GetRows();
	int last = -1;
	bool continuous = true;
	wxArrayInt selections;

	// Scan
	for (int i=0;i<nrows;i++) {
		if (IsInSelection(i,0)) {
			selections.Add(i);
			if (last != -1 && i != last+1) continuous = false;
			last = i;
		}
	}

	// Return
	if (cont) *cont = continuous;
	return selections;
}


////////////////////////////////
// Sets display by frame or not
void SubtitlesGrid::SetByFrame (bool state) {
	// Check if it's already the same
	if (byFrame == state) return;
	byFrame = state;
	SetColumnWidths();
	Refresh(false);
}


//////////////////////////////
// Get name of temp work file
wxString SubtitlesGrid::GetTempWorkFile () {
	if (tempfile.IsEmpty()) {
		tempfile = wxFileName::CreateTempFileName(_T("aegisub"));
		tempfile += _T(".ass");
	}
	return tempfile;
}


/////////////////////////
// Selects visible lines
void SubtitlesGrid::SelectVisible() {
	int rows = GetRows();
	bool selectedOne = false;
	for (int i=0;i<rows;i++) {
		if (IsDisplayed(GetDialogue(i))) {
			if (!selectedOne) {
				SelectRow(i,false);
				MakeCellVisible(i,0);
				selectedOne = true;
			}
			else {
				SelectRow(i,true);
			}
		}
	}
}
