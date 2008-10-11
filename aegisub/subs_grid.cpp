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
#include "dialog_paste_over.h"


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
	EVT_MENU(MENU_RECOMBINE,SubtitlesGrid::OnRecombine)
	EVT_MENU(MENU_AUDIOCLIP,SubtitlesGrid::OnAudioClip)
	EVT_MENU_RANGE(MENU_SHOW_COL,MENU_SHOW_COL+15,SubtitlesGrid::OnShowColMenu)
END_EVENT_TABLE()


///////////////
// Constructor
SubtitlesGrid::SubtitlesGrid(FrameMain* parentFr, wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
                        : BaseGrid(parent,id,pos,size,style,name)
{
	// Vars
	byFrame = false;
	ass = NULL;
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
		state = (sels == 1 && VideoContext::Get()->IsLoaded());
		menu.Append(MENU_INSERT_BEFORE_VIDEO,_("Insert at video time (before)"),_T("Inserts a line after current, starting at video time"))->Enable(state);
		menu.Append(MENU_INSERT_AFTER_VIDEO,_("Insert at video time (after)"),_T("Inserts a line after current, starting at video time"))->Enable(state);
		menu.AppendSeparator();

		// Video/time sync
		//state = (video && VideoContext::Get()->IsLoaded());
		//menu.Append(MENU_SET_VIDEO_TO_START,_("Jump video to start"),_T("Sets current video time to start time"))->Enable(state);
		//menu.Append(MENU_SET_VIDEO_TO_END,_("Jump video to end"),_T("Sets current video time to end time"))->Enable(state);
		//menu.Append(MENU_SET_START_TO_VIDEO,_("Set start to video"),_T("Sets start times to current video time"))->Enable(state);
		//menu.Append(MENU_SET_END_TO_VIDEO,_("Set end to video"),_T("Sets end times to current video time"))->Enable(state);
		//menu.AppendSeparator();

		// Duplicate selection
		menu.Append(MENU_DUPLICATE,_("&Duplicate"),_("Duplicate the selected lines"))->Enable(continuous);
		menu.Append(MENU_DUPLICATE_NEXT_FRAME,_("&Duplicate and shift by 1 frame"),_("Duplicate lines and shift by one frame"))->Enable(continuous && VFR_Output.IsLoaded());
		menu.Append(MENU_SPLIT_BY_KARAOKE,_("Split (by karaoke)"),_("Uses karaoke timing to split line into multiple smaller lines"))->Enable(sels > 0);

		// Swaps selection
		state = (sels == 2);
		menu.Append(MENU_SWAP,_("&Swap"),_("Swaps the two selected lines"))->Enable(state);

		// Join selection
		state = (sels >= 2 && continuous);
		menu.Append(MENU_JOIN_CONCAT,_("&Join (concatenate)"),_("Joins selected lines in a single one, concatenating text together"))->Enable(state);
		menu.Append(MENU_JOIN_REPLACE,_("Join (keep first)"),_("Joins selected lines in a single one, keeping text of first and discarding remaining"))->Enable(state);
		menu.Append(MENU_JOIN_AS_KARAOKE,_("Join (as Karaoke)"),_("Joins selected lines in a single one, making each line into a karaoke syllable"))->Enable(state);
		menu.AppendSeparator();

		// Adjoin selection
		menu.Append(MENU_ADJOIN,_("&Make times continuous (change start)"),_("Changes times of subs so start times begin on previous's end time"))->Enable(state);
		menu.Append(MENU_ADJOIN2,_("&Make times continuous (change end)"),_("Changes times of subs so end times begin on next's start time"))->Enable(state);

		// Recombine selection
		state = (sels == 2 || sels == 3) && continuous;
		menu.Append(MENU_RECOMBINE,_("Recombine Lines"),_("Recombine subtitles when they have been split and merged"))->Enable(state);
		menu.AppendSeparator();

		//Make audio clip
		state = parentFrame->audioBox->audioDisplay->loaded==true;
		menu.Append(MENU_AUDIOCLIP,_("Create audio clip"),_("Create an audio clip of the selected line"))->Enable(state);
		menu.AppendSeparator();


		// Copy/cut/paste
		menu.Append(MENU_COPY,_("&Copy"),_("Copies selected lines to clipboard"));
		menu.Append(MENU_CUT,_("C&ut"),_("Cuts selected lines to clipboard"));
		menu.Append(MENU_PASTE,_("&Paste"),_("Paste lines from clipboard"));
		menu.AppendSeparator();

		// Delete
		menu.Append(MENU_DELETE,_("Delete"),_("Delete currently selected lines"));

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
#ifdef __APPLE__
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_metaDown,event.m_altDown,event.m_shiftDown);
#else
	Hotkeys.SetPressed(event.GetKeyCode(),event.m_controlDown,event.m_altDown,event.m_shiftDown);
#endif

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
			return;
		}

		// Cut
		if (Hotkeys.IsPressed(_T("Cut"))) {
			CutLines(GetSelection());
			return;
		}

		// Paste
		if (Hotkeys.IsPressed(_T("Paste"))) {
			PasteLines(GetFirstSelRow());
			return;
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
	BeginBatch();
	wxArrayInt sels = GetSelection();
	DuplicateLines(sels.front(),sels.back());
	EndBatch();
}


