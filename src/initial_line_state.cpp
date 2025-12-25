// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "initial_line_state.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "selection_controller.h"
#include "subs_controller.h"

InitialLineState::InitialLineState(agi::Context *c)
: context(c)
, active_line_connection(c->selectionController->AddActiveLineListener(&InitialLineState::OnActiveLineChanged, this))
, file_open_connection(c->subsController->AddFileOpenListener(&InitialLineState::OnFileOpen, this))
{
	OnActiveLineChanged(c->selectionController->GetActiveLine());
}

void InitialLineState::OnFileOpen(agi::fs::path const&) {
	SaveAllInitialTexts(context->ass.get());
	AssDialogue *active_line = context->selectionController->GetActiveLine();
	if (active_line) {
		auto it = initial_texts_map.find(active_line->Id);
		if (it != initial_texts_map.end()) {
			initial_text = it->second;
			line_id = active_line->Id;
			InitialStateChanged(initial_text);
		}
	}
}

void InitialLineState::SaveAllInitialTexts(AssFile *ass) {
	initial_texts_map.clear();
	for (auto const& line : ass->Events) {
		initial_texts_map[line.Id] = line.Text;
	}
}

void InitialLineState::OnActiveLineChanged(AssDialogue *new_line) {
	if (new_line) {
		if (new_line->Id == line_id) return;
		line_id = new_line->Id;
		auto it = initial_texts_map.find(new_line->Id);
		if (it != initial_texts_map.end()) {
			initial_text = it->second;
		} else {
			initial_text = new_line->Text;
		}
	}
	else {
		line_id = 0;
		initial_text.clear();
	}

	InitialStateChanged(initial_text);
}

std::string const& InitialLineState::GetInitialText() const {
	return initial_text;
}
