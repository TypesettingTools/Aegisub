// Copyright (c) 2021, Qirui Wang
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

#include "subs_edit_ctrl_native.h"

#include "command/command.h"
#include "compat.h"
#include "options.h"
#include "include/aegisub/context.h"
#include "include/aegisub/spellchecker.h"
#include "text_selection_controller.h"
#include "selection_controller.h"
#include "thesaurus.h"
#include "utils.h"
#include "format.h"

#include <libaegisub/character_count.h>
#include <libaegisub/spellchecker.h>

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <functional>
#include <string_view>

#include <wx/menu.h>
#include <wx/settings.h>

// Maximum number of languages (locales)
#define LANGS_MAX 1000

/// Event ids
enum {
	EDIT_MENU_SPLIT_PRESERVE = (wxID_HIGHEST + 1) + 4000,
	EDIT_MENU_SPLIT_ESTIMATE,
	EDIT_MENU_SPLIT_VIDEO,
	EDIT_MENU_CUT,
	EDIT_MENU_COPY,
	EDIT_MENU_PASTE,
	EDIT_MENU_SELECT_ALL,
	EDIT_MENU_ADD_TO_DICT,
	EDIT_MENU_REMOVE_FROM_DICT,
	EDIT_MENU_SUGGESTION,
	EDIT_MENU_SUGGESTIONS,
	EDIT_MENU_THESAURUS = (wxID_HIGHEST + 1) + 5000,
	EDIT_MENU_THESAURUS_SUGS,
	EDIT_MENU_SPELL_LANGUAGE = (wxID_HIGHEST + 1) + 6000,
	EDIT_MENU_SPELL_LANGS,
	EDIT_MENU_THES_LANGUAGE = EDIT_MENU_SPELL_LANGUAGE + LANGS_MAX,
	EDIT_MENU_THES_LANGS,
	EDIT_MENU_RTL = (wxID_HIGHEST + 1) + 7000
};

SubsNativeTextEditCtrl::SubsNativeTextEditCtrl(wxWindow* parent, wxSize wsize, long style, agi::Context* context)
	: wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wsize, style | wxTE_MULTILINE)
	, thesaurus(std::make_unique<Thesaurus>())
	, spellchecker(SpellCheckerFactory::GetSpellChecker())
	, context(context)
{
	SetStyles();

	using std::bind;

	Bind(wxEVT_CHAR_HOOK, &SubsNativeTextEditCtrl::OnKeyDown, this);
	Bind(wxEVT_KEY_DOWN, &SubsNativeTextEditCtrl::OnRawKeyDown, this);

	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::Cut, this), EDIT_MENU_CUT);
	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::Copy, this), EDIT_MENU_COPY);
	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::Paste, this), EDIT_MENU_PASTE);
	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::SelectAll, this), EDIT_MENU_SELECT_ALL);

	if (context) {
		Bind(wxEVT_MENU, bind(&cmd::call, "edit/line/split/preserve", context), EDIT_MENU_SPLIT_PRESERVE);
		Bind(wxEVT_MENU, bind(&cmd::call, "edit/line/split/estimate", context), EDIT_MENU_SPLIT_ESTIMATE);
		Bind(wxEVT_MENU, bind(&cmd::call, "edit/line/split/video", context), EDIT_MENU_SPLIT_VIDEO);
	}
	Bind(wxEVT_CONTEXT_MENU, &SubsNativeTextEditCtrl::OnContextMenu, this);
	// Bind RTL toggle so native mode has an explicit toggle
	Bind(wxEVT_MENU, &SubsNativeTextEditCtrl::OnToggleRTL, this, EDIT_MENU_RTL);
	// Bind spell checker suggestion handlers
	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::OnUseSuggestion, this, std::placeholders::_1), EDIT_MENU_SUGGESTIONS, EDIT_MENU_SUGGESTIONS+LANGS_MAX);
	Bind(wxEVT_MENU, &SubsNativeTextEditCtrl::OnAddToDict, this, EDIT_MENU_ADD_TO_DICT);
	Bind(wxEVT_MENU, &SubsNativeTextEditCtrl::OnRemoveFromDict, this, EDIT_MENU_REMOVE_FROM_DICT);
	Bind(wxEVT_MENU, &SubsNativeTextEditCtrl::OnSetSpellLang, this, EDIT_MENU_SPELL_LANGS, EDIT_MENU_SPELL_LANGS+LANGS_MAX);
	// Bind thesaurus suggestion handlers
	Bind(wxEVT_MENU, bind(&SubsNativeTextEditCtrl::OnUseSuggestion, this, std::placeholders::_1), EDIT_MENU_THESAURUS_SUGS, EDIT_MENU_THESAURUS_SUGS+LANGS_MAX);
	Bind(wxEVT_MENU, &SubsNativeTextEditCtrl::OnSetThesLanguage, this, EDIT_MENU_THES_LANGS, EDIT_MENU_THES_LANGS+LANGS_MAX);

	SetLayoutDirection(OPT_GET("Subtitle/Edit Box/RTL Mode")->GetBool() ?
		wxLayout_RightToLeft : wxLayout_LeftToRight);

	BindConnection(OPT_SUB("Subtitle/Edit Box/Font Face", &SubsNativeTextEditCtrl::SetStyles, this));
	BindConnection(OPT_SUB("Subtitle/Edit Box/Font Size", &SubsNativeTextEditCtrl::SetStyles, this));
	BindConnection(OPT_SUB("Colour/Subtitle/Background", &SubsNativeTextEditCtrl::SetStyles, this));
	BindConnection(OPT_SUB("Colour/Subtitle/Syntax/Normal", &SubsNativeTextEditCtrl::SetStyles, this));
	BindConnection(OPT_SUB("Subtitle/Edit Box/RTL Mode", [this](agi::OptionValue const& opt) {
		SetLayoutDirection(opt.GetBool() ? wxLayout_RightToLeft : wxLayout_LeftToRight);
		Refresh();
	}));
}

