// Copyright (c) 2005, Rodrigo Braz Monteiro, Niels Martin Hansen
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


///////////////////
// Include headers
#include <wx/wxprec.h>
#include <wx/sysopt.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include "subs_grid.h"
#include "frame_main.h"
#include "avisynth_wrap.h"
#include "video_display.h"
#include "video_provider.h"
#include "video_slider.h"
#include "video_zoom.h"
#include "ass_file.h"
#include "dialog_search_replace.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "main.h"
#include "version.h"
#include "splash.h"
#include "tip.h"
#include "automation_filter.h"
#include "audio_box.h"
#include "dialog_spellcheck.h"
#include "video_box.h"
#include "drop.h"
#include "hotkeys.h"
#include "utils.h"
#include "text_file_reader.h"


/////////////////////////
// FrameMain constructor

FrameMain::FrameMain (wxArrayString args)
					: wxFrame ((wxFrame*)NULL,-1,_T(""),wxDefaultPosition,wxSize(800,600),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
{
	// Splash screen
	#ifndef _DEBUG
	if (Options.AsBool(_T("Show Splash"))) {
		SplashScreen *splash = new SplashScreen(NULL);
		splash->Show(true);
	}
	else {
		// Show tip of the day
		TipOfTheDay::Show(this);
	}
	#endif

	// Initialize flags
	HasSelection = false;
	menuCreated = false;
	blockAudioLoad = false;
	blockAudioLoad = false;

	// Create menu and tool bars
	InitToolbar();
	InitMenu();
	
	// Create status bar
    CreateStatusBar(2);

	// Set icon
	SetIcon(wxICON(wxicon));

	// Contents
	curMode = -1;
	InitContents();

	// Parse arguments
	LoadSubtitles(_T(""));
	LoadList(args);

	// Set autosave timer
	AutoSave.SetOwner(this,AutoSave_Timer);
	int time = Options.AsInt(_T("Auto save every seconds"));
	if (time > 0) {
		AutoSave.Start(time*1000);
	}

	// Set accelerator keys
	PreviousFocus = NULL;
	SetAccelerators();

	// Set drop target
	SetDropTarget(new AegisubFileDropTarget(this));
}


////////////////////////
// FrameMain destructor
FrameMain::~FrameMain () {
	DeInitContents();
}


//////////////////////
// Initialize toolbar
void FrameMain::InitToolbar () {
	// Create toolbar
	wxSystemOptions::SetOption(_T("msw.remap"), 0);
	Toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL,-1,_T("Toolbar"));

	// Subtitle control buttons
	Toolbar->AddTool(Menu_File_New_Subtitles,_("New"),wxBITMAP(new_toolbutton),_("New subtitles"));
	Toolbar->AddTool(Menu_File_Open_Subtitles,_("Open"),wxBITMAP(open_toolbutton),_("Open subtitles"));
	Toolbar->AddTool(Menu_File_Save_Subtitles,_("Save"),wxBITMAP(save_toolbutton),_("Save subtitles"));
	Toolbar->AddSeparator();

	// Video zoom controls
	Toolbar->AddTool(Menu_Video_JumpTo,_("Jump To..."),wxBITMAP(jumpto_button),wxNullBitmap,wxITEM_NORMAL,_("Jump video to time/frame"));
	Toolbar->AddTool(Menu_Video_Zoom_In,_("Zoom in"),wxBITMAP(zoom_in_button),wxNullBitmap,wxITEM_NORMAL,_("Zoom video in"));
	Toolbar->AddTool(Menu_Video_Zoom_Out,_("Zoom out"),wxBITMAP(zoom_out_button),wxNullBitmap,wxITEM_NORMAL,_("Zoom video out"));
	wxArrayString choices;
	for (int i=1;i<=16;i++) {
		wxString toAdd = wxString::Format(_T("%i"),int(i*12.5));
		if (i%2) toAdd += _T(".5");
		toAdd += _T("%");
		choices.Add(toAdd);
	}
	ZoomBox = new wxComboBox(Toolbar,Toolbar_Zoom_Dropdown,_T("75%"),wxDefaultPosition,wxSize(100,20),choices,wxCB_READONLY);
	Toolbar->AddControl(ZoomBox);
	Toolbar->AddSeparator();

	// More video buttons
	Toolbar->AddTool(Menu_Subs_Snap_Video_To_Start,_("Jump video to start"),wxBITMAP(video_to_substart),_("Jumps the video to the start frame of current subtitle"));
	Toolbar->AddTool(Menu_Subs_Snap_Video_To_End,_("Jump video to end"),wxBITMAP(video_to_subend),_("Jumps the video to the end frame of current subtitle"));
	Toolbar->AddTool(Menu_Subs_Snap_Start_To_Video,_("Snap start to video"),wxBITMAP(substart_to_video),_("Set start of selected subtitles to current video frame"));
	Toolbar->AddTool(Menu_Subs_Snap_End_To_Video,_("Snap end to video"),wxBITMAP(subend_to_video),_("Set end of selected subtitles to current video frame"));
	Toolbar->AddTool(Menu_Video_Select_Visible,_("Select visible"),wxBITMAP(select_visible_button),_("Selects all lines that are currently visible on video frame"));
	Toolbar->AddTool(Menu_Video_Snap_To_Scene,_("Snap subtitles to scene"),wxBITMAP(snap_subs_to_scene),_("Snap selected subtitles so they match current scene start/end"));
	Toolbar->AddTool(Menu_Video_Shift_To_Frame,_("Shift subtitles to frame"),wxBITMAP(shift_to_frame),_("Shift selected subtitles so first selected starts at this frame"));
	Toolbar->AddSeparator();

	// Property stuff
	Toolbar->AddTool(Menu_Tools_Properties,_("Properties"),wxBITMAP(properties_toolbutton),_("Open Properties"));
	Toolbar->AddTool(Menu_Tools_Styles_Manager,_("Styles Manager"),wxBITMAP(style_toolbutton),_("Open Styles Manager"));
	Toolbar->AddTool(Menu_Tools_Attachments,_("Attachments"),wxBITMAP(attach_button),_("Open Attachment List"));
	Toolbar->AddSeparator();

	// Automation
	Toolbar->AddTool(Menu_Tools_Automation,_("Automation"),wxBITMAP(automation_toolbutton),_("Open Automation manager"));
	Toolbar->AddSeparator();

	// Tools
	Toolbar->AddTool(Menu_Tools_Styling,_("Styling Assistant"),wxBITMAP(styling_toolbutton),_("Open Styling Assistant"));
	Toolbar->AddTool(Menu_Tools_Translation,_("Translation Assistant"),wxBITMAP(translation_toolbutton),_("Open Translation Assistant"));
	Toolbar->AddTool(Menu_Tools_Fonts_Collector,_("Fonts Collector"),wxBITMAP(font_collector_button),_("Open Fonts Collector"));
	Toolbar->AddTool(Menu_Tools_Resample,_("Resample"),wxBITMAP(resample_toolbutton),_("Resample script resolution"));
	Toolbar->AddTool(Menu_Tools_Timing_Processor,_("Timing Post-Processor"),wxBITMAP(timing_processor_toolbutton),_("Open Timing Post-processor dialog"));
	#ifndef NO_SPELLCHECKER
	Toolbar->AddTool(Menu_Tools_SpellCheck,_("Spellchecker"),wxBITMAP(spellcheck_toolbutton),_("Open Spell checker"));
	#endif
	Toolbar->AddSeparator();

	// Misc
	Toolbar->AddTool(Grid_Toggle_Tags,_("Cycle Tag Hidding Mode"),wxBITMAP(toggle_tag_hiding),_("Cycle through tag-hiding modes"));

	// Update
	Toolbar->Realize();
}