//////////////////////////////////////////////
// Duplicate selection and shift by one frame
void SubtitlesGrid::OnDuplicateNextFrame (wxCommandEvent &WXUNUSED(&event)) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	DuplicateLines(sels.front(),sels.back(),true);
	EndBatch();
}


/////////////
// Call swap
void SubtitlesGrid::OnSwap (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	SwapLines(sels.front(),sels.back());
	EndBatch();
}


///////////////////////////
// Call join (concatenate)
void SubtitlesGrid::OnJoinConcat (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	JoinLines(sels.front(),sels.back(),true);
	EndBatch();
}


///////////////////////
// Call join (replace)
void SubtitlesGrid::OnJoinReplace (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	JoinLines(sels.front(),sels.back(),false);
	EndBatch();
}


////////////////
// Adjoin lines
void SubtitlesGrid::OnAdjoin (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	AdjoinLines(sels.front(),sels.back(),true);
	EndBatch();
}

void SubtitlesGrid::OnAdjoin2 (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	AdjoinLines(sels.front(),sels.back(),false);
	EndBatch();
}


////////////////////////
// Call join as karaoke
void SubtitlesGrid::OnJoinAsKaraoke (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	JoinAsKaraoke(sels.front(),sels.back());
	EndBatch();
}


/////////////////////////
// Call split by karaoke
void SubtitlesGrid::OnSplitByKaraoke (wxCommandEvent &event) {
	BeginBatch();
	wxArrayInt sels = GetSelection();
	for (int i = sels.size()-1; i >= 0; i--) {
		SplitLineByKaraoke(sels[i]);
	}
	ass->FlagAsModified(_("splitting"));
	CommitChanges();
	EndBatch();
}


//////////////////////
// Call insert before
void SubtitlesGrid::OnInsertBefore (wxCommandEvent &event) {
	BeginBatch();
	// Find line
	int n = GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	if (n == 0) {
		def->Start.SetMS(0);
		def->End = GetDialogue(n)->Start;
	}
	else if (GetDialogue(n-1)->End.GetMS() > GetDialogue(n)->Start.GetMS()) {
		def->Start.SetMS(GetDialogue(n)->Start.GetMS()-Options.AsInt(_T("Timing Default Duration")));
		def->End = GetDialogue(n)->Start;
	}
	else {
		def->Start = GetDialogue(n-1)->End;
		def->End = GetDialogue(n)->Start;
	}
	if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+Options.AsInt(_T("Timing Default Duration")));
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,false);
	SelectRow(n);
	editBox->SetToLine(n);
	EndBatch();
}


/////////////////////
// Call insert after
void SubtitlesGrid::OnInsertAfter (wxCommandEvent &event) {
	BeginBatch();
	// Find line
	int n = GetFirstSelRow();
	int nrows = GetRows();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	if (n == nrows-1) {
		def->Start = GetDialogue(n)->End;
		def->End = GetDialogue(n)->End;
		def->End.SetMS(def->End.GetMS()+Options.AsInt(_T("Timing Default Duration")));
	}
	else {
		def->Start = GetDialogue(n)->End;
		def->End = GetDialogue(n+1)->Start;
	}
	if (def->End.GetMS() < def->Start.GetMS()) def->End.SetMS(def->Start.GetMS()+Options.AsInt(_T("Timing Default Duration")));
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,true);
	SelectRow(n+1);
	editBox->SetToLine(n+1);
	EndBatch();
}