SubsNativeTextEditCtrl::~SubsNativeTextEditCtrl() {
}

void SubsNativeTextEditCtrl::OnKeyDown(wxKeyEvent& event) {
	// Handle Shift+Return for soft line breaks
	if (event.GetKeyCode() == WXK_RETURN && event.GetModifiers() == wxMOD_SHIFT) {
		long sel_start, sel_end;
		GetSelection(&sel_start, &sel_end);
		std::string linebreak = OPT_GET("Subtitle/Edit Box/Soft Line Break")->GetBool() ? "\\n" : "\\N";
		wxString data = GetRange(0, sel_start) + to_wx(linebreak) + GetRange(sel_end, GetLastPosition());
		SetValue(data);
		SetSelection(sel_start + 2, sel_start + 2);
		return;  // We handled it, don't skip
	}

	// For all other keys, let the native widget and OS handle them
	// This includes Ctrl+Shift+Right for word selection, etc.
	event.Skip();
}

void SubsNativeTextEditCtrl::SetStyles() {
	wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
	font.SetEncoding(wxFONTENCODING_DEFAULT);
	wxString fontname = FontFace("Subtitle/Edit Box");
	if (!fontname.empty()) font.SetFaceName(fontname);
	font.SetPointSize(OPT_GET("Subtitle/Edit Box/Font Size")->GetInt());
	SetFont(font);

	SetBackgroundColour(to_wx(OPT_GET("Colour/Subtitle/Background")->GetColor()));
	SetForegroundColour(to_wx(OPT_GET("Colour/Subtitle/Syntax/Normal")->GetColor()));
}

