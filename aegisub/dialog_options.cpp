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
#else
#define AddSubPage(a,b,c) AddPage(a,b,c)
#endif
#include "options.h"
#include <wx/spinctrl.h>
#include "frame_main.h"
#include "main.h"
#include "validators.h"
#include "colour_button.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "video_box.h"
#include "video_slider.h"
#include "audio_box.h"
#include "audio_display.h"


///////////////
// Constructor
DialogOptions::DialogOptions(wxWindow *parent)
: wxDialog(parent, -1, _T("Options"), wxDefaultPosition, wxDefaultSize)
{
	// Create book
	book = new wxTreebook(this,-1,wxDefaultPosition,wxSize(400,300));
	needsRestart = false;

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
		wxSizer *genSizer4 = new wxFlexGridSizer(2,2,5,5);
		wxCheckBox *box = new wxCheckBox(generalPage,-1,_("Show Splash Screen"));
		Bind(box,_T("Show splash"));
		genSizer4->Add(box,1,wxALL,0);
		box = new wxCheckBox(generalPage,-1,_("Show Tip of the Day"));
		Bind(box,_T("Tips enabled"));
		genSizer4->Add(box,1,wxALL,0);
		box = new wxCheckBox(generalPage,-1,_("Auto Check for Updates"));
		Bind(box,_T("Auto check for updates"));
		genSizer4->Add(box,1,wxALL,0);
		genSizer1->Add(genSizer4,1,wxEXPAND|wxALL,5);
		wxSizer *genSizer2 = new wxStaticBoxSizer(wxVERTICAL,generalPage,_("Limits for levels and recent files"));
		wxFlexGridSizer *genSizer3 = new wxFlexGridSizer(8,2,5,5);
		wxString options[8] = { _T("Undo levels"), _T("Recent timecodes max"), _T("Recent keyframes max"), _T("Recent sub max"), _T("Recent vid max"), _T("Recent aud max"), _T("Recent find max"), _T("Recent replace max") };
		wxString labels[8] = { _("Maximum undo levels"), _("Maximum recent timecode files"), _("Maximum recent keyframe files"), _("Maximum recent subtitle files"), _("Maximum recent video files"), _("Maximum recent audio files"), _("Maximum recent find strings"), _("Maximum recent replace strings") };
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
		wxSizer *fileSizer3 = new wxStaticBoxSizer(wxHORIZONTAL,filePage,_("File paths"));
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
		fileMainSizer->Add(fileSizer3,0,wxEXPAND | wxTOP,5);
		fileMainSizer->Add(fileSizer5,0,wxEXPAND | wxTOP,5);
		fileMainSizer->AddStretchSpacer(1);
		fileMainSizer->Fit(filePage);
		filePage->SetSizer(fileMainSizer);
	}

	// Edit box page
	{
		// Sizers
		wxSizer *editMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *editSizer1 = new wxStaticBoxSizer(wxVERTICAL,editPage,_("Options"));
		wxFlexGridSizer *editSizer2 = new wxFlexGridSizer(4,2,5,5);
		wxSizer *editSizer3 = new wxStaticBoxSizer(wxVERTICAL,editPage,_("Style"));
		wxFlexGridSizer *editSizer4 = new wxFlexGridSizer(9,2,2,2);
		wxSizer *editSizer5 = new wxBoxSizer(wxHORIZONTAL);

		// First static box
		wxString labels1[4] = { _("Path to dictionaries"), _("Enable call tips"), _("Link commiting of times"), _("Enable syntax highlighting") };
		wxString options1[4] = { _T("Dictionaries path"), _T("Call Tips Enabled"), _T("Link Time Boxes Commit"), _T("Syntax Highlight Enabled") };
		editSizer2->Add(new wxStaticText(editPage,-1,labels1[0]+_T(": ")),0,wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
		wxTextCtrl *edit = new wxTextCtrl(editPage,-1,_T(""));
		Bind(edit,options1[0]);
		editSizer2->Add(edit,0,wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
		for (int i=1;i<4;i++) {
			wxCheckBox *control = new wxCheckBox(editPage,-1,labels1[i]);
			Bind(control,options1[i]);
			editSizer2->Add(control,1,wxEXPAND,0);
		}
		editSizer2->AddGrowableCol(0,1);
		editSizer2->AddGrowableCol(1,1);

		// Second static box
		wxControl *control;
		wxString labels2[9] = { _("Normal"), _("Brackets"), _("Slashes and Parentheses"), _("Tags"), _("Parameters") ,
			                    _("Error"), _("Error Background"), _("Line Break"), _("Modified Background") };
		wxString options2[11] = { _T("Normal"), _T("Brackets"), _T("Slashes"), _T("Tags"), _T("Parameters") ,
			                      _T("Error"), _T("Error Background"), _T("Line Break"), _T("Edit box need enter background"), _T("Edit Font Face"), _T("Edit Font Size") };
		for (int i=0;i<9;i++) {
			wxString caption = labels2[i]+_T(": ");
			wxString option = options2[i];
			if (i < 8) {
				caption = _("Syntax highlighter - ") + caption;
				option = _T("Syntax highlight ") + option;
			}
			control = new ColourButton(editPage,-1,wxSize(40,10));
			Bind(control,option);
			editSizer4->Add(new wxStaticText(editPage,-1,caption),0,wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
			editSizer4->Add(control,1,wxALIGN_RIGHT,0);
		}
		editSizer4->AddGrowableCol(1,1);

		// Third sizer
		editSizer5->Add(new wxStaticText(editPage,-1,_("Font: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(editPage,-1);
		Bind(control,options2[9]);
		editSizer5->Add(control,1,wxEXPAND | wxRIGHT,5);
		control = new wxTextCtrl(editPage,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(NULL,false));;
		Bind(control,options2[10]);
		editSizer5->Add(control,0,wxEXPAND | wxRIGHT,0);

		// Sizers
		editSizer1->Add(editSizer2,1,wxEXPAND | wxALL,5);
		editSizer3->Add(editSizer4,1,wxEXPAND | wxALL,5);
		editSizer3->Add(editSizer5,0,wxEXPAND | wxALL,5);
		editMainSizer->Add(editSizer1,0,wxEXPAND | wxALL,0);
		editMainSizer->Add(editSizer3,0,wxEXPAND | wxTOP,5);
		editMainSizer->AddStretchSpacer(1);
		editMainSizer->Fit(editPage);
		editPage->SetSizer(editMainSizer);
	}

	// Grid page
	{
		// Sizers
		wxSizer *gridMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *gridSizer1 = new wxStaticBoxSizer(wxVERTICAL,gridPage,_("Options"));
		wxSizer *gridSizer2 = new wxStaticBoxSizer(wxVERTICAL,gridPage,_("Style"));
		wxFlexGridSizer *gridSizer3 = new wxFlexGridSizer(11,2,2,2);
		wxSizer *gridSizer4 = new wxBoxSizer(wxHORIZONTAL);
		wxSizer *gridSizer5 = new wxBoxSizer(wxHORIZONTAL);

		// First sizer
		wxString labels1[2] = { _("Allow grid to take focus"), _("Highlight subtitles that are currently visible in video") };
		wxString options1[2] = { _T("Grid allow focus"), _T("Highlight subs in frame") };
		for (int i=0;i<2;i++) {
			wxCheckBox *control = new wxCheckBox(gridPage,-1,labels1[i]);
			Bind(control,options1[i]);
			gridSizer1->Add(control,1,wxEXPAND | wxALL,5);
		}

		// Second sizer
		wxControl *control;
		wxString labels2[12] = { _("Standard foreground"), _("Standard background"), _("Selection foreground"), 
			                     _("Selection background"), _("Comment background"), _("Selected comment background"),
								 _("Collision foreground"), _("Line in frame background"), _("Header"),
		                         _("Left Column"), _("Active Line Border"), _("Lines") };
		wxString options2[12] = { _T("standard foreground"), _T("background"), _T("selection foreground"),
		                         _("selection background"), _T("comment background"), _T("selected comment background"),
		                        _T("collision foreground") , _T("inframe background"), _T("header"),
		                         _T("left column"), _T("active border"), _T("lines") };
		for (int i=0;i<12;i++) {
			wxString caption = labels2[i] + _T(": ");
			wxString option = _T("Grid ") + options2[i];
			control = new ColourButton(gridPage,-1,wxSize(40,10));
			Bind(control,option);
			gridSizer3->Add(new wxStaticText(gridPage,-1,caption),0,wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
			gridSizer3->Add(control,1,wxALIGN_CENTER,0);
		}
		gridSizer3->AddGrowableCol(0,1);

		// Third sizer
		gridSizer4->Add(new wxStaticText(gridPage,-1,_("Font: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(gridPage,-1);
		Bind(control,_T("Grid font face"));
		gridSizer4->Add(control,1,wxEXPAND | wxRIGHT,5);
		control = new wxTextCtrl(gridPage,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(NULL,false));;
		Bind(control,_T("Grid font size"));
		gridSizer4->Add(control,0,wxEXPAND | wxRIGHT,0);

		// Fourth sizer
		gridSizer5->Add(new wxStaticText(gridPage,-1,_("Replace override tags with: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(gridPage,-1);
		Bind(control,_T("Grid hide overrides char"));
		gridSizer5->Add(control,1,wxEXPAND | wxRIGHT,5);

		// Sizers
		gridSizer2->Add(gridSizer3,1,wxEXPAND | wxALL,5);
		gridSizer2->Add(gridSizer4,0,wxEXPAND | wxALL,5);
		gridSizer2->Add(gridSizer5,0,wxEXPAND | wxALL,5);
		gridMainSizer->Add(gridSizer1,0,wxEXPAND | wxALL,0);
		gridMainSizer->Add(gridSizer2,0,wxEXPAND | wxTOP,5);
		gridMainSizer->AddStretchSpacer(1);
		gridMainSizer->Fit(gridPage);
		gridPage->SetSizer(gridMainSizer);
	}

	// Video page
	{
		// Sizers
		wxSizer *videoMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *videoSizer1 = new wxStaticBoxSizer(wxVERTICAL,videoPage,_("Options"));
		wxSizer *videoSizer2 = new wxStaticBoxSizer(wxVERTICAL,videoPage,_("Advanced - EXPERT USERS ONLY"));
		wxFlexGridSizer *videoSizer3 = new wxFlexGridSizer(5,2,5,5);
		wxFlexGridSizer *videoSizer4 = new wxFlexGridSizer(4,2,5,5);
		wxControl *control;

		// First sizer
		videoSizer3->Add(new wxStaticText(videoPage,-1,_("Check video resolution on open: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString choices1[3] = { _("Never"), _("Ask"), _("Always") };
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices1,wxCB_READONLY | wxCB_DROPDOWN);
		Bind(control,_T("Video check script res"));
		videoSizer3->Add(control,1,wxEXPAND);
		videoSizer3->Add(new wxStaticText(videoPage,-1,_("Default Zoom: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxArrayString choices2;
		for (int i=1;i<=16;i++) {
			wxString toAdd = wxString::Format(_T("%i"),int(i*12.5));
			if (i%2) toAdd += _T(".5");
			toAdd += _T("%");
			choices2.Add(toAdd);
		}
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,choices2,wxCB_READONLY | wxCB_DROPDOWN);
		Bind(control,_T("Video Default Zoom"));
		videoSizer3->Add(control,1,wxEXPAND);
		videoSizer3->Add(new wxStaticText(videoPage,-1,_("Fast jump step in frames: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Video fast jump step"));
		videoSizer3->Add(control,1,wxEXPAND);
		videoSizer3->Add(new wxStaticText(videoPage,-1,_("Screenshot save path: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString choices3[3] = { _T("?video"), _T("?script"), _T(".") };
		//control = new wxTextCtrl(videoPage,-1);
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices3,wxCB_DROPDOWN);
		Bind(control,_T("Video screenshot path"));
		videoSizer3->Add(control,1,wxEXPAND);
		control = new wxCheckBox(videoPage,-1,_("Show keyframes in slider"));
		Bind(control,_T("Show keyframes on video slider"));
		videoSizer3->Add(control,0,wxEXPAND);
		videoSizer3->AddGrowableCol(1,1);

		// Second sizer
		videoSizer4->Add(new wxStaticText(videoPage,-1,_("Video provider: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxArrayString choices4;
#ifdef __WINDOWS__
		choices4.Add(_T("Avisynth"));
#endif
#if USE_LAVC == 1
		choices4.Add(_T("ffmpeg"));
#endif
#if USE_DIRECTSHOW == 1
		choices4.Add(_T("dshow"));
#endif
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,choices4,wxCB_DROPDOWN | wxCB_READONLY);
		Bind(control,_T("Video provider"),1);
		videoSizer4->Add(control,1,wxEXPAND);
		videoSizer4->Add(new wxStaticText(videoPage,-1,_("Avisynth video resizer: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString choices5[3] = { _T("BilinearResize"), _T("BicubicResize"), _T("LanczosResize") };
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices5,wxCB_DROPDOWN);
		Bind(control,_T("Video resizer"));
		videoSizer4->Add(control,1,wxEXPAND);
		videoSizer4->Add(new wxStaticText(videoPage,-1,_("Avisynth memory limit: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator(NULL,false));
		Bind(control,_T("Avisynth memorymax"));
		videoSizer4->Add(control,1,wxEXPAND);
		control = new wxCheckBox(videoPage,-1,_("Threaded video"));
		Bind(control,_T("Threaded video"));
		videoSizer4->Add(control,1,wxEXPAND);
		control = new wxCheckBox(videoPage,-1,_("Allow pre-2.56a Avisynth"));
		Bind(control,_T("Allow Ancient Avisynth"));
		videoSizer4->Add(control,1,wxEXPAND);
		videoSizer4->AddGrowableCol(1,1);

		// Sizers
		videoSizer1->Add(videoSizer3,1,wxEXPAND | wxALL,5);
		videoSizer2->Add(new wxStaticText(videoPage,-1,_("WARNING: Changing these settings might result in bugs,\ncrashes, glitches and/or movax.\nDon't touch these unless you know what you're doing.")),0,wxEXPAND | wxALL,5);
		videoSizer2->Add(videoSizer4,1,wxEXPAND | wxALL,5);
		videoMainSizer->Add(videoSizer1,0,wxEXPAND | wxALL,0);
		videoMainSizer->Add(videoSizer2,0,wxEXPAND | wxTOP,5);
		videoMainSizer->AddStretchSpacer(1);
		videoMainSizer->Fit(videoPage);
		videoPage->SetSizer(videoMainSizer);
	}

	// Audio page
	{
		// Sizers
		wxSizer *audioMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *audioSizer1 = new wxStaticBoxSizer(wxVERTICAL,audioPage,_("Options"));
		wxSizer *audioSizer2 = new wxStaticBoxSizer(wxVERTICAL,audioPage,_("Advanced - EXPERT USERS ONLY"));
		wxFlexGridSizer *audioSizer3 = new wxFlexGridSizer(2,2,5,5);
		wxFlexGridSizer *audioSizer4 = new wxFlexGridSizer(4,2,5,5);
		wxFlexGridSizer *audioSizer5 = new wxFlexGridSizer(4,2,5,5);
		wxControl *control;

		// First sizer
		control = new wxCheckBox(audioPage,-1,_("Next line on commit"));
		Bind(control,_T("Audio Next Line on Commit"));
		audioSizer3->Add(control,1,wxEXPAND,0);
		control = new wxCheckBox(audioPage,-1,_("Auto-focus on mouse over"));
		Bind(control,_T("Audio Autofocus"));
		audioSizer3->Add(control,1,wxEXPAND,0);
		control = new wxCheckBox(audioPage,-1,_("Default mouse wheel to zoom"));
		Bind(control,_T("Audio Wheel Default To Zoom"));
		audioSizer3->Add(control,1,wxEXPAND,0);
		control = new wxCheckBox(audioPage,-1,_("Lock scroll on Cursor"));
		Bind(control,_T("Audio lock scroll on cursor"));
		audioSizer3->Add(control,1,wxEXPAND,0);
		audioSizer3->AddGrowableCol(0,1);

		// Second sizer
		control = new wxTextCtrl(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Timing Default Duration"));
		audioSizer4->Add(new wxStaticText(audioPage,-1,_("Default timing length: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer4->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Audio lead in"));
		audioSizer4->Add(new wxStaticText(audioPage,-1,_("Default lead-in length: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer4->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Audio lead out"));
		audioSizer4->Add(new wxStaticText(audioPage,-1,_("Default lead-out length: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer4->Add(control,1,wxEXPAND,0);
		wxString choices1[3] = { _("Don't show"), _("Show previous"), _("Show all") };
		control = new wxComboBox(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices1,wxCB_READONLY | wxCB_DROPDOWN);
		Bind(control,_T("Audio Inactive Lines Display Mode"));
		audioSizer4->Add(new wxStaticText(audioPage,-1,_("Show inactive lines: ")),0,wxRIGHT,5);
		audioSizer4->Add(control,1,wxEXPAND,0);
		audioSizer4->AddGrowableCol(0,1);

		// Third sizer
		wxString choices2[3] = { _("None (NOT RECOMMENDED)"), _("RAM"), _("Hard Disk") };
		control = new wxComboBox(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices2,wxCB_READONLY | wxCB_DROPDOWN);
		Bind(control,_T("Audio Cache"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("Cache type: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		wxString choices3[3] = { _T("ConvertToMono"), _T("GetLeftChannel"), _T("GetRightChannel") };
		control = new wxComboBox(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices3,wxCB_DROPDOWN);
		Bind(control,_T("Audio Downmixer"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("Avisynth down-mixer: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1);
		Bind(control,_T("Audio HD Cache Location"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("HD cache path: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1);
		Bind(control,_T("Audio HD Cache Name"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("HD cache name: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Audio Spectrum Cutoff"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("Spectrum cutoff: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		control = new wxTextCtrl(audioPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
		Bind(control,_T("Audio Spectrum Window"));
		audioSizer5->Add(new wxStaticText(audioPage,-1,_("Spectrum FFT window exponent: ")),0,wxRIGHT | wxALIGN_CENTER_VERTICAL,5);
		audioSizer5->Add(control,1,wxEXPAND,0);
		audioSizer5->AddGrowableCol(0,1);

		// Sizers
		audioSizer1->Add(audioSizer3,0,wxEXPAND | wxALL,5);
		audioSizer1->Add(audioSizer4,1,wxEXPAND | wxALL,5);
		audioSizer2->Add(new wxStaticText(audioPage,-1,_("WARNING: Changing these settings might result in bugs,\ncrashes, glitches and/or movax.\nDon't touch these unless you know what you're doing.")),0,wxEXPAND | wxALL,5);
		audioSizer2->Add(audioSizer5,1,wxEXPAND | wxALL,5);
		audioMainSizer->Add(audioSizer1,0,wxEXPAND | wxALL,0);
		audioMainSizer->Add(audioSizer2,0,wxEXPAND | wxTOP,5);
		audioMainSizer->AddStretchSpacer(1);
		audioMainSizer->Fit(audioPage);
		audioPage->SetSizer(audioMainSizer);
	}

	// Audio display page
	{
		// Sizers
		wxSizer *displayMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *displaySizer1 = new wxStaticBoxSizer(wxVERTICAL,displayPage,_("Options"));
		wxSizer *displaySizer2 = new wxStaticBoxSizer(wxVERTICAL,displayPage,_("Style"));
		wxFlexGridSizer *displaySizer3 = new wxFlexGridSizer(2,2,2,2);
		wxFlexGridSizer *displaySizer4 = new wxFlexGridSizer(14,2,2,2);

		// First sizer
		wxString labels1[4] = { _("Draw secondary lines"), _("Draw selection background"), _("Draw timeline"), _("Draw cursor time") };
		wxString options1[4] = { _T("Draw Secondary Lines"), _T("Draw Selection Background") , _T("Draw Timeline"), _T("Draw Cursor Time")};
		for (int i=0;i<4;i++) {
			wxCheckBox *control = new wxCheckBox(displayPage,-1,labels1[i]);
			Bind(control,_T("Audio ") + options1[i]);
			displaySizer3->Add(control,1,wxEXPAND | wxALL,5);
		}

		// Second sizer
		wxControl *control;
		wxString labels2[14] = { _("Play cursor"), _("Background"), _("Selection background"), 
		                         _("Selection background - modified"), _("Seconds boundary"), _("Waveform"), 
		                         _("Waveform - selection"), _("Waveform - modified"), _("Waveform - inactive"), 
		                         _("Boundary - start"), _("Boundary - end"), _("Boundary - inactive"), 
		                         _("Syllable text"), _("Syllable boundary") };
		wxString options2[14] = { _T("Play cursor"), _T("Background"), _T("Selection background"), 
		                          _T("Selection background modified"), _T("Seconds boundaries"), _T("Waveform"), 
		                          _T("Waveform selected"), _T("Waveform Modified"), _T("Waveform Inactive"), 
		                          _T("Line boundary start"), _T("Line boundary end"), _T("Line boundary inactive line"), 
		                          _T("Syllable text"), _T("Syllable boundaries") };
		for (int i=0;i<14;i++) {
			wxString caption = labels2[i] + _T(": ");
			wxString option = _T("Audio ") + options2[i];
			control = new ColourButton(displayPage,-1,wxSize(40,10));
			Bind(control,option);
			displaySizer4->Add(new wxStaticText(displayPage,-1,caption),0,wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
			displaySizer4->Add(control,1,wxALIGN_CENTER,0);
		}
		displaySizer4->AddGrowableCol(0,1);

		// Sizers
		displaySizer1->Add(displaySizer3,1,wxEXPAND | wxALL,5);
		displaySizer2->Add(displaySizer4,1,wxEXPAND | wxALL,5);
		displayMainSizer->Add(displaySizer1,0,wxEXPAND | wxALL,0);
		displayMainSizer->Add(displaySizer2,0,wxEXPAND | wxTOP,5);
		displayMainSizer->AddStretchSpacer(1);
		displayMainSizer->Fit(displayPage);
		displayPage->SetSizer(displayMainSizer);
	}

	// Automation page
	{
		// Sizers
		wxSizer *autoMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *autoSizer1 = new wxStaticBoxSizer(wxVERTICAL,autoPage,_("Options"));
		wxFlexGridSizer *autoSizer2 = new wxFlexGridSizer(4,2,5,5);
		wxControl *control;

		// First sizer
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Base path: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(autoPage,-1);
		Bind(control,_T("Automation Base Path"));
		autoSizer2->Add(control,1,wxEXPAND);
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Include path: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(autoPage,-1);
		Bind(control,_T("Automation Include Path"));
		autoSizer2->Add(control,1,wxEXPAND);
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Auto-load path: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(autoPage,-1);
		Bind(control,_T("Automation Autoload Path"));
		autoSizer2->Add(control,1,wxEXPAND);
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Trace level: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString trace_choices[6] = { _("0: Fatal"), _("1: Error"), _("2: Warning"), _("3: Hint"), _("4: Debug"), _("5: Trace") };
		control = new wxComboBox(autoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,6,trace_choices,wxCB_READONLY | wxCB_DROPDOWN);
		Bind(control,_T("Automation Trace Level"));
		autoSizer2->Add(control,1,wxEXPAND);
		autoSizer2->AddGrowableCol(1,1);
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Thread priority: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString prio_choices[3] = { _("Normal"), _("Below Normal (recommended)"), _("Lowest") };
		control = new wxComboBox(autoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,prio_choices,wxCB_READONLY|wxCB_DROPDOWN);
		Bind(control, _T("Automation Thread Priority"));
		autoSizer2->Add(control, 1, wxEXPAND);
		autoSizer2->Add(new wxStaticText(autoPage,-1,_("Autoreload on Export: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxString reload_choices[4] = { _("No scripts"), _("Subtitle-local scripts"), _("Global autoload scripts"), _("All scripts") };
		control = new wxComboBox(autoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,4,reload_choices,wxCB_READONLY|wxCB_DROPDOWN);
		Bind(control, _T("Automation Autoreload Mode"));
		autoSizer2->Add(control, 1, wxEXPAND);

		// Sizers
		autoSizer1->Add(autoSizer2,1,wxEXPAND | wxALL,5);
		autoMainSizer->Add(autoSizer1,0,wxEXPAND | wxALL,0);
		autoMainSizer->AddStretchSpacer(1);
		autoMainSizer->Fit(autoPage);
		autoPage->SetSizer(autoMainSizer);
	}

	// List book
	book->AddPage(generalPage,_("General"),true);
	book->AddSubPage(filePage,_("File save/load"),true);
	book->AddSubPage(editPage,_("Subtitles edit box"),true);
	book->AddSubPage(gridPage,_("Subtitles grid"),true);
	book->AddPage(videoPage,_("Video"),true);
	book->AddPage(audioPage,_("Audio"),true);
	book->AddSubPage(displayPage,_("Display"),true);
	book->AddPage(autoPage,_("Automation"),true);
	#ifdef wxUSE_TREEBOOK
	book->ChangeSelection(Options.AsInt(_T("Options page")));
	#endif

	// Buttons Sizer
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT,5);
	buttonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT,5);
	buttonSizer->Add(new wxButton(this,wxID_APPLY),0,wxRIGHT,5);

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book,1,wxEXPAND | wxALL,5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);
	CenterOnParent();

	// Read
	ReadFromOptions();
}


//////////////
// Destructor
DialogOptions::~DialogOptions() {
}


//////////////////////////
// Bind control to option
void DialogOptions::Bind(wxControl *ctrl, wxString option,int param) {
	OptionsBind bind;
	bind.ctrl = ctrl;
	bind.option = option;
	bind.param = param;
	binds.push_back(bind);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogOptions,wxDialog)
	EVT_BUTTON(wxID_OK,DialogOptions::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogOptions::OnCancel)
	EVT_BUTTON(wxID_APPLY,DialogOptions::OnApply)
END_EVENT_TABLE()


//////
// OK
void DialogOptions::OnOK(wxCommandEvent &event) {
	Options.SetInt(_T("Options page"),book->GetSelection());
	WriteToOptions();
	EndModal(0);

	// Restart
	if (needsRestart) {
		int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
		if (answer == wxYES) {
			FrameMain *frame = (FrameMain*) GetParent();
			if (frame->Close()) wxExecute(AegisubApp::fullPath);
		}
	}
}


/////////
// Apply
void DialogOptions::OnApply(wxCommandEvent &event) {
	Options.SetInt(_T("Options page"),book->GetSelection());
	WriteToOptions(true);
}


//////////
// Cancel
void DialogOptions::OnCancel(wxCommandEvent &event) {
	Options.SetInt(_T("Options page"),book->GetSelection());
	Options.Save();
	EndModal(0);

	// Restart
	if (needsRestart) {
		int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
		if (answer == wxYES) {
			FrameMain *frame = (FrameMain*) GetParent();
			if (frame->Close()) wxExecute(AegisubApp::fullPath);
		}
	}
}



////////////////////
// Write to options
void DialogOptions::WriteToOptions(bool justApply) {
	// Flags
	bool mustRestart = false;
	bool editBox = false;
	bool grid = false;
	bool video = false;
	bool audio = false;

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
			int style = combo->GetWindowStyleFlag();

			// Read-only, use as value
			if (style & wxCB_READONLY && binds[i].param == 0) {
				if (combo->GetSelection() != Options.AsInt(binds[i].option)) {
					Options.SetInt(binds[i].option,combo->GetSelection());
					modified = true;
				}
			}

			// Editable, use as text
			else {
				if (!combo->GetValue().IsEmpty() && combo->GetValue() != Options.AsText(binds[i].option)) {
					Options.SetText(binds[i].option,combo->GetValue());
					modified = true;
				}
			}
		}

		// Colour button
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxBitmapButton))) {
			ColourButton *button = (ColourButton*) binds[i].ctrl;
			if (button->GetColour() != Options.AsColour(binds[i].option)) {
				Options.SetColour(binds[i].option,button->GetColour());
				modified = true;
			}
		}

		// Set modification type
		if (modified) {
			ModType type = Options.GetModType(binds[i].option);
			if (type == MOD_RESTART) mustRestart = true;
			if (type == MOD_EDIT_BOX) editBox = true;
			if (type == MOD_GRID) grid = true;
			if (type == MOD_VIDEO) video = true;
			if (type == MOD_AUDIO) audio = true;
		}
	}

	// Save options
	Options.Save();

	// Need restart?
	if (mustRestart) {
		if (justApply) needsRestart = true;
		else {
			int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
			if (answer == wxYES) {
				FrameMain *frame = (FrameMain*) GetParent();
				if (frame->Close()) wxExecute(AegisubApp::fullPath);
			}
		}
	}

	// Other modifications
	if (!mustRestart || justApply) {
		// Edit box
		if (editBox) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->EditBox->TextEdit->SetStyles();
			frame->EditBox->TextEdit->UpdateStyle();
		}
		
		// Grid
		if (grid) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->SubsBox->UpdateStyle();
		}

		// Video
		if (video) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->videoBox->videoSlider->Refresh();
		}

		// Audio
		if (audio) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->audioBox->audioDisplay->RecreateImage();
			frame->audioBox->audioDisplay->Refresh();
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
			int style = combo->GetWindowStyleFlag();

			// Read-only, use as value
			if (style & wxCB_READONLY && binds[i].param == 0) {
				combo->SetSelection(Options.AsInt(binds[i].option));
			}

			// Editable, set text
			else {
				wxString value = Options.AsText(binds[i].option);
				if (!(style & wxCB_READONLY) && combo->FindString(value) == wxNOT_FOUND) combo->Append(value);
				combo->SetValue(value);
			}
		}

		// Colour button
		if (binds[i].ctrl->IsKindOf(CLASSINFO(wxBitmapButton))) {
			ColourButton *button = (ColourButton*) binds[i].ctrl;
			button->SetColour(Options.AsColour(binds[i].option));
		}
	}
}
