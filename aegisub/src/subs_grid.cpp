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
#include "main.h"
#include "utils.h"
#include "video_context.h"

SubtitlesGrid::SubtitlesGrid(wxWindow *parent, agi::Context *context)
: BaseGrid(parent, context, wxDefaultSize, wxWANTS_CHARS | wxSUNKEN_BORDER)
{
}

static void trim_text(AssDialogue *diag) {
	static wxRegEx start("^( |\t|\\\\[nNh])+");
	static wxRegEx end("( |\t|\\\\[nNh])+$");
	start.ReplaceFirst(&diag->Text, "");
	end.ReplaceFirst(&diag->Text, "");
}

static void expand_times(AssDialogue *src, AssDialogue *dst) {
	dst->Start = std::min(dst->Start, src->Start);
	dst->End = std::max(dst->End, src->End);
}

/// @brief Recombine
void SubtitlesGrid::RecombineLines() {
	Selection selectedSet = GetSelectedSet();
	if (selectedSet.size() < 2) return;

	AssDialogue *activeLine = GetActiveLine();

	std::vector<AssDialogue*> sel(selectedSet.begin(), selectedSet.end());
	for_each(sel.begin(), sel.end(), trim_text);
	sort(sel.begin(), sel.end(), &AssFile::CompStart);

	typedef std::vector<AssDialogue*>::iterator diag_iter;
	diag_iter end = sel.end() - 1;
	for (diag_iter cur = sel.begin(); cur != end; ++cur) {
		AssDialogue *d1 = *cur;
		diag_iter d2 = cur + 1;

		// 1, 1+2 (or 2+1), 2 gets turned into 1, 2, 2 so kill the duplicate
		if (d1->Text == (*d2)->Text) {
			expand_times(d1, *d2);
			delete d1;
			continue;
		}

		// 1, 1+2, 1 turns into 1, 2, [empty]
		if (d1->Text.empty()) {
			delete d1;
			continue;
		}
		// If d2 is the last line in the selection it'll never hit the above test
		if (d2 == end && (*d2)->Text.empty()) {
			delete *d2;
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
	if (!newSel.count(activeLine))
		activeLine = *newSel.begin();
	SetSelectionAndActive(newSel, activeLine);

	context->ass->Commit(_("combining"), AssFile::COMMIT_DIAG_ADDREM | AssFile::COMMIT_DIAG_FULL);
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