void SubsNativeTextEditCtrl::OnContextMenu(wxContextMenuEvent& event) {
	wxPoint pos = event.GetPosition();
	int activePos;
	if (pos == wxDefaultPosition)
		activePos = GetInsertionPoint();
	else {
		long hitPos = -1;
		if (HitTest(ScreenToClient(pos), &hitPos) == wxTE_HT_ON_TEXT)
			activePos = hitPos;
		else
			activePos = -1;
	}

	currentWordPos = GetBoundsOfWordAtPosition(activePos);
	wxString textValue = GetValue();
	if (currentWordPos.second > 0 && currentWordPos.first + currentWordPos.second <= (int)textValue.length()) {
		// Use wxString::Mid to get the substring by character indices, then convert to UTF-8
		wxString wxcur = textValue.Mid(currentWordPos.first, currentWordPos.second);
		currentWord = from_wx(wxcur);
	} else {
		currentWord.clear();
	}

	wxMenu menu;

	// Spell checker
	if (spellchecker)
		AddSpellCheckerEntries(menu);

	// Thesaurus
	AddThesaurusEntries(menu);

	// Standard actions
	menu.Append(EDIT_MENU_CUT, _("Cu&t"))->Enable(!GetStringSelection().IsEmpty());
	menu.Append(EDIT_MENU_COPY, _("&Copy"))->Enable(!GetStringSelection().IsEmpty());
	menu.Append(EDIT_MENU_PASTE, _("&Paste"))->Enable(CanPaste());
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_SELECT_ALL, _("Select &All"));

	// Split
	if (context) {
		menu.AppendSeparator();
		menu.Append(EDIT_MENU_SPLIT_PRESERVE, _("Split at cursor (preserve times)"));
		menu.Append(EDIT_MENU_SPLIT_ESTIMATE, _("Split at cursor (estimate times)"));
		cmd::Command* split_video = cmd::get("edit/line/split/video");
		menu.Append(EDIT_MENU_SPLIT_VIDEO, split_video->StrMenu(context))->Enable(split_video->Validate(context));
	}

	// Add an explicit RTL toggle alongside standard text actions.
	menu.AppendSeparator();
	menu.Append(EDIT_MENU_RTL, OPT_GET("Subtitle/Edit Box/RTL Mode")->GetBool() ?
		_("Switch to LTR mode") : _("Switch to RTL mode"));

	// Use PopupMenu to properly route menu events through the event system
	PopupMenu(&menu);
}

void SubsNativeTextEditCtrl::Paste() {
	std::string data = GetClipboard();

	boost::replace_all(data, "\r\n", "\\N");
	boost::replace_all(data, "\n", "\\N");
	boost::replace_all(data, "\r", "\\N");

	long sel_start, sel_end;
	GetSelection(&sel_start, &sel_end);
	wxString data_first_half = GetRange(0, sel_start) + to_wx(data);
	wxString data_full = data_first_half + GetRange(sel_end, GetLastPosition());
	Freeze();
	SetValue(data_first_half);
	sel_start = GetLastPosition();
	SetValue(data_full);
	SetSelection(sel_start, sel_start);
	Thaw();
}

void SubsNativeTextEditCtrl::SetTextTo(std::string const& text) {
	// Mirror STC behaviour: preserve insertion point, update selection controller
	SetEvtHandlerEnabled(false);
	Freeze();

	long insertion_point = GetInsertionPoint();

	// Get current value as std::string
	wxCharBuffer prefix = GetRange(0, insertion_point).utf8_str();
	std::string_view prefix_view(prefix.data() ? prefix.data() : "", prefix.length());
	size_t old_pos = agi::CharacterCount(
		prefix_view, 0);

	if (context) {
		context->textSelectionController->SetSelection(0, 0);
		SetValue(to_wx(text));
		auto pos = agi::IndexOfCharacter(text, old_pos);
		context->textSelectionController->SetSelection(pos, pos);
	}
	else {
		SetSelection(0, 0);
		SetValue(to_wx(text));
		SetInsertionPoint(static_cast<long>(std::min<size_t>(old_pos, GetLastPosition())));
	}

	SetEvtHandlerEnabled(true);
	Thaw();
}

std::pair<int, int> SubsNativeTextEditCtrl::GetBoundsOfWordAtPosition(int pos) {
	// Simple word boundary detection for native wxTextCtrl
	// Returns {start_pos, length} of word at position pos
	wxString text = GetValue();
	// Handle empty control or invalid position
	if (text.empty() || pos < 0 || pos > (int)text.length()) return {0, 0};

	// Find start of word
	int start = pos;
	while (start > 0 && wxIsalnum(text[start - 1])) {
		start--;
	}

	// Find end of word
	int end = pos;
	while (end < (int)text.length() && wxIsalnum(text[end])) {
		end++;
	}

	return {start, end - start};
}

