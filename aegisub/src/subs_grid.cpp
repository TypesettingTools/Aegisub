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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file subs_grid.cpp
/// @brief Subtitles grid control in main window
/// @ingroup main_ui
///

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#include <utility>

#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#endif

#include "command/command.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "include/aegisub/audio_provider.h"

#include "ass_file.h"
#include "ass_karaoke.h"
#include "ass_override.h"
#include "ass_style.h"
#include "include/aegisub/audio_provider.h"
#include "selection_controller.h"
#include "audio_controller.h"
#include "audio_box.h"
#include "charset_conv.h"
#include "dialog_paste_over.h"
#include "frame_main.h"
#include "main.h"
#include "subs_edit_box.h"
#include "subs_grid.h"
#include "utils.h"
#include "video_context.h"

BEGIN_EVENT_TABLE(SubtitlesGrid, BaseGrid)
	EVT_KEY_DOWN(SubtitlesGrid::OnKeyDown)
	EVT_MENU_RANGE(MENU_SHOW_COL,MENU_SHOW_COL+15,SubtitlesGrid::OnShowColMenu)
END_EVENT_TABLE()

/// @brief Constructor 
/// @param parentFr 
/// @param parent   
/// @param id       
/// @param pos      
/// @param size     
/// @param style    
/// @param name     
SubtitlesGrid::SubtitlesGrid(wxWindow *parent, agi::Context *context,  const wxSize& size, long style, const wxString& name)
: BaseGrid(parent,context,size,style,name)
, seekListener(context->videoController->AddSeekListener(&SubtitlesGrid::Refresh, this, false, (const wxRect *)NULL))
{
	OnHighlightVisibleChange(*OPT_GET("Subtitle/Grid/Highlight Subtitles in Frame"));
	OPT_SUB("Subtitle/Grid/Highlight Subtitles in Frame", &SubtitlesGrid::OnHighlightVisibleChange, this);
	OPT_SUB("Subtitle/Grid/Hide Overrides", std::tr1::bind(&SubtitlesGrid::Refresh, this, false, (const wxRect*)0));
	context->ass->AddCommitListener(&SubtitlesGrid::OnSubtitlesCommit, this);
	context->ass->AddFileOpenListener(&SubtitlesGrid::OnSubtitlesOpen, this);

	Bind(wxEVT_COMMAND_MENU_SELECTED, &SubtitlesGrid::OnCommand, this);
}

/// @brief Destructor 
SubtitlesGrid::~SubtitlesGrid() {
	ClearMaps();
}

void SubtitlesGrid::OnSubtitlesCommit(int type) {
	if (type == AssFile::COMMIT_FULL)
		UpdateMaps();
	else if (type == AssFile::COMMIT_UNDO)
		UpdateMaps(true);

	if (type == AssFile::COMMIT_TIMES) {
		// Refresh just the audio times columns
		RefreshRect(wxRect(colWidth[0] + colWidth[1], 0, colWidth[2] + colWidth[3], GetClientSize().GetHeight()), false);
	}
	else {
		SetColumnWidths();
		Refresh(false);
	}
}

void SubtitlesGrid::OnSubtitlesOpen() {
	BeginBatch();
	ClearMaps();
	UpdateMaps();

	if (GetRows()) {
		SetActiveLine(GetDialogue(0));
		SelectRow(0);
	}
	EndBatch();
	SetColumnWidths();
}

void SubtitlesGrid::OnCommand(wxCommandEvent& event) {
	int id = event.GetId();
	if (id < MENU_SHOW_COL)
		cmd::call(context, id);
	else
		event.Skip();
}

static inline void append_command(wxMenu &menu, const char *name, bool state) {
	cmd::Command *c = cmd::get(name);
	menu.Append(cmd::id(name), c->StrMenu(), c->StrHelp())->Enable(state);
}