///////////////////////
// Initialize menu bar
void FrameMain::InitMenu() {
	// Deinit menu if needed
	if (menuCreated) {
		SetMenuBar(NULL);
	}

	// Generate menubar
	MenuBar = new wxMenuBar();

	// Create recent subs submenus
	RecentSubs = new wxMenu();
	RecentVids = new wxMenu();
	RecentAuds = new wxMenu();
	RecentTimecodes = new wxMenu();

	// Create file menu
	fileMenu = new wxMenu();
	AppendBitmapMenuItem(fileMenu,Menu_File_New_Subtitles, _("&New Subtitles\t") + Hotkeys.GetText(_T("New Subtitles")), _("New subtitles"),wxBITMAP(new_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Open_Subtitles, _("&Open Subtitles...\t") + Hotkeys.GetText(_T("Open Subtitles")), _("Opens a subtitles file"),wxBITMAP(open_toolbutton));
	fileMenu->Append(Menu_File_Open_Subtitles_Charset, _("&Open Subtitles with Charset..."), _("Opens a subtitles file with a specific charset"));
	AppendBitmapMenuItem(fileMenu,Menu_File_Save_Subtitles, _("&Save Subtitles\t") + Hotkeys.GetText(_T("Save Subtitles")), _("Saves subtitles"),wxBITMAP(save_toolbutton));
	fileMenu->Append(Menu_File_Save_Subtitles_As, _("Save Subtitles as..."), _("Saves subtitles with another name"));
	fileMenu->Append(Menu_File_Export_Subtitles, _("Export Subtitles..."), _("Saves a copy of subtitles with processing applied to it."));
	fileMenu->AppendSeparator();
	wxMenuItem *RecentParent = new wxMenuItem(fileMenu, Menu_File_Recent_Subs_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentSubs);
	fileMenu->Append(RecentParent);
	fileMenu->AppendSeparator();
	fileMenu->Append(Menu_File_Exit, _("E&xit\t") + Hotkeys.GetText(_T("Exit")), _("Exit the application"));
	MenuBar->Append(fileMenu, _("&File"));

	// Create Edit menu
	editMenu = new wxMenu();
	AppendBitmapMenuItem (editMenu,Menu_Edit_Undo, _("&Undo\t") + Hotkeys.GetText(_T("Undo")), _("Undoes last action"),wxBITMAP(undo_button));
	AppendBitmapMenuItem (editMenu,Menu_Edit_Redo, _("&Redo\t") + Hotkeys.GetText(_T("Redo")), _("Redoes last action"),wxBITMAP(redo_button));
	editMenu->AppendSeparator();
	editMenu->Append(Menu_Edit_Select, _("&Select lines...\t") + Hotkeys.GetText(_T("Select lines")), _("Selects lines based on defined criterea"));
	editMenu->Append(Menu_Edit_Shift, _("S&hift times...\t") + Hotkeys.GetText(_T("Shift times")), _("Shift subtitles by time or frames"));
	editMenu->Append(Menu_Edit_Sort, _("Sort by Time"), _("Sort all subtitles by their start times"));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem (editMenu,Menu_Edit_Find, _("&Find...\t") + Hotkeys.GetText(_T("Find")), _("Find words in subtitles"),wxBITMAP(find_button));
	editMenu->Append(Menu_Edit_Find_Next, _("Find next\t") + Hotkeys.GetText(_T("Find Next")), _("Find next match of last word"));
	editMenu->Append(Menu_Edit_Replace, _("&Replace...\t") + Hotkeys.GetText(_T("Replace")) , _("Find and replace words in subtitles"));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem (editMenu,Menu_Edit_Cut, _("Cut...\t") + Hotkeys.GetText(_T("Cut")), _("Cut subtitles"), wxBITMAP(cut_button));
	AppendBitmapMenuItem (editMenu,Menu_Edit_Copy, _("Copy...\t") + Hotkeys.GetText(_T("Copy")), _("Copy subtitles"), wxBITMAP(copy_button));
	AppendBitmapMenuItem (editMenu,Menu_Edit_Paste, _("Paste...\t") + Hotkeys.GetText(_T("Paste")), _("Paste subtitles"), wxBITMAP(paste_button));
	MenuBar->Append(editMenu, _("&Edit"));

	// Create view menu
	viewMenu = new wxMenu();
	viewMenu->Append(Menu_View_Language, _T("&Language..."), _("Select Aegisub interface language"));
	viewMenu->AppendSeparator();
	viewMenu->AppendRadioItem(Menu_View_Subs, _("Subs only view"), _("Display subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Video, _("Video+Subs view"), _("Display video and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Audio, _("Audio+Subs view"), _("Display audio and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Standard, _("Full view"), _("Display audio, video and subtitles"));
	MenuBar->Append(viewMenu, _("Vie&w"));

	// Create video menu
	videoMenu = new wxMenu();
	videoMenu->Append(Menu_File_Open_Video, _("&Open Video..."), _("Opens a video file"));
	videoMenu->Append(Menu_File_Close_Video, _("&Close Video"), _("Closes the currently open video file"));
	wxMenuItem *RecentVidParent = new wxMenuItem(videoMenu, Menu_File_Recent_Vids_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentVids);
	videoMenu->Append(RecentVidParent);
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_File_Open_VFR, _("Open timecodes file..."), _("Opens a VFR timecodes v1 or v2 file"));
	videoMenu->Append(Menu_File_Close_VFR, _("Close timecodes file"), _("Closes the currently open timecodes file"))->Enable(false);
	wxMenuItem *RecentTimesParent = new wxMenuItem(videoMenu, Menu_File_Recent_Timecodes_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentTimecodes);
	videoMenu->Append(RecentTimesParent);
	videoMenu->AppendSeparator();
	AppendBitmapMenuItem (videoMenu,Menu_Video_JumpTo, _("&Jump To...\t") + Hotkeys.GetText(_T("Video Jump")), _("Jump to frame or time"), wxBITMAP(jumpto_button));
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_View_Zoom_50, _("Zoom &50%\t") + Hotkeys.GetText(_T("Zoom 50%")), _("Set zoom to 50%"));
	videoMenu->Append(Menu_View_Zoom_100, _("Zoom &100%\t") + Hotkeys.GetText(_T("Zoom 100%")), _("Set zoom to 100%"));
	videoMenu->Append(Menu_View_Zoom_200, _("Zoom &200%\t") + Hotkeys.GetText(_T("Zoom 200%")), _("Set zoom to 200%"));
	videoMenu->AppendSeparator();
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_Start, _("Jump video to start\t") + Hotkeys.GetText(_T("Jump Video To Start")), _("Jumps the video to the start frame of current subtitle"), wxBITMAP(video_to_substart));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_End, _("Jump video to end\t") + Hotkeys.GetText(_T("Jump Video To End")), _("Jumps the video to the end frame of current subtitle"), wxBITMAP(video_to_subend));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Start_To_Video, _("Snap start to video\t") + Hotkeys.GetText(_T("Set Start To Video")), _("Set start of selected subtitles to current video frame"), wxBITMAP(substart_to_video));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_End_To_Video, _("Snap end to video\t") + Hotkeys.GetText(_T("Set End to Video")), _("Set end of selected subtitles to current video frame"), wxBITMAP(subend_to_video));
	AppendBitmapMenuItem(videoMenu,Menu_Video_Snap_To_Scene, _("Snap to scene\t") + Hotkeys.GetText(_T("Snap to Scene")), _("Set start and end of subtitles to the keyframes around current video frame"), wxBITMAP(snap_subs_to_scene));
	AppendBitmapMenuItem(videoMenu,Menu_Video_Shift_To_Frame, _("Shift to Current Frame\t") + Hotkeys.GetText(_T("Shift by Current Time")), _("Shift selection so first selected line starts at current frame"), wxBITMAP(shift_to_frame));
	videoMenu->AppendSeparator();
	videoMenu->AppendCheckItem(Menu_Video_AR_Default, _("&Default Aspect Ratio"), _("Leave video on original aspect ratio"));
	videoMenu->AppendCheckItem(Menu_Video_AR_Full, _("&Fullscreen Aspect Ratio (4:3)"), _("Forces video to fullscreen aspect ratio"));
	videoMenu->AppendCheckItem(Menu_Video_AR_Wide, _("&Widescreen Aspect Ratio (16:9)"), _("Forces video to widescreen aspect ratio"));
	videoMenu->AppendCheckItem(Menu_Video_AR_235, _("2.&35 Aspect Ratio"), _("Forces video to 2.35 aspect ratio"));
	videoMenu->AppendCheckItem(Menu_Video_AR_Custom, _("Custom Aspect Ratio..."), _("Forces video to a custom aspect ratio"));
	MenuBar->Append(videoMenu, _("&Video"));

	// Create audio menu
	audioMenu = new wxMenu();
	audioMenu->Append(Menu_Audio_Open_File, _("&Open Audio file..."), _("Opens an audio file"));
	audioMenu->Append(Menu_Audio_Open_From_Video, _("Open Audio from &Video"), _("Opens the audio from the current video file"));
	audioMenu->Append(Menu_Audio_Close, _("&Close Audio"), _("Closes the currently open audio file"));
	wxMenuItem *RecentAudParent = new wxMenuItem(audioMenu, Menu_File_Recent_Auds_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentAuds);
	audioMenu->Append(RecentAudParent);
	MenuBar->Append(audioMenu, _("&Audio"));

	// Create Tools menu
	toolMenu = new wxMenu();
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Properties, _("&Properties..."), _("Open script properties window"),wxBITMAP(properties_toolbutton));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Styles_Manager, _("&Styles Manager..."), _("Open styles manager"), wxBITMAP(style_toolbutton));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Attachments, _("&Attachments..."), _("Open the attachment list"), wxBITMAP(attach_button));
	toolMenu->AppendSeparator();
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Automation, _("&Automation..."),_("Open automation manager"), wxBITMAP(automation_toolbutton));
	toolMenu->AppendSeparator();
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Styling, _("St&yling Assistant..."), _("Open styling assistant"), wxBITMAP(styling_toolbutton));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Translation, _("&Translation Assistant..."),_("Open translation assistant"), wxBITMAP(translation_toolbutton));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Fonts_Collector, _("&Fonts Collector..."),_("Open fonts collector"), wxBITMAP(font_collector_button));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Resample,_("Resample resolution..."), _("Changes resolution and modifies subtitles to conform to change"), wxBITMAP(resample_toolbutton));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Timing_Processor,_("Timing Post-Processor..."), _("Runs a post-processor for timing to deal with lead-ins, lead-outs, scene timing and etc."), wxBITMAP(timing_processor_toolbutton));
	#ifndef NO_SPELLCHECKER
	AppendBitmapMenuItem (toolMenu,Menu_Tools_SpellCheck, _("Spe&ll checker..."),_("Open spell checker"), wxBITMAP(spellcheck_toolbutton));
	#endif
	toolMenu->AppendSeparator();
	//AppendBitmapMenuItem (toolMenu,Menu_Tools_Options, _("&Options..."), _("Configure Aegisub"), wxBITMAP(hotkeys_button));
	AppendBitmapMenuItem (toolMenu,Menu_Tools_Hotkeys, _("&Hotkeys..."), _("Remap hotkeys"), wxBITMAP(hotkeys_button));
	MenuBar->Append(toolMenu, _("&Tools"));

	// Create help menu
	helpMenu = new wxMenu();
	AppendBitmapMenuItem (helpMenu,Menu_Help_Contents, _("&Contents...\t") + Hotkeys.GetText(_T("Help")), _("Help topics"), wxBITMAP(contents_button));
	helpMenu->AppendSeparator();
	helpMenu->Append(Menu_Help_Website, _("&Website..."), _("Visit Aegisub's official website"));
	helpMenu->Append(Menu_Help_Forums, _("&Forums..."), _("Visit Aegisub's forums"));
	helpMenu->Append(Menu_Help_BugTracker, _("&Bug tracker..."), _("Visit Aegisub's bug tracker"));
	AppendBitmapMenuItem (helpMenu,Menu_Help_IRCChannel, _("&IRC channel..."), _("Visit Aegisub's official IRC channel"), wxBITMAP(irc_button));
	helpMenu->AppendSeparator();
	helpMenu->Append(Menu_Help_About, _("&About..."), _("About Aegisub"));
	MenuBar->Append(helpMenu, _("&Help"));

	// Set the bar as this frame's
	SetMenuBar(MenuBar);

	// Set menu created flag
	menuCreated = true;
}


