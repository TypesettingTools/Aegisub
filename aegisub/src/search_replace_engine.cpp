// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "config.h"

#include "search_replace_engine.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "include/aegisub/context.h"
#include "subs_grid.h"
#include "text_selection_controller.h"

#include <libaegisub/of_type_adaptor.h>

#include <wx/msgdlg.h>
#include <wx/regex.h>

struct MatchState {
	wxRegEx *re;
	size_t start, end;

	MatchState() : re(nullptr), start(0), end(-1) { }
	MatchState(size_t s, size_t e, wxRegEx *re = nullptr) : re(re), start(s), end(e) { }
	operator bool() { return end != -1; }
};

namespace {
template<typename AssDialogue>
auto get_dialogue_field(AssDialogue *cur, SearchReplaceSettings::Field field) -> decltype(&cur->Text) {
	switch (field) {
		case SearchReplaceSettings::Field::TEXT: return &cur->Text;
		case SearchReplaceSettings::Field::STYLE: return &cur->Style;
		case SearchReplaceSettings::Field::ACTOR: return &cur->Actor;
		case SearchReplaceSettings::Field::EFFECT: return &cur->Effect;
	}
	throw agi::InternalError("Bad field for search", 0);
}

std::function<MatchState (const AssDialogue*, size_t)> get_matcher(SearchReplaceSettings::Field field, wxString look_for, bool use_regex, bool match_case, wxRegEx *regex) {
	if (use_regex) {
		int flags = wxRE_ADVANCED;
		if (!match_case)
			flags |= wxRE_ICASE;

		regex->Compile(look_for, flags);
		if (!regex->IsValid())
			return [](const AssDialogue*, size_t) { return MatchState(); };

		return [=](const AssDialogue *diag, size_t start) {
			auto const& str = *get_dialogue_field(diag, field);
			if (!regex->Matches(str.get().substr(start)))
				return MatchState();

			size_t match_start, match_len;
			regex->GetMatch(&match_start, &match_len, 0);
			return MatchState(match_start + start, match_start + match_len + start, regex);
		};
	}

	if (!match_case)
		look_for.MakeLower();

	return [=](const AssDialogue *diag, size_t start) {
		auto str = get_dialogue_field(diag, field)->get().substr(start);
		if (!match_case)
			str.MakeLower();

		size_t pos = str.find(look_for);
		if (pos == wxString::npos)
			return MatchState();

		return MatchState(pos + start, pos + look_for.size() + start);
	};
}

template<typename Iterator, typename Container>
Iterator circular_next(Iterator it, Container& c) {
	++it;
	if (it == c.end())
		it = c.begin();
	return it;
}

}

SearchReplaceEngine::SearchReplaceEngine(agi::Context *c)
: context(c)
, initialized(false)
{
}

void SearchReplaceEngine::Replace(AssDialogue *diag, MatchState &ms) {
	auto diag_field = get_dialogue_field(diag, settings.field);
	auto text = diag_field->get();

	wxString replacement = settings.replace_with;
	if (ms.re) {
		wxString to_replace = text.substr(ms.start, ms.end - ms.start);
		ms.re->ReplaceFirst(&to_replace, settings.replace_with);
		replacement = to_replace;
	}

	*diag_field = text.substr(0, ms.start) + replacement + text.substr(ms.end);
	ms.end = ms.start + replacement.size();
}

bool SearchReplaceEngine::FindReplace(bool replace) {
	if (!initialized)
		return false;

	wxRegEx r;
	auto matches = get_matcher(settings.field, settings.find, settings.use_regex, settings.match_case, &r);

	AssDialogue *line = context->selectionController->GetActiveLine();
	auto it = context->ass->Line.iterator_to(*line);
	size_t pos = 0;

	MatchState replace_ms;
	if (replace) {
		if (settings.field == SearchReplaceSettings::Field::TEXT)
			pos = context->textSelectionController->GetSelectionStart();

		if ((replace_ms = matches(line, pos))) {
			size_t end = -1;
			if (settings.field == SearchReplaceSettings::Field::TEXT)
				end = context->textSelectionController->GetSelectionEnd();

			if (end != -1 || (pos == replace_ms.start && end == replace_ms.end)) {
				Replace(line, replace_ms);
				pos = replace_ms.end;
				context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
			}
			else {
				// The current line matches, but it wasn't already selected,
				// so the match hasn't been "found" and displayed to the user
				// yet, so do that rather than replacing
				context->textSelectionController->SetSelection(replace_ms.start, replace_ms.end);
				return true;
			}
		}
	}
	// Search from the end of the selection to avoid endless matching the same thing
	else if (settings.field == SearchReplaceSettings::Field::TEXT)
		pos = context->textSelectionController->GetSelectionEnd();
	// For non-text fields we just look for matching lines rather than each
	// match within the line, so move to the next line
	else if (settings.field != SearchReplaceSettings::Field::TEXT)
		it = circular_next(it, context->ass->Line);

	auto const& sel = context->selectionController->GetSelectedSet();
	bool selection_only = settings.limit_to == SearchReplaceSettings::Limit::SELECTED;

	do {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(&*it);
		if (!diag) continue;
		if (selection_only && !sel.count(diag)) continue;

		if (MatchState ms = matches(diag, pos)) {
			if (selection_only)
				// We're cycling through the selection, so don't muck with it
				context->selectionController->SetActiveLine(diag);
			else {
				SubtitleSelection new_sel;
				new_sel.insert(diag);
				context->selectionController->SetSelectionAndActive(new_sel, diag);
			}

			if (settings.field == SearchReplaceSettings::Field::TEXT)
				context->textSelectionController->SetSelection(ms.start, ms.end);

			return true;
		}
	} while (pos = 0, &*(it = circular_next(it, context->ass->Line)) != line);

	// Replaced something and didn't find another match, so select the newly
	// inserted text
	if (replace_ms && settings.field == SearchReplaceSettings::Field::TEXT)
		context->textSelectionController->SetSelection(replace_ms.start, replace_ms.end);

	return true;
}

bool SearchReplaceEngine::ReplaceAll() {
	if (!initialized)
		return false;

	size_t count = 0;

	wxRegEx r;
	auto matches = get_matcher(settings.field, settings.find, settings.use_regex, settings.match_case, &r);

	SubtitleSelection const& sel = context->selectionController->GetSelectedSet();
	bool selection_only = settings.limit_to == SearchReplaceSettings::Limit::SELECTED;

	for (auto diag : context->ass->Line | agi::of_type<AssDialogue>()) {
		if (selection_only && !sel.count(diag)) continue;

		if (settings.use_regex) {
			if (MatchState ms = matches(diag, 0)) {
				auto diag_field = get_dialogue_field(diag, settings.field);
				auto text = diag_field->get();
				// A zero length match (such as '$') will always be replaced
				// maxMatches times, which is almost certainly not what the user
				// wanted, so limit it to one replacement in that situation
				count += ms.re->Replace(&text, settings.replace_with, ms.start == ms.end);
				*diag_field = text;
			}
			continue;
		}

		size_t pos = 0;
		while (MatchState ms = matches(diag, pos)) {
			++count;
			Replace(diag, ms);
			pos = ms.end;
		}
	}

	if (count > 0) {
		context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		wxMessageBox(wxString::Format(_("%i matches were replaced."), (int)count));
	}
	else {
		wxMessageBox(_("No matches found."));
	}

	return true;
}

void SearchReplaceEngine::Configure(SearchReplaceSettings const& new_settings) {
	settings = new_settings;
	initialized = true;
}
