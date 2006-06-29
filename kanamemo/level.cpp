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
#include "level.h"
#include "game_display.h"
#include "kana_table.h"


///////////////
// Constructor
LevelWindow::LevelWindow(wxWindow *parent,GameDisplay *dsp)
: wxDialog(parent,-1,_T("Set Level"),wxDefaultPosition,wxDefaultSize)
{
	// Set display
	display = dsp;

	// Sliders
	int levelHira = display->level[0];
	int levelKata = display->level[1];
	int maxLevelHira = display->table->GetLevels(0);
	int maxLevelKata = display->table->GetLevels(1);
	hiraSlider = new wxSlider(this,-1,levelHira,1,maxLevelHira,wxDefaultPosition,wxSize(200,-1));
	kataSlider = new wxSlider(this,-1,levelKata,1,maxLevelKata,wxDefaultPosition,wxSize(200,-1));

	// Checkbox
	autoLevelCheck = new wxCheckBox(this,-1,_T("Automatically level up"));
	autoLevelCheck->SetValue(display->autoLevel);

	// Sizers
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *sizer1 = new wxStaticBoxSizer(wxVERTICAL,this,_T("Hiragana and Katakana"));
	wxSizer *sizer2 = new wxFlexGridSizer(2,2,5,5);
	wxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);

	// Insert controls
	sizer2->Add(new wxStaticText(this,-1,_T("Hiragana")),0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT,5);
	sizer2->Add(hiraSlider,1,wxEXPAND,0);
	sizer2->Add(new wxStaticText(this,-1,_T("Katakana")),0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT,5);
	sizer2->Add(kataSlider,1,wxEXPAND,0);

	// Sizer layout
	sizer1->Add(sizer2,1,wxEXPAND,0);
	sizer1->Add(autoLevelCheck,0,wxALIGN_LEFT | wxTOP,5);
	sizer3->AddStretchSpacer(1);
	sizer3->Add(new wxButton(this,wxID_OK),0,0,0);
	sizer3->Add(new wxButton(this,wxID_CANCEL),0,0,0);
	mainSizer->Add(sizer1,1,wxEXPAND | wxALL,5);
	mainSizer->Add(sizer3,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);
}


//////////////
// Destructor
LevelWindow::~LevelWindow() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(LevelWindow,wxDialog)
	EVT_BUTTON(wxID_OK,LevelWindow::OnOK)
	EVT_BUTTON(wxID_CANCEL,LevelWindow::OnCancel)
END_EVENT_TABLE()


//////
// OK
void LevelWindow::OnOK(wxCommandEvent &event) {
	display->autoLevel = autoLevelCheck->GetValue();
	display->SetLevel(0,hiraSlider->GetValue());
	display->SetLevel(1,kataSlider->GetValue());
	Destroy();
}


//////////
// Cancel
void LevelWindow::OnCancel(wxCommandEvent &event) {
	Destroy();
}
