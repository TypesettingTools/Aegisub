// Copyright (c) 2005, Rodrigo Braz Monteiro
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

SearchReplaceEngine::SearchReplaceEngine(agi::Context *c)
: context(c)
, cur_line(0)
, pos(0)
, match_len(0)
, replace_len(0)
, last_was_find(true)
, initialized(false)
{
}

static boost::flyweight<wxString> *get_text(AssDialogue *cur, SearchReplaceSettings::Field field) {
	switch (field) {
		case SearchReplaceSettings::Field::TEXT: return &cur->Text;
		case SearchReplaceSettings::Field::STYLE: return &cur->Style;
		case SearchReplaceSettings::Field::ACTOR: return &cur->Actor;
		case SearchReplaceSettings::Field::EFFECT: return &cur->Effect;
	}
	throw agi::InternalError("Bad field for search", 0);
}

bool SearchReplaceEngine::FindReplace(bool replace) {
	if (!initialized)
		return false;

	wxArrayInt sels = context->subsGrid->GetSelection();
	int firstLine = sels.empty() ? 0 : sels.front();

	// if selection has changed reset values
	if (firstLine != cur_line) {
		cur_line = firstLine;
		last_was_find = true;
		pos = 0;
		match_len = 0;
		replace_len = 0;
	}

	// Setup
	int start = cur_line;
	int nrows = context->subsGrid->GetRows();
	bool found = false;
	int regFlags = wxRE_ADVANCED;
	if (!settings.match_case) {
		if (settings.use_regex)
			regFlags |= wxRE_ICASE;
		else
			settings.find.MakeLower();
	}
	wxRegEx regex;
	if (settings.use_regex) {
		regex.Compile(settings.find, regFlags);

		if (!regex.IsValid()) {
			last_was_find = !replace;
			return true;
		}
	}

	// Search for it
	boost::flyweight<wxString> *Text = nullptr;
	while (!found) {
		Text = get_text(context->subsGrid->GetDialogue(cur_line), settings.field);
		size_t tempPos;
		if (replace && last_was_find)
			tempPos = pos;
		else
			tempPos = pos + replace_len;

		if (settings.use_regex) {
			if (regex.Matches(Text->get().substr(tempPos))) {
				size_t match_start;
				regex.GetMatch(&match_start, &match_len, 0);
				pos = match_start + tempPos;
				found = true;
			}
		}
		else {
			wxString src = Text->get().substr(tempPos);
			if (!settings.match_case) src.MakeLower();
			size_t textPos = src.find(settings.find);
			if (textPos != src.npos) {
				pos = tempPos+textPos;
				found = true;
				match_len = settings.find.size();
			}
		}

		// Didn't find, go to next line
		if (!found) {
			cur_line = (cur_line + 1) % nrows;
			pos = 0;
			match_len = 0;
			replace_len = 0;
			if (cur_line == start) break;
		}
	}

	if (found) {
		if (!replace)
			replace_len = match_len;
		else {
			if (settings.use_regex) {
				wxString toReplace = Text->get().substr(pos,match_len);
				regex.ReplaceFirst(&toReplace,settings.replace_with);
				*Text = Text->get().Left(pos) + toReplace + Text->get().substr(pos+match_len);
				replace_len = toReplace.size();
			}
			else {
				*Text = Text->get().Left(pos) + settings.replace_with + Text->get().substr(pos+match_len);
				replace_len = settings.replace_with.size();
			}

			context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		}

		context->subsGrid->SelectRow(cur_line,false);
		context->subsGrid->MakeCellVisible(cur_line,0);
		if (settings.field == SearchReplaceSettings::Field::TEXT) {
			context->selectionController->SetActiveLine(context->subsGrid->GetDialogue(cur_line));
			context->textSelectionController->SetSelection(pos, pos + replace_len);
		}
		// hAx to prevent double match on style/actor
		else
			replace_len = 99999;
	}
	last_was_find = !replace;

	return true;
}

bool SearchReplaceEngine::ReplaceAll() {
	if (!initialized)
		return false;

	size_t count = 0;

	int regFlags = wxRE_ADVANCED;
	if (!settings.match_case)
		regFlags |= wxRE_ICASE;
	wxRegEx reg;
	if (settings.use_regex)
		reg.Compile(settings.find, regFlags);

	SubtitleSelection const& sel = context->selectionController->GetSelectedSet();
	bool hasSelection = !sel.empty();
	bool inSel = settings.limit_to == SearchReplaceSettings::Limit::SELECTED;

	for (auto diag : context->ass->Line | agi::of_type<AssDialogue>()) {
		if (inSel && hasSelection && !sel.count(diag))
			continue;

		boost::flyweight<wxString> *Text = get_text(diag, settings.field);

		if (settings.use_regex) {
			if (reg.Matches(*Text)) {
				size_t start, len;
				reg.GetMatch(&start, &len);

				// A zero length match (such as '$') will always be replaced
				// maxMatches times, which is almost certainly not what the user
				// wanted, so limit it to one replacement in that situation
				wxString repl(*Text);
				count += reg.Replace(&repl, settings.replace_with, len > 0 ? 1000 : 1);
				*Text = repl;
			}
		}
		else {
			if (!settings.match_case) {
				bool replaced = false;
				wxString Left, Right = *Text;
				size_t pos = 0;
				Left.reserve(Right.size());
				while (pos + settings.find.size() <= Right.size()) {
					if (Right.substr(pos, settings.find.size()).CmpNoCase(settings.find) == 0) {
						Left.Append(Right.Left(pos)).Append(settings.replace_with);
						Right = Right.substr(pos + settings.find.size());
						++count;
						replaced = true;
						pos = 0;
					}
					else {
						pos++;
					}
				}
				if (replaced) {
					*Text = Left + Right;
				}
			}
			else if(Text->get().Contains(settings.find)) {
				wxString repl(*Text);
				count += repl.Replace(settings.find, settings.replace_with);
				*Text = repl;
			}
		}
	}

	if (count > 0) {
		context->ass->Commit(_("replace"), AssFile::COMMIT_DIAG_TEXT);
		wxMessageBox(wxString::Format(_("%i matches were replaced."), (int)count));
	}
	else {
		wxMessageBox(_("No matches found."));
	}
	last_was_find = false;

	return true;
}

void SearchReplaceEngine::Configure(SearchReplaceSettings const& new_settings) {
	wxArrayInt sels = context->subsGrid->GetSelection();
	cur_line = 0;
	if (sels.size() > 0) cur_line = sels[0];

	last_was_find = true;
	pos = 0;
	match_len = 0;
	replace_len = 0;

	settings = new_settings;
}