/// @brief Popup menu 
/// @param alternate 
void SubtitlesGrid::OnPopupMenu(bool alternate) {
	// Alternate
	if (alternate) {
		const wxString strings[] = {
			_("Line Number"),
			_("Layer"),
			_("Start"),
			_("End"),
			_("Style"),
			_("Actor"),
			_("Effect"),
			_("Left"),
			_("Right"),
			_("Vert"),
		};

		// Create Menu
		wxMenu menu;
		for (size_t i=0;i<columns;i++) {
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
		append_command(menu, "subtitle/insert/before", state);
		append_command(menu, "subtitle/insert/after", state);
		state = (sels == 1 && context->videoController->IsLoaded());
		append_command(menu, "subtitle/insert/before/videotime", state);
		append_command(menu, "subtitle/insert/after/videotime", state);
		menu.AppendSeparator();

		// Duplicate selection
		append_command(menu, "edit/line/duplicate", state);
		append_command(menu, "edit/line/duplicate/shift", state);
		append_command(menu, "edit/line/join/as_karaoke", state);

		// Swaps selection
		state = (sels == 2);
		//append_command(menu, "MENU_SWAP", state);

		// Join selection
		state = (sels >= 2 && continuous);
		append_command(menu, "edit/line/join/concatenate", state);
		append_command(menu, "edit/line/join/keep_first", state);
		append_command(menu, "edit/line/join/as_karaoke", state);
		menu.AppendSeparator();

		// Adjoin selection
		state = (sels >= 1 && continuous);
		append_command(menu, "time/continuous/start", state);
		append_command(menu, "time/continuous/end", state);

		// Recombine selection
		state = (sels > 1);
		append_command(menu, "edit/line/recombine", state);
		menu.AppendSeparator();

		//Make audio clip
		state = context->audioController->IsAudioOpen();
		//append_command(menu, "MENU_AUDIOCLIP", state);
		menu.AppendSeparator();


		// Copy/cut/paste
		append_command(menu, "edit/line/copy", true);
		append_command(menu, "edit/line/cut", true);
		append_command(menu, "edit/line/paste", true);
		menu.AppendSeparator();

		// Delete
		append_command(menu, "edit/line/delete", true);

		PopupMenu(&menu);
	}
}

/// @brief Process a show/hide column event 
/// @param event 
void SubtitlesGrid::OnShowColMenu(wxCommandEvent &event) {
	int item = event.GetId()-MENU_SHOW_COL;
	showCol[item] = !showCol[item];

	std::vector<bool> map(showCol, showCol + columns);
	OPT_SET("Subtitle/Grid/Column")->SetListBool(map);

	// Update
	SetColumnWidths();
	Refresh(false);
}

/// @brief Process keyboard events 
/// @param event 
void SubtitlesGrid::OnKeyDown(wxKeyEvent &event) {

	hotkey::check("Subtitle Grid", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers());
	event.StopPropagation();

//H Fix below.
/*
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
				//editBox->SetToLine(n+1);
			}
			return;
		}

		// Move up
		if (Hotkeys.IsPressed(_T("Grid move row up"))) {
			if (n > 0) {
				SwapLines(n-1,n);
				SelectRow(n-1);
				//editBox->SetToLine(n-1);
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
			if (context->videoController->TimecodesLoaded()) {
				if (Hotkeys.IsPressed(_T("Grid duplicate and shift one frame"))) {
					DuplicateLines(n,n2,true);
					return;
				}
			}
		}
	}

	event.Skip();
*/
}

static void trim_text(AssDialogue *diag) {
	static wxRegEx start(L"^( |\\t|\\\\[nNh])+");
	static wxRegEx end(L"( |\\t|\\\\[nNh])+$");
	start.ReplaceFirst(&diag->Text, "");
	end.ReplaceFirst(&diag->Text, "");
}
static void expand_times(AssDialogue *src, AssDialogue *dst) {
	dst->Start.SetMS(std::min(dst->Start.GetMS(), src->Start.GetMS()));
	dst->End.SetMS(std::max(dst->End.GetMS(), src->End.GetMS()));
}

/// @brief Recombine 
void SubtitlesGrid::RecombineLines() {
	using namespace std;

	Selection selectedSet = GetSelectedSet();
	if (selectedSet.size() < 2) return;

	AssDialogue *activeLine = GetActiveLine();

	vector<AssDialogue*> sel;
	sel.reserve(selectedSet.size());
	copy(selectedSet.begin(), selectedSet.end(), back_inserter(sel));
	for_each(sel.begin(), sel.end(), trim_text);
	sort(sel.begin(), sel.end(), &AssFile::CompStart);

	typedef vector<AssDialogue*>::iterator diag_iter;
	diag_iter end = sel.end() - 1;
	for (diag_iter cur = sel.begin(); cur != end; ++cur) {
		AssDialogue *d1 = *cur;
		diag_iter d2 = cur + 1;

		// 1, 1+2 (or 2+1), 2 gets turned into 1, 2, 2 so kill the duplicate
		if (d1->Text == (*d2)->Text) {
			expand_times(d1, *d2);
			delete d1;
			context->ass->Line.remove(d1);
			continue;
		}

		// 1, 1+2, 1 turns into 1, 2, [empty]
		if (d1->Text.empty()) {
			delete d1;
			context->ass->Line.remove(d1);
			continue;
		}

		// 1, 1+2
		while (d2 <= end && (*d2)->Text.StartsWith(d1->Text, &(*d2)->Text)) {
			expand_times(*d2, d1);
			trim_text(*d2);
			++d2;
		}

		// 1, 2+1
		while (d2 <= end && (*d2)->Text.EndsWith(d1->Text, &(*d2)->Text)) {
			expand_times(*d2, d1);
			trim_text(*d2);
			++d2;
		}

		// 1+2, 2
		while (d2 <= end && d1->Text.EndsWith((*d2)->Text, &d1->Text)) {
			expand_times(d1, *d2);
			trim_text(d1);
			++d2;
		}

		// 2+1, 2
		while (d2 <= end && d1->Text.StartsWith((*d2)->Text, &d1->Text)) {
			expand_times(d1, *d2);
			trim_text(d1);
			++d2;
		}
	}

	context->ass->Commit(_("combining"));

	// Remove now non-existent lines from the selection
	Selection lines;
	transform(context->ass->Line.begin(), context->ass->Line.end(), inserter(lines, lines.begin()), cast<AssDialogue*>());
	Selection newSel;
	set_intersection(lines.begin(), lines.end(), selectedSet.begin(), selectedSet.end(), inserter(newSel, newSel.begin()));

	if (newSel.empty()) return;

	// Restore selection
	SetSelectedSet(newSel);
	if (find(newSel.begin(), newSel.end(), activeLine) == newSel.end()) {
		activeLine = *newSel.begin();
	}
	SetActiveLine(activeLine);
}

/// @brief Export audio clip of line 
/*void SubtitlesGrid::OnAudioClip(wxCommandEvent &) {
	int64_t num_samples,start=0,end=0,temp;
	AudioController *audioController = context->audioController;
	const AudioProvider *provider = audioController->GetAudioProvider();
	AssDialogue *cur;
	wxArrayInt sel = GetSelection();

	num_samples = provider->GetNumSamples();
	
	for(unsigned int i=0;i!=sel.GetCount();i++) {
		cur = GetDialogue(sel[i]);
		
		temp = audioController->SamplesFromMilliseconds(cur->Start.GetMS());
		start = (i==0||temp<start)?temp:start;
		temp = audioController->SamplesFromMilliseconds(cur->End.GetMS());
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
		std::ofstream outfile(filename.mb_str(csConvLocal),std::ios::binary);

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
}*/

/// @brief Swaps two lines 
/// @param n1 
/// @param n2 
void SubtitlesGrid::SwapLines(int n1,int n2) {
	AssDialogue *dlg1 = GetDialogue(n1);
	AssDialogue *dlg2 = GetDialogue(n2);
	if (n1 == 0 || n2 == 0) return;
	
	std::swap(*dlg1, *dlg2);

	context->ass->Commit(_("swap lines"));
}

/// @brief Insert a line 
/// @param line   
/// @param n      
/// @param after  
/// @param update 
void SubtitlesGrid::InsertLine(AssDialogue *line,int n,bool after,bool update) {
	AssDialogue *rel_line = GetDialogue(n);
	entryIter pos = std::find(context->ass->Line.begin(), context->ass->Line.end(), rel_line);
	if (after) ++pos;

	entryIter newIter = context->ass->Line.insert(pos,line);

	// Update
	if (update) {
		context->ass->Commit(_("line insertion"));
	}
	else {
		UpdateMaps();
	}
}

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

void SubtitlesGrid::CutLines(wxArrayInt target) {
	BeginBatch();
	CopyLines(target);
	DeleteLines(target);
	EndBatch();
}

/// @brief Paste lines from clipboard
/// @param n         
/// @param pasteOver 
void SubtitlesGrid::PasteLines(int n,bool pasteOver) {
	BeginBatch();

	// Prepare text
	wxString data;

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
		std::vector<bool> pasteOverOptions;
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
				// Line didn't parse correctly, assume it's plain text that
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
					if (pasteOverOptions.empty()) {
						DialogPasteOver diag(NULL, pasteOverOptions);
						if (!diag.ShowModal()) {
							delete curdiag;
							return;
						}
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
			context->ass->Commit(_("paste"), pasteOver ? AssFile::COMMIT_TEXT : AssFile::COMMIT_FULL);

			// Set selection
			if (!pasteOver) {
				Selection newsel;
				for (int i=n;i<n+inserted;i++) {
					newsel.insert(GetDialogue(i));
				}
				SetSelectedSet(newsel);
				SetActiveLine(GetDialogue(GetFirstSelRow()));
			}
		}
	}
	EndBatch();
}

