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

#include "subs_grid.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "utils.h"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <utility>

SubtitlesGrid::SubtitlesGrid(wxWindow *parent, agi::Context *context)
: BaseGrid(parent, context, wxDefaultSize, wxWANTS_CHARS | wxSUNKEN_BORDER)
{
}

static std::string trim_text(std::string text) {
	boost::regex start("^( |\t|\\\\[nNh])+");
	boost::regex end("( |\t|\\\\[nNh])+$");

	regex_replace(text, start, "", boost::format_first_only);
	regex_replace(text, end, "", boost::format_first_only);
	return text;
}

static void expand_times(AssDialogue *src, AssDialogue *dst) {
	dst->Start = std::min(dst->Start, src->Start);
	dst->End = std::max(dst->End, src->End);
}

static bool check_lines(AssDialogue *d1, AssDialogue *d2, bool (*pred)(std::string const&, std::string const&)) {
	if (pred(d1->Text.get(), d2->Text.get())) {
		d1->Text = trim_text(d1->Text.get().substr(d2->Text.get().size()));
		expand_times(d1, d2);
		return true;
	}
	return false;
}

static bool check_start(AssDialogue *d1, AssDialogue *d2) {
	return check_lines(d1, d2, &boost::starts_with<std::string, std::string>);
}

static bool check_end(AssDialogue *d1, AssDialogue *d2) {
	return check_lines(d1, d2, &boost::ends_with<std::string, std::string>);
}

void SubtitlesGrid::RecombineLines() {
	Selection selectedSet = GetSelectedSet();
	if (selectedSet.size() < 2) return;

	AssDialogue *activeLine = GetActiveLine();

	std::vector<AssDialogue*> sel(selectedSet.begin(), selectedSet.end());
	sort(sel.begin(), sel.end(), &AssFile::CompStart);
	for (auto &diag : sel)
		diag->Text = trim_text(diag->Text);

	auto end = sel.end() - 1;
	for (auto cur = sel.begin(); cur != end; ++cur) {
		AssDialogue *d1 = *cur;
		auto d2 = cur + 1;

		// 1, 1+2 (or 2+1), 2 gets turned into 1, 2, 2 so kill the duplicate
		if (d1->Text == (*d2)->Text) {
			expand_times(d1, *d2);
			delete d1;
			continue;
		}

		// 1, 1+2, 1 turns into 1, 2, [empty]
		if (d1->Text.get().empty()) {
			delete d1;
			continue;
		}

		// If d2 is the last line in the selection it'll never hit the above test
		if (d2 == end && (*d2)->Text.get().empty()) {
			delete *d2;
			continue;
		}

		// 1, 1+2
		while (d2 <= end && check_start(*d2, d1))
			++d2;

		// 1, 2+1
		while (d2 <= end && check_end(*d2, d1))
			++d2;

		// 1+2, 2
		while (d2 <= end && check_end(d1, *d2))
			++d2;

		// 2+1, 2
		while (d2 <= end && check_start(d1, *d2))
			++d2;
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
