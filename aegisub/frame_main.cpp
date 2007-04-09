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
#include <wx/image.h>
#include "subs_grid.h"
#include "frame_main.h"
#include "avisynth_wrap.h"
#include "video_display.h"
#include "video_provider.h"
#include "video_slider.h"
#include "ass_file.h"
#include "dialog_search_replace.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "main.h"
#include "version.h"
#include "dialog_splash.h"
#include "dialog_tip.h"
#include "audio_box.h"
#include "audio_display.h"
#include "video_box.h"
#include "drop.h"
#include "hotkeys.h"
#include "utils.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "auto4_base.h"
#include "dialog_version_check.h"
#include "dialog_detached_video.h"


/////////////////////////
// FrameMain constructor

FrameMain::FrameMain (wxArrayString args)
: wxFrame ((wxFrame*)NULL,-1,_T(""),wxDefaultPosition,wxSize(800,600),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
{
#ifdef __WXGTK__
/* XXX HACK XXX
 * Gtk just got initialized. And if we're using the SCIM IME,
 * it just did a setlocale(LC_ALL, ""). so, BOOM. "§!$)(!"§$.
 */
 	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "C");
	setlocale(LC_NUMERIC, "C");
/* XXX HACK XXX */
#endif

	// Set application's frame
	AegisubApp::Get()->frame = this;

	LogWindow = new wxLogWindow(this, _T("Aegisub log window"), false);

	// Initialize flags
	HasSelection = false;
	menuCreated = false;
	blockAudioLoad = false;
	blockAudioLoad = false;

	// Create PNG handler
	wxPNGHandler *png = new wxPNGHandler;
	wxImage::AddHandler(png);

	// Storage for subs-file-local scripts
	local_scripts = new Automation4::ScriptManager();

	// Create menu and tool bars
	if (Options.AsBool(_T("Maximized"))) Maximize(true);
	InitToolbar();
	InitMenu();
	
	// Create status bar
	CreateStatusBar(2);

	// Set icon
	SetIcon(wxICON(wxicon));

	// Contents
	showVideo = true;
	showAudio = true;
	detachedVideo = NULL;
	InitContents();
	Show();

	// Splash screen
	#ifndef _DEBUG
	if (Options.AsBool(_T("Show Splash"))) {
		SplashScreen *splash = new SplashScreen(this);
		splash->Show(true);
		splash->Update();
	}
	else {
		// Show tip of the day
		TipOfTheDay::Show(this);
	}
	#endif
	wxSafeYield();

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

	// Parse arguments
	LoadSubtitles(_T(""));
	LoadList(args);

	// Version checker
	int option = Options.AsInt(_T("Auto check for updates"));
	if (option == -1) {
		int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"),wxYES_NO);
		option = 0;
		if (result == wxYES) option = 1;
		Options.SetInt(_T("Auto check for updates"),option);
		Options.Save();
	}
	if (option == 1) {
		DialogVersionCheck *checker = new DialogVersionCheck (this,true);
		(void)checker;
	}
}


