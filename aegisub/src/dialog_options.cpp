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
#include "config.h"

#include "dialog_options.h"
#if wxUSE_TREEBOOK && !__WXMAC__
#include <wx/treebook.h>
#else
#define AddSubPage(page,text,select) AddPage(page,wxString::Format(_T("\t%s"),text),select)
#endif
#include "options.h"
#include <wx/spinctrl.h>
#include <wx/stdpaths.h>
#include <wx/filefn.h>
#include "frame_main.h"
#include "standard_paths.h"
#include "validators.h"
#include "colour_button.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "video_box.h"
#include "video_slider.h"
#include "video_provider_manager.h"
#include "subtitles_provider_manager.h"
#include "audio_box.h"
#include "audio_display.h"
#include "video_context.h"
#include "browse_button.h"
#include "tooltip_manager.h"
#include "utils.h"
#include "help_button.h"


///////
// IDs
enum {
	 BUTTON_DEFAULTS = 2500,
	 HOTKEY_LIST,
	 BUTTON_HOTKEY_SET,
	 BUTTON_HOTKEY_CLEAR,
	 BUTTON_HOTKEY_DEFAULT,
	 BUTTON_HOTKEY_DEFAULT_ALL
};


///////////////
// Constructor
DialogOptions::DialogOptions(wxWindow *parent)
: wxDialog(parent, -1, _("Options"), wxDefaultPosition, wxDefaultSize)
{
	// Set icon
	SetIcon(BitmapToIcon(wxBITMAP(options_button)));

	// Create book
	book = new wxTreebook(this,-1,wxDefaultPosition,wxSize(400,300));
	needsRestart = false;

	// Panels
	wxPanel *generalPage = new wxPanel(book,-1);
	wxPanel *filePage = new wxPanel(book,-1);
	wxPanel *gridPage = new wxPanel(book,-1);
	wxPanel *editPage = new wxPanel(book,-1);
	wxPanel *videoPage = new wxPanel(book,-1);
	wxPanel *audioPage = new wxPanel(book,-1);
	wxPanel *audioAdvPage = new wxPanel(book,-1);
	wxPanel *displayPage = new wxPanel(book,-1);
	wxPanel *autoPage = new wxPanel(book,-1);
	wxPanel *hotkeysPage = new wxPanel(book,-1);
	BrowseButton *browse;

	// General page
	{
		wxSizer *genMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *genSizer1 = new wxStaticBoxSizer(wxHORIZONTAL,generalPage,_("Startup"));
		wxFlexGridSizer *genSizer4 = new wxFlexGridSizer(2,2,5,5);

		AddCheckBox(generalPage,genSizer4,_("Show Splash Screen"),_T("Show splash"));
		AddCheckBox(generalPage,genSizer4,_("Show Tip of the Day"),_T("Tips enabled"));
#ifdef __WXMSW__
		AddCheckBox(generalPage,genSizer4,_("Auto Check for Updates"),_T("Auto check for updates"));
#endif
		AddCheckBox(generalPage,genSizer4,_("Save config.dat locally"),_T("Local config"));
		genSizer4->AddGrowableCol(0,1);

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
		wxFlexGridSizer *fileSizer4 = new wxFlexGridSizer(3,3,5,5);
		wxSizer *fileSizer5 = new wxStaticBoxSizer(wxHORIZONTAL,filePage,_("Miscellanea"));
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
		browse = new BrowseButton(filePage,-1,_T(""),BROWSE_FOLDER);
		browse->Bind(edit);
		fileSizer4->Add(browse,0,wxEXPAND);

		fileSizer4->Add(new wxStaticText(filePage,-1,_("Auto-backup path:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Auto backup path"));
		fileSizer4->Add(edit,1,wxEXPAND);
		browse = new BrowseButton(filePage,-1,_T(""),BROWSE_FOLDER);
		browse->Bind(edit);
		fileSizer4->Add(browse,0,wxEXPAND);

		fileSizer4->Add(new wxStaticText(filePage,-1,_("Crash recovery path:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		edit = new wxTextCtrl(filePage,-1);
		Bind(edit,_T("Auto recovery path"));
		fileSizer4->Add(edit,1,wxEXPAND);
		browse = new BrowseButton(filePage,-1,_T(""),BROWSE_FOLDER);
		browse->Bind(edit);
		fileSizer4->Add(browse,0,wxEXPAND);
		fileSizer4->AddGrowableCol(1,1);

		// Third static box
		fileSizer6->Add(new wxStaticText(filePage,-1,_("Auto-load linked files:")),0,wxRIGHT | wxALIGN_CENTRE_VERTICAL,5);
		wxString choices[3] = { _("Never"), _("Always"), _("Ask") };
		wxComboBox *combo = new wxComboBox(filePage,-1,_T(""),wxDefaultPosition,wxDefaultSize,3,choices,wxCB_DROPDOWN | wxCB_READONLY);
		Bind(combo,_T("Autoload linked files"));
		fileSizer6->Add(combo,1,wxEXPAND);
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
		wxSizer *editSizer6 = new wxBoxSizer(wxHORIZONTAL);
		wxFlexGridSizer *editSizer2 = new wxFlexGridSizer(4,2,5,5);
		wxSizer *editSizer3 = new wxStaticBoxSizer(wxVERTICAL,editPage,_("Style"));
		wxFlexGridSizer *editSizer4 = new wxFlexGridSizer(9,2,2,2);
		wxSizer *editSizer5 = new wxBoxSizer(wxHORIZONTAL);

		// First static box
		wxString labels1[4] = { _("Enable call tips"), _("Enable syntax highlighting"), _("Link commiting of times"), _("Overwrite-Insertion in time boxes") };
		wxString options1[4] = { _T("Call Tips Enabled"), _T("Syntax Highlight Enabled"), _T("Link Time Boxes Commit"), _T("Insert Mode on Time Boxes") };
		for (int i=0;i<4;i++) {
			wxCheckBox *control = new wxCheckBox(editPage,-1,labels1[i]);
			Bind(control,options1[i]);
			editSizer2->Add(control,1,wxEXPAND,0);
		}
		//editSizer2->AddGrowableCol(0,1);
		editSizer2->AddGrowableCol(1,1);
		editSizer6->Add(new wxStaticText(editPage,-1,_("Path to dictionary files:")),0,wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
		wxTextCtrl *edit = new wxTextCtrl(editPage,-1,_T(""));
		Bind(edit,_T("Dictionaries path"));
		editSizer6->Add(edit,1,wxEXPAND|wxALIGN_CENTER_VERTICAL|wxRIGHT,5);
		browse = new BrowseButton(editPage,-1,_T(""),BROWSE_FOLDER);
		browse->Bind(edit);
		editSizer6->Add(browse,0,wxEXPAND);

		// Second static box
		wxControl *control;
		wxString labels2[10] = { _("Normal"), _("Brackets"), _("Slashes and Parentheses"), _("Tags"), _("Parameters") ,
			                    _("Error"), _("Error Background"), _("Line Break"), _("Karaoke templates"), _("Modified Background") };
		wxString options2[12] = { _T("Normal"), _T("Brackets"), _T("Slashes"), _T("Tags"), _T("Parameters") ,
			                      _T("Error"), _T("Error Background"), _T("Line Break"), _T("Karaoke Template"), _T("Edit box need enter background"), _T("Edit Font Face"), _T("Edit Font Size") };
		for (int i=0;i<10;i++) {
			wxString caption = labels2[i]+_T(": ");
			wxString option = options2[i];
			if (i < 9) {
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
		Bind(control,options2[10]);
		browse = new BrowseButton(editPage,-1,_T(""),BROWSE_FONT);
		browse->Bind((wxTextCtrl*)control);
		editSizer5->Add(control,1,wxEXPAND | wxRIGHT,5);
		control = new wxTextCtrl(editPage,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(NULL,false));;
		Bind(control,options2[11]);
		editSizer5->Add(control,0,wxEXPAND | wxRIGHT,5);
		browse->Bind((wxTextCtrl*)control,1);
		editSizer5->Add(browse,0,wxEXPAND);

		// Sizers
		editSizer1->Add(editSizer2,1,wxEXPAND | wxALL,5);
		editSizer1->Add(editSizer6,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
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
		                         _T("selection background"), _T("comment background"), _T("selected comment background"),
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
		browse = new BrowseButton(gridPage,-1,_T(""),BROWSE_FONT);
		browse->Bind((wxTextCtrl*)control);
		gridSizer4->Add(control,1,wxEXPAND | wxRIGHT,5);
		control = new wxTextCtrl(gridPage,-1,_T(""),wxDefaultPosition,wxSize(50,-1),0,NumValidator(NULL,false));;
		Bind(control,_T("Grid font size"));
		browse->Bind((wxTextCtrl*)control,1);
		gridSizer4->Add(control,0,wxEXPAND | wxRIGHT,5);
		gridSizer4->Add(browse,0,wxEXPAND);

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
		wxFlexGridSizer *videoSizer4 = new wxFlexGridSizer(5,2,5,5);
		wxControl *control;

		// First sizer
		videoSizer3->Add(new wxStaticText(videoPage,-1,_("Match video resolution on open: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
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
		wxArrayString choices4 = VideoProviderFactoryManager::GetFactoryList();
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,choices4,wxCB_DROPDOWN | wxCB_READONLY);
		Bind(control,_T("Video provider"),1);
		videoSizer4->Add(control,1,wxEXPAND);
		videoSizer4->Add(new wxStaticText(videoPage,-1,_("Subtitles provider: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		wxArrayString choices5 = SubtitlesProviderFactoryManager::GetFactoryList();
		control = new wxComboBox(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,choices5,wxCB_DROPDOWN | wxCB_READONLY);
		Bind(control,_T("Subtitles provider"),1);
		videoSizer4->Add(control,1,wxEXPAND);
#ifdef WIN32
		videoSizer4->Add(new wxStaticText(videoPage,-1,_("Avisynth memory limit: ")),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
		control = new wxTextCtrl(videoPage,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator(NULL,false));
		Bind(control,_T("Avisynth memorymax"));
		videoSizer4->Add(control,1,wxEXPAND);
		//control = new wxCheckBox(videoPage,-1,_("Threaded video"));
		//Bind(control,_T("Threaded video"));
		//videoSizer4->Add(control,1,wxEXPAND);
		//control = new wxCheckBox(videoPage,-1,_("Use pixel shaders if available"));
		//Bind(control,_T("Video use pixel shaders"));
		//videoSizer4->Add(control,1,wxEXPAND);
		control = new wxCheckBox(videoPage,-1,_("Allow pre-2.56a Avisynth"));
		Bind(control,_T("Allow Ancient Avisynth"));
		videoSizer4->Add(control,1,wxEXPAND);
		videoSizer4->AddGrowableCol(1,1);
		control = new wxCheckBox(videoPage,-1,_("Avisynth renders its own subs"));
		Bind(control,_T("Avisynth render own subs"));
		videoSizer4->Add(control,1,wxEXPAND);
#endif

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
		wxFlexGridSizer *audioSizer3 = new wxFlexGridSizer(3,2,5,5);
		wxFlexGridSizer *audioSizer4 = new wxFlexGridSizer(4,2,5,5);

		// First sizer
		AddCheckBox(audioPage,audioSizer3,_("Grab times from line upon selection"),_T("Audio grab times on select"));
		AddCheckBox(audioPage,audioSizer3,_("Default mouse wheel to zoom"),_T("Audio Wheel Default To Zoom"));
		AddCheckBox(audioPage,audioSizer3,_("Lock scroll on Cursor"),_T("Audio lock scroll on cursor"));
		AddCheckBox(audioPage,audioSizer3,_("Snap to keyframes"),_T("Audio snap to keyframes"));
		AddCheckBox(audioPage,audioSizer3,_("Snap to adjacent lines"),_T("Audio snap to other lines"));
		AddCheckBox(audioPage,audioSizer3,_("Auto-focus on mouse over"),_T("Audio Autofocus"));
		audioSizer3->AddGrowableCol(1,1);

		// Second sizer
		wxString choices1[3] = { _("Don't show"), _("Show previous"), _("Show all") };
		AddTextControl(audioPage,audioSizer4,_("Default timing length"),_T("Timing Default Duration"),TEXT_TYPE_NUMBER);
		AddTextControl(audioPage,audioSizer4,_("Default lead-in length"),_T("Audio lead in"),TEXT_TYPE_NUMBER);
		AddTextControl(audioPage,audioSizer4,_("Default lead-out length"),_T("Audio lead out"),TEXT_TYPE_NUMBER);
		AddComboControl(audioPage,audioSizer4,_("Show inactive lines"),_T("Audio Inactive Lines Display Mode"),wxArrayString(3,choices1));
		/*
		 * Option not in dialogue because it breaks the documentation.
		 * The default should be good enough for most people and it can still be edited manually.
		 * This should be enabled when we can raise the UI/feature freeze towards 2.2.0.
		 *   -jfs
		 *
		AddTextControl(audioPage,audioSizer4,_("Start-marker drag sensitivity"),_T("Audio Start Drag Sensitivity"),TEXT_TYPE_NUMBER);
		*/
		audioSizer4->AddGrowableCol(0,1);

		// Sizers
		audioSizer1->Add(audioSizer3,0,wxEXPAND | wxALL,5);
		audioSizer1->Add(audioSizer4,1,wxEXPAND | wxALL,5);
		audioMainSizer->Add(audioSizer1,0,wxEXPAND | wxALL,0);
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
		wxFlexGridSizer *displaySizer3 = new wxFlexGridSizer(3,2,2,2);
		wxFlexGridSizer *displaySizer4 = new wxFlexGridSizer(14,2,2,2);

		// First sizer
		wxString labels1[6] = { _("Draw secondary lines"), _("Draw selection background"), _("Draw timeline"),
								_("Draw cursor time"), _("Draw keyframes"), _("Draw video position") };
		wxString options1[6] = { _T("Draw Secondary Lines"), _T("Draw Selection Background") , _T("Draw Timeline"),
								_T("Draw Cursor Time"), _T("Draw keyframes"), _T("Draw video position")};
		for (int i=0;i<6;i++) {
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

	// Audio advanced page
	{
		// Sizers
		wxFlexGridSizer *audioAdvSizer1 = new wxFlexGridSizer(6,2,5,5);
		wxSizer *audioAdvSizer2 = new wxStaticBoxSizer(wxVERTICAL,audioAdvPage,_("Advanced - EXPERT USERS ONLY"));
		wxSizer *audioAdvSizer3 = new wxBoxSizer(wxVERTICAL);

		// Controls
		wxString choices2[3] = { _("None (NOT RECOMMENDED)"), _("RAM"), _("Hard Disk") };
#ifdef WIN32
		wxString choices3[3] = { _T("ConvertToMono"), _T("GetLeftChannel"), _T("GetRightChannel") };
#endif
		
		AddComboControl(audioAdvPage,audioAdvSizer1,_("Audio provider"),_T("Audio Provider"),AudioProviderFactoryManager::GetFactoryList(),true,1);
		AddComboControl(audioAdvPage,audioAdvSizer1,_("Audio player"),_T("Audio Player"),AudioPlayerFactoryManager::GetFactoryList(),true,1);
		AddComboControl(audioAdvPage,audioAdvSizer1,_("Cache type"),_T("Audio Cache"),wxArrayString(3,choices2),true);
#ifdef WIN32
		AddComboControl(audioAdvPage,audioAdvSizer1,_("Avisynth down-mixer"),_T("Audio Downmixer"),wxArrayString(3,choices3),false);
#endif
		AddTextControl(audioAdvPage,audioAdvSizer1,_("HD cache path"),_T("Audio HD Cache Location"),TEXT_TYPE_FOLDER);
		AddTextControl(audioAdvPage,audioAdvSizer1,_("HD cache name"),_T("Audio HD CAche Name"));
		AddTextControl(audioAdvPage,audioAdvSizer1,_("Spectrum cutoff"),_T("Audio spectrum cutoff"),TEXT_TYPE_NUMBER);
		wxString spectrum_quality_choices[] = { _("0 - Regular quality"), _("1 - Better quality"), _("2 - High quality"), _("3 - Insane quality") };
		AddComboControl(audioAdvPage,audioAdvSizer1,_("Spectrum quality"),_T("Audio spectrum quality"),wxArrayString(4,spectrum_quality_choices));
		AddTextControl(audioAdvPage,audioAdvSizer1,_("Spectrum cache memory max (MB)"),_T("Audio spectrum memory max"),TEXT_TYPE_NUMBER);
		audioAdvSizer1->AddGrowableCol(0,1);

		// Main sizer
		audioAdvSizer2->Add(new wxStaticText(audioAdvPage,-1,_("WARNING: Changing these settings might result in bugs,\ncrashes, glitches and/or movax.\nDon't touch these unless you know what you're doing.")),0,wxEXPAND | wxALL,5);
		audioAdvSizer2->Add(audioAdvSizer1,1,wxEXPAND | wxALL,5);
		audioAdvSizer3->Add(audioAdvSizer2,0,wxEXPAND);
		audioAdvSizer3->AddStretchSpacer(1);
		audioAdvSizer3->Fit(audioAdvPage);
		audioAdvPage->SetSizer(audioAdvSizer3);
	}

	// Automation page
	{
		// Sizers
		wxSizer *autoMainSizer = new wxBoxSizer(wxVERTICAL);
		wxSizer *autoSizer1 = new wxStaticBoxSizer(wxVERTICAL,autoPage,_("Options"));
		wxFlexGridSizer *autoSizer2 = new wxFlexGridSizer(4,2,5,5);

		// First sizer
		AddTextControl(autoPage,autoSizer2,_("Base path"),_T("Automation Base Path"));
		AddTextControl(autoPage,autoSizer2,_("Include path"),_T("Automation Include Path"));
		AddTextControl(autoPage,autoSizer2,_("Auto-load path"),_T("Automation Autoload Path"));
		wxString trace_choices[] = { _("0: Fatal"), _("1: Error"), _("2: Warning"), _("3: Hint"), _("4: Debug"), _("5: Trace") };
		wxString prio_choices[] = { _("Normal"), _("Below Normal (recommended)"), _("Lowest") };
		wxString reload_choices[] = { _("No scripts"), _("Subtitle-local scripts"), _("Global autoload scripts"), _("All scripts") };
		AddComboControl(autoPage,autoSizer2,_("Trace level"),_T("Automation Trace Level"),wxArrayString(6,trace_choices));
		AddComboControl(autoPage,autoSizer2,_("Thread priority"),_T("Automation Thread Priority"),wxArrayString(3,prio_choices));
		AddComboControl(autoPage,autoSizer2,_("Autoreload on Export"),_T("Automation Autoreload Mode"),wxArrayString(4,reload_choices));
		autoSizer2->AddGrowableCol(1,1);

		// Sizers
		autoSizer1->Add(autoSizer2,1,wxEXPAND | wxALL,5);
		autoMainSizer->Add(autoSizer1,0,wxEXPAND | wxALL,0);
		autoMainSizer->AddStretchSpacer(1);
		autoMainSizer->Fit(autoPage);
		autoPage->SetSizer(autoMainSizer);
	}

	// Hotkeys page
	{
		// Variables
		hotkeysModified = false;
		origKeys = Hotkeys.key;

		// List of shortcuts
		Shortcuts = new wxListView(hotkeysPage,HOTKEY_LIST,wxDefaultPosition,wxSize(250,150),wxLC_REPORT | wxLC_SINGLE_SEL);
		Shortcuts->InsertColumn(0,_("Function"),wxLIST_FORMAT_LEFT,200);
		Shortcuts->InsertColumn(1,_("Key"),wxLIST_FORMAT_LEFT,120);

		// Populate list
		std::map<wxString,HotkeyType>::iterator cur;
		for (cur = Hotkeys.key.end();cur-- != Hotkeys.key.begin();) {
			wxListItem item;
			item.SetText(wxGetTranslation(cur->second.origName));
			item.SetData(&cur->second);
			int pos = Shortcuts->InsertItem(item);
			Shortcuts->SetItem(pos,1,cur->second.GetText());
		}

		// Create buttons
		wxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
		buttons->Add(new wxButton(hotkeysPage,BUTTON_HOTKEY_SET,_("Set Hotkey...")),1,wxEXPAND|wxRIGHT,5);
		buttons->Add(new wxButton(hotkeysPage,BUTTON_HOTKEY_CLEAR,_("Clear Hotkey")),0,wxEXPAND|wxRIGHT,5);
		buttons->Add(new wxButton(hotkeysPage,BUTTON_HOTKEY_DEFAULT,_("Default")),0,wxEXPAND|wxRIGHT,5);
		buttons->Add(new wxButton(hotkeysPage,BUTTON_HOTKEY_DEFAULT_ALL,_("Default All")),0,wxEXPAND|wxRIGHT,0);

		// Main sizer
		wxSizer *hotkeysSizer = new wxBoxSizer(wxVERTICAL);
		hotkeysSizer->Add(Shortcuts,1,wxLEFT|wxRIGHT|wxTOP|wxEXPAND,5);
		hotkeysSizer->Add(buttons,0,wxALL|wxEXPAND,5);
		hotkeysSizer->Fit(hotkeysPage);
		hotkeysPage->SetSizer(hotkeysSizer);
	}

	// List book
	book->AddPage(generalPage,_("General"),true);
	book->AddSubPage(filePage,_("File save/load"),true);
	book->AddSubPage(editPage,_("Subtitles edit box"),true);
	book->AddSubPage(gridPage,_("Subtitles grid"),true);
	book->AddPage(videoPage,_("Video"),true);
	book->AddPage(audioPage,_("Audio"),true);
	book->AddSubPage(displayPage,_("Display"),true);
	book->AddSubPage(audioAdvPage,_("Advanced"),true);
	book->AddPage(autoPage,_("Automation"),true);
	book->AddPage(hotkeysPage,_("Hotkeys"),true);
	#ifdef wxUSE_TREEBOOK
	book->ChangeSelection(Options.AsInt(_T("Options page")));
	#endif

	// Buttons Sizer
	wxStdDialogButtonSizer *stdButtonSizer = new wxStdDialogButtonSizer();
	stdButtonSizer->AddButton(new wxButton(this,wxID_OK));
	stdButtonSizer->AddButton(new wxButton(this,wxID_CANCEL));
	stdButtonSizer->AddButton(new wxButton(this,wxID_APPLY));
	stdButtonSizer->AddButton(new HelpButton(this,_T("Options")));
	stdButtonSizer->Realize();
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *defaultButton = new wxButton(this,BUTTON_DEFAULTS,_("Restore Defaults"));
	buttonSizer->Add(defaultButton,0,wxEXPAND);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(stdButtonSizer,0,wxEXPAND);

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


////////////////////
// Add a wxTextCtrl
void DialogOptions::AddTextControl(wxWindow *parent,wxSizer *sizer,wxString label,wxString option,TextType type) {
	sizer->Add(new wxStaticText(parent,-1,label + wxString(_T(": "))),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
	wxTextCtrl *control;
	if (type == TEXT_TYPE_NUMBER) control = new wxTextCtrl(parent,-1,_T(""),wxDefaultPosition,wxDefaultSize,0,NumValidator());
	else control = new wxTextCtrl(parent,-1);
	Bind(control,option);
	sizer->Add(control,1,wxEXPAND);
}


////////////////////
// Add a wxComboBox
void DialogOptions::AddComboControl(wxWindow *parent,wxSizer *sizer,wxString label,wxString option,wxArrayString choices,bool readOnly,int bindParam) {
	sizer->Add(new wxStaticText(parent,-1,label + wxString(_T(": "))),0,wxALIGN_CENTER_VERTICAL | wxRIGHT,10);
	int flags = wxCB_DROPDOWN;
	if (readOnly) flags |= wxCB_READONLY;
	wxComboBox *control = new wxComboBox(parent,-1,_T(""),wxDefaultPosition,wxDefaultSize,choices,flags);
	Bind(control,option,bindParam);
	sizer->Add(control,1,wxEXPAND);
}


//////////////////
// Add a checkbox
void DialogOptions::AddCheckBox(wxWindow *parent,wxSizer *sizer,wxString label,wxString option) {
	wxControl *control = new wxCheckBox(parent,-1,label);
	Bind(control,option);
	sizer->Add(control,1,wxEXPAND,0);
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogOptions,wxDialog)
	EVT_BUTTON(wxID_OK,DialogOptions::OnOK)
	EVT_BUTTON(wxID_CANCEL,DialogOptions::OnCancel)
	EVT_BUTTON(wxID_APPLY,DialogOptions::OnApply)
	EVT_BUTTON(BUTTON_DEFAULTS,DialogOptions::OnDefaults)
	EVT_BUTTON(BUTTON_HOTKEY_SET,DialogOptions::OnEditHotkey)
	EVT_BUTTON(BUTTON_HOTKEY_CLEAR,DialogOptions::OnClearHotkey)
	EVT_BUTTON(BUTTON_HOTKEY_DEFAULT,DialogOptions::OnDefaultHotkey)
	EVT_BUTTON(BUTTON_HOTKEY_DEFAULT_ALL,DialogOptions::OnDefaultAllHotkey)
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
			if (frame->Close()) {
				RestartAegisub();
				//wxStandardPaths stand;
				//wxExecute(stand.GetExecutablePath());
			}
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
	// Undo hotkeys
	if (hotkeysModified) Hotkeys.key = origKeys;

	// Set options
	Options.SetInt(_T("Options page"),book->GetSelection());
	Options.Save();
	EndModal(0);

	// Restart
	if (needsRestart) {
		int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
		if (answer == wxYES) {
			FrameMain *frame = (FrameMain*) GetParent();
			if (frame->Close()) {
				RestartAegisub();
				//wxStandardPaths stand;
				//wxExecute(stand.GetExecutablePath());
			}
		}
	}
}


////////////////////
// Restore defaults
void DialogOptions::OnDefaults(wxCommandEvent &event) {
	int result = wxMessageBox(_("Are you sure that you want to restore the defaults? All your settings will be overriden."),_("Restore defaults?"),wxYES_NO);
	if (result == wxYES) {
		Options.LoadDefaults(true);
		Options.Save();
		ReadFromOptions();
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
	bool videoReload = false;
	bool audioReload = false;

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
			if (type == MOD_VIDEO_RELOAD) videoReload = true;
			if (type == MOD_AUDIO) audio = true;
			if (type == MOD_AUDIO_RELOAD) audioReload = true;
		}
	}

	// Apply hotkey changes if modified
	if (hotkeysModified) {
		// Save changes
		Hotkeys.modified = true;
		Hotkeys.Save();
		hotkeysModified = false;
		origKeys = Hotkeys.key;

		// Rebuild menu
		FrameMain *parent = (FrameMain*) GetParent();
		parent->InitMenu();

		// Rebuild accelerator table
		parent->SetAccelerators();

		// Update tooltips
		ToolTipManager::Update();
	}

	// Save options
	if (Options.AsBool(_T("Local config"))) Options.SetFile(StandardPaths::DecodePath(_T("?data/config.dat")));
	else {
		Options.SetFile(StandardPaths::DecodePath(_T("?user/config.dat")));
		wxRemoveFile(StandardPaths::DecodePath(_T("?data/config.dat")));
	}
	Options.Save();

	// Need restart?
	if (mustRestart) {
		if (justApply) needsRestart = true;
		else {
			int answer = wxMessageBox(_("Aegisub must restart for the changes to take effect. Restart now?"),_("Restart Aegisub"),wxYES_NO);
			if (answer == wxYES) {
				FrameMain *frame = (FrameMain*) GetParent();
				if (frame->Close()) {
					RestartAegisub();
					//wxStandardPaths stand;
					//wxExecute(stand.GetExecutablePath());
				}
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
		if (videoReload) {
			VideoContext::Get()->Reload();
		}
		else if (video) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->videoBox->videoSlider->Refresh();
		}

		// Audio
		if (audioReload) {
			FrameMain *frame = (FrameMain*) GetParent();
			frame->audioBox->audioDisplay->Reload();
		}
		else if (audio) {
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


/////////////////
// Edit a hotkey
void DialogOptions::OnEditHotkey(wxCommandEvent &event) {
	// Get selection
	int sel = Shortcuts->GetFirstSelected();
	if (sel == wxNOT_FOUND) return;

	// Get key and store old
	HotkeyType *curKey = (HotkeyType *) wxUIntToPtr(Shortcuts->GetItemData(sel));
	int oldKeycode = curKey->keycode;
	int oldFlags = curKey->flags;

	// Open dialog
	DialogInputHotkey input(curKey,Shortcuts->GetItemText(sel),Shortcuts);
	input.ShowModal();

	// Update stuff if it changed
	if (oldKeycode != curKey->keycode || oldFlags != curKey->flags) {
		Shortcuts->SetItem(sel,1,curKey->GetText());
		hotkeysModified = true;
	}
}


//////////////////
// Clear a hotkey
void DialogOptions::OnClearHotkey(wxCommandEvent &event) {
	for (int item=-1;true;) {
		item = Shortcuts->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
		if (item == -1) break;

		HotkeyType *curKey = (HotkeyType *) wxUIntToPtr(Shortcuts->GetItemData(item));
		if (curKey->keycode != 0 || curKey->flags != 0) {
			hotkeysModified = true;
			curKey->keycode = 0;
			curKey->flags = 0;
			Shortcuts->SetItem(item,1,curKey->GetText());
		}
	}
}


///////////////////////////
// Reset hotkey to default
void DialogOptions::OnDefaultHotkey(wxCommandEvent &event) {
	// Load defaults
	HotkeyManager defs;
	defs.LoadDefaults();

	// Replace
	for (int item=-1;true;) {
		item = Shortcuts->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
		if (item == -1) break;

		HotkeyType *curKey = (HotkeyType *) wxUIntToPtr(Shortcuts->GetItemData(item));
		HotkeyType *origKey = &defs.key[curKey->origName.Lower()];
		if (origKey->keycode != curKey->keycode || origKey->flags != curKey->flags) {
			hotkeysModified = true;
			curKey->keycode = origKey->keycode;
			curKey->flags = origKey->flags;
			Shortcuts->SetItem(item,1,curKey->GetText());

			// Unmap duplicate
			HotkeyType *dup = Hotkeys.Find(origKey->keycode,origKey->flags);
			if (dup) {
				dup->keycode = 0;
				dup->flags = 0;
				int item = Shortcuts->FindItem(-1,wxPtrToUInt(dup));
				if (item != -1) Shortcuts->SetItem(item,1,dup->GetText());
			}
		}
	}
}


////////////////////////////////
// Reset all hotkeys to default
void DialogOptions::OnDefaultAllHotkey(wxCommandEvent &event) {
	Hotkeys.LoadDefaults();
	Shortcuts->Freeze();
	Shortcuts->ClearAll();
	Shortcuts->InsertColumn(0,_("Function"),wxLIST_FORMAT_LEFT,200);
	Shortcuts->InsertColumn(1,_("Key"),wxLIST_FORMAT_LEFT,120);

	// Populate list
	std::map<wxString,HotkeyType>::iterator cur;
	for (cur = Hotkeys.key.end();cur-- != Hotkeys.key.begin();) {
		wxListItem item;
		item.SetText(wxGetTranslation(cur->second.origName));
		item.SetData(&cur->second);
		int pos = Shortcuts->InsertItem(item);
		Shortcuts->SetItem(pos,1,cur->second.GetText());
	}
	hotkeysModified = true;
	Shortcuts->Thaw();
}


/////////////////////
// Input constructor
DialogInputHotkey::DialogInputHotkey(HotkeyType *_key,wxString name,wxListView *shorts)
: wxDialog(NULL, -1, _("Press Key"), wxDefaultPosition, wxSize(200,50), wxCAPTION | wxWANTS_CHARS , _T("Press key"))
{
	// Key
	key = _key;
	shortcuts = shorts;

	// Text
	wxStaticText *text = new wxStaticText(this,-1,wxString::Format(_("Press key to bind to \"%s\" or Esc to cancel."), name.c_str()));

	// Key capturer
	capture = new CaptureKey(this);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(text,1,wxALL,5);
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);
}


////////////////////////
// Capturer constructor
CaptureKey::CaptureKey(DialogInputHotkey *_parent)
: wxTextCtrl(_parent,-1,_T(""),wxDefaultPosition,wxSize(0,0),wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB)
{
	parent = _parent;
	SetFocus();
}


/////////////////////
// Input event table
BEGIN_EVENT_TABLE(CaptureKey,wxTextCtrl)
	EVT_KEY_DOWN(CaptureKey::OnKeyDown)
	EVT_KILL_FOCUS(CaptureKey::OnLoseFocus)
END_EVENT_TABLE()


///////////////
// On key down
void CaptureKey::OnKeyDown(wxKeyEvent &event) {
	// Get key
	int keycode = event.GetKeyCode();
	if (keycode == WXK_ESCAPE) parent->EndModal(0);
	else if (keycode != WXK_SHIFT && keycode != WXK_CONTROL && keycode != WXK_ALT) {
		// Get modifier
		int mod = 0;
		if (event.m_altDown) mod |= wxACCEL_ALT;
#ifdef __APPLE__
		if (event.m_metaDown) mod |= wxACCEL_CTRL;
#else
		if (event.m_controlDown) mod |= wxACCEL_CTRL;
#endif
		if (event.m_shiftDown) mod |= wxACCEL_SHIFT;

		// Check if keycode is free
		HotkeyType *dup = Hotkeys.Find(keycode,mod);
		if (dup) {
			int result = wxMessageBox(wxString::Format(_("The hotkey %s is already mapped to %s. If you proceed, that hotkey will be cleared. Proceed?"),dup->GetText().c_str(),dup->origName.c_str()),_("Hotkey conflict"),wxYES_NO | wxICON_EXCLAMATION);
			if (result == wxNO) {
				parent->EndModal(0);
				return;
			}
			dup->keycode = 0;
			dup->flags = 0;
			int item = parent->shortcuts->FindItem(-1,wxPtrToUInt(dup));
			if (item != -1) parent->shortcuts->SetItem(item,1,dup->GetText());
		}

		// Set keycode
		parent->key->keycode = keycode;
		parent->key->flags = mod;

		// End dialogue
		parent->EndModal(0);
	}
	else event.Skip();
}


//////////////
// Keep focus
void CaptureKey::OnLoseFocus(wxFocusEvent &event) {
	SetFocus();
}
