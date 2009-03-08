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
#include "game_panel.h"
#include "game_display.h"


///////////////
// Constructor
GamePanel::GamePanel(wxWindow *parent,GameDisplay *dsp)
: wxPanel (parent,-1,wxDefaultPosition,wxDefaultSize,wxRAISED_BORDER)
{
	// Set display
	display = dsp;

	// Controls
	enterField = new wxTextCtrl(this,Enter_Box,_T(""),wxDefaultPosition,wxSize(50,-1),wxTE_PROCESS_ENTER);
	enterField->SetMaxLength(3);
	enterField->SetToolTip(_T("Enter the hepburn romaji for the kana you see here and press the Enter key or the enter button to accept. Type '?' for the answer."));
	wxButton *enterButton = new wxButton(this,Enter_Button,_T("Enter"),wxDefaultPosition,wxSize(60,-1));
	enterButton->SetToolTip(_T("Enter text. (Pressing the enter key on the keyboard will also work)"));
	wxButton *questionButton = new wxButton(this,Question_Button,_T("?"),wxDefaultPosition,wxSize(30,-1));
	questionButton->SetToolTip(_T("Shows the correct hepburn romaji for the shown kana."));

	// Sizers
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->AddStretchSpacer(1);
	topSizer->Add(new wxStaticText(this,-1,_T("Enter hepburn romaji:")),0,wxALIGN_CENTER | wxRIGHT,8);
	topSizer->Add(enterField,0,wxALIGN_CENTER | wxRIGHT,2);
	topSizer->Add(enterButton,0,0,0);
	topSizer->Add(questionButton,0,0,0);
	topSizer->AddStretchSpacer(1);
	mainSizer->Add(topSizer,1,wxEXPAND,0);
	mainSizer->Add(new wxStaticText(this,-1,_T("Copyright © 2006 - Rodrigo Braz Monteiro. All rights reserved.")),0,wxALIGN_CENTER | wxALL,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);
}


//////////////
// Destructor
GamePanel::~GamePanel() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(GamePanel,wxPanel)
	EVT_BUTTON(Enter_Button,OnEnterPress)
	EVT_BUTTON(Question_Button,OnQuestionPress)
	EVT_TEXT_ENTER(Enter_Box,OnEnterPress)
END_EVENT_TABLE()


/////////////////
// Enter pressed
void GamePanel::OnEnterPress(wxCommandEvent &event) {
	wxString value = enterField->GetValue();
	if (!value.IsEmpty()) {
		display->EnterRomaji(value);
		enterField->Clear();
	}
	enterField->SetFocus();
}


////////////////////
// Question pressed
void GamePanel::OnQuestionPress(wxCommandEvent &event) {
	display->EnterRomaji(_T("?"));
	enterField->Clear();
	enterField->SetFocus();
}
