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

#include "subs_grid.h"

#include "include/aegisub/context.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "dialog_paste_over.h"
#include "utils.h"
#include "video_context.h"

SubtitlesGrid::SubtitlesGrid(wxWindow *parent, agi::Context *context,  const wxSize& size, long style, const wxString& name)
: BaseGrid(parent,context,size,style,name)
{
}

static void trim_text(AssDialogue *diag) {
	static wxRegEx start("^( |\\t|\\\\[nNh])+");
	static wxRegEx end("( |\\t|\\\\[nNh])+$");
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

	// Remove now non-existent lines from the selection
	Selection lines;
	transform(context->ass->Line.begin(), context->ass->Line.end(), inserter(lines, lines.begin()), cast<AssDialogue*>());
	Selection newSel;
	set_intersection(lines.begin(), lines.end(), selectedSet.begin(), selectedSet.end(), inserter(newSel, newSel.begin()));

	if (newSel.empty())
		newSel.insert(*lines.begin());

	// Restore selection
	SetSelectedSet(newSel);
	if (find(newSel.begin(), newSel.end(), activeLine) == newSel.end()) {
		activeLine = *newSel.begin();
	}
	SetActiveLine(activeLine);

	context->ass->Commit(_("combining"), AssFile::COMMIT_DIAG_ADDREM | AssFile::COMMIT_DIAG_FULL);
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

	context->ass->Line.insert(pos,line);

	// Update
	if (update) {
		context->ass->Commit(_("line insertion"), AssFile::COMMIT_DIAG_ADDREM);
	}
	else {
		UpdateMaps();
	}
}

void SubtitlesGrid::CopyLines(wxArrayInt target) {
	// Prepare text
	wxString data = "";
	AssDialogue *cur;
	int nrows = target.Count();
	bool first = true;
	for (int i=0;i<nrows;i++) {
		if (!first) data += "\r\n";
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
		wxStringTokenizer token (data,"\r\n",wxTOKEN_STRTOK);
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
			context->ass->Commit(_("paste"), pasteOver ? AssFile::COMMIT_DIAG_FULL : AssFile::COMMIT_DIAG_ADDREM);

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

	int row = -1;
	size_t deleted = 0;
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
	}

	if (flagModified) {
		context->ass->Commit(_("delete"), AssFile::COMMIT_DIAG_ADDREM);
	}
	else {
		UpdateMaps();
	}
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

	context->ass->Commit(_("adjoin"), AssFile::COMMIT_DIAG_TIME);
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

	context->ass->Commit(_("split"), AssFile::COMMIT_DIAG_ADDREM | AssFile::COMMIT_DIAG_FULL);
}

/// @brief Retrieve a list of selected lines in the actual ASS file (ie. not as displayed in the grid but as represented in the file)
/// @return 
///
std::vector<int> SubtitlesGrid::GetAbsoluteSelection() {
	Selection sel = GetSelectedSet();

	std::vector<int> result;
	result.reserve(sel.size());

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