///////////////////////
// Initialize contents
void FrameMain::InitContents() {
	// Set a background panel
	Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	// Initialize sizers
	MainSizer = new wxBoxSizer(wxVERTICAL);
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer = new wxBoxSizer(wxHORIZONTAL);

	// Video area;
	videoBox = new VideoBox(Panel);
	TopSizer->Add(videoBox->VideoSizer,0,wxEXPAND,0);
	videoBox->videoDisplay->zoomBox = ZoomBox;

	// Subtitles area
	SubsBox = new SubtitlesGrid(this,Panel,-1,videoBox->videoDisplay,wxDefaultPosition,wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,_T("Subs grid"));
	BottomSizer->Add(SubsBox,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,0);
	AssFile::StackReset();
	videoBox->videoSlider->grid = SubsBox;
	videoBox->videoDisplay->grid = SubsBox;
	videoBox->videoDisplay->SetZoomPos(Options.AsInt(_T("Video Default Zoom")));
	Search.grid = SubsBox;

	// Audio area
	audioBox = new AudioBox(Panel,videoBox->videoDisplay);
	audioBox->frameMain = this;
	videoBox->videoDisplay->audio = audioBox->audioDisplay;

	// Top sizer
	EditBox = new SubsEditBox(Panel,SubsBox);
	EditBox->audio = audioBox->audioDisplay;
	EditBox->video = videoBox->videoDisplay;
	ToolSizer = new wxBoxSizer(wxVERTICAL);
	ToolSizer->Add(audioBox,0,wxEXPAND | wxBOTTOM,5);
	ToolSizer->Add(EditBox,1,wxEXPAND,5);
	TopSizer->Add(ToolSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);

	// Set sizers/hints
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,0);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxALL,0);
	Panel->SetSizer(MainSizer);
	//MainSizer->SetSizeHints(Panel);
	//SetSizer(MainSizer);

	// Set display
	SetDisplayMode(0);
	Layout();
}