void SubtitlesGrid::DeleteLines(wxArrayInt target, bool flagModified) {
	entryIter before_first = std::find_if(context->ass->Line.begin(), context->ass->Line.end(), cast<AssDialogue*>()); --before_first;
	int old_active_line_index = GetDialogueIndex(GetActiveLine());

	int row = -1;
	int deleted = 0;
	for (entryIter cur = context->ass->Line.begin(); cur != context->ass->Line.end();) {
		if (dynamic_cast<AssDialogue*>(*cur) && ++row == target[deleted]) {
			cur = context->ass->Line.erase(cur);
			++deleted;
			if (deleted == target.size()) break;
		}
		else {
			++cur;
		}
	}

	// Add default line if file was wiped
	if (GetRows() == deleted) {
		AssDialogue *def = new AssDialogue;
		++before_first;
		context->ass->Line.insert(before_first, def);
		old_active_line_index = 0;
	}

	if (flagModified) {
		context->ass->Commit(_("delete"));
	}
	else {
		UpdateMaps();
	}
}

void SubtitlesGrid::JoinLines(int n1,int n2,bool concat) {
	int min_ms = INT_MAX;
	int max_ms = INT_MIN;
	wxString finalText;

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

	// Delete remaining lines (this will auto commit)
	DeleteLines(GetRangeArray(n1+1,n2), false);

	context->ass->Commit(_("join lines"));

	// Select new line
	SetActiveLine(cur);
	SelectRow(n1);
}

