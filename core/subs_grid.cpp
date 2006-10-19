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
#include <algorithm>
#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
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
#include "utils.h"
#include "ass_override.h"


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
	EVT_MENU(MENU_SPLIT_BY_KARAOKE,SubtitlesGrid::OnSplitByKaraoke)
	EVT_MENU(MENU_1_12_2_RECOMBINE,SubtitlesGrid::On1122Recombine)
	EVT_MENU(MENU_12_2_RECOMBINE,SubtitlesGrid::On122Recombine)
	EVT_MENU(MENU_1_12_RECOMBINE,SubtitlesGrid::On112Recombine)
	EVT_MENU_RANGE(MENU_SHOW_COL,MENU_SHOW_COL+15,SubtitlesGrid::OnShowColMenu)
END_EVENT_TABLE()


///////////////
// Constructor
SubtitlesGrid::SubtitlesGrid(FrameMain* parentFr, wxWindow *parent, wxWindowID id, VideoDisplay *_video, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
                        : BaseGrid(parent,id,pos,size,style,name)
{
	// Vars
	byFrame = false;
	ass = NULL;
	video = _video;
	editBox = NULL;
	parentFrame = parentFr;
}


//////////////
// Destructor
SubtitlesGrid::~SubtitlesGrid() {
}


//////////////
// Popup menu
void SubtitlesGrid::OnPopupMenu(bool alternate) {
	// Alternate
	if (alternate) {
		// Prepare strings
		wxArrayString strings;
		strings.Add(_("Line Number"));
		strings.Add(_("Layer"));
		strings.Add(_("Start"));
		strings.Add(_("End"));
		strings.Add(_("Style"));
		strings.Add(_("Actor"));
		strings.Add(_("Effect"));
		strings.Add(_("Left"));
		strings.Add(_("Right"));
		strings.Add(_("Vert"));

		// Create Menu
		wxMenu menu;
		for (size_t i=0;i<strings.Count();i++) {
			menu.Append(MENU_SHOW_COL + i,strings[i],_T(""),wxITEM_CHECK)->Check(showCol[i]);
		}
		PopupMenu(&menu);

		return;
	}

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
		//state = (video && video->loaded);
		//menu.Append(MENU_SET_VIDEO_TO_START,_("Jump video to start"),_T("Sets current video time to start time"))->Enable(state);
		//menu.Append(MENU_SET_VIDEO_TO_END,_("Jump video to end"),_T("Sets current video time to end time"))->Enable(state);
		//menu.Append(MENU_SET_START_TO_VIDEO,_("Set start to video"),_T("Sets start times to current video time"))->Enable(state);
		//menu.Append(MENU_SET_END_TO_VIDEO,_("Set end to video"),_T("Sets end times to current video time"))->Enable(state);
		//menu.AppendSeparator();

		// Duplicate selection
		menu.Append(MENU_DUPLICATE,_("&Duplicate"),_T("Duplicate the selected lines"))->Enable(continuous);
		menu.Append(MENU_DUPLICATE_NEXT_FRAME,_("&Duplicate and shift by 1 frame"),_T("Duplicate lines and shift by one frame"))->Enable(continuous && VFR_Output.IsLoaded());
		menu.Append(MENU_SPLIT_BY_KARAOKE,_("Split (by karaoke)"),_T("Uses karaoke timing to split line into multiple smaller lines"))->Enable(sels > 0);

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


////////////////////////////////////
// Process a show/hide column event
void SubtitlesGrid::OnShowColMenu(wxCommandEvent &event) {
	// Set width
	int item = event.GetId()-MENU_SHOW_COL;
	showCol[item] = !showCol[item];

	// Save options
	Options.SetBool(_T("Grid show column ") + IntegerToString(item),showCol[item]);
	Options.Save();

	// Update
	SetColumnWidths();
	Refresh(false);
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
			CopyLines(GetSelection());
		}

		// Cut
		if (Hotkeys.IsPressed(_T("Cut"))) {
			CutLines(GetSelection());
		}

		// Paste
		if (Hotkeys.IsPressed(_T("Paste"))) {
			PasteLines(GetFirstSelRow());
		}

		// Delete
		if (Hotkeys.IsPressed(_T("Grid delete rows"))) {
			DeleteLines(GetSelection());
			return;
		}

		if (continuous) {
			// Duplicate
			if (Hotkeys.IsPressed(_T("Grid duplicate rows"))) {
				DuplicateLines(n,n2,false);
				return;
			}

			// Duplicate and shift
			if (VFR_Output.IsLoaded()) {
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
	wxArrayInt sels = GetSelection();
	DuplicateLines(sels.front(),sels.back());
}


//////////////////////////////////////////////
// Duplicate selection and shift by one frame
void SubtitlesGrid::OnDuplicateNextFrame (wxCommandEvent &WXUNUSED(&event)) {
	wxArrayInt sels = GetSelection();
	DuplicateLines(sels.front(),sels.back(),true);
}


/////////////
// Call swap
void SubtitlesGrid::OnSwap (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	SwapLines(sels.front(),sels.back());
}


///////////////////////////
// Call join (concatenate)
void SubtitlesGrid::OnJoinConcat (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	JoinLines(sels.front(),sels.back(),true);
}


///////////////////////
// Call join (replace)
void SubtitlesGrid::OnJoinReplace (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	JoinLines(sels.front(),sels.back(),false);
}


////////////////
// Adjoin lines
void SubtitlesGrid::OnAdjoin (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	AdjoinLines(sels.front(),sels.back(),true);
}

void SubtitlesGrid::OnAdjoin2 (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	AdjoinLines(sels.front(),sels.back(),false);
}


////////////////////////
// Call join as karaoke
void SubtitlesGrid::OnJoinAsKaraoke (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	JoinAsKaraoke(sels.front(),sels.back());
}


/////////////////////////
// Call split by karaoke
void SubtitlesGrid::OnSplitByKaraoke (wxCommandEvent &event) {
	wxArrayInt sels = GetSelection();
	for (int i = sels.size()-1; i >= 0; i--) {
		SplitLineByKaraoke(sels[i]);
	}
	ass->FlagAsModified();
	CommitChanges();
}


//////////////////////
// Call insert before
void SubtitlesGrid::OnInsertBefore (wxCommandEvent &event) {
	// Find line
	int n = GetFirstSelRow();

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
	int n = GetFirstSelRow();
	int nrows = GetRows();

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
	int n = GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.GetTimeAtFrame(video->frame_n,true);
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
	int n = GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.GetTimeAtFrame(video->frame_n,true);
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
	CopyLines(GetSelection());
}


///////////////////////////////
// Cuts selection to clipboard
void SubtitlesGrid::OnCutLines (wxCommandEvent &WXUNUSED(&event)) {
	CutLines(GetSelection());
}


////////////////////////
// Paste from clipboard
void SubtitlesGrid::OnPasteLines (wxCommandEvent &WXUNUSED(&event)) {
	PasteLines(GetFirstSelRow());
}


///////////////////////////////
// Copy selection to clipboard
void SubtitlesGrid::OnDeleteLines (wxCommandEvent &WXUNUSED(&event)) {
	DeleteLines(GetSelection());
}


//////////////////////////
// Set start to video pos
void SubtitlesGrid::OnSetStartToVideo(wxCommandEvent &event) {
	SetSubsToVideo(true);
}


////////////////////////
// Set end to video pos
void SubtitlesGrid::OnSetEndToVideo(wxCommandEvent &event) {
	SetSubsToVideo(false);
}


//////////////////////////
// Set video pos to start
void SubtitlesGrid::OnSetVideoToStart(wxCommandEvent &event) {
	SetVideoToSubs(true);
}


////////////////////////
// Set video pos to end
void SubtitlesGrid::OnSetVideoToEnd(wxCommandEvent &event) {
	SetVideoToSubs(false);
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
	DeleteLines(GetRangeArray(n+1,n+1));
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

	// Clear grid
	BeginBatch();
	int oldPos = yPos;
	Clear();
	if (keepSelection) yPos = oldPos;

	// Get subtitles
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

	// Finish setting layout
	AdjustScrollbar();
	EndBatch();
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

	// Update mapping
	diagMap[n1] = src1;
	diagPtrMap[n1] = (AssDialogue*) *src1;
	diagMap[n2] = src2;
	diagPtrMap[n2] = (AssDialogue*) *src2;
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
	diagPtrMap.insert(diagPtrMap.begin() + n,(AssDialogue*)(*newIter));
	selMap.insert(selMap.begin() + n,false);

	// Update
	if (update) {
		ass->FlagAsModified();
		CommitChanges();
	}
}


///////////////////////////
// Copy lines to clipboard
void SubtitlesGrid::CopyLines(wxArrayInt target) {
	// Prepare text
	wxString data = _T("");
	AssDialogue *cur;
	int nrows = target.Count();
	bool first = true;
	for (int i=0;i<nrows;i++) {
		if (!first) data += _T("\r\n");
		first = false;
		cur = GetDialogue(target[i]);
		data += cur->GetEntryData();
	}

	// Send to clipboard
	if (wxTheClipboard->Open()) {
		wxTheClipboard->SetData(new wxTextDataObject(data));
		wxTheClipboard->Close();
	}
}


////////////////////
// Cut to clipboard
void SubtitlesGrid::CutLines(wxArrayInt target) {
	CopyLines(target);
	DeleteLines(target);
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
				InsertLine(curdiag,n+inserted,false,false);
				inserted++;
			}
			catch (...) {
			}
		}

		if (inserted > 0) {
			// Commit
			UpdateMaps();
			AdjustScrollbar();
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
void SubtitlesGrid::DeleteLines(wxArrayInt target) {
	// Check if it's wiping file
	int deleted = 0;

	// Delete lines
	int size = target.Count();
	for (int i=0;i<size;i++) {
		delete (*diagMap.at(target[i]));
		ass->Line.erase(diagMap.at(target[i]));
		deleted++;
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

	// Update editbox
	editBox->SetToLine(MID(0,editBox->linen,GetRows()-1));
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
	DeleteLines(GetRangeArray(n1+1,n2));

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
		len2 = (end - start) / 10;

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
	DeleteLines(GetRangeArray(n1+1,n2));

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
		cur = new AssDialogue(GetDialogue(i)->GetEntryData());

		// Shift to next frame
		if (nextFrame) {
			int posFrame = VFR_Output.GetFrameAtTime(cur->End.GetMS(),false) + 1;
			cur->Start.SetMS(VFR_Output.GetTimeAtFrame(posFrame,true));
			cur->End.SetMS(VFR_Output.GetTimeAtFrame(posFrame,false));
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
	if (type != 2) cur->Start.SetMS(VFR_Output.GetTimeAtFrame(len + VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true),true));
	// End
	if (type != 1) cur->End.SetMS(VFR_Output.GetTimeAtFrame(len + VFR_Output.GetFrameAtTime(cur->End.GetMS(),false),false));

	// Update data
	cur->UpdateData();
}


//////////////
// Split line
void SubtitlesGrid::SplitLine(int n,int pos,int mode) {
	// Split
	AssDialogue *n1,*n2;
	n1 = GetDialogue(n);
	n2 = new AssDialogue(n1->GetEntryData());
	InsertLine(n2,n,true,false);

	// Modify text
	wxString orig = n1->Text;
	n1->Text = orig.Left(pos);
	n2->Text = orig.Mid(pos);

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

	// Update editbox and audio
	editBox->SetToLine(n);

	// Commit
	ass->FlagAsModified();
	CommitChanges();
}


//////////////////
// Split line by karaoke
// ---------------------
// Splits the line into as many new lines as there are karaoke syllables,
// timed as the syllables.
// DOES NOT FLAG AS MODIFIED OR COMMIT CHANGES
void SubtitlesGrid::SplitLineByKaraoke(int lineNumber) {
	AssDialogue *line = GetDialogue(lineNumber);
	line->ParseASSTags();

	AssDialogue *nl = new AssDialogue(line->GetEntryData());
	nl->Text = _T("");
	nl->End = nl->Start;
	nl->UpdateData();
	int kcount = 0;
	int start_time = line->Start.GetMS();

	// copying lost of code from automation.cpp here
	// maybe it should be refactored, since a similar proc is also needed in audio_karaoke ?
	for (std::vector<AssDialogueBlock*>::iterator block = line->Blocks.begin(); block != line->Blocks.end(); block++) {
		switch ((*block)->type) {
			case BLOCK_BASE:
				throw wxString(_T("BLOCK_BASE found processing dialogue blocks. This should never happen."));

			case BLOCK_PLAIN:
				nl->Text += (*block)->text;
				break;

			case BLOCK_DRAWING:
				nl->Text += (*block)->text;
				break;

			case BLOCK_OVERRIDE: {
				bool brackets_open = false;
				std::vector<AssOverrideTag*> &tags = (*block)->GetAsOverride(*block)->Tags;

				for (std::vector<AssOverrideTag*>::iterator tag = tags.begin(); tag != tags.end(); tag++) {
					if (!(*tag)->Name.Mid(0,2).CmpNoCase(_T("\\k")) && (*tag)->IsValid()) {
						// it's a karaoke tag
						if (brackets_open) {
							nl->Text += _T("}");
							brackets_open = false;
						}
						if (nl->Text == _T("")) {
							// don't create blank lines
							delete nl;
						} else {
							InsertLine(nl, lineNumber+kcount, true, false);
							kcount++;
						}
						nl = new AssDialogue(line->GetEntryData());
						nl->Text = _T("");
						nl->Start.SetMS(start_time);
						nl->End.SetMS(start_time + (*tag)->Params[0]->AsInt()*10);
						nl->UpdateData();
						start_time = nl->End.GetMS();;
					} else {
						if (!brackets_open) {
							nl->Text += _T("{");
							brackets_open = true;
						}
						nl->Text += (*tag)->ToString();
					}
				}

				if (brackets_open) {
					nl->Text += _T("}");
				}

				break;}

		}
	}

	if (nl->Text == _T("")) {
		// don't create blank lines
		delete nl;
	} else {
		InsertLine(nl, lineNumber+kcount, true, false);
		kcount++;
	}

	// POSSIBLE BUG! If the above code throws an exception, the blocks are never cleared!!
	line->ClearBlocks();

	{
		wxArrayInt oia;
		oia.Add(lineNumber);
		DeleteLines(oia);
	}
}


//////////////////
// Commit changes
// --------------
// This will save the work .ass and refresh it
void SubtitlesGrid::CommitChanges(bool force) {
	if (video->loaded || force) {
		// Check if it's playing
		bool playing = false;
		if (video->IsPlaying) {
			playing = true;
			video->Stop();
		}

		// Export
		wxString workfile = video->GetTempWorkFile();
		ass->Export(workfile);

		if (video->loaded)
			video->RefreshSubtitles();

		// Resume play
		if (playing) video->Play();
	}
	parentFrame->UpdateTitle();
	SetColumnWidths();
	Refresh(false);
}


//////////////////////////
// Set start to video pos
void SubtitlesGrid::SetSubsToVideo(bool start) {
	// Check if it's OK to do it
	if (!VFR_Output.IsLoaded()) return;

	// Get new time
	int ms = VFR_Output.GetTimeAtFrame(video->frame_n,start);

	// Update selection
	wxArrayInt sel = GetSelection();
	AssDialogue *cur;
	int modified =0;
	for (size_t i=0;i<sel.Count();i++) {
		cur = GetDialogue(sel[i]);
		if (cur) {
			modified++;
			if (start) cur->Start.SetMS(ms);
			else cur->End.SetMS(ms);
			cur->UpdateData();
		}
	}

	// Commit
	if (modified) {
		ass->FlagAsModified();
		CommitChanges();
		editBox->Update();
	}
}


//////////////////////////////
// Set video pos to start/end
void SubtitlesGrid::SetVideoToSubs(bool start) {
	wxArrayInt sel = GetSelection();
	if (sel.Count() == 0) return;
	AssDialogue *cur = GetDialogue(sel[0]);
	if (cur) {
		if (start) 
			video->JumpToFrame(VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true));
		else 
			video->JumpToFrame(VFR_Output.GetFrameAtTime(cur->End.GetMS(),false));
	}
}