void FrameMain::DeInitContents() {
	//ghetto hack to free all AssFile junk properly, eliminates lots of memory leaks
	AssFile::StackReset();
	delete AssFile::top;

	delete EditBox;
	delete videoBox;
}

//////////////////
// Update toolbar
void FrameMain::UpdateToolbar() {
	// Collect flags
	bool isVideo = (curMode == 1) || (curMode == 2);
	HasSelection = true;
	int selRows = SubsBox->GetNumberSelection();

	// Update
	wxToolBar* toolbar = GetToolBar();
	toolbar->FindById(Menu_Video_JumpTo)->Enable(isVideo);
	toolbar->FindById(Menu_Video_Zoom_In)->Enable(isVideo);
	toolbar->FindById(Menu_Video_Zoom_Out)->Enable(isVideo);
	ZoomBox->Enable(isVideo);
	toolbar->FindById(Menu_Subs_Snap_Start_To_Video)->Enable(isVideo && selRows > 0);
	toolbar->FindById(Menu_Subs_Snap_End_To_Video)->Enable(isVideo && selRows > 0);
	toolbar->FindById(Menu_Subs_Snap_Video_To_Start)->Enable(isVideo && selRows == 1);
	toolbar->FindById(Menu_Subs_Snap_Video_To_End)->Enable(isVideo && selRows == 1);
	toolbar->FindById(Menu_Video_Select_Visible)->Enable(isVideo);
	toolbar->FindById(Menu_Video_Snap_To_Scene)->Enable(isVideo && selRows > 0);
	toolbar->FindById(Menu_Video_Shift_To_Frame)->Enable(isVideo && selRows > 0);
	toolbar->Realize();
}


//////////////////////////////////
// Append a menu item with bitmap
void FrameMain::AppendBitmapMenuItem (wxMenu* parentMenu,int id,wxString text,wxString help,wxBitmap bmp) {
	wxMenuItem *cur = new wxMenuItem(parentMenu,id,text,help);
	cur->SetBitmap(bmp);
	parentMenu->Append(cur);
}


////////////////////////////
// Menu item enable/disable
void FrameMain::MenuItemEnable (int id, bool state,wxBitmap &bmp1,wxBitmap &bmp2) {
	wxMenuItem *item = MenuBar->FindItem(id);
	wxBitmap bmp = item->GetBitmap();

	// No image
	if (bmp.GetWidth() == 0) {
		item->Enable(state);
	}

	// Has image
	else {
		RebuildMenuItem(item->GetMenu(),id,bmp1,bmp2,state);
	}
}


