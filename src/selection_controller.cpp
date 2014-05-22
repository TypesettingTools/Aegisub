// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "selection_controller.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "subs_controller.h"
#include "utils.h"

#include <algorithm>

SelectionController::SelectionController(agi::Context *c)
: context(c)
, open_connection(c->subsController->AddFileOpenListener(&SelectionController::OnSubtitlesOpen, this))
{
}

void SelectionController::OnSubtitlesOpen() {
	selection.clear();
	active_line = nullptr;
	if (!context->ass->Events.empty()) {
		int row = mid<int>(0, context->ass->Properties.active_row, context->ass->Events.size() - 1);
		active_line = &*std::next(context->ass->Events.begin(), row);
		selection.insert(active_line);
	}
	AnnounceSelectedSetChanged();
	AnnounceActiveLineChanged(active_line);
}

void SelectionController::SetSelectedSet(Selection new_selection) {
	selection = std::move(new_selection);
	AnnounceSelectedSetChanged();
}

void SelectionController::SetActiveLine(AssDialogue *new_line) {
	if (new_line != active_line) {
		active_line = new_line;
		if (active_line)
			context->ass->Properties.active_row = active_line->Row;
		AnnounceActiveLineChanged(new_line);
	}
}

void SelectionController::SetSelectionAndActive(Selection new_selection, AssDialogue *new_line) {
	bool active_line_changed = new_line != active_line;
	selection = std::move(new_selection);
	active_line = new_line;
	if (active_line)
		context->ass->Properties.active_row = active_line->Row;

	AnnounceSelectedSetChanged();
	if (active_line_changed)
		AnnounceActiveLineChanged(new_line);
}

std::vector<AssDialogue *> SelectionController::GetSortedSelection() const {
	std::vector<AssDialogue *> ret(selection.begin(), selection.end());
	sort(begin(ret), end(ret), [](AssDialogue *a, AssDialogue *b) { return a->Row < b->Row; });
	return ret;
}

void SelectionController::PrevLine() {
	if (!active_line) return;
	auto it = context->ass->iterator_to(*active_line);
	if (it != context->ass->Events.begin()) {
		--it;
		SetSelectionAndActive({&*it}, &*it);
	}
}

void SelectionController::NextLine() {
	if (!active_line) return;
	auto it = context->ass->iterator_to(*active_line);
	if (++it != context->ass->Events.end())
		SetSelectionAndActive({&*it}, &*it);
}