// Placeholder implementations for menu items (simplified for wxTextCtrl)
void SubsNativeTextEditCtrl::AddSpellCheckerEntries(wxMenu &menu) {
	if (currentWord.empty()) return;

	if (spellchecker->CanRemoveWord(currentWord))
		menu.Append(EDIT_MENU_REMOVE_FROM_DICT, fmt_tl("Remove \"%s\" from dictionary", currentWord));

	sugs = spellchecker->GetSuggestions(currentWord);
	if (spellchecker->CheckWord(currentWord)) {
		// Word is spelled correctly
		if (sugs.empty())
			menu.Append(EDIT_MENU_SUGGESTION,_("No spell checker suggestions"))->Enable(false);
		else {
			auto subMenu = new wxMenu;
			for (size_t i = 0; i < sugs.size(); ++i)
				subMenu->Append(EDIT_MENU_SUGGESTIONS+i, to_wx(sugs[i]));

			menu.Append(-1, fmt_tl("Spell checker suggestions for \"%s\"", currentWord), subMenu);
		}
	}
	else {
		// Word is misspelled - show suggestions directly
		if (sugs.empty())
			menu.Append(EDIT_MENU_SUGGESTION,_("No correction suggestions"))->Enable(false);
		else {
			for (size_t i = 0; i < sugs.size(); ++i)
				menu.Append(EDIT_MENU_SUGGESTIONS+i, to_wx(sugs[i]));
		}

		// Append "add word" option for misspelled words
		menu.Append(EDIT_MENU_ADD_TO_DICT, fmt_tl("Add \"%s\" to dictionary", currentWord))->Enable(spellchecker->CanAddWord(currentWord));
	}
	menu.Append(-1,_("Spell checker language"), GetLanguagesMenu(
		EDIT_MENU_SPELL_LANGS,
		to_wx(OPT_GET("Tool/Spell Checker/Language")->GetString()),
		to_wx(spellchecker->GetLanguageList())));
	menu.AppendSeparator();
}

void SubsNativeTextEditCtrl::AddThesaurusEntries(wxMenu &menu) {
	if (currentWord.empty()) return;

	auto results = thesaurus->Lookup(currentWord);

	thesSugs.clear();

	if (results.size()) {
		auto thesMenu = new wxMenu;

		int curThesEntry = 0;
		for (auto const& result : results) {
			// Single word, insert directly
			if (result.second.empty()) {
				thesMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry, to_wx(result.first));
				thesSugs.push_back(result.first);
				++curThesEntry;
			}
			// Multiple, create submenu
			else {
				auto subMenu = new wxMenu;
				for (auto const& sug : result.second) {
					subMenu->Append(EDIT_MENU_THESAURUS_SUGS+curThesEntry, to_wx(sug));
					thesSugs.push_back(sug);
					++curThesEntry;
				}

				thesMenu->Append(-1, to_wx(result.first), subMenu);
			}
		}

		menu.Append(-1, fmt_tl("Thesaurus suggestions for \"%s\"", currentWord), thesMenu);
	}
	else
		menu.Append(EDIT_MENU_THESAURUS,_("No thesaurus suggestions"))->Enable(false);

	// Append language list
	menu.Append(-1,_("Thesaurus language"), GetLanguagesMenu(
		EDIT_MENU_THES_LANGS,
		to_wx(OPT_GET("Tool/Thesaurus/Language")->GetString()),
		to_wx(thesaurus->GetLanguageList())));
	menu.AppendSeparator();
}

wxMenu *SubsNativeTextEditCtrl::GetLanguagesMenu(int base_id, wxString const& curLang, wxArrayString const& langs) {
	auto languageMenu = new wxMenu;
	languageMenu->AppendRadioItem(base_id, _("Disable"))->Check(curLang.empty());

	for (size_t i = 0; i < langs.size(); ++i)
		languageMenu->AppendRadioItem(base_id + i + 1, LocalizedLanguageName(langs[i]))->Check(langs[i] == curLang);

	return languageMenu;
}