/////////////////////////////////
// Helper to rebuild menu items
wxMenuItem *FrameMain::RebuildMenuItem(wxMenu *menu,int findId,wxBitmap bmp1,wxBitmap bmp2,bool state) {
	// Find pos
	wxMenuItemList &items = menu->GetMenuItems();
	int pos = -1;
	for (size_t i=0;i<items.GetCount();i++) {
		if (items[i]->GetId() == findId) {
			pos = (int)i;
			break;
		}
	}
	if (pos == -1) return NULL;

	// Get ID and pointer
	wxMenuItem *cur = items[pos];
	int id = cur->GetId();

	// Rebuild
	wxMenuItem *newItem = new wxMenuItem(menu,id,cur->GetText(),cur->GetHelp(),cur->GetKind(),cur->GetSubMenu());
	if (state) newItem->SetBitmap(bmp1);
	else newItem->SetBitmap(bmp2);

	// Swap them
	menu->Destroy(id);
	menu->Insert(pos,newItem);
	menu->Enable(id,state);
	return cur;
}


//////////////////
// Open subtitles
void FrameMain::LoadSubtitles (wxString filename,wxString charset) {
	// First check if there is some loaded
	if (AssFile::top && AssFile::top->loaded) {
		if (TryToCloseSubs() == wxCANCEL) return;
	}

	// Setup
	bool isFile = (filename != _T(""));

	// Load
	try {
		// File exists?
		if (isFile) {
			wxFileName fileCheck(filename);
			if (!fileCheck.FileExists()) throw _T("File does not exist.");

			// Make sure that file isn't actually a timecode file
			TextFileReader testSubs(filename);
			if (testSubs.HasMoreLines()) {
				wxString cur = testSubs.ReadLineFromFile();
				if (cur.Left(10) == _T("# timecode")) {
					LoadVFR(filename);
					return;
				}
			}
		}

		// Proceed into loading
		SubsBox->Clear();
		AssFile::StackReset();
		if (isFile) {
			AssFile::top->Load(filename,charset);
			SubsBox->LoadFromAss(AssFile::top,false,true);
		}
		else {
			SubsBox->LoadDefault(AssFile::top);
		}
	}
	catch (const wchar_t *err) {
		wxMessageBox(wxString(err), _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (AutomationError &err) {
		wxMessageBox(wxString(_T("Automation exception: ")) + err.message, _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}

	// Save copy
	wxFileName origfile(filename);
	if (Options.AsBool(_T("Auto backup")) && origfile.FileExists()) {
		// Get path
		wxString path = Options.AsText(_T("Auto backup path"));
		if (path.IsEmpty()) path = origfile.GetPath();
		wxFileName dstpath(path);
		if (!dstpath.IsAbsolute()) path = AegisubApp::folderName + path;
		path += _T("/");
		dstpath.Assign(path);
		if (!dstpath.DirExists()) wxMkdir(path);

		// Save
		wxString backup = path + origfile.GetName() + _T(".ORIGINAL.") + origfile.GetExt();
		Backup(filename,backup);
	}

	// Sync
	SynchronizeProject(true);

	// Update title bar
	UpdateTitle();
}


//////////////////
// Save subtitles
bool FrameMain::SaveSubtitles(bool saveas,bool withCharset) {
	// Try to get filename from file
	wxString filename;
	if (saveas == false && AssFile::top->CanSave()) filename = AssFile::top->filename;

	// Failed, ask user
	if (filename.IsEmpty()) {
		videoBox->videoDisplay->Stop();
		wxString path = Options.AsText(_T("Last open subtitles path"));
		wxFileName origPath(AssFile::top->filename);
		filename = 	wxFileSelector(_("Save subtitles file"),path,origPath.GetName() + _T(".ass"),_T("ass"),_T("Advanced Substation Alpha (*.ass)|*.ass"),wxSAVE | wxOVERWRITE_PROMPT,this);
	}

	// Actually save
	if (!filename.empty()) {
		// Store path
		wxFileName filepath(filename);
		Options.SetText(_T("Last open subtitles path"), filepath.GetPath());

		// Fix me, ghetto hack for correct relative path generation in SynchronizeProject()
		AssFile::top->filename = filename;

		// Synchronize
		SynchronizeProject();

		// Get charset
		wxString charset = _T("");
		if (withCharset) {
			wxArrayString choices = GetEncodings();
			charset = wxGetSingleChoice(_("Choose charset code:"), _T("Charset"),choices,this,-1, -1,true,250,200);
			if (charset.IsEmpty()) return false;
		}

		// Save
		try {
			AssFile::top->Save(filename,true,true,charset);
			UpdateTitle();
		}
		catch (const wchar_t *err) {
			wxMessageBox(wxString(err), _T("Error"), wxOK | wxICON_ERROR, NULL);
			return false;
		}
		catch (...) {
			wxMessageBox(_T("Unknown error"), _T("Error"), wxOK | wxICON_ERROR, NULL);
			return false;
		}
		return true;
	}
	return false;
}


//////////////////////////
// Try to close subtitles
int FrameMain::TryToCloseSubs(bool enableCancel) {
	AssFile *ass = AssFile::top;
	if (ass->IsModified()) {
		int flags = wxYES_NO;
		if (enableCancel) flags |= wxCANCEL;
		int result = wxMessageBox(_("Save before continuing?"), _("Unsaved changes"), flags,this);
		if (result == wxYES) {
			// If it fails saving, return cancel anyway
			if (SaveSubtitles(false)) return wxYES;
			else return wxCANCEL;
		}
		return result;
	}
	else return wxYES;
}


////////////////////
// Set display mode
// ----------------
// 0: subs only
// 1: video
// 2: audio
void FrameMain::SetDisplayMode(int mode) {
	Freeze();
	videoBox->videoDisplay->Stop();
	if (mode != curMode) {
		// Automatic mode
		bool showVid=false, showAudio=false;
		if (mode == -1) {
			// See what's loaded
			if (videoBox->videoDisplay->loaded) showVid = true;
			if (audioBox->loaded) showAudio = true;

			// Set mode
			if (!showVid && !showAudio) mode = 0;
			if (showVid && !showAudio) mode = 1;
			if (showVid && showAudio) mode = 2;
			if (!showVid && showAudio) mode = 3;
		}

		// Subs only
		else if (mode == 0) {
			showVid = false;
			showAudio = false;
		}

		// Video only
		else if (mode == 1) {
			showVid = true;
			showAudio = false;
		}

		// Video+Audio
		else if (mode == 2) {
			showVid = true;
			showAudio = true;
		}

		// Audio only
		else if (mode == 3) {
			showVid = false;
			showAudio = true;
		}

		// Set display
		TopSizer->Show(videoBox->VideoSizer,showVid,true);
		ToolSizer->Show(audioBox,showAudio,true);
	}

	// Update
	curMode = mode;
	UpdateToolbar();
	MainSizer->CalcMin();
	MainSizer->RecalcSizes();
	videoBox->VideoSizer->Layout();
	MainSizer->Layout();
	Layout();
	Show(true);
	//int cw,ch;
	//GetSize(&cw,&ch);
	//SetSize(cw-1,ch-1);
	Thaw();
}


////////////////////
// Update title bar
void FrameMain::UpdateTitle() {
	// Determine if current subs are modified
	bool subsMod = AssFile::top->IsModified();
	
	// Create ideal title
	wxString newTitle = _T("");
	if (subsMod) newTitle << _T("* ");
	if (AssFile::top->filename != _T("")) {
		wxFileName file (AssFile::top->filename);
		newTitle << file.GetFullName();
	}
	else newTitle << _T("Untitled");
	newTitle << _T(" - Aegisub ") << GetAegisubLongVersionString();

	// Get current title
	wxString curTitle = GetTitle();
	if (curTitle != newTitle) SetTitle(newTitle);
}


/////////////////////////////////////////
// Updates subs with video/whatever data
void FrameMain::SynchronizeProject(bool fromSubs) {
	// Gather current data
	AssFile *subs = AssFile::top;

	// Retrieve data from subs
	if (fromSubs) {
		// Reset the state
		long videoPos = 0;
		long videoAr = 0;
		double videoArValue = 0.0;
		long videoZoom = 0;
		{
			std::list<AssAutomationFilter*>::const_iterator next = AssAutomationFilter::GetFilterList().begin(), f;
			while (next != AssAutomationFilter::GetFilterList().end()) {
				f = next++;
				AutomationScript *s = (*f)->GetScript();
				delete (*f);
				delete s;
			}
		}

		// Get AR
		wxString arString = subs->GetScriptInfo(_T("Video Aspect Ratio"));
		if (arString.Left(1) == _T("c")) {
			videoAr = 4;
			arString = arString.Mid(1);
			arString.ToDouble(&videoArValue);
		}
		else if (arString.IsNumber()) arString.ToLong(&videoAr);

		// Get new state info
		subs->GetScriptInfo(_T("Video Position")).ToLong(&videoPos);
		subs->GetScriptInfo(_T("Video Zoom")).ToLong(&videoZoom);
		wxString curSubsVideo = DecodeRelativePath(subs->GetScriptInfo(_T("Video File")),AssFile::top->filename);
		wxString curSubsVFR = DecodeRelativePath(subs->GetScriptInfo(_T("VFR File")),AssFile::top->filename);
		wxString curSubsAudio = DecodeRelativePath(subs->GetScriptInfo(_T("Audio File")),AssFile::top->filename);
		wxString AutoScriptString = subs->GetScriptInfo(_T("Automation Scripts"));

		// Automation script
		if (AutoScriptString != _T("")) {
			wxStringTokenizer toker(AutoScriptString, _T("|"), wxTOKEN_STRTOK);
			wxFileName AssFileName(subs->filename);
			while (toker.HasMoreTokens()) {
				wxString sfnames;
				try {
					sfnames = toker.GetNextToken().Trim(false).Trim(true);
					wxFileName sfname(sfnames);
					sfname.Normalize(wxPATH_NORM_ALL, AssFileName.GetPath());
					sfnames = sfname.GetFullPath();
					AutomationScriptFile *sfile = AutomationScriptFile::CreateFromFile(sfnames);
					if (!sfile) {
						wxLogWarning(_T("Automation script referenced in subtitle file not found: %s"), sfname.GetName().c_str());
						continue;
					}
					
					//AssAutomationFilters are added to a global list when constructed, this is not a leak
					new AssAutomationFilter(new AutomationScript(sfile));
				}
				catch (AutomationError &err) {
					wxMessageBox(wxString::Format(_T("Error loading Automation script '%s':\r\n\r\n%s"), sfnames.c_str(), err.message.c_str()), _T("Error loading Automation script"), wxOK | wxICON_ERROR, this);
				}
				catch (...) {
					wxMessageBox(_T("An unknown error occurred loading an Automation script referenced in the subtitle file."), _T("Error loading Automation script"), wxOK | wxICON_ERROR, this);
					continue;
				}
			}
		}

		// Check if there is anything to change
		int autoLoadMode = Options.AsInt(_T("Autoload linked files"));
		bool hasToLoad = false;
		if (curSubsAudio != audioBox->audioName || curSubsVFR != VFR_Output.GetFilename() || curSubsVideo != videoBox->videoDisplay->videoName) {
			hasToLoad = true;
		}

		// Decide whether to load or not
		bool doLoad = false;
		if (hasToLoad) {
			if (autoLoadMode == 1) doLoad = true;
			else if (autoLoadMode == 2) {
				int result = wxMessageBox(_("Do you want to load/unload the associated files?"),_("(Un)Load files?"),wxYES_NO);
				if (result == wxYES) doLoad = true;
			}
		}

		if (doLoad) {
			// Variable frame rate
			LoadVFR(curSubsVFR);

			// Video
			if (curSubsVideo != videoBox->videoDisplay->videoName) {
				if (curSubsVideo != _T("")) {
					LoadVideo(curSubsVideo);
					if (videoBox->videoDisplay->loaded) {
						videoBox->videoDisplay->JumpToFrame(videoPos);
						videoBox->videoDisplay->SetAspectRatio(videoAr,videoArValue);
						videoBox->videoDisplay->SetZoomPos(videoZoom-1);
					}
				}
			}

			// Audio
			if (curSubsAudio != audioBox->audioName) {
				if (curSubsAudio == _T("?video")) LoadAudio(_T(""),true);
				else LoadAudio(curSubsAudio);
			}
		}

		// Display
		SetDisplayMode(-1);
	}

	// Store data on subs
	else {
		// Setup
		wxString seekpos = _T("0");
		wxString ar = _T("0");
		wxString zoom = _T("6");
		if (videoBox->videoDisplay->loaded) {
			seekpos = wxString::Format(_T("%i"),videoBox->videoDisplay->ControlSlider->GetValue());
			zoom = wxString::Format(_T("%i"),videoBox->videoDisplay->zoomBox->GetSelection()+1);

			int arType = videoBox->videoDisplay->GetAspectRatioType();
			if (arType == 4) ar = wxString(_T("c")) + FloatToString(videoBox->videoDisplay->GetAspectRatioValue());
			else ar = wxString::Format(_T("%i"),arType);
		}
		
		// Store audio data
		subs->SetScriptInfo(_T("Audio File"),MakeRelativePath(audioBox->audioName,AssFile::top->filename));

		// Store video data
		subs->SetScriptInfo(_T("Video File"),MakeRelativePath(videoBox->videoDisplay->videoName,AssFile::top->filename));
		subs->SetScriptInfo(_T("Video Aspect Ratio"),ar);
		subs->SetScriptInfo(_T("Video Zoom"),zoom);
		subs->SetScriptInfo(_T("Video Position"),seekpos);
		subs->SetScriptInfo(_T("VFR File"),MakeRelativePath(VFR_Output.GetFilename(),AssFile::top->filename));

		// Create list of Automation scripts
		wxString scripts;
		std::list<AssAutomationFilter*>::const_iterator f = AssAutomationFilter::GetFilterList().begin();
		wxFileName AssFileName(subs->filename);
		for (;f != AssAutomationFilter::GetFilterList().end(); ++f) {
			if (!(*f)->GetScript()->filename.empty()) {
				wxFileName fn((*f)->GetScript()->filename);
				fn.MakeRelativeTo(AssFileName.GetPath());
				scripts += wxString::Format(_T("%s|"), fn.GetFullPath().c_str());
			}
		}
		if (!scripts.empty()) {
			scripts.RemoveLast();
			subs->SetScriptInfo(_T("Automation Scripts"), scripts);
		}
	}
}


///////////////
// Loads video
void FrameMain::LoadVideo(wxString file,bool autoload) {
	if (blockVideoLoad) return;
	videoBox->videoDisplay->Stop();
	try {
		if (videoBox->videoDisplay->loaded && VFR_Output.GetFrameRateType() == VFR && !autoload) {
			int result = wxMessageBox(_("You have timecodes loaded currently. Would you like to unload them?"), _("Unload timecodes?"), wxYES_NO, this);
			if (result == wxYES) {
				VFR_Output.Unload();
			}
		}
		videoBox->videoDisplay->SetVideo(file);
	}
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}

	// Check that the video size matches the script video size specified
	if (videoBox->videoDisplay->loaded) {
		int scriptx = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResX"));
		int scripty = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResY"));
		int vidx = videoBox->videoDisplay->provider->GetSourceWidth(), vidy = videoBox->videoDisplay->provider->GetSourceHeight();
		if (scriptx != vidx || scripty != vidy) {
			switch (Options.AsInt(_T("Video Check Script Res"))) {
				case 1:
					// Ask to change on mismatch
					if (wxMessageBox(wxString::Format(_("The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\nVideo resolution:\t%d x %d\nScript resolution:\t%d x %d\n\nChange subtitles resolution to match video?"), vidx, vidy, scriptx, scripty), _("Resolution mismatch"), wxYES_NO, this) != wxYES)
						break;
					// Fallthrough to case 2
				case 2:
					// Always change script res
					SubsBox->ass->SetScriptInfo(_T("PlayResX"), wxString::Format(_T("%d"), vidx));
					SubsBox->ass->SetScriptInfo(_T("PlayResY"), wxString::Format(_T("%d"), vidy));
					break;
				case 0:
				default:
					// Never change
					break;
			}
		}
	}

	SubsBox->CommitChanges(true);
	SetDisplayMode(-1);
	EditBox->UpdateFrameTiming();
}


///////////////
// Loads audio
void FrameMain::LoadAudio(wxString filename,bool FromVideo) {
	if (blockAudioLoad) return;
	videoBox->videoDisplay->Stop();
	try {
		audioBox->SetFile(filename,FromVideo);
		SetDisplayMode(-1);
	}
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error opening audio file"), wxOK | wxICON_ERROR, this);
	}
	#ifdef __WINDOWS__ 
	catch (AvisynthError err) {
		wxMessageBox (wxString(_T("AviSynth error: ")) + wxString(err.msg,wxConvUTF8), _T("Error loading audio"), wxOK | wxICON_ERROR);
		return;
	}
	#endif
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening audio file"), wxOK | wxICON_ERROR, this);
	}
}


