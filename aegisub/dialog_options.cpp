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
#include "dialog_options.h"
#ifdef wxUSE_TREEBOOK
#include <wx/treebook.h>
#endif
#include "options.h"
#include "frame_main.h"
#include "main.h"


///////////////
// Constructor
DialogOptions::DialogOptions(wxWindow *parent)
: wxDialog(parent, -1, _T("Options"), wxDefaultPosition, wxDefaultSize)
{
#ifdef wxUSE_TREEBOOK
	// Create book
	book = new wxTreebook(this,-1,wxDefaultPosition,wxSize(100,100));

	// Image list
	//wxImageList *imgList = new wxImageList(16,15);
	//imgList->Add(wxBITMAP(resample_toolbutton));
	//book->AssignImageList(imgList);

	// Panels
	wxPanel *generalPage = new wxPanel(book,-1);
	wxPanel *filePage = new wxPanel(book,-1);
	wxPanel *gridPage = new wxPanel(book,-1);
	wxPanel *editPage = new wxPanel(book,-1);
	wxPanel *videoPage = new wxPanel(book,-1);
	wxPanel *audioPage = new wxPanel(book,-1);
	wxPanel *displayPage = new wxPanel(book,-1);
	wxPanel *autoPage = new wxPanel(book,-1);

	// General page
	{
		wxSizer *genMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *genSizer1 = new wxStaticBoxSizer(wxHORIZONTAL,generalPage,_("Startup"));
		wxCheckBox *box1 = new wxCheckBox(generalPage,-1,_("Show Splash Screen"));
		Bind(box1,_T("Show splash"));
		wxCheckBox *box2 = new wxCheckBox(generalPage,-1,_("Show Tip of the Day"));
		Bind(box2,_T("Tips enabled"));
		genSizer1->Add(box1,1,wxALL,5);
		genSizer1->Add(box2,1,wxALL,5);
		wxSizer *genSizer2 = new wxStaticBoxSizer(wxVERTICAL,generalPage,_("Limits for Levels and Recent Files"));
		wxFlexGridSizer *genSizer3 = new wxFlexGridSizer(8,2,5,5);
		wxString options[8] = { _T("Undo levels"), _T("Recent timecodes max"), _T("Recent keyframes max"), _T("Recent sub max"), _T("Recent vid max"), _T("Recent aud max"), _T("Recent find max"), _T("Recent replace max") };
		wxString labels[8] = { _T("Maximum undo levels"), _T("Maximum recent timecode files"), _T("Maximum recent keyframe files"), _T("Maximum recent subtitle files"), _T("Maximum recent video files"), _T("Maximum recent audio files"), _T("Maximum recent find strings"), _T("Maximum recent replace strings") };
		for (int i=0;i<8;i++) {
			wxSpinCtrl *spin = new wxSpinCtrl(generalPage,-1,_T(""),wxDefaultPosition,wxSize(70,-1),wxSP_ARROW_KEYS,0,32,0);
			Bind(spin,options[i]);
			genSizer3->Add(new wxStaticText(generalPage,-1,labels[i] + _T(": ")),1,wxALIGN_CENTRE_VERTICAL);
			genSizer3->Add(spin,0);
		}
		genSizer3->AddGrowableCol(0,1);
		genSizer2->Add(genSizer3,1,wxEXPAND | wxALL,5);
		genMainSizer->Add(genSizer1,0,wxEXPAND | wxBOTTOM,5);
		genMainSizer->Add(genSizer2,0,wxEXPAND,0);
		genMainSizer->AddStretchSpacer(1);
		genMainSizer->Fit(generalPage);
		generalPage->SetSizer(genMainSizer);
	}

	//

	// List book
	book->AddPage(generalPage,_T("General"),true);
	book->AddSubPage(filePage,_T("File Save/Load"),true);
	book->AddSubPage(gridPage,_T("Subtitles Grid"),true);
	book->AddSubPage(editPage,_T("Subtitles Edit Box"),true);
	book->AddPage(videoPage,_T("Video"),true);
	book->AddPage(audioPage,_T("Audio"),true);
	book->AddSubPage(displayPage,_T("Display"),true);
	book->AddPage(autoPage,_T("Automation"),true);
	book->ChangeSelection(0);

	// Buttons Sizer
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT,5);
	buttonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT,5);

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book,1,wxEXPAND | wxALL,5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);

	// Read
	ReadFromOptions();
#endif
}


//////////////
// Destructor
DialogOptions::~DialogOptions() {
}


//////////////////////////
// Bind control to option
void DialogOptions::Bind(wxControl *ctrl, wxString option) {
	OptionsBind bind;
	bind.ctrl = ctrl;
	bind.option = option;
	binds.push_back(bind);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogOptions,wxDialog)
	EVT_BUTTON(wxID_OK,DialogOptions::OnOK)
END_EVENT_TABLE()


//////
// OK
void DialogOptions::OnOK(wxCommandEvent &event) {
	WriteToOptions();
	EndModal(0);
}


////////////////////
// Write to options
void DialogOptions::WriteToOptions() {
	// Flags
	bool mustRestart = false;

	// For each bound item
	for (unsigned int i=0;i<binds.size();i++) {
		// Modified?
		bool modified = false;

		// Checkbox
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxCheckBox))) {
			wxCheckBox *check = (wxCheckBox*) binds[i].ctrl;
			if (Options.AsBool(binds[i].option) != check->GetValue()) {
				Options.SetBool(binds[i].option,check->GetValue());
				modified = true;
			}
		}

		// Spin control
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxSpinCtrl))) {
			wxSpinCtrl *spin = (wxSpinCtrl*) binds[i].ctrl;
			if (spin->GetValue() != Options.AsInt(binds[i].option)) {
				Options.SetInt(binds[i].option,spin->GetValue());
				modified = true;
			}
		}

		// Set modification type
		if (modified) {
			ModType type = Options.GetModType(binds[i].option);
			if (type == MOD_RESTART) mustRestart = true;
		}
	}

	// Need restart?
	if (mustRestart) {
		int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
		if (answer == wxYES) {
			FrameMain *frame = (FrameMain*) GetParent();
			if (frame->Close()) wxExecute(AegisubApp::fullPath);
		}
	}
}


/////////////////////
// Read form options
void DialogOptions::ReadFromOptions() {
	for (unsigned int i=0;i<binds.size();i++) {
		// Checkbox
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxCheckBox))) {
			wxCheckBox *check = (wxCheckBox*) binds[i].ctrl;
			check->SetValue(Options.AsBool(binds[i].option));
		}

		// Spin control
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxSpinCtrl))) {
			wxSpinCtrl *spin = (wxSpinCtrl*) binds[i].ctrl;
			spin->SetValue(Options.AsInt(binds[i].option));
		}
	}
}
