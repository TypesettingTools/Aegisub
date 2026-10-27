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

#include <libaegisub/signal.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <wx/textctrl.h>

namespace agi {
	struct Context;
	class SpellChecker;
}
class Thesaurus;

/// @class SubsNativeTextEditCtrl
/// @brief Native wxTextCtrl-based subtitle editor
/// Better platform-specific support: keyboard shortcuts, IME, RTL languages
class SubsNativeTextEditCtrl final : public wxTextCtrl, private agi::signal::ConnectionScope {
	/// Backend thesaurus to use
	std::unique_ptr<Thesaurus> thesaurus;

	/// Backend spell checker to use
	std::unique_ptr<agi::SpellChecker> spellchecker;

	/// Project context, for splitting lines
	agi::Context *context;

	/// The word right-clicked on, used for spellchecker replacing
	std::string currentWord;

	/// The beginning of the word right-clicked on, for spellchecker replacing
	std::pair<int, int> currentWordPos;

	/// Spellchecker suggestions for the last right-clicked word
	std::vector<std::string> sugs;

	/// Thesaurus suggestions for the last right-clicked word
	std::vector<std::string> thesSugs;

	void OnContextMenu(wxContextMenuEvent &);
	void OnKeyDown(wxKeyEvent &event);

	void SetStyles();

	/// Add the spell checker suggestions to a menu
	void AddSpellCheckerEntries(wxMenu &menu);

	/// Add the thesaurus suggestions to a menu
	void AddThesaurusEntries(wxMenu &menu);

	/// Generate a languages submenu from a list of locales and a current language
	/// @param base_id ID to use for the first menu item
	/// @param curLang Currently selected language
	/// @param lang Full list of languages
	wxMenu *GetLanguagesMenu(int base_id, wxString const& curLang, wxArrayString const& langs);

	/// Handle spell checker or thesaurus suggestion menu item click
	void OnUseSuggestion(wxCommandEvent &event);

	/// Handle spell checker language selection
	void OnSetSpellLang(wxCommandEvent &event);

	/// Add word to spell checker dictionary
	void OnAddToDict(wxCommandEvent &event);

	/// Remove word from spell checker dictionary
	void OnRemoveFromDict(wxCommandEvent &event);

	/// Handle thesaurus language selection
	void OnSetThesLanguage(wxCommandEvent &event);

	/// Toggle Right-to-Left reading order (context menu action)
	void OnToggleRTL(wxCommandEvent &event);
	void OnRawKeyDown(wxKeyEvent &event);

public:
	SubsNativeTextEditCtrl(wxWindow* parent, wxSize size, long style, agi::Context *context);
	~SubsNativeTextEditCtrl();

	void SetTextTo(std::string const& text);
	void Paste() override;

	std::pair<int, int> GetBoundsOfWordAtPosition(int pos);
};