/////////////
// Loads VFR
void FrameMain::LoadVFR(wxString filename) {
	videoBox->videoDisplay->Stop();
	if (filename != _T("")) {
		try {
			VFR_Output.Load(filename);
			SubsBox->Refresh(false);
		}
		// Fail
		catch (const wchar_t *error) {
			wxString err(error);
			wxMessageBox(err, _T("Error opening timecodes file"), wxOK | wxICON_ERROR, this);
		}
		catch (...) {
			wxMessageBox(_T("Unknown error"), _T("Error opening video file"), wxOK | wxICON_ERROR, this);
		}
	}

	else {
		VFR_Output.Unload();
		if (videoBox->videoDisplay->loaded && !VFR_Output.IsLoaded()) {
			VFR_Output.SetCFR(videoBox->videoDisplay->fps);
		}
	}

	SubsBox->CommitChanges();
	EditBox->UpdateFrameTiming();
}


/////////////
// Open help
void FrameMain::OpenHelp(wxString page) {
	if (!page.IsEmpty()) page = _T("::") + page;
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("chm"));
	if (type) {
		wxString command = type->GetOpenCommand(AegisubApp::folderName + _T("Aegisub.chm"));
		if (!command.empty()) wxExecute(command + page);
	}
}


/////////////////
// Get encodings
wxArrayString FrameMain::GetEncodings() {
	wxArrayString choices;
	choices.Add(_T("UTF-8"));
	choices.Add(_T("UTF-16"));
	choices.Add(_T("UTF-16BE"));
	choices.Add(_T("UTF-16LE"));
	choices.Add(_T("UTF-7"));
	choices.Add(_T("Local"));
	choices.Add(_T("US-ASCII"));
	choices.Add(_T("SHIFT_JIS"));
	choices.Add(_T("GB2312"));
	choices.Add(_T("BIG5"));
	choices.Add(_T("EUC-JP"));
	choices.Add(_T("KOI8-R"));
	choices.Add(_T("KOI8-RU"));
	choices.Add(_T("KOI8-U"));
	choices.Add(_T("ISO-8859-1"));
	choices.Add(_T("ISO-8859-2"));
	choices.Add(_T("ISO-8859-3"));
	choices.Add(_T("ISO-8859-4"));
	choices.Add(_T("ISO-8859-5"));
	choices.Add(_T("ISO-8859-6"));
	choices.Add(_T("ISO-8859-7"));
	choices.Add(_T("ISO-8859-8"));
	choices.Add(_T("ISO-8859-9"));
	choices.Add(_T("ISO-8859-13"));
	choices.Add(_T("ISO-8859-15"));
	choices.Add(_T("WINDOWS-1250"));
	choices.Add(_T("WINDOWS-1251"));
	choices.Add(_T("WINDOWS-1252"));
	choices.Add(_T("WINDOWS-1253"));
	choices.Add(_T("WINDOWS-1254"));
	choices.Add(_T("WINDOWS-1255"));
	choices.Add(_T("WINDOWS-1256"));
	choices.Add(_T("WINDOWS-1257"));
	choices.Add(_T("WINDOWS-1258"));
	choices.Add(_T("WINDOWS-874"));
	choices.Add(_T("WINDOWS-932"));
	choices.Add(_T("WINDOWS-936"));
	choices.Add(_T("WINDOWS-949"));
	choices.Add(_T("WINDOWS-950"));
	return choices;
}