void SubtitlesGrid::AdjoinLines(int n1,int n2,bool setStart) {
	if (n1 == n2) {
		if (setStart) {
			--n1;
		}
		else {
			++n2;
		}
	}
	// Set start
	if (setStart) {
		AssDialogue *prev = GetDialogue(n1);
		AssDialogue *cur;
		for (int i=n1+1;i<=n2;i++) {
			cur = GetDialogue(i);
			if (!cur) return;
			cur->Start = prev->End;
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
			cur = next;
		}
	}

	context->ass->Commit(_("adjoin"));
}

void SubtitlesGrid::JoinAsKaraoke(int n1,int n2) {
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

	// Delete remaining lines (this will auto commit)
	DeleteLines(GetRangeArray(n1+1,n2), false);

	context->ass->Commit(_("join as karaoke"));

	// Select new line
	SetActiveLine(cur);
	SelectRow(n1);
}

void SubtitlesGrid::DuplicateLines(int n1,int n2,bool nextFrame) {
	AssDialogue *cur;
	bool update = false;
	int step=0;
	for (int i=n1;i<=n2;i++) {
		// Create
		if (i == n2) update = true;
		cur = new AssDialogue(GetDialogue(i)->GetEntryData());

		// Shift to next frame
		if (nextFrame) {
			int posFrame = context->videoController->FrameAtTime(cur->End.GetMS(),agi::vfr::END) + 1;
			cur->Start.SetMS(context->videoController->TimeAtFrame(posFrame,agi::vfr::START));
			cur->End.SetMS(context->videoController->TimeAtFrame(posFrame,agi::vfr::END));
		}

		// Insert
		InsertLine(cur,n2+step,true,update);
		step++;
	}

	// Select new lines
	Selection newsel;
	for (int i=n1;i<=n2;i++) {
		newsel.insert(GetDialogue(i+step));
	}
	SetSelectedSet(newsel);
	SetActiveLine(GetDialogue(n1+step));
}

void SubtitlesGrid::ShiftLineByTime(int n,int len,int type) {
	assert(type >= 0 && type <= 2);
	AssDialogue *cur = GetDialogue(n);

	if (type != 2) cur->Start.SetMS(cur->Start.GetMS() + len);
	if (type != 1) cur->End.SetMS(cur->End.GetMS() + len);
}

void SubtitlesGrid::ShiftLineByFrames(int n,int len,int type) {
	assert(type >= 0 && type <= 2);
	AssDialogue *cur = GetDialogue(n);

	if (type != 2) cur->Start.SetMS(context->videoController->TimeAtFrame(len + context->videoController->FrameAtTime(cur->Start.GetMS(),agi::vfr::START),agi::vfr::START));
	if (type != 1) cur->End.SetMS(context->videoController->TimeAtFrame(len + context->videoController->FrameAtTime(cur->End.GetMS(),agi::vfr::END),agi::vfr::END));
}