////////////////////////
// FrameMain destructor
FrameMain::~FrameMain () {
	DeInitContents();
	delete local_scripts;
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
	Toolbar->AddTool(Menu_Edit_Shift,_("Shift Times"),wxBITMAP(shift_times_toolbutton),_("Open Shift Times Dialogue"));
	Toolbar->AddTool(Menu_Tools_Styling,_("Styling Assistant"),wxBITMAP(styling_toolbutton),_("Open Styling Assistant"));
	Toolbar->AddTool(Menu_Tools_Translation,_("Translation Assistant"),wxBITMAP(translation_toolbutton),_("Open Translation Assistant"));
	Toolbar->AddTool(Menu_Tools_Fonts_Collector,_("Fonts Collector"),wxBITMAP(font_collector_button),_("Open Fonts Collector"));
	Toolbar->AddTool(Menu_Tools_Resample,_("Resample"),wxBITMAP(resample_toolbutton),_("Resample Script Resolution"));
	Toolbar->AddTool(Menu_Tools_Timing_Processor,_("Timing Post-Processor"),wxBITMAP(timing_processor_toolbutton),_("Open Timing Post-processor dialog"));
#if USE_HUNSPELL == 1
	Toolbar->AddTool(Menu_Tools_SpellCheck,_("Spell Checker"),wxBITMAP(spellcheck_toolbutton),_("Open Spell checker"));
#endif
	Toolbar->AddSeparator();

	// Options
	Toolbar->AddTool(Menu_Tools_Options,_("Options"),wxBITMAP(options_button),_("Configure Aegisub"));
	Toolbar->AddTool(Menu_Tools_Hotkeys,_("Hotkeys"),wxBITMAP(hotkeys_button),_("Remap hotkeys"));
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
	RecentKeyframes = new wxMenu();

	// Create file menu
	fileMenu = new wxMenu();
	AppendBitmapMenuItem(fileMenu,Menu_File_New_Subtitles, _("&New Subtitles\t") + Hotkeys.GetText(_T("New Subtitles")), _("New subtitles"),wxBITMAP(new_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Open_Subtitles, _("&Open Subtitles...\t") + Hotkeys.GetText(_T("Open Subtitles")), _("Opens a subtitles file"),wxBITMAP(open_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Open_Subtitles_Charset, _("&Open Subtitles with Charset..."), _("Opens a subtitles file with a specific charset"),wxBITMAP(open_with_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Save_Subtitles, _("&Save Subtitles\t") + Hotkeys.GetText(_T("Save Subtitles")), _("Saves subtitles"),wxBITMAP(save_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Save_Subtitles_As, _("Save Subtitles as..."), _("Saves subtitles with another name"), wxBITMAP(save_as_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Export_Subtitles, _("Export Subtitles..."), _("Saves a copy of subtitles with processing applied to it."), wxBITMAP(blank_button));
	wxMenuItem *RecentParent = new wxMenuItem(fileMenu, Menu_File_Recent_Subs_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentSubs);
	RecentParent->SetBitmap(wxBITMAP(blank_button));
	fileMenu->Append(RecentParent);
	fileMenu->AppendSeparator();
	AppendBitmapMenuItem (fileMenu,Menu_Tools_Properties, _("&Properties..."), _("Open script properties window"),wxBITMAP(properties_toolbutton));
	AppendBitmapMenuItem (fileMenu,Menu_Tools_Attachments, _("&Attachments..."), _("Open the attachment list"), wxBITMAP(attach_button));
	fileMenu->AppendSeparator();
	AppendBitmapMenuItem(fileMenu,Menu_File_Exit, _("E&xit\t") + Hotkeys.GetText(_T("Exit")), _("Exit the application"),wxBITMAP(exit_button));
	MenuBar->Append(fileMenu, _("&File"));

	// Create Edit menu
	// NOTE: Undo and Redo are actually controlled in frame_main_events, OnMenuOpen(). They will always be the first two items.
	editMenu = new wxMenu();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Undo, _("&Undo") + wxString(_T("\t")) + Hotkeys.GetText(_T("Undo")), _("Undoes last action"),wxBITMAP(undo_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Redo, _("&Redo") + wxString(_T("\t")) + Hotkeys.GetText(_T("Redo")), _("Redoes last action"),wxBITMAP(redo_button));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Cut, _("Cut Lines\t") + Hotkeys.GetText(_T("Cut")), _("Cut subtitles"), wxBITMAP(cut_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Copy, _("Copy Lines\t") + Hotkeys.GetText(_T("Copy")), _("Copy subtitles"), wxBITMAP(copy_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Paste, _("Paste Lines\t") + Hotkeys.GetText(_T("Paste")), _("Paste subtitles"), wxBITMAP(paste_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Paste_Over, _("Paste Lines Over...\t") + Hotkeys.GetText(_T("Paste Over")) , _("Paste subtitles over others"),wxBITMAP(paste_over_button));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Find, _("&Find...\t") + Hotkeys.GetText(_T("Find")), _("Find words in subtitles"),wxBITMAP(find_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Find_Next, _("Find Next\t") + Hotkeys.GetText(_T("Find Next")), _("Find next match of last word"),wxBITMAP(find_next_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Replace, _("Search and &Replace...\t") + Hotkeys.GetText(_T("Replace")) , _("Find and replace words in subtitles"),wxBITMAP(replace_button));
	MenuBar->Append(editMenu, _("&Edit"));

	// Create subtitles menu
	subtitlesMenu = new wxMenu();
	wxMenu *InsertMenu = new wxMenu;
	wxMenuItem *InsertParent = new wxMenuItem(subtitlesMenu,Menu_Subtitles_Insert,_("&Insert Lines"),_T(""),wxITEM_NORMAL,InsertMenu);
	InsertParent->SetBitmap(wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_BEFORE,_("&Before Current"),_("Inserts a line before current"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_AFTER,_("&After Current"),_("Inserts a line after current"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_BEFORE_VIDEO,_("Before Current, at Video Time"),_("Inserts a line before current, starting at video time"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_AFTER_VIDEO,_("After Current, at Video Time"),_("Inserts a line after current, starting at video time"),wxBITMAP(blank_button));
	subtitlesMenu->Append(InsertParent);
	AppendBitmapMenuItem(subtitlesMenu,MENU_DUPLICATE,wxString(_("&Duplicate Lines")) + _T("\t") + Hotkeys.GetText(_T("Grid duplicate rows")),_("Duplicate the selected lines"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_DUPLICATE_NEXT_FRAME,wxString(_("&Duplicate and shift by 1 frame")) + _T("\t") + Hotkeys.GetText(_T("Grid duplicate and shift one frame")),_("Duplicate lines and shift by one frame"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_DELETE,wxString(_("Delete Lines")) + _T("\t") + Hotkeys.GetText(_T("Grid delete rows")),_("Delete currently selected lines"),wxBITMAP(blank_button));
	subtitlesMenu->AppendSeparator();
	wxMenu *JoinMenu = new wxMenu;
	wxMenuItem *JoinParent = new wxMenuItem(subtitlesMenu,Menu_Subtitles_Join,_("Join Lines"),_T(""),wxITEM_NORMAL,JoinMenu);
	JoinParent->SetBitmap(wxBITMAP(blank_button));
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_CONCAT,_("&Concatenate"),_("Joins selected lines in a single one, concatenating text together"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_REPLACE,_("Keep &First"),_("Joins selected lines in a single one, keeping text of first and discarding remaining"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_AS_KARAOKE,_("As &Karaoke"),_("Joins selected lines in a single one, as karaoke"),wxBITMAP(blank_button));
	subtitlesMenu->Append(JoinParent);
	AppendBitmapMenuItem(subtitlesMenu,MENU_RECOMBINE,_("Recombine Lines"),_("Recombine subtitles when they have been split and merged"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_SPLIT_BY_KARAOKE,_("Split Lines (by karaoke)"),_("Uses karaoke timing to split line into multiple smaller lines"),wxBITMAP(blank_button));
	subtitlesMenu->AppendSeparator();
	AppendBitmapMenuItem(subtitlesMenu,MENU_SWAP,_("Swap Lines"),_("Swaps the two selected lines"),wxBITMAP(blank_button));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Edit_Select, _("Select Lines...\t") + Hotkeys.GetText(_T("Select lines")), _("Selects lines based on defined criterea"),wxBITMAP(blank_button));
	subtitlesMenu->AppendSeparator();
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Styles_Manager, _("&Styles Manager..."), _("Open styles manager"), wxBITMAP(style_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Styling, _("St&yling Assistant..."), _("Open styling assistant"), wxBITMAP(styling_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Translation, _("&Translation Assistant..."),_("Open translation assistant"), wxBITMAP(translation_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Resample,_("Resample resolution..."), _("Changes resolution and modifies subtitles to conform to change"), wxBITMAP(resample_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Fonts_Collector, _("&Fonts Collector..."),_("Open fonts collector"), wxBITMAP(font_collector_button));
#if USE_HUNSPELL == 1
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_SpellCheck, _("Spe&ll Checker..."),_("Open spell checker"), wxBITMAP(spellcheck_toolbutton));
#endif
	MenuBar->Append(subtitlesMenu, _("&Subtitles"));

	// Create timing menu
	timingMenu = new wxMenu();
	AppendBitmapMenuItem(timingMenu,Menu_Edit_Shift, _("S&hift Times...") + wxString(_T("\t")) + Hotkeys.GetText(_T("Shift times")), _("Shift subtitles by time or frames"),wxBITMAP(shift_times_toolbutton));
	AppendBitmapMenuItem(timingMenu,Menu_Edit_Sort, _("Sort by Time"), _("Sort all subtitles by their start times"),wxBITMAP(sort_times_button));
	AppendBitmapMenuItem(timingMenu,Menu_Tools_Timing_Processor,_("Timing Post-Processor..."), _("Runs a post-processor for timing to deal with lead-ins, lead-outs, scene timing and etc."), wxBITMAP(timing_processor_toolbutton));
	AppendBitmapMenuItem (timingMenu,Menu_Tools_Kanji_Timer,_("Kanji Timer..."),_("Open Kanji timer"),wxBITMAP(blank_button));
	timingMenu->AppendSeparator();
	AppendBitmapMenuItem(timingMenu,Menu_Subs_Snap_Start_To_Video, _("Snap Start to Video") + wxString(_T("\t")) + Hotkeys.GetText(_T("Set Start To Video")), _("Set start of selected subtitles to current video frame"), wxBITMAP(substart_to_video));
	AppendBitmapMenuItem(timingMenu,Menu_Subs_Snap_End_To_Video, _("Snap End to Video") + wxString(_T("\t")) + Hotkeys.GetText(_T("Set End to Video")), _("Set end of selected subtitles to current video frame"), wxBITMAP(subend_to_video));
	AppendBitmapMenuItem(timingMenu,Menu_Video_Snap_To_Scene, _("Snap to Scene") + wxString(_T("\t")) + Hotkeys.GetText(_T("Snap to Scene")), _("Set start and end of subtitles to the keyframes around current video frame"), wxBITMAP(snap_subs_to_scene));
	AppendBitmapMenuItem(timingMenu,Menu_Video_Shift_To_Frame, _("Shift to Current Frame") + wxString(_T("\t")) + Hotkeys.GetText(_T("Shift by Current Time")), _("Shift selection so first selected line starts at current frame"), wxBITMAP(shift_to_frame));
	timingMenu->AppendSeparator();
	wxMenu *ContinuousMenu = new wxMenu;
	wxMenuItem *ContinuousParent = new wxMenuItem(subtitlesMenu,-1,_("Make Times Continuous"),_T(""),wxITEM_NORMAL,ContinuousMenu);
	ContinuousParent->SetBitmap(wxBITMAP(blank_button));
	AppendBitmapMenuItem(ContinuousMenu,MENU_ADJOIN,_("Change &Start"),_("Changes times of subs so start times begin on previous's end time"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(ContinuousMenu,MENU_ADJOIN2,_("Change &End"),_("Changes times of subs so end times begin on next's start time"),wxBITMAP(blank_button));
	timingMenu->Append(ContinuousParent);
	MenuBar->Append(timingMenu, _("&Timing"));

	// Create video menu
	videoMenu = new wxMenu();
	videoMenu->Append(Menu_File_Open_Video, _("&Open Video..."), _("Opens a video file"));
	videoMenu->Append(Menu_File_Close_Video, _("&Close Video"), _("Closes the currently open video file"));
	wxMenuItem *RecentVidParent = new wxMenuItem(videoMenu, Menu_File_Recent_Vids_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentVids);
	videoMenu->Append(RecentVidParent);
	videoMenu->Append(Menu_Video_Dummy, _("Use dummy video..."), _("Opens a video clip with solid colour"));
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_File_Open_VFR, _("Open timecodes file..."), _("Opens a VFR timecodes v1 or v2 file"));
	videoMenu->Append(Menu_File_Close_VFR, _("Close timecodes file"), _("Closes the currently open timecodes file"))->Enable(false);
	wxMenuItem *RecentTimesParent = new wxMenuItem(videoMenu, Menu_File_Recent_Timecodes_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentTimecodes);
	videoMenu->Append(RecentTimesParent);
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_Video_Load_Keyframes, _("Open keyframes..."), _("Opens a keyframe list file"));
	videoMenu->Append(Menu_Video_Save_Keyframes, _("Save keyframes..."), _("Saves the current keyframe list"))->Enable(false);
	videoMenu->Append(Menu_Video_Close_Keyframes, _("Close keyframes"), _("Closes the currently open keyframes list"))->Enable(false);
	wxMenuItem *RecentKeyframesParent = new wxMenuItem(videoMenu, Menu_File_Recent_Keyframes_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentKeyframes);
	videoMenu->Append(RecentKeyframesParent);
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_Video_Detach, _("Detach Video"), _("Detach video, displaying it in a separate Window."));
	wxMenu *ZoomMenu = new wxMenu;
	wxMenuItem *ZoomParent = new wxMenuItem(subtitlesMenu,Menu_View_Zoom,_("Set Zoom"),_T(""),wxITEM_NORMAL,ZoomMenu);
	ZoomParent->SetBitmap(wxBITMAP(blank_button));
	ZoomMenu->Append(Menu_View_Zoom_50, _T("&50%\t") + Hotkeys.GetText(_T("Zoom 50%")), _("Set zoom to 50%"));
	ZoomMenu->Append(Menu_View_Zoom_100, _T("&100%\t") + Hotkeys.GetText(_T("Zoom 100%")), _("Set zoom to 100%"));
	ZoomMenu->Append(Menu_View_Zoom_200, _T("&200%\t") + Hotkeys.GetText(_T("Zoom 200%")), _("Set zoom to 200%"));
	videoMenu->Append(ZoomParent);
	wxMenu *AspectMenu = new wxMenu;
	wxMenuItem *AspectParent = new wxMenuItem(subtitlesMenu,Menu_Video_AR,_("Override Aspect Ratio"),_T(""),wxITEM_NORMAL,AspectMenu);
	AspectParent->SetBitmap(wxBITMAP(blank_button));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Default, _("&Default"), _("Leave video on original aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Full, _("&Fullscreen (4:3)"), _("Forces video to 4:3 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Wide, _("&Widescreen (16:9)"), _("Forces video to 16:9 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_235, _("&Cinematic (2.35)"), _("Forces video to 2.35 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Custom, _("Custom..."), _("Forces video to a custom aspect ratio"));
	videoMenu->Append(AspectParent);
	videoMenu->AppendSeparator();
	AppendBitmapMenuItem(videoMenu,Menu_Video_JumpTo, _("&Jump To...\t") + Hotkeys.GetText(_T("Video Jump")), _("Jump to frame or time"), wxBITMAP(jumpto_button));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_Start, _("Jump video to start\t") + Hotkeys.GetText(_T("Jump Video To Start")), _("Jumps the video to the start frame of current subtitle"), wxBITMAP(video_to_substart));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_End, _("Jump video to end\t") + Hotkeys.GetText(_T("Jump Video To End")), _("Jumps the video to the end frame of current subtitle"), wxBITMAP(video_to_subend));
	MenuBar->Append(videoMenu, _("&Video"));

	// Create audio menu
	audioMenu = new wxMenu();
	audioMenu->Append(Menu_Audio_Open_File, _("&Open Audio file..."), _("Opens an audio file"));
	audioMenu->Append(Menu_Audio_Open_From_Video, _("Open Audio from &Video"), _("Opens the audio from the current video file"));
	audioMenu->Append(Menu_Audio_Close, _("&Close Audio"), _("Closes the currently open audio file"));
	wxMenuItem *RecentAudParent = new wxMenuItem(audioMenu, Menu_File_Recent_Auds_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentAuds);
	audioMenu->Append(RecentAudParent);
	MenuBar->Append(audioMenu, _("&Audio"));

	// Create Automation menu
	automationMenu = new wxMenu();
	AppendBitmapMenuItem (automationMenu,Menu_Tools_Automation, _("&Automation..."),_("Open automation manager"), wxBITMAP(automation_toolbutton));
	automationMenu->AppendSeparator();
	MenuBar->Append(automationMenu, _("&Automation"));

	// Create view menu
	viewMenu = new wxMenu();
	AppendBitmapMenuItem(viewMenu,Menu_View_Language, _T("&Language..."), _("Select Aegisub interface language"), wxBITMAP(blank_button));
	AppendBitmapMenuItem(viewMenu,Menu_Tools_Options, _("&Options...") + wxString(_T("\t")) + Hotkeys.GetText(_T("Options")), _("Configure Aegisub"), wxBITMAP(options_button));
	AppendBitmapMenuItem(viewMenu,Menu_Tools_Hotkeys, _("&Hotkeys..."), _("Remap hotkeys"), wxBITMAP(hotkeys_button));
	AppendBitmapMenuItem(viewMenu,Menu_Tools_Log, _("Lo&g window..."), _("Open log window"), wxBITMAP(blank_button));
	viewMenu->AppendSeparator();
	viewMenu->AppendRadioItem(Menu_View_Subs, _("Subs only view"), _("Display subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Video, _("Video+Subs view"), _("Display video and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Audio, _("Audio+Subs view"), _("Display audio and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Standard, _("Full view"), _("Display audio, video and subtitles"));
	MenuBar->Append(viewMenu, _("Vie&w"));

	// Create help menu
	helpMenu = new wxMenu();
	AppendBitmapMenuItem (helpMenu,Menu_Help_Contents, _("&Contents...\t") + Hotkeys.GetText(_T("Help")), _("Help topics"), wxBITMAP(contents_button));
	helpMenu->AppendSeparator();
	AppendBitmapMenuItem(helpMenu,Menu_Help_Website, _("&Website..."), _("Visit Aegisub's official website"),wxBITMAP(website_button));
	AppendBitmapMenuItem(helpMenu,Menu_Help_Forums, _("&Forums..."), _("Visit Aegisub's forums"),wxBITMAP(forums_button));
	AppendBitmapMenuItem(helpMenu,Menu_Help_BugTracker, _("&Bug tracker..."), _("Visit Aegisub's bug tracker"),wxBITMAP(bugtracker_button));
	AppendBitmapMenuItem (helpMenu,Menu_Help_IRCChannel, _("&IRC channel..."), _("Visit Aegisub's official IRC channel"), wxBITMAP(irc_button));
	helpMenu->AppendSeparator();
	AppendBitmapMenuItem(helpMenu,Menu_Help_Check_Updates, _("&Check for Updates..."), _("Check to see if there is a new version of Aegisub available"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(helpMenu,Menu_Help_About, _("&About..."), _("About Aegisub"),wxBITMAP(about_button));
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
	TopSizer->Add(videoBox,0,wxEXPAND,0);
	videoBox->videoDisplay->zoomBox = ZoomBox;

	// Subtitles area
	SubsBox = new SubtitlesGrid(this,Panel,-1,wxDefaultPosition,wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,_T("Subs grid"));
	BottomSizer->Add(SubsBox,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,0);
	AssFile::StackReset();
	videoBox->videoSlider->grid = SubsBox;
	VideoContext::Get()->grid = SubsBox;
	videoBox->videoDisplay->SetZoomPos(Options.AsInt(_T("Video Default Zoom")));
	Search.grid = SubsBox;

	// Audio area
	audioBox = new AudioBox(Panel);
	audioBox->frameMain = this;
	VideoContext::Get()->audio = audioBox->audioDisplay;

	// Top sizer
	EditBox = new SubsEditBox(Panel,SubsBox);
	EditBox->audio = audioBox->audioDisplay;
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
	SetDisplayMode(0,0);
	Layout();
}


/////////////////////////
// Deinitialize controls
void FrameMain::DeInitContents() {
	if (detachedVideo) detachedVideo->Destroy();
	AssFile::StackReset();
	delete AssFile::top;
	delete EditBox;
	delete videoBox;
}


//////////////////
// Update toolbar
void FrameMain::UpdateToolbar() {
	// Collect flags
	bool isVideo = VideoContext::Get()->IsLoaded();
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
		VideoContext::Get()->Stop();
		wxString path = Options.AsText(_T("Last open subtitles path"));
		wxFileName origPath(AssFile::top->filename);
		filename = 	wxFileSelector(_("Save subtitles file"),path,origPath.GetName() + _T(".ass"),_T("ass"),AssFile::GetWildcardList(1),wxFD_SAVE | wxFD_OVERWRITE_PROMPT,this);
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
void FrameMain::SetDisplayMode(int _showVid,int _showAudio) {
	// Shown?
	static bool firstRun = true;
	if (!IsShownOnScreen() && !firstRun) return;
	firstRun = false;

	// Stop
	Freeze();
	VideoContext::Get()->Stop();

	// Automatic
	if (_showVid == -1) _showVid = (VideoContext::Get()->IsLoaded() && !detachedVideo) ? 1 : 0;
	if (_showAudio == -1) _showAudio = audioBox->loaded ? 1 : 0;

	// See if anything changed
	if (_showVid == (showVideo?1:0) && _showAudio == (showAudio?1:0)) {
		Thaw();
		return;
	}
	showAudio = _showAudio == 1;
	showVideo = _showVid == 1;

	// Set display
	TopSizer->Show(videoBox,showVideo,true);
	ToolSizer->Show(audioBox,showAudio,true);

	// Update
	UpdateToolbar();
	EditBox->SetSplitLineMode();
	MainSizer->CalcMin();
	MainSizer->RecalcSizes();
	//videoBox->VideoSizer->Layout();
	MainSizer->Layout();
	Layout();
	Show(true);
	VideoContext::Get()->UpdateDisplays(true);
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
		wxString curSubsKeyframes = DecodeRelativePath(subs->GetScriptInfo(_T("Keyframes File")),AssFile::top->filename);
		wxString curSubsAudio = DecodeRelativePath(subs->GetScriptInfo(_T("Audio File")),AssFile::top->filename);
		wxString AutoScriptString = subs->GetScriptInfo(_T("Automation Scripts"));

		// Check if there is anything to change
		int autoLoadMode = Options.AsInt(_T("Autoload linked files"));
		bool hasToLoad = false;
		if (curSubsAudio != audioBox->audioName ||
			curSubsVFR != VFR_Output.GetFilename() ||
			curSubsVideo != VideoContext::Get()->videoName ||
			curSubsKeyframes != VideoContext::Get()->GetKeyFramesName() ||
			!AutoScriptString.IsEmpty() ||
			local_scripts->GetScripts().size() > 0) {
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
			if (curSubsVideo != VideoContext::Get()->videoName) {
				if (curSubsVideo != _T("")) {
					LoadVideo(curSubsVideo);
					if (VideoContext::Get()->IsLoaded()) {
						VideoContext::Get()->SetAspectRatio(videoAr,videoArValue);
						videoBox->videoDisplay->SetZoomPos(videoZoom-1);
						VideoContext::Get()->JumpToFrame(videoPos);
					}
				}
			}

			// Keyframes
			LoadKeyframes(curSubsKeyframes);

			// Audio
			if (curSubsAudio != audioBox->audioName) {
				if (curSubsAudio == _T("?video")) LoadAudio(_T(""),true);
				else LoadAudio(curSubsAudio);
			}

			// Automation scripts
			local_scripts->RemoveAll();
			wxStringTokenizer tok(AutoScriptString, _T("|"), wxTOKEN_STRTOK);
			wxFileName subsfn(subs->filename);
			wxString autobasefn(Options.AsText(_T("Automation Base Path")));
			while (tok.HasMoreTokens()) {
				wxString sfnames = tok.GetNextToken().Trim(true).Trim(false);
				wxString sfnamel = sfnames.Left(1);
				sfnames.Remove(0, 1);
				wxString basepath;
				if (sfnamel == _T("~")) {
					basepath = subsfn.GetPath();
				} else if (sfnamel == _T("$")) {
					basepath = autobasefn;
				} else if (sfnamel == _T("/")) {
					basepath = _T("");
				} else {
					wxLogWarning(_T("Automation Script referenced with unknown location specifier character.\nLocation specifier found: %s\nFilename specified: %s"),
						sfnamel.c_str(), sfnames.c_str());
					continue;
				}
				wxFileName sfname(sfnames);
				sfname.MakeAbsolute(basepath);
				if (sfname.FileExists()) {
					sfnames = sfname.GetFullPath();
					local_scripts->Add(Automation4::ScriptFactory::CreateFromFile(sfnames));
				} else {
					wxLogWarning(_T("Automation Script referenced could not be found.\nFilename specified: %s%s\nSearched relative to: %s\nResolved filename: %s"),
						sfnamel.c_str(), sfnames.c_str(), basepath.c_str(), sfname.GetFullPath().c_str());
				}
			}
		}

		// Display
		SetDisplayMode(-1,-1);
	}

	// Store data on subs
	else {
		// Setup
		wxString seekpos = _T("0");
		wxString ar = _T("0");
		wxString zoom = _T("6");
		if (VideoContext::Get()->IsLoaded()) {
			seekpos = wxString::Format(_T("%i"),videoBox->videoDisplay->ControlSlider->GetValue());
			zoom = wxString::Format(_T("%i"),videoBox->videoDisplay->zoomBox->GetSelection()+1);

			int arType = VideoContext::Get()->GetAspectRatioType();
			if (arType == 4) ar = wxString(_T("c")) + FloatToString(VideoContext::Get()->GetAspectRatioValue());
			else ar = wxString::Format(_T("%i"),arType);
		}
		
		// Store audio data
		subs->SetScriptInfo(_T("Audio File"),MakeRelativePath(audioBox->audioName,AssFile::top->filename));

		// Store video data
		subs->SetScriptInfo(_T("Video File"),MakeRelativePath(VideoContext::Get()->videoName,AssFile::top->filename));
		subs->SetScriptInfo(_T("Video Aspect Ratio"),ar);
		subs->SetScriptInfo(_T("Video Zoom"),zoom);
		subs->SetScriptInfo(_T("Video Position"),seekpos);
		subs->SetScriptInfo(_T("VFR File"),MakeRelativePath(VFR_Output.GetFilename(),AssFile::top->filename));
		subs->SetScriptInfo(_T("Keyframes File"),MakeRelativePath(VideoContext::Get()->GetKeyFramesName(),AssFile::top->filename));

		// Store Automation script data
		// Algorithm:
		// 1. If script filename has Automation Base Path as a prefix, the path is relative to that (ie. "$")
		// 2. Otherwise try making it relative to the subs filename
		// 3. If step 2 failed, or absolut path is shorter than path relative to subs, use absolute path ("/")
		// 4. Otherwise, use path relative to subs ("~")
		wxString scripts_string;
		wxString autobasefn(Options.AsText(_T("Automation Base Path")));

		const std::vector<Automation4::Script*> &scripts = local_scripts->GetScripts();
		for (unsigned int i = 0; i < scripts.size(); i++) {
			Automation4::Script *script = scripts[i];

			if (i != 0)
				scripts_string += _T("|");

			wxString autobase_rel, subsfile_rel;
			wxString scriptfn(script->GetFilename());
			autobase_rel = MakeRelativePath(scriptfn, autobasefn);
			subsfile_rel = MakeRelativePath(scriptfn, AssFile::top->filename);

			if (autobase_rel.size() <= scriptfn.size() && autobase_rel.size() <= subsfile_rel.size()) {
				scriptfn = _T("$") + autobase_rel;
			} else if (subsfile_rel.size() <= scriptfn.size() && subsfile_rel.size() <= autobase_rel.size()) {
				scriptfn = _T("~") + subsfile_rel;
			} else {
				scriptfn = _T("/") + wxFileName(scriptfn).GetFullPath(wxPATH_UNIX);
			}

			scripts_string += scriptfn;
		}
		subs->SetScriptInfo(_T("Automation Scripts"), scripts_string);
	}
}


///////////////
// Loads video
void FrameMain::LoadVideo(wxString file,bool autoload) {
	if (blockVideoLoad) return;
	VideoContext::Get()->Stop();
	try {
		if (VideoContext::Get()->IsLoaded() && VFR_Output.GetFrameRateType() == VFR && !autoload) {
			int result = wxMessageBox(_("You have timecodes loaded currently. Would you like to unload them?"), _("Unload timecodes?"), wxYES_NO, this);
			if (result == wxYES) {
				VFR_Output.Unload();
			}
		}
		SetDisplayMode(1,-1);
		VideoContext::Get()->SetVideo(file);
		SetDisplayMode(0,-1);
	}
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}

	// Check that the video size matches the script video size specified
	if (VideoContext::Get()->IsLoaded()) {
		int scriptx = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResX"));
		int scripty = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResY"));
		int vidx = VideoContext::Get()->GetWidth(), vidy = VideoContext::Get()->GetHeight();
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
					SubsBox->ass->FlagAsModified(_("Change script resolution"));
					SubsBox->CommitChanges();
					break;
				case 0:
				default:
					// Never change
					break;
			}
		}
	}

	SubsBox->CommitChanges(true);
	SetDisplayMode(-1,-1);
	EditBox->UpdateFrameTiming();
}


///////////////
// Loads audio
void FrameMain::LoadAudio(wxString filename,bool FromVideo) {
	if (blockAudioLoad) return;
	VideoContext::Get()->Stop();
	try {
		audioBox->SetFile(filename,FromVideo);
		SetDisplayMode(-1,-1);
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
	VideoContext::Get()->Stop();
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
			wxMessageBox(_T("Unknown error"), _T("Error opening timecodes file"), wxOK | wxICON_ERROR, this);
		}
	}

	else {
		VFR_Output.Unload();
		if (VideoContext::Get()->IsLoaded() && !VFR_Output.IsLoaded()) {
			VFR_Output.SetCFR(VideoContext::Get()->GetFPS());
		}
	}

	SubsBox->CommitChanges();
	EditBox->UpdateFrameTiming();
}


//////////////////
// Load Keyframes
void FrameMain::LoadKeyframes(wxString filename) {
	// Unload
	if (filename.IsEmpty()) {
		wxArrayInt keyFrames;
		keyFrames.Empty();
		VideoContext::Get()->CloseOverKeyFrames();
		videoBox->videoSlider->Refresh();
		audioBox->audioDisplay->Update();
		Refresh();
		return;
	}

	// Load
	try {
		// Open file
		wxArrayInt keyFrames;
		keyFrames.Empty();
		TextFileReader file(filename,_T("ASCII"));

		// Read header
		wxString cur = file.ReadLineFromFile();
		if (cur != _T("# keyframe format v1")) throw _T("Invalid keyframes file.");
		cur = file.ReadLineFromFile();
		if (cur.Left(4) != _T("fps ")) throw _T("Invalid keyframes file.");
		cur = cur.Mid(4);
		double fps;
		cur.ToDouble(&fps);
		if (fps == 0.0) throw _T("Invalid FPS.");

		// Read lines
		while (file.HasMoreLines()) {
			cur = file.ReadLineFromFile();
			if (!cur.IsEmpty() && cur.IsNumber()) {
				long temp;
				cur.ToLong(&temp);
				keyFrames.Add(temp);
			}
		}

		// Set keyframes
		VideoContext::Get()->SetOverKeyFrames(keyFrames);
		VideoContext::Get()->SetKeyFramesName(filename);

		// Set FPS
		if (!VideoContext::Get()->IsLoaded()) {
			VideoContext::Get()->SetFPS(fps);
			VFR_Input.SetCFR(fps);
			if (!VFR_Output.IsLoaded()) VFR_Output.SetCFR(fps);
		}

		// Add to recent
		Options.AddToRecentList(filename,_T("Recent keyframes"));

		// Refresh display
		Refresh();
		audioBox->audioDisplay->Update();
	}

	// Fail
	catch (const wchar_t *error) {
		wxString err(error);
		wxMessageBox(err, _T("Error opening keyframes file"), wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening keyframes file"), wxOK | wxICON_ERROR, this);
	}
}


//////////////////
// Save Keyframes
void FrameMain::SaveKeyframes(wxString filename) {
	// Get keyframes
	wxArrayInt keyFrames = VideoContext::Get()->GetKeyFrames();

	// Write header
	TextFileWriter file(filename,_T("ASCII"));
	file.WriteLineToFile(_T("# keyframe format v1"));
	file.WriteLineToFile(wxString::Format(_T("fps %f"),VideoContext::Get()->GetFPS()));

	// Write keyframes
	for (unsigned int i=0;i<keyFrames.Count();i++) {
		file.WriteLineToFile(wxString::Format(_T("%i"),keyFrames[i]));
	}

	// Add to recent
	Options.AddToRecentList(filename,_T("Recent keyframes"));
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
	wxAcceleratorEntry entry[20];
	int i = 0;

	// Standard
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global prev frame"),Video_Prev_Frame);
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global next frame"),Video_Next_Frame);
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global focus seek"),Video_Focus_Seek);
	entry[i++] = Hotkeys.GetAccelerator(_T("Grid global prev line"),Grid_Prev_Line);
	entry[i++] = Hotkeys.GetAccelerator(_T("Grid global next line"),Grid_Next_Line);
	entry[i++] = Hotkeys.GetAccelerator(_T("Save Subtitles Alt"),Menu_File_Save_Subtitles);
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global zoom in"),Menu_Video_Zoom_In);
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global zoom out"),Menu_Video_Zoom_Out);
	entry[i++] = Hotkeys.GetAccelerator(_T("Video global play"),Video_Frame_Play);
	entry[i++] = Hotkeys.GetAccelerator(_T("Edit box commit"),Edit_Box_Commit);

	// Medusa
	bool medusaPlay = Options.AsBool(_T("Audio Medusa Timing Hotkeys"));
	if (medusaPlay && audioBox->audioDisplay->loaded) {
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Play"),Medusa_Play);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Stop"),Medusa_Stop);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Play Before"),Medusa_Play_Before);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Play After"),Medusa_Play_After);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Next"),Medusa_Next);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Previous"),Medusa_Prev);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Shift Start Forward"),Medusa_Shift_Start_Forward);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Shift Start Back"),Medusa_Shift_Start_Back);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Shift End Forward"),Medusa_Shift_End_Forward);
		entry[i++] = Hotkeys.GetAccelerator(_T("Audio Medusa Shift End Back"),Medusa_Shift_End_Back);
	}

	// Set table
	wxAcceleratorTable table(i,entry);
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



//////////////////////
// Sets the descriptions for undo/redo
void FrameMain::SetUndoRedoDesc() {
	editMenu->SetHelpString(0,_T("Undo ")+AssFile::GetUndoDescription());
	editMenu->SetHelpString(1,_T("Redo ")+AssFile::GetRedoDescription());
}