/////////////////////////////////////////////
// Sets status and clear after n miliseconds
void FrameMain::StatusTimeout(wxString text,int ms) {
	SetStatusText(text,1);
	StatusClear.SetOwner(this,StatusClear_Timer);
	StatusClear.Start(ms,true);
}


///////////////////////////
// Setup accelerator table
void FrameMain::SetAccelerators() {
	wxAcceleratorEntry entry[9];
	entry[0] = Hotkeys.GetAccelerator(_T("Video global prev frame"),Video_Prev_Frame);
	entry[1] = Hotkeys.GetAccelerator(_T("Video global next frame"),Video_Next_Frame);
	entry[2] = Hotkeys.GetAccelerator(_T("Video global focus seek"),Video_Focus_Seek);
	entry[3] = Hotkeys.GetAccelerator(_T("Grid global prev line"),Grid_Prev_Line);
	entry[4] = Hotkeys.GetAccelerator(_T("Grid global next line"),Grid_Next_Line);
	entry[5] = Hotkeys.GetAccelerator(_T("Save Subtitles Alt"),Menu_File_Save_Subtitles);
	entry[6] = Hotkeys.GetAccelerator(_T("Video global zoom in"),Menu_Video_Zoom_In);
	entry[7] = Hotkeys.GetAccelerator(_T("Video global zoom out"),Menu_Video_Zoom_Out);
	wxAcceleratorEntry temp;
	temp.Set(wxACCEL_CTRL | wxACCEL_ALT,WXK_F12,Kana_Game);
	entry[8] = temp;
	wxAcceleratorTable table(9,entry);
	SetAcceleratorTable(table);
}