void SubtitlesGrid::SplitLine(AssDialogue *n1,int pos,bool estimateTimes) {
	AssDialogue *n2 = new AssDialogue(*n1);
	InsertLine(n2,GetDialogueIndex(n1),true,false);

	wxString orig = n1->Text;
	n1->Text = orig.Left(pos).Trim(true); // Trim off trailing whitespace
	n2->Text = orig.Mid(pos).Trim(false); // Trim off leading whitespace

	if (estimateTimes) {
		double splitPos = double(pos)/orig.Length();
		int splitTime = (int)((n1->End.GetMS() - n1->Start.GetMS())*splitPos) + n1->Start.GetMS();
		n1->End.SetMS(splitTime);
		n2->Start.SetMS(splitTime);
	}

	context->ass->Commit(_("split"));
}

bool SubtitlesGrid::SplitLineByKaraoke(int lineNumber) {
	AssDialogue *line = GetDialogue(lineNumber);

	line->ParseASSTags();
	AssKaraokeVector syls;
	ParseAssKaraokeTags(line, syls);
	line->ClearBlocks();

	// If there's only 1 or 0 syllables, splitting would be counter-productive.
	// 1 syllable means there's no karaoke tags in the line at all and that is
	// the case that triggers bug #929.
	if (syls.size() < 2) return false;

	// Insert a new line for each syllable
	int start_ms = line->Start.GetMS();
	int nextpos = lineNumber;
	for (AssKaraokeVector::iterator syl = syls.begin(); syl != syls.end(); ++syl)
	{
		// Skip blank lines
		if (syl->unstripped_text.IsEmpty()) continue;

		AssDialogue *nl = new AssDialogue(line->GetEntryData());
		nl->Start.SetMS(start_ms);
		start_ms += syl->duration * 10;
		nl->End.SetMS(start_ms);
		nl->Text = syl->unstripped_text;
		InsertLine(nl, nextpos++, true, false);
	}

	// Remove the source line
	{
		wxArrayInt oia;
		oia.Add(lineNumber);
		DeleteLines(oia, false);
	}

	return true;
}

void SubtitlesGrid::SetSubsToVideo(bool start) {
	if (!context->videoController->IsLoaded()) return;

	// Get new time
	int ms = context->videoController->TimeAtFrame(context->videoController->GetFrameN(),start ? agi::vfr::START : agi::vfr::END);

	// Update selection
	wxArrayInt sel = GetSelection();
	bool modified = false;
	for (size_t i=0;i<sel.Count();i++) {
		AssDialogue *cur = GetDialogue(sel[i]);
		if (cur) {
			modified = true;
			if (start) cur->Start.SetMS(ms);
			else cur->End.SetMS(ms);
		}
	}

	if (modified) {
		context->ass->Commit(_("timing"), AssFile::COMMIT_TIMES);
	}
}

void SubtitlesGrid::SetVideoToSubs(bool start) {
	wxArrayInt sel = GetSelection();
	if (sel.Count() == 0) return;
	AssDialogue *cur = GetDialogue(sel[0]);
	if (cur) {
		if (start) 
			context->videoController->JumpToTime(cur->Start.GetMS());
		else 
			context->videoController->JumpToTime(cur->End.GetMS(), agi::vfr::END);
	}
}



/// @brief Retrieve a list of selected lines in the actual ASS file (ie. not as displayed in the grid but as represented in the file)
/// @return 
///
std::vector<int> SubtitlesGrid::GetAbsoluteSelection() {
	std::vector<int> result;
	result.reserve(GetNumberSelection());

	Selection sel;
	GetSelectedSet(sel);

	int line_index = 0;
	for (entryIter it = context->ass->Line.begin(); it != context->ass->Line.end(); ++it, ++line_index) {
		if (sel.find(dynamic_cast<AssDialogue*>(*it)) != sel.end())
			result.push_back(line_index);
	}

	return result;
}

/// @brief Update list of selected lines from absolute selection
/// @param selection 
///
void SubtitlesGrid::SetSelectionFromAbsolute(std::vector<int> &selection) {
	Selection newsel;

	std::sort(selection.begin(), selection.end());

	int i = 0;
	std::list<AssEntry*>::iterator j = context->ass->Line.begin();
	
	for (size_t selveci = 0; selveci < selection.size(); ++selveci) {
		while (i != selection[selveci] && j != context->ass->Line.end()) ++i, ++j;
		if (j == context->ass->Line.end()) break; /// @todo Report error somehow
		AssDialogue *dlg = dynamic_cast<AssDialogue*>(*j);
		if (dlg) newsel.insert(dlg);
	}

	SetSelectedSet(newsel);
}

void SubtitlesGrid::OnHighlightVisibleChange(agi::OptionValue const& opt) {
	if (opt.GetBool()) {
		seekListener.Unblock();
	}
	else {
		seekListener.Block();
	}
}
