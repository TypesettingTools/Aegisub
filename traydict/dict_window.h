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
//   * Neither the name of the TrayDict Group nor the names of its contributors
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
// -----------------------------------------------------------------------------
//
// TRAYDICT
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <vector>
#include <wx/aui/aui.h>


//////////////
// Prototypes
class Systray;
class Dictionary;
class DictionaryDisplay;


//////////////////////////
// Dictionary main window
class DictWindow : public wxFrame {
private:
	wxMenuBar *menu;
	DictionaryDisplay *results;
	Systray *systray;
	std::vector<Dictionary*> dict;

	void GoToTray();
	void ComeFromTray();
	void OnIconize(wxIconizeEvent &event);
	void OnSearch(wxCommandEvent &event);
	void OnHotkey(wxKeyEvent &event);
	void OnClose(wxCloseEvent &event);

	void Search(wxString text);

public:
	wxAuiManager *manager;
	wxComboBox *entry;
	wxCheckBox *checkKanji;
	wxCheckBox *checkKana;
	wxCheckBox *checkRomaji;
	wxCheckBox *checkEnglish;
	wxCheckBox *checkEdict;
	wxCheckBox *checkEnamdict;
	wxCheckBox *checkCompdic;
	wxCheckBox *checkJplaces;

	DictWindow();
	~DictWindow();

	DECLARE_EVENT_TABLE();
};


///////
// IDs
enum {
	BUTTON_SEARCH = 1200,
	ENTRY_FIELD,
	CHECK_KANJI,
	CHECK_KANA,
	CHECK_ROMAJI,
	CHECK_ENGLISH,
	HOTKEY_ID = 0x19E2		// Totally arbitrary
};
