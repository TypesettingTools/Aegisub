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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/wxprec.h>
#include <vector>


//////////////
// Prototypes
class KanaTable;
class KanaEntry;
class SwitchUserDialog;
class GamePanel;


//////////////////////
// Game display class
class GameDisplay : public wxWindow {
	friend class SwitchUserDialog;

private:
	wxStatusBar *statusBar;
	const KanaEntry *current;
	const KanaEntry *previous;
	int curn;
	int curTable;
	int lastTable;
	std::vector<int> scores[2];
	bool enabled[2];
	wxMenuItem *menuCheck[2];
	int status;
	int levelUp;
	wxString lastEntry;

	wxString playerName;

	void OnPaint(wxPaintEvent &event);
	void OnClick(wxMouseEvent &event);

	void Reset();
	int GetNAtLevel(int level,int table);
	void UpdateStatusBar();

public:
	bool autoLevel;
	bool isOn;
	int level[2];
	KanaTable *table;
	GamePanel *panel;

	GameDisplay(wxWindow *parent);
	~GameDisplay();

	void ResetTable(int id);
	void GetNextKana();
	void EnterRomaji(wxString romaji);
	void EnableTable(int table,bool enable);
	void SetLevel(int table,int level);
	void EnableGame(bool enable);

	void Save();
	void Load();

	DECLARE_EVENT_TABLE();
};
