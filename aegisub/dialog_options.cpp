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
#include "validators.h"


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

	// File save/load page
	{
		// Sizers
		wxSizer *fileMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *fileSizer1 = new wxStaticBoxSizer(wxVERTICAL,filePage,_("Auto-save"));
		wxSizer *fileSizer2 = new wxBoxSizer(wxHORIZONTAL);
		wxSizer *fileSizer3 = new wxStaticBoxSizer(wxHORIZONTAL,filePage,_("File Paths"));
		wxFlexGridSizer *fileSizer4 = new wxFlexGridSizer(3,2,5,5);
		wxSizer *fileSizer5 = new wxStaticBoxSizer(wxHORIZONTAL,filePage,_("Miscelanea"));
		wxFlexGridSizer *fileSizer6 = new wxFlexGridSizer(3,2,5,5);

		// First static box
		wxCheckBox *check = new wxCheckBox(filePage,-1,_("Auto-backup"));
		Bind(check,_T("Auto backup"));
		wxTextCtrl *edit = new wxTextCtrl(filePage,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(NULL,false));
		Bind(edit,_T("Auto save every seconds"));
		fileSizer2->Add(check,0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,10);
		fileSizer2->AddStretchSpacer(1);
		fileSizer2->Add(new wxStaticText(filePage,-1,_("Auto-save every")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		fileSizer2->Add(edit,0,wxRIGHT,5);
		fileSizer2->Add(new wxStaticText(filePage,-1,_("seconds.")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,0);

		// Second static box
		fileSizer4->Add(new wxStaticText(filePage,-1,_("Auto-save path:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Auto save path"));
		fileSizer4->Add(edit,1,wxEXPAND);
		fileSizer4->Add(new wxStaticText(filePage,-1,_("Auto-backup path:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Auto backup path"));
		fileSizer4->Add(edit,1,wxEXPAND);
		fileSizer4->Add(new wxStaticText(filePage,-1,_("Crash recovery path:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Auto recovery path"));
		fileSizer4->Add(edit,1,wxEXPAND);
		fileSizer4->AddGrowableCol(1,1);

		// Third static box
		fileSizer6->Add(new wxStaticText(filePage,-1,_("Auto-load linked files:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		wxString choices[3] = { _("Never"), _("Always"), _("Ask") };
		wxComboBox *combo = new wxComboBox(filePage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices,wxCB_DROPDOWN | wxCB_READONLY);
		Bind(combo,_T("Autoload linked files"));
		fileSizer6->Add(combo,1,wxEXPAND);
		fileSizer6->Add(new wxStaticText(filePage,-1,_("Text import actor separator:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Text actor separator"));
		fileSizer6->Add(edit,1,wxEXPAND);
		fileSizer6->Add(new wxStaticText(filePage,-1,_("Text import comment starter:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Text comment starter"));
		fileSizer6->Add(edit,1,wxEXPAND);
		fileSizer6->AddGrowableCol(1,1);

		// Sizers
		fileSizer1->Add(fileSizer2,0,wxEXPAND | wxALL,5);
		fileSizer3->Add(fileSizer4,1,wxEXPAND | wxALL,5);
		fileSizer5->Add(fileSizer6,1,wxEXPAND | wxALL,5);
		fileMainSizer->Add(fileSizer1,0,wxEXPAND | wxALL,0);
		fileMainSizer->Add(fileSizer3,0,wxEXPAND | wxALL,0);
		fileMainSizer->Add(fileSizer5,0,wxEXPAND | wxALL,0);
		fileMainSizer->AddStretchSpacer(1);
		fileMainSizer->Fit(filePage);
		filePage->SetSizer(fileMainSizer);
	}

	// List book
	book->AddPage(generalPage,_("General"),true);
	book->AddSubPage(filePage,_("File Save/Load"),true);
	book->AddSubPage(editPage,_("Subtitles Edit Box"),true);
	book->AddSubPage(gridPage,_("Subtitles Grid"),true);
	book->AddPage(videoPage,_("Video"),true);
	book->AddPage(audioPage,_("Audio"),true);
	book->AddSubPage(displayPage,_("Display"),true);
	book->AddPage(autoPage,_("Automation"),true);
	book->ChangeSelection(Options.AsInt(_T("Options Page")));

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
	EVT_BUTTON(wxID_CANCEL,DialogOptions::OnCancel)
END_EVENT_TABLE()


//////
// OK
void DialogOptions::OnOK(wxCommandEvent &event) {
	WriteToOptions();
	Options.SetInt(_T("Options page"),book->GetSelection());
	Options.Save();
	EndModal(0);
}


//////////
// Cancel
void DialogOptions::OnCancel(wxCommandEvent &event) {
	Options.SetInt(_T("Options page"),book->GetSelection());
	Options.Save();
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

		// Text control
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxTextCtrl))) {
			wxTextCtrl *text = (wxTextCtrl*) binds[i].ctrl;
			if (text->GetValue() != Options.AsText(binds[i].option)) {
				Options.ResetWith(binds[i].option,text->GetValue());
				modified = true;
			}
		}

		// Combo box
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxComboBox))) {
			wxComboBox *combo = (wxComboBox*) binds[i].ctrl;
			if (combo->GetSelection != Options.AsInt(binds[i].option)) {
				Options.SetInt(binds[i].option,combo->GetSelection());
				modified = true;
			}
		}

		// Set modification type
		if (modified) {
			ModType type = Options.GetModType(binds[i].option);
			if (type == MOD_RESTART) mustRestart = true;
		}
	}

	// Save options
	Options.Save();

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

		// Text control
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxTextCtrl))) {
			wxTextCtrl *text = (wxTextCtrl*) binds[i].ctrl;
			text->SetValue(Options.AsText(binds[i].option));
		}

		// Combo box
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxComboBox))) {
			wxComboBox *combo = (wxComboBox*) binds[i].ctrl;
			combo->SetSelection(Options.AsInt(binds[i].option));
		}
	}
}