//////////////////////
// Load list of files
bool FrameMain::LoadList(wxArrayString list) {
	// Build list
	wxArrayString List;
	for (size_t i=0;i<list.Count();i++) {
		wxFileName file(list[i]);
		if (file.IsRelative()) file.MakeAbsolute();
		if (file.FileExists()) List.Add(file.GetFullPath());
	}

	// Video formats
	wxArrayString videoList;
	videoList.Add(_T("avi"));
	videoList.Add(_T("mkv"));
	videoList.Add(_T("mp4"));
	videoList.Add(_T("d2v"));
	videoList.Add(_T("mpg"));
	videoList.Add(_T("mpeg"));
	videoList.Add(_T("ogm"));
	videoList.Add(_T("avs"));
	videoList.Add(_T("wmv"));
	videoList.Add(_T("asf"));
	videoList.Add(_T("mov"));
	videoList.Add(_T("rm"));

	// Subtitle formats
	wxArrayString subsList;
	subsList.Add(_T("ass"));
	subsList.Add(_T("ssa"));
	subsList.Add(_T("srt"));
	subsList.Add(_T("txt"));

	// Audio formats
	wxArrayString audioList;
	audioList.Add(_T("wav"));
	audioList.Add(_T("mp3"));
	audioList.Add(_T("ogg"));
	audioList.Add(_T("wma"));
	audioList.Add(_T("ac3"));
	audioList.Add(_T("aac"));
	audioList.Add(_T("mpc"));
	audioList.Add(_T("ape"));
	audioList.Add(_T("flac"));
	audioList.Add(_T("mka"));

	// Scan list
	wxString audio = _T("");
	wxString video = _T("");
	wxString subs = _T("");
	wxString ext;
	for (size_t i=0;i<List.Count();i++) {
		wxFileName file(List[i]);
		ext = file.GetExt().Lower();

		if (subs.IsEmpty() && subsList.Index(ext) != wxNOT_FOUND) subs = List[i];
		if (video.IsEmpty() && videoList.Index(ext) != wxNOT_FOUND) video = List[i];
		if (audio.IsEmpty() && audioList.Index(ext) != wxNOT_FOUND) audio = List[i];
	}

	// Set blocking
	blockAudioLoad = (audio != _T(""));
	blockVideoLoad = (video != _T(""));

	// Load files
	if (subs != _T("")) {
		LoadSubtitles(subs);
	}
	if (blockVideoLoad) {
		blockVideoLoad = false;
		LoadVideo(video);
	}
	if (blockAudioLoad) {
		blockAudioLoad = false;
		LoadAudio(audio);
	}

	// Result
	return ((subs != _T("")) || (audio != _T("")) || (video != _T("")));
}