void SubsNativeTextEditCtrl::OnUseSuggestion(wxCommandEvent &event) {
	std::string suggestion;
	int eventId = event.GetId();

	// Check if this is a spell checker suggestion
	if (eventId >= EDIT_MENU_SUGGESTIONS && eventId < EDIT_MENU_SUGGESTIONS + LANGS_MAX) {
		int sugIdx = eventId - EDIT_MENU_SUGGESTIONS;
		if (sugIdx >= 0 && (size_t)sugIdx < sugs.size())
			suggestion = sugs[sugIdx];
		else
			return;
	}
	// Or a thesaurus suggestion
	else if (eventId >= EDIT_MENU_THESAURUS_SUGS && eventId < EDIT_MENU_THESAURUS_SUGS + LANGS_MAX) {
		int sugIdx = eventId - EDIT_MENU_THESAURUS_SUGS;
		if (sugIdx >= 0 && (size_t)sugIdx < thesSugs.size())
			suggestion = thesSugs[sugIdx];
		else
			return;
	}
	else
		return;

	size_t pos;
	while ((pos = suggestion.rfind('(')) != std::string::npos) {
		// If there's only one suggestion for a word it'll be in the form "(noun) word",
		// so we need to trim the "(noun) " part
		if (pos == 0) {
			pos = suggestion.find(')');
			if (pos != std::string::npos) {
				if (pos + 1< suggestion.size() && suggestion[pos + 1] == ' ') ++pos;
				suggestion.erase(0, pos + 1);
			}
			break;
		}

		// Some replacements have notes about their usage after the word in the
		// form "word (generic term)" that we need to remove (plus the leading space)
		suggestion.resize(pos - 1);
	}

	long sel_start, sel_end;
	GetSelection(&sel_start, &sel_end);
	wxString beforeWord = GetRange(0, currentWordPos.first);
	wxString afterWord = GetRange(currentWordPos.first + currentWordPos.second, GetLastPosition());
	wxString newValue = beforeWord + to_wx(suggestion) + afterWord;

	SetValue(newValue);
	SetSelection(currentWordPos.first + suggestion.length(), currentWordPos.first + suggestion.length());
}

void SubsNativeTextEditCtrl::OnSetThesLanguage(wxCommandEvent &event) {
	if (!thesaurus) return;

	std::vector<std::string> langs = thesaurus->GetLanguageList();

	int index = event.GetId() - EDIT_MENU_THES_LANGS - 1;
	std::string lang;
	if (index >= 0 && (size_t)index < langs.size())
		lang = langs[index];

	OPT_SET("Tool/Thesaurus/Language")->SetString(lang);
}

void SubsNativeTextEditCtrl::OnSetSpellLang(wxCommandEvent &event) {
	if (!spellchecker) return;

	std::vector<std::string> langs = spellchecker->GetLanguageList();

	int index = event.GetId() - EDIT_MENU_SPELL_LANGS - 1;
	std::string lang;
	if (index >= 0 && (size_t)index < langs.size())
		lang = langs[index];

	OPT_SET("Tool/Spell Checker/Language")->SetString(lang);
}

void SubsNativeTextEditCtrl::OnAddToDict(wxCommandEvent &event) {
	if (spellchecker)
		spellchecker->AddWord(currentWord);
}

void SubsNativeTextEditCtrl::OnRemoveFromDict(wxCommandEvent &event) {
	if (spellchecker)
		spellchecker->RemoveWord(currentWord);
}

void SubsNativeTextEditCtrl::OnToggleRTL(wxCommandEvent &) {
	OPT_SET("Subtitle/Edit Box/RTL Mode")->SetBool(!OPT_GET("Subtitle/Edit Box/RTL Mode")->GetBool());
}

void SubsNativeTextEditCtrl::OnRawKeyDown(wxKeyEvent &event) {
	if ((event.GetKeyCode() == WXK_SHIFT && event.ControlDown()) ||
		(event.GetKeyCode() == WXK_CONTROL && event.ShiftDown())) {
		OPT_SET("Subtitle/Edit Box/RTL Mode")->SetBool(!OPT_GET("Subtitle/Edit Box/RTL Mode")->GetBool());
	}
	event.Skip();
}