/////////////////////////////////
// Call insert before with video
void SubtitlesGrid::OnInsertBeforeVideo (wxCommandEvent &event) {
	BeginBatch();
	// Find line
	int n = GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.GetTimeAtFrame(VideoContext::Get()->GetFrameN(),true);
	def->Start.SetMS(video_ms);
	def->End.SetMS(video_ms+Options.AsInt(_T("Timing Default Duration")));
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,false);
	SelectRow(n);
	editBox->SetToLine(n);
	EndBatch();
}


////////////////////////////////
// Call insert after with video
void SubtitlesGrid::OnInsertAfterVideo (wxCommandEvent &event) {
	BeginBatch();
	// Find line
	int n = GetFirstSelRow();

	// Create line to add
	AssDialogue *def = new AssDialogue;
	int video_ms = VFR_Output.GetTimeAtFrame(VideoContext::Get()->GetFrameN(),true);
	def->Start.SetMS(video_ms);
	def->End.SetMS(video_ms+Options.AsInt(_T("Timing Default Duration")));
	def->Style = GetDialogue(n)->Style;

	// Insert it
	InsertLine(def,n,true);
	SelectRow(n+1);
	editBox->SetToLine(n+1);
	EndBatch();
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
	BeginBatch();
	DeleteLines(GetSelection());
	EndBatch();
}


//////////////////////////
// Set start to video pos
void SubtitlesGrid::OnSetStartToVideo(wxCommandEvent &event) {
	BeginBatch();
	SetSubsToVideo(true);
	EndBatch();
}


////////////////////////
// Set end to video pos
void SubtitlesGrid::OnSetEndToVideo(wxCommandEvent &event) {
	BeginBatch();
	SetSubsToVideo(false);
	EndBatch();
}


//////////////////////////
// Set video pos to start
void SubtitlesGrid::OnSetVideoToStart(wxCommandEvent &event) {
	BeginBatch();
	SetVideoToSubs(true);
	EndBatch();
}


////////////////////////
// Set video pos to end
void SubtitlesGrid::OnSetVideoToEnd(wxCommandEvent &event) {
	BeginBatch();
	SetVideoToSubs(false);
	EndBatch();
}


//////////////
// Recombine
void SubtitlesGrid::OnRecombine(wxCommandEvent &event) {
	// Get selection
	bool cont;
	wxArrayInt sel = GetSelection(&cont);
	int nSel = sel.Count();
	if ((nSel != 2 && nSel != 3) || !cont) throw _T("Invalid selection for recombining");
	int n = sel[0];

	// Get dialogues
	AssDialogue *n1,*n2,*n3;
	n1 = GetDialogue(n);
	n2 = GetDialogue(n+1);

	// 1,1+2,2 -> 1,2
	if (nSel == 3) {
		n3 = GetDialogue(n+2);
		n1->End = n2->End;
		n3->Start = n2->Start;
		n1->UpdateData();
		n3->UpdateData();
		DeleteLines(GetRangeArray(n+1,n+1));
	}

	// 2 Line recombine
	else {
		// Trim dialogues
		n1->Text.Trim(true).Trim(false);
		n2->Text.Trim(true).Trim(false);

		// Detect type
		int type = -1;
		bool invert = false;
		if (n1->Text.Right(n2->Text.Length()) == n2->Text) type = 0;
		else if (n1->Text.Left(n2->Text.Length()) == n2->Text) { type = 1; invert = true; }
		else if (n2->Text.Left(n1->Text.Length()) == n1->Text) type = 1;
		else if (n2->Text.Right(n1->Text.Length()) == n1->Text) { type = 0; invert = true; }
		else {
			// Unknown type
			parentFrame->StatusTimeout(_T("Unable to recombine: Neither line is a suffix of the other one."));
			return;
		}

		// Invert?
		if (invert) {
			n3 = n1;
			n1 = n2;
			n2 = n3;
			n3 = NULL;
		}

		// 1+2,2 -> 1,2
		if (type == 0) {
			n1->Text = n1->Text.SubString(0, n1->Text.Length() - n2->Text.Length() - 1).Trim(true).Trim(false);
			while (n1->Text.Left(2) == _T("\\N") || n1->Text.Left(2) == _T("\\n")) n1->Text = n1->Text.Mid(2);
			while (n1->Text.Right(2) == _T("\\N") || n1->Text.Right(2) == _T("\\n")) n1->Text = n1->Text.Mid(0,n1->Text.Length()-2);
			n2->Start = n1->Start;
		}

		// 1,1+2 -> 1,2
		else if (type == 1) {
			n2->Text = n2->Text.Mid(n1->Text.Length()).Trim(true).Trim(false);
			while (n2->Text.Left(2) == _T("\\N") || n2->Text.Left(2) == _T("\\n")) n2->Text = n2->Text.Mid(2);
			while (n2->Text.Right(2) == _T("\\N") || n2->Text.Right(2) == _T("\\n")) n2->Text = n2->Text.Mid(0,n2->Text.Length()-2);
			n1->End = n2->End;
		}

		// Commit
		n1->UpdateData();
		n2->UpdateData();
		ass->FlagAsModified(_("combining"));
		CommitChanges();
	}

	// Adjus scrollbar
	AdjustScrollbar();
}



