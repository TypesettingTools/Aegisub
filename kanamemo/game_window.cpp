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
#include "game_window.h"
#include "game_display.h"
#include "game_panel.h"
#include "version.h"
#include "level.h"
#include "switch_user.h"


///////////////
// Constructor
GameWindow::GameWindow()
: GameWindowBase(NULL,-1,GetVersionString(),wxDefaultPosition,wxDefaultSize,wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
	// Menu bar
	menu = new wxMenuBar();
	SetMenuBar(menu);
	SetIcon(wxICON(wxicon));

	// File menu
	wxMenu *fileMenu = new wxMenu;
	//fileMenu->Append(Menu_File_New,_T("&New Player"),_T(""));
	fileMenu->Append(Menu_File_Load,_T("&Switch User..."),_T(""));
	fileMenu->Append(Menu_File_Exit,_T("E&xit"),_T(""));
	menu->Append(fileMenu,_T("&File"));

	// Options menu
	wxMenu *optionsMenu = new wxMenu;
	optionsMenu->Append(Menu_Options_Hiragana,_T("Hiragana"),_T(""),wxITEM_CHECK);
	optionsMenu->Append(Menu_Options_Katakana,_T("Katakana"),_T(""),wxITEM_CHECK);
	optionsMenu->Append(Menu_Options_Level,_T("Set Level..."),_T(""));
	menu->Append(optionsMenu,_T("&Options"));

	// Status bar
	wxStatusBar *bar = CreateStatusBar(3,0);
	int widths[] = { 85, -1, -1 };
	bar->SetStatusWidths(3,widths);

	// Subwindows
	display = new GameDisplay(this);
	panel = new GamePanel(this,display);
	display->panel = panel;

	// Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(display,1,wxEXPAND,0);
	mainSizer->Add(panel,0,wxEXPAND,0);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);

	// Prompt user for name
	Show();
	SwitchUserDialog user(display);
	user.ShowModal();
	if (!display->isOn) Destroy();
}


//////////////
// Destructor
GameWindow::~GameWindow() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(GameWindow,GameWindowBase)
	EVT_CLOSE(GameWindow::OnClose)
	EVT_MENU(Menu_File_Exit,GameWindow::OnFileExit)
	EVT_MENU(Menu_File_New,GameWindow::OnFileNew)
	EVT_MENU(Menu_File_Load,GameWindow::OnFileLoad)
	EVT_MENU(Menu_Options_Hiragana,GameWindow::OnOptionsHiragana)
	EVT_MENU(Menu_Options_Katakana,GameWindow::OnOptionsKatakana)
	EVT_MENU(Menu_Options_Level,GameWindow::OnOptionsLevel)
END_EVENT_TABLE()


///////////////
// Close event
void GameWindow::OnClose(wxCloseEvent &event) {
	Destroy();
}


////////
// Exit
void GameWindow::OnFileExit(wxCommandEvent &event) {
	Close();
}


//////////////
// New player
void GameWindow::OnFileNew(wxCommandEvent &event) {
}


///////////////
// Load player
void GameWindow::OnFileLoad(wxCommandEvent &event) {
	display->Save();
	SwitchUserDialog user(display);
	user.ShowModal();
}


///////////////////
// Toggle hiragana
void GameWindow::OnOptionsHiragana(wxCommandEvent &event) {
	display->EnableTable(0,event.IsChecked());
	display->Save();
}


///////////////////
// Toggle katakana
void GameWindow::OnOptionsKatakana(wxCommandEvent &event) {
	display->EnableTable(1,event.IsChecked());
	display->Save();
}


////////////////////
// Difficulty Level
void GameWindow::OnOptionsLevel(wxCommandEvent &event) {
	LevelWindow level(this,display);
	level.ShowModal();
	display->Save();
}