//////////////
// Export audio clip of line
void SubtitlesGrid::OnAudioClip(wxCommandEvent &event) {
	int64_t num_samples,start=0,end=0,temp;
	AudioDisplay *audioDisplay = parentFrame->audioBox->audioDisplay;
	AudioProvider *provider = audioDisplay->provider;
	AssDialogue *cur;
	wxArrayInt sel = GetSelection();

	num_samples = provider->GetNumSamples();
	
	for(unsigned int i=0;i!=sel.GetCount();i++) {
		cur = GetDialogue(sel[i]);
		
		temp = audioDisplay->GetSampleAtMS(cur->Start.GetMS());
		start = (i==0||temp<start)?temp:start;
		temp = audioDisplay->GetSampleAtMS(cur->End.GetMS());
		end = (i==0||temp>end)?temp:end;
	}

	if (start > num_samples) {
		wxMessageBox(_("The starting point is beyond the length of the audio loaded."),_("Error"));
		return;
	}
	if (start==end||end==0) {
		wxMessageBox(_("There is no audio to save."),_("Error"));
		return;
	}

	end=(end>num_samples)?num_samples:end;


	wxString filename = wxFileSelector(_("Save audio clip"),_T(""),_T(""),_T("wav"),_T(""),wxFD_SAVE|wxFD_OVERWRITE_PROMPT,this);

	if (!filename.empty()) {
		std::ofstream outfile(filename.mb_str(wxConvLocal),std::ios::binary);
		
		size_t bufsize=(end-start)*provider->GetChannels()*provider->GetBytesPerSample();
		int intval;
		short shortval;

		outfile << "RIFF";
		outfile.write((char*)&(intval=bufsize+36),4);
		outfile<< "WAVEfmt ";
		outfile.write((char*)&(intval=16),4);
		outfile.write((char*)&(shortval=1),2);
		outfile.write((char*)&(shortval=provider->GetChannels()),2);
		outfile.write((char*)&(intval=provider->GetSampleRate()),4);
		outfile.write((char*)&(intval=provider->GetSampleRate()*provider->GetChannels()*provider->GetBytesPerSample()),4);
		outfile.write((char*)&(intval=provider->GetChannels()*provider->GetBytesPerSample()),2);
		outfile.write((char*)&(shortval=provider->GetBytesPerSample()<<3),2);
		outfile << "data";
		outfile.write((char*)&bufsize,4);

		//samples per read
		size_t spr = 65536/(provider->GetBytesPerSample()*provider->GetChannels());
		for(int64_t i=start;i<end;i+=spr) {
			int len=(i+(int64_t)spr>end)?(end-i):spr;
			bufsize=len*(provider->GetBytesPerSample()*provider->GetChannels());
			void *buf = malloc(bufsize);
			if (buf) {
				provider->GetAudio(buf,i,len);
				outfile.write((char*)buf,bufsize);
				free(buf);
			}
			else if (spr>128) {
				//maybe we can allocate a smaller amount of memory
				i-=spr; //effectively redo this loop again
				spr=128;
			}
			else {
				wxMessageBox(_("Couldn't allocate memory."),_("Error"),wxICON_ERROR | wxOK);
				break; // don't return, we need to close the file
			}
		}
		
		outfile.close();
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

	// Clear from video
	VideoContext::Get()->curLine = NULL;

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
		if (dontModify) AssFile::StackPush(_("load"));
		else ass->FlagAsModified(_("load"));
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
	ass->FlagAsModified(_("swap lines"));
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
		ass->FlagAsModified(_("line insertion"));
		CommitChanges();
		AdjustScrollbar();
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
	BeginBatch();
	CopyLines(target);
	DeleteLines(target);
	EndBatch();
}


//////////////////////////////
// Paste lines from clipboard
void SubtitlesGrid::PasteLines(int n,bool pasteOver) {
	BeginBatch();

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
		// Insert data
		int inserted = 0;
		bool asked = false;
		wxArrayInt pasteOverOptions;
		wxStringTokenizer token (data,_T("\r\n"),wxTOKEN_STRTOK);
		while (token.HasMoreTokens()) {
			// Convert data into an AssDialogue
			wxString curdata = token.GetNextToken();
			curdata.Trim(true);
			curdata.Trim(false);
			AssDialogue *curdiag;
			try { 
				// Try to interpret the line as an ASS line
				curdiag = new AssDialogue(curdata);
			}
			catch (...) {
				// Line didn't parse correcly, assume it's plain text that
				// should be pasted in the Text field only
				curdiag = new AssDialogue();
				curdiag->Text = curdata;
				// Make sure pasted plain-text lines always are blank-timed
				curdiag->Start.SetMS(0);
				curdiag->End.SetMS(0);
			}

			// Paste over
			if (pasteOver) {
				if (n+inserted < GetRows()) {
					// Get list of options to paste over, if not asked yet
					if (asked == false) {
						asked = true;
						DialogPasteOver diag(NULL);
						if (!diag.ShowModal()) {
							delete curdiag;
							return;
						}
						pasteOverOptions = diag.GetOptions();
					}

					// Paste over
					AssDialogue *target = GetDialogue(n+inserted);
					if (pasteOverOptions[0]) target->Layer = curdiag->Layer;
					if (pasteOverOptions[1]) target->Start = curdiag->Start;
					if (pasteOverOptions[2]) target->End = curdiag->End;
					if (pasteOverOptions[3]) target->Style = curdiag->Style;
					if (pasteOverOptions[4]) target->Actor = curdiag->Actor;
					if (pasteOverOptions[5]) target->Margin[0] = curdiag->Margin[0];
					if (pasteOverOptions[6]) target->Margin[1] = curdiag->Margin[1];
					if (pasteOverOptions[7]) target->Margin[2] = curdiag->Margin[2];
					//if (pasteOverOptions[8]) target->Margin[3] = curdiag->Margin[3];
					if (pasteOverOptions[8]) target->Effect = curdiag->Effect;
					if (pasteOverOptions[9]) target->Text = curdiag->Text;
				}
				delete curdiag;
			}

			// Paste normally
			else InsertLine(curdiag,n+inserted,false,false);

			// Increment insertion
			inserted++;
		}

		// Update data post-insertion
		if (inserted > 0) {
			// Commit
			UpdateMaps();
			AdjustScrollbar();
			ass->FlagAsModified(_("paste"));
			CommitChanges();

			// Set selection
			if (!pasteOver) {
				SelectRow(n);
				for (int i=n+1;i<n+inserted;i++) {
					SelectRow(i,true);
				}
				editBox->SetToLine(n);
			}
		}
	}

	// Done
	EndBatch();
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
	ass->FlagAsModified(_("delete"));
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
	bool gottime = false;
	for (int i=n1;i<=n2;i++) {
		// Get start and end time of current line
		cur = GetDialogue(i);
		start = cur->Start.GetMS();
		end = cur->End.GetMS();

		// Don't take the timing of zero lines
		if (start != 0 || end != 0) {
			if (start < min_ms) min_ms = start;
			if (end > max_ms) max_ms = end;
			gottime = true;
		}

		// Set text
		if (concat || !gotfirst) {
			if (gotfirst) finalText += _T("\\N");
			gotfirst = true;
			finalText += cur->Text;
		}
	}

	// If it didn't get any times, then it's probably because they were all 0 lines.
	if (!gottime) {
		min_ms = 0;
		max_ms = 0;
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
	AssFile::top->FlagAsModified(_("adjoin"));
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
	int firststart = 0;
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
void SubtitlesGrid::SplitLine(int n,int pos,int mode,wxString textIn) {
	// Split
	AssDialogue *n1,*n2;
	// No textIn? Get saved text
	if (textIn.IsEmpty()) { 
		n1 = GetDialogue(n);
		n2 = new AssDialogue(n1->GetEntryData());
	}
	// Otherwise use textIn
	else { 
		n1 = GetDialogue(n);
		n1->Text = textIn;
		n2 = new AssDialogue(n1->GetEntryData());
	}
	InsertLine(n2,n,true,false);

	// Modify text
	wxString orig = n1->Text;
	n1->Text = orig.Left(pos).Trim(true); // Trim off trailing whitespace
	n2->Text = orig.Mid(pos).Trim(false); // Trim off leading whitespace

	// Modify time
	if (mode == 1) {
		double splitPos = double(pos)/orig.Length();
		int splitTime = (int)((n1->End.GetMS() - n1->Start.GetMS())*splitPos) + n1->Start.GetMS();
		n1->End.SetMS(splitTime);
		n2->Start.SetMS(splitTime);
	}

	// Update data
	n1->UpdateData();
	n2->UpdateData();

	// Update editbox and audio
	editBox->SetToLine(n);

	// Commit
	ass->FlagAsModified(_("split"));
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
		switch ((*block)->GetType()) {
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

	// POSSIBLE BUG/LEAK! If the above code throws an exception, the blocks are never cleared!!
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
void SubtitlesGrid::CommitChanges(bool force,bool videoOnly) {
	if (VideoContext::Get()->IsLoaded() || force) {
		// Check if it's playing
		bool playing = false;
		if (VideoContext::Get()->IsPlaying()) {
			playing = true;
			VideoContext::Get()->Stop();
		}

		// Export
		//wxString workfile = VideoContext::Get()->GetTempWorkFile();
		//ass->Export(workfile);

		// Update video
		if (VideoContext::Get()->IsLoaded()) VideoContext::Get()->Refresh(false,true);

		// Resume play
		if (playing) VideoContext::Get()->Play();
	}

	if (!videoOnly) {
		// Autosave if option is enabled
		if (Options.AsBool(_T("Auto Save on Every Change"))) {
			if (ass->IsModified() && !ass->filename.IsEmpty()) parentFrame->SaveSubtitles(false);
		}

		// Update parent frame
		parentFrame->UpdateTitle();
		SetColumnWidths();
		Refresh(false);
	}
}


//////////////////////////
// Set start to video pos
void SubtitlesGrid::SetSubsToVideo(bool start) {
	// Check if it's OK to do it
	if (!VFR_Output.IsLoaded()) return;

	// Get new time
	int ms = VFR_Output.GetTimeAtFrame(VideoContext::Get()->GetFrameN(),start);

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
		ass->FlagAsModified(_("timing"));
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
			VideoContext::Get()->JumpToFrame(VFR_Output.GetFrameAtTime(cur->Start.GetMS(),true));
		else 
			VideoContext::Get()->JumpToFrame(VFR_Output.GetFrameAtTime(cur->End.GetMS(),false));
	}
}


/////////////////////////////////////////////////////////////////////
// Retrieve a list of selected lines in the actual ASS file
// (ie. not as displayed in the grid but as represented in the file)
std::vector<int> SubtitlesGrid::GetAbsoluteSelection() {
	std::vector<int> result;
	result.reserve(GetNumberSelection());

	int nrows = GetRows();
	for (int i = 0; i != nrows; ++i) {
		if (selMap.at(i)) {
			entryIter l = diagMap.at(i);
			int n = 0;
			for (std::list<AssEntry*>::iterator j = ass->Line.begin(); j != ass->Line.end(); ++j, ++n) {
				if (j == l) {
					result.push_back(n);
					break;
				}
			}
		}
	}

	return result;
}


/////////////////////////////////////////////////////////////////////
// Update list of selected lines from absolute selection
// selection vector must be sorted
void SubtitlesGrid::SetSelectionFromAbsolute(std::vector<int> &selection) {

	int nrows = GetRows();
	std::list<AssEntry*>::iterator j = ass->Line.begin();
	int index = 0;
	for (int i = 0; i != nrows; ++i) {
		entryIter l = diagMap.at(i);
		while(j != l && j != ass->Line.end()) ++j, ++index;
		if(j == l && binary_search(selection.begin(), selection.end(), index)) {
			selMap[i] = true;
		} else selMap[i] = false;
	}
}
