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
#include "config.h"

#include <wx/wxprec.h>
#include <wx/sysopt.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/image.h>
#include <wx/statline.h>

#include "subs_grid.h"
#include "frame_main.h"
#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#endif
#include "video_display.h"
#include "video_provider_manager.h"
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
#include "dialog_version_check.h"
#include "dialog_detached_video.h"
#include "standard_paths.h"
#include "keyframe.h"
#include "help_button.h"
#include "dialog_styling_assistant.h"
#ifdef WITH_AUTOMATION
#include "auto4_base.h"
#endif
#ifdef __WXMAC__
#include <Carbon/Carbon.h>
#endif



#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, a, _T("Aegisub startup log"), 0)
#else
#define StartupLog(a)
#endif

/////////////////////////
// FrameMain constructor

FrameMain::FrameMain (wxArrayString args)
: wxFrame ((wxFrame*)NULL,-1,_T(""),wxDefaultPosition,wxSize(920,700),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
{
	StartupLog(_T("Entering FrameMain constructor"));
#ifdef __WXGTK__
/* XXX HACK XXX
 * Gtk just got initialized. And if we're using the SCIM IME,
 * it just did a setlocale(LC_ALL, ""). so, BOOM.
 */
 	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "C");
	setlocale(LC_NUMERIC, "C");
/* XXX HACK XXX */
#endif

	// Set application's frame
	AegisubApp::Get()->frame = this;

	StartupLog(_T("Create log window"));
	LogWindow = new wxLogWindow(this, _T("Aegisub log window"), false);

	// Initialize flags
	HasSelection = false;
	menuCreated = false;
	blockAudioLoad = false;
	blockAudioLoad = false;

	StartupLog(_T("Install PNG handler"));
	// Create PNG handler
	wxPNGHandler *png = new wxPNGHandler;
	wxImage::AddHandler(png);

	// Storage for subs-file-local scripts
#ifdef WITH_AUTOMATION
	StartupLog(_T("Create local Automation script manager"));
	local_scripts = new Automation4::ScriptManager();
#endif

	// Create menu and tool bars
	StartupLog(_T("Apply saved Maximized state"));
	if (Options.AsBool(_T("Maximized"))) Maximize(true);
	StartupLog(_T("Initialize toolbar"));
	InitToolbar();
	StartupLog(_T("Initialize menu bar"));
	InitMenu();
	
	// Create status bar
	StartupLog(_T("Create status bar"));
	CreateStatusBar(2);

	// Set icon
	StartupLog(_T("Set icon"));
	SetIcon(wxICON(wxicon));

	// Contents
	showVideo = true;
	showAudio = true;
	detachedVideo = NULL;
	stylingAssistant = NULL;
	StartupLog(_T("Initialize inner main window controls"));
	InitContents();
	StartupLog(_T("Display main window"));
	Show();

	// Splash screen
	// It doesn't work properly on wxMac, and the jumping dock icon
	// signals the same as the splash screen either way.
#if !_DEBUG && !__WXMAC__
	if (Options.AsBool(_T("Show Splash"))) {
		SplashScreen *splash = new SplashScreen(this);
		splash->Show(true);
		splash->Update();
	}
	else
#endif

	wxSafeYield();

	// Set autosave timer
	StartupLog(_T("Set up Auto Save"));
	AutoSave.SetOwner(this,AutoSave_Timer);
	int time = Options.AsInt(_T("Auto save every seconds"));
	if (time > 0) {
		AutoSave.Start(time*1000);
	}

	// Set accelerator keys
	StartupLog(_T("Install hotkeys"));
	PreviousFocus = NULL;
	SetAccelerators();

	// Set drop target
	StartupLog(_T("Set up drag/drop target"));
	SetDropTarget(new AegisubFileDropTarget(this));

	// Parse arguments
	StartupLog(_T("Initialize empty file"));
	LoadSubtitles(_T(""));
	StartupLog(_T("Load files specified on command line"));
	LoadList(args);

	// Version checker
	// Fails on non-Windows platforms with a crash
	StartupLog(_T("Possibly perform automatic updates check"));
	int option = Options.AsInt(_T("Auto check for updates"));
	if (option == -1) {
		int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"),wxYES_NO);
		option = (result == wxYES);
		Options.SetInt(_T("Auto check for updates"),option);
		Options.Save();
	}
	PerformVersionCheck(false);

	//ShowFullScreen(true,wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
	StartupLog(_T("Leaving FrameMain constructor"));
}


////////////////////////
// FrameMain destructor
FrameMain::~FrameMain () {
	DeInitContents();
#ifdef WITH_AUTOMATION
	delete local_scripts;
#endif
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
	ZoomBox = new wxComboBox(Toolbar,Toolbar_Zoom_Dropdown,_T("75%"),wxDefaultPosition,wxDefaultSize,choices,wxCB_READONLY);
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
	Toolbar->AddTool(Menu_Tools_Styles_Manager,_("Styles Manager"),wxBITMAP(style_toolbutton),_("Open Styles Manager"));
	Toolbar->AddTool(Menu_Tools_Properties,_("Properties"),wxBITMAP(properties_toolbutton),_("Open Properties"));
	Toolbar->AddTool(Menu_Tools_Attachments,_("Attachments"),wxBITMAP(attach_button),_("Open Attachment List"));
	Toolbar->AddTool(Menu_Tools_Fonts_Collector,_("Fonts Collector"),wxBITMAP(font_collector_button),_("Open Fonts Collector"));
	Toolbar->AddSeparator();

	// Automation
#ifdef WITH_AUTOMATION
	Toolbar->AddTool(Menu_Tools_Automation,_("Automation"),wxBITMAP(automation_toolbutton),_("Open Automation manager"));
	Toolbar->AddSeparator();
#endif

	// Tools
	if (HasASSDraw()) {
		Toolbar->AddTool(Menu_Tools_ASSDraw,_T("ASSDraw3"),wxBITMAP(assdraw),_("Launches ai-chan's \"ASSDraw3\" tool for vector drawing."));
		Toolbar->AddSeparator();
	}
	Toolbar->AddTool(Menu_Edit_Shift,_("Shift Times"),wxBITMAP(shift_times_toolbutton),_("Open Shift Times Dialogue"));
	Toolbar->AddTool(Menu_Tools_Styling,_("Styling Assistant"),wxBITMAP(styling_toolbutton),_("Open Styling Assistant"));
	Toolbar->AddTool(Menu_Tools_Translation,_("Translation Assistant"),wxBITMAP(translation_toolbutton),_("Open Translation Assistant"));
	Toolbar->AddTool(Menu_Tools_Resample,_("Resample"),wxBITMAP(resample_toolbutton),_("Resample Script Resolution"));
	Toolbar->AddTool(Menu_Tools_Timing_Processor,_("Timing Post-Processor"),wxBITMAP(timing_processor_toolbutton),_("Open Timing Post-processor dialog"));
	Toolbar->AddTool(Menu_Tools_Kanji_Timer,_("Kanji Timer"),wxBITMAP(kanji_timer_button),_("Open Kanji Timer dialog"));
	Toolbar->AddTool(Menu_Tools_SpellCheck,_("Spell Checker"),wxBITMAP(spellcheck_toolbutton),_("Open Spell checker"));
	Toolbar->AddSeparator();

	// Options
	Toolbar->AddTool(Menu_Tools_Options,_("Options"),wxBITMAP(options_button),_("Configure Aegisub"));
	Toolbar->AddTool(Grid_Toggle_Tags,_("Cycle Tag Hidding Mode"),wxBITMAP(toggle_tag_hiding),_("Cycle through tag-hiding modes"));

	// Update
	Toolbar->Realize();
}


wxString MakeHotkeyText(const wxString &item_text, const wxString &hotkey_name) {
	return item_text + wxString(_T("\t")) + Hotkeys.GetText(hotkey_name);
 }

///////////////////////
// Initialize menu bar
void FrameMain::InitMenu() {
	// Deinit menu if needed
	if (menuCreated) {
		SetMenuBar(NULL);
		MenuBar->Destroy();
	}
	
#ifdef __WXMAC__
	// Make sure special menu items are placed correctly on Mac
	wxApp::s_macAboutMenuItemId = Menu_Help_About;
	wxApp::s_macExitMenuItemId = Menu_File_Exit;
	wxApp::s_macPreferencesMenuItemId = Menu_Tools_Options;
	wxApp::s_macHelpMenuTitleName = _("&Help");
#endif

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
	AppendBitmapMenuItem(fileMenu,Menu_File_New_Subtitles, MakeHotkeyText(_("&New Subtitles"), _T("New Subtitles")), _("New subtitles"),wxBITMAP(new_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Open_Subtitles, MakeHotkeyText(_("&Open Subtitles..."), _T("Open Subtitles")), _("Opens a subtitles file"),wxBITMAP(open_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Open_Subtitles_Charset, _("&Open Subtitles with Charset..."), _("Opens a subtitles file with a specific charset"),wxBITMAP(open_with_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Save_Subtitles, MakeHotkeyText(_("&Save Subtitles"), _T("Save Subtitles")), _("Saves subtitles"),wxBITMAP(save_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Save_Subtitles_As, _("Save Subtitles as..."), _("Saves subtitles with another name"), wxBITMAP(save_as_toolbutton));
	AppendBitmapMenuItem(fileMenu,Menu_File_Export_Subtitles, _("Export Subtitles..."), _("Saves a copy of subtitles with processing applied to it."), wxBITMAP(blank_button));
	wxMenuItem *RecentParent = new wxMenuItem(fileMenu, Menu_File_Recent_Subs_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentSubs);
#ifndef __APPLE__
	RecentParent->SetBitmap(wxBITMAP(blank_button));
#endif
	fileMenu->Append(RecentParent);
	fileMenu->AppendSeparator();
	AppendBitmapMenuItem (fileMenu,Menu_Tools_Properties, _("&Properties..."), _("Open script properties window"),wxBITMAP(properties_toolbutton));
	AppendBitmapMenuItem (fileMenu,Menu_Tools_Attachments, _("&Attachments..."), _("Open the attachment list"), wxBITMAP(attach_button));
	AppendBitmapMenuItem (fileMenu,Menu_Tools_Fonts_Collector, _("&Fonts Collector..."),_("Open fonts collector"), wxBITMAP(font_collector_button));
	fileMenu->AppendSeparator();
#ifndef __APPLE__
	// Doesn't work on Mac, only one instance is ever allowed there from OS side
	AppendBitmapMenuItem(fileMenu,Menu_File_New_Window, _("New Window"), _("Open a new application window"),wxBITMAP(blank_button));
#endif
	AppendBitmapMenuItem(fileMenu,Menu_File_Exit, MakeHotkeyText(_("E&xit"), _T("Exit")), _("Exit the application"),wxBITMAP(exit_button));
	MenuBar->Append(fileMenu, _("&File"));

	// Create Edit menu
	// NOTE: Undo and Redo are actually controlled in frame_main_events, OnMenuOpen(). They will always be the first two items.
	editMenu = new wxMenu();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Undo, MakeHotkeyText(_("&Undo"), _T("Undo")), _("Undoes last action"),wxBITMAP(undo_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Redo, MakeHotkeyText(_("&Redo"), _T("Redo")), _("Redoes last action"),wxBITMAP(redo_button));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Cut, MakeHotkeyText(_("Cut Lines"), _T("Cut")), _("Cut subtitles"), wxBITMAP(cut_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Copy, MakeHotkeyText(_("Copy Lines"), _T("Copy")), _("Copy subtitles"), wxBITMAP(copy_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Paste, MakeHotkeyText(_("Paste Lines"), _T("Paste")), _("Paste subtitles"), wxBITMAP(paste_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Paste_Over, MakeHotkeyText(_("Paste Lines Over..."), _T("Paste Over")) , _("Paste subtitles over others"),wxBITMAP(paste_over_button));
	editMenu->AppendSeparator();
	AppendBitmapMenuItem(editMenu,Menu_Edit_Find, MakeHotkeyText(_("&Find..."), _T("Find")), _("Find words in subtitles"),wxBITMAP(find_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Find_Next, MakeHotkeyText(_("Find Next"), _T("Find Next")), _("Find next match of last word"),wxBITMAP(find_next_button));
	AppendBitmapMenuItem(editMenu,Menu_Edit_Replace, MakeHotkeyText(_("Search and &Replace..."), _T("Replace")) , _("Find and replace words in subtitles"),wxBITMAP(replace_button));
	MenuBar->Append(editMenu, _("&Edit"));

	// Create subtitles menu
	subtitlesMenu = new wxMenu();
	wxMenu *InsertMenu = new wxMenu;
	wxMenuItem *InsertParent = new wxMenuItem(subtitlesMenu,Menu_Subtitles_Insert,_("&Insert Lines"),_T(""),wxITEM_NORMAL,InsertMenu);
#ifndef __APPLE__
	InsertParent->SetBitmap(wxBITMAP(blank_button));
#endif
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Styles_Manager, _("&Styles Manager..."), _("Open styles manager"), wxBITMAP(style_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Styling, _("St&yling Assistant..."), _("Open styling assistant"), wxBITMAP(styling_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Translation, _("&Translation Assistant..."),_("Open translation assistant"), wxBITMAP(translation_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_Resample,_("Resample Resolution..."), _("Changes resolution and modifies subtitles to conform to change"), wxBITMAP(resample_toolbutton));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_SpellCheck, _("Spe&ll Checker..."),_("Open spell checker"), wxBITMAP(spellcheck_toolbutton));
	if (HasASSDraw()) {
		subtitlesMenu->AppendSeparator();
		AppendBitmapMenuItem (subtitlesMenu,Menu_Tools_ASSDraw,_T("ASSDraw3..."),_("Launches ai-chan's \"ASSDraw3\" tool for vector drawing."), wxBITMAP(assdraw));
	}
	subtitlesMenu->AppendSeparator();
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_BEFORE,_("&Before Current"),_("Inserts a line before current"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_AFTER,_("&After Current"),_("Inserts a line after current"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_BEFORE_VIDEO,_("Before Current, at Video Time"),_("Inserts a line before current, starting at video time"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(InsertMenu,MENU_INSERT_AFTER_VIDEO,_("After Current, at Video Time"),_("Inserts a line after current, starting at video time"),wxBITMAP(blank_button));
	subtitlesMenu->Append(InsertParent);
	AppendBitmapMenuItem(subtitlesMenu,MENU_DUPLICATE,MakeHotkeyText(_("&Duplicate Lines"), _T("Grid duplicate rows")),_("Duplicate the selected lines"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_DUPLICATE_NEXT_FRAME,MakeHotkeyText(_("&Duplicate and Shift by 1 Frame"), _T("Grid duplicate and shift one frame")),_("Duplicate lines and shift by one frame"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_DELETE,MakeHotkeyText(_("Delete Lines"), _T("Grid delete rows")),_("Delete currently selected lines"),wxBITMAP(delete_button));
	subtitlesMenu->AppendSeparator();
	wxMenu *JoinMenu = new wxMenu;
	wxMenuItem *JoinParent = new wxMenuItem(subtitlesMenu,Menu_Subtitles_Join,_("Join Lines"),_T(""),wxITEM_NORMAL,JoinMenu);
#ifndef __APPLE__
	JoinParent->SetBitmap(wxBITMAP(blank_button));
#endif
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_CONCAT,_("&Concatenate"),_("Joins selected lines in a single one, concatenating text together"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_REPLACE,_("Keep &First"),_("Joins selected lines in a single one, keeping text of first and discarding remaining"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(JoinMenu,MENU_JOIN_AS_KARAOKE,_("As &Karaoke"),_("Joins selected lines in a single one, as karaoke"),wxBITMAP(blank_button));
	subtitlesMenu->Append(JoinParent);
	AppendBitmapMenuItem(subtitlesMenu,MENU_RECOMBINE,_("Recombine Lines"),_("Recombine subtitles when they have been split and merged"),wxBITMAP(blank_button));
	AppendBitmapMenuItem(subtitlesMenu,MENU_SPLIT_BY_KARAOKE,_("Split Lines (by karaoke)"),_("Uses karaoke timing to split line into multiple smaller lines"),wxBITMAP(blank_button));
	subtitlesMenu->AppendSeparator();
	AppendBitmapMenuItem(subtitlesMenu,MENU_SWAP,_("Swap Lines"),_("Swaps the two selected lines"),wxBITMAP(arrow_sort));
	AppendBitmapMenuItem (subtitlesMenu,Menu_Edit_Select, MakeHotkeyText(_("Select Lines..."), _T("Select lines")), _("Selects lines based on defined criterea"),wxBITMAP(select_lines_button));
	MenuBar->Append(subtitlesMenu, _("&Subtitles"));

	// Create timing menu
	timingMenu = new wxMenu();
	AppendBitmapMenuItem(timingMenu,Menu_Edit_Shift, MakeHotkeyText(_("S&hift Times..."), _T("Shift times")), _("Shift subtitles by time or frames"),wxBITMAP(shift_times_toolbutton));
	AppendBitmapMenuItem(timingMenu,Menu_Edit_Sort, _("Sort by Time"), _("Sort all subtitles by their start times"),wxBITMAP(sort_times_button));
	AppendBitmapMenuItem(timingMenu,Menu_Tools_Timing_Processor,_("Timing Post-Processor..."), _("Runs a post-processor for timing to deal with lead-ins, lead-outs, scene timing and etc."), wxBITMAP(timing_processor_toolbutton));
	AppendBitmapMenuItem (timingMenu,Menu_Tools_Kanji_Timer,_("Kanji Timer..."),_("Open Kanji timer"),wxBITMAP(kanji_timer_button));
	timingMenu->AppendSeparator();
	AppendBitmapMenuItem(timingMenu,Menu_Subs_Snap_Start_To_Video, MakeHotkeyText(_("Snap Start to Video"), _T("Set Start To Video")), _("Set start of selected subtitles to current video frame"), wxBITMAP(substart_to_video));
	AppendBitmapMenuItem(timingMenu,Menu_Subs_Snap_End_To_Video, MakeHotkeyText(_("Snap End to Video"), _T("Set End to Video")), _("Set end of selected subtitles to current video frame"), wxBITMAP(subend_to_video));
	AppendBitmapMenuItem(timingMenu,Menu_Video_Snap_To_Scene, MakeHotkeyText(_("Snap to Scene"), _T("Snap to Scene")), _("Set start and end of subtitles to the keyframes around current video frame"), wxBITMAP(snap_subs_to_scene));
	AppendBitmapMenuItem(timingMenu,Menu_Video_Shift_To_Frame, MakeHotkeyText(_("Shift to Current Frame"), _T("Shift by Current Time")), _("Shift selection so first selected line starts at current frame"), wxBITMAP(shift_to_frame));
	timingMenu->AppendSeparator();
	wxMenu *ContinuousMenu = new wxMenu;
	wxMenuItem *ContinuousParent = new wxMenuItem(subtitlesMenu,-1,_("Make Times Continuous"),_T(""),wxITEM_NORMAL,ContinuousMenu);
#ifndef __APPLE__
	ContinuousParent->SetBitmap(wxBITMAP(blank_button));
#endif
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
	videoMenu->Append(Menu_Video_Dummy, _("Use Dummy Video..."), _("Opens a video clip with solid colour"));
	videoMenu->Append(Menu_Video_Details, _("Show Video Details..."), _("Shows video details"));
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_File_Open_VFR, _("Open Timecodes File..."), _("Opens a VFR timecodes v1 or v2 file"));
	videoMenu->Append(Menu_File_Save_VFR, _("Save Timecodes File..."), _("Saves a VFR timecodes v2 file"));
	videoMenu->Append(Menu_File_Close_VFR, _("Close Timecodes File"), _("Closes the currently open timecodes file"))->Enable(false);
	wxMenuItem *RecentTimesParent = new wxMenuItem(videoMenu, Menu_File_Recent_Timecodes_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentTimecodes);
	videoMenu->Append(RecentTimesParent);
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_Video_Load_Keyframes, _("Open Keyframes..."), _("Opens a keyframe list file"));
	videoMenu->Append(Menu_Video_Save_Keyframes, _("Save Keyframes..."), _("Saves the current keyframe list"))->Enable(false);
	videoMenu->Append(Menu_Video_Close_Keyframes, _("Close Keyframes"), _("Closes the currently open keyframes list"))->Enable(false);
	wxMenuItem *RecentKeyframesParent = new wxMenuItem(videoMenu, Menu_File_Recent_Keyframes_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentKeyframes);
	videoMenu->Append(RecentKeyframesParent);
	videoMenu->AppendSeparator();
	videoMenu->Append(Menu_Video_Detach, _("Detach Video"), _("Detach video, displaying it in a separate Window."));
	wxMenu *ZoomMenu = new wxMenu;
	wxMenuItem *ZoomParent = new wxMenuItem(subtitlesMenu,Menu_View_Zoom,_("Set Zoom"),_T(""),wxITEM_NORMAL,ZoomMenu);
#ifndef __APPLE__
	ZoomParent->SetBitmap(wxBITMAP(blank_button));
#endif
	ZoomMenu->Append(Menu_View_Zoom_50, MakeHotkeyText(_T("&50%"), _T("Zoom 50%")), _("Set zoom to 50%"));
	ZoomMenu->Append(Menu_View_Zoom_100, MakeHotkeyText(_T("&100%"), _T("Zoom 100%")), _("Set zoom to 100%"));
	ZoomMenu->Append(Menu_View_Zoom_200, MakeHotkeyText(_T("&200%"), _T("Zoom 200%")), _("Set zoom to 200%"));
	videoMenu->Append(ZoomParent);
	wxMenu *AspectMenu = new wxMenu;
	wxMenuItem *AspectParent = new wxMenuItem(subtitlesMenu,Menu_Video_AR,_("Override Aspect Ratio"),_T(""),wxITEM_NORMAL,AspectMenu);
#ifndef __APPLE__
	AspectParent->SetBitmap(wxBITMAP(blank_button));
#endif
	AspectMenu->AppendCheckItem(Menu_Video_AR_Default, _("&Default"), _("Leave video on original aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Full, _("&Fullscreen (4:3)"), _("Forces video to 4:3 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Wide, _("&Widescreen (16:9)"), _("Forces video to 16:9 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_235, _("&Cinematic (2.35)"), _("Forces video to 2.35 aspect ratio"));
	AspectMenu->AppendCheckItem(Menu_Video_AR_Custom, _("Custom..."), _("Forces video to a custom aspect ratio"));
	videoMenu->Append(AspectParent);
	videoMenu->AppendCheckItem(Menu_Video_Overscan, _("Show Overscan Mask"), _("Show a mask over the video, indicating areas that might get cropped off by overscan on televisions."));
	videoMenu->AppendSeparator();
	AppendBitmapMenuItem(videoMenu,Menu_Video_JumpTo, MakeHotkeyText(_("&Jump to..."), _T("Video Jump")), _("Jump to frame or time"), wxBITMAP(jumpto_button));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_Start, MakeHotkeyText(_("Jump Video to Start"), _T("Jump Video To Start")), _("Jumps the video to the start frame of current subtitle"), wxBITMAP(video_to_substart));
	AppendBitmapMenuItem(videoMenu,Menu_Subs_Snap_Video_To_End, MakeHotkeyText(_("Jump Video to End"), _T("Jump Video To End")), _("Jumps the video to the end frame of current subtitle"), wxBITMAP(video_to_subend));
	MenuBar->Append(videoMenu, _("&Video"));

	// Create audio menu
	audioMenu = new wxMenu();
	audioMenu->Append(Menu_Audio_Open_File, _("&Open Audio File..."), _("Opens an audio file"));
	audioMenu->Append(Menu_Audio_Open_From_Video, _("Open Audio from &Video"), _("Opens the audio from the current video file"));
	audioMenu->Append(Menu_Audio_Close, _("&Close Audio"), _("Closes the currently open audio file"));
	wxMenuItem *RecentAudParent = new wxMenuItem(audioMenu, Menu_File_Recent_Auds_Parent, _("Recent"), _T(""), wxITEM_NORMAL, RecentAuds);
	audioMenu->Append(RecentAudParent);
#ifdef _DEBUG
	audioMenu->AppendSeparator();
	audioMenu->Append(Menu_Audio_Open_Dummy, _T("Open 2h30 Blank Audio"), _T("Open a 150 minutes blank audio clip, for debugging"));
	audioMenu->Append(Menu_Audio_Open_Dummy_Noise, _T("Open 2h30 Noise Audio"), _T("Open a 150 minutes noise-filled audio clip, for debugging"));
#endif
	MenuBar->Append(audioMenu, _("&Audio"));

	// Create Automation menu
#ifdef WITH_AUTOMATION
	automationMenu = new wxMenu();
	AppendBitmapMenuItem (automationMenu,Menu_Tools_Automation, _("&Automation..."),_("Open automation manager"), wxBITMAP(automation_toolbutton));
	automationMenu->AppendSeparator();
	MenuBar->Append(automationMenu, _("&Automation"));
#endif

	// Create view menu
	viewMenu = new wxMenu();
	AppendBitmapMenuItem(viewMenu,Menu_View_Language, _T("&Language..."), _("Select Aegisub interface language"), wxBITMAP(blank_button));
	AppendBitmapMenuItem(viewMenu,Menu_Tools_Options, MakeHotkeyText(_("&Options..."), _T("Options")), _("Configure Aegisub"), wxBITMAP(options_button));
#ifdef WIN32
	if (!Options.AsBool(_T("Local config")))
		AppendBitmapMenuItem(viewMenu,Menu_View_Associations, _("&Associations..."), _("Associate file types with Aegisub"), wxBITMAP(blank_button));
#endif
#ifdef __WXDEBUG__
	AppendBitmapMenuItem(viewMenu,Menu_Tools_Log, _("Lo&g Window..."), _("Open log window"), wxBITMAP(blank_button));
#endif
	viewMenu->AppendSeparator();
	viewMenu->AppendRadioItem(Menu_View_Subs, _("Subs Only View"), _("Display subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Video, _("Video+Subs View"), _("Display video and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Audio, _("Audio+Subs View"), _("Display audio and subtitles only"));
	viewMenu->AppendRadioItem(Menu_View_Standard, _("Full view"), _("Display audio, video and subtitles"));
	MenuBar->Append(viewMenu, _("Vie&w"));

	// Create help menu
	helpMenu = new wxMenu();
	AppendBitmapMenuItem (helpMenu,Menu_Help_Contents, MakeHotkeyText(_("&Contents..."), _T("Help")), _("Help topics"), wxBITMAP(contents_button));
#ifdef __WXMAC__
	AppendBitmapMenuItem(helpMenu,Menu_Help_Files, _("&Resource Files..."), _("Resource files distributed with Aegisub"),wxBITMAP(contents_button));
#endif
	helpMenu->AppendSeparator();
	AppendBitmapMenuItem(helpMenu,Menu_Help_Website, _("&Website..."), _("Visit Aegisub's official website"),wxBITMAP(website_button));
	AppendBitmapMenuItem(helpMenu,Menu_Help_Forums, _("&Forums..."), _("Visit Aegisub's forums"),wxBITMAP(forums_button));
	AppendBitmapMenuItem(helpMenu,Menu_Help_BugTracker, _("&Bug Tracker..."), _("Visit Aegisub's bug tracker to report bugs and request new features"),wxBITMAP(bugtracker_button));
	AppendBitmapMenuItem (helpMenu,Menu_Help_IRCChannel, _("&IRC Channel..."), _("Visit Aegisub's official IRC channel"), wxBITMAP(irc_button));
#ifndef __WXMAC__
	helpMenu->AppendSeparator();
#endif
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
	StartupLog(_T("Create background panel"));
	Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	// Initialize sizers
	StartupLog(_T("Create main sizers"));
	MainSizer = new wxBoxSizer(wxVERTICAL);
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	BottomSizer = new wxBoxSizer(wxHORIZONTAL);

	// Video area;
	StartupLog(_T("Create video box"));
	videoBox = new VideoBox(Panel, false);
	TopSizer->Add(videoBox,0,wxEXPAND,0);
	videoBox->videoDisplay->zoomBox = ZoomBox;

	// Subtitles area
	StartupLog(_T("Create subtitles grid"));
	SubsBox = new SubtitlesGrid(this,Panel,-1,wxDefaultPosition,wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,_T("Subs grid"));
	BottomSizer->Add(SubsBox,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,0);
	StartupLog(_T("Reset undo stack"));
	AssFile::StackReset();
	videoBox->videoSlider->grid = SubsBox;
	VideoContext::Get()->grid = SubsBox;
	StartupLog(_T("Reset video zoom"));
	videoBox->videoDisplay->SetZoomPos(Options.AsInt(_T("Video Default Zoom")));
	Search.grid = SubsBox;

	// Audio area
	StartupLog(_T("Create audio box"));
	audioBox = new AudioBox(Panel);
	audioBox->frameMain = this;
	VideoContext::Get()->audio = audioBox->audioDisplay;

	// Top sizer
	StartupLog(_T("Create subtitle editing box"));
	EditBox = new SubsEditBox(Panel,SubsBox);
	EditBox->audio = audioBox->audioDisplay;
	StartupLog(_T("Arrange controls in sizers"));
	ToolSizer = new wxBoxSizer(wxVERTICAL);
	ToolSizer->Add(audioBox,0,wxEXPAND | wxBOTTOM,5);
	ToolSizer->Add(EditBox,1,wxEXPAND,5);
	TopSizer->Add(ToolSizer,1,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);

	// Set sizers/hints
	StartupLog(_T("Arrange main sizers"));
	MainSizer->Add(new wxStaticLine(Panel),0,wxEXPAND | wxALL,0);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,0);
	MainSizer->Add(BottomSizer,1,wxEXPAND | wxALL,0);
	Panel->SetSizer(MainSizer);
	//MainSizer->SetSizeHints(Panel);
	//SetSizer(MainSizer);

	// Set display
	StartupLog(_T("Set display mode"));
	SetDisplayMode(0,0);
	StartupLog(_T("Perform layout"));
	Layout();
	StartupLog(_T("Set focus to edting box"));
	EditBox->TextEdit->SetFocus();
	StartupLog(_T("Leaving InitContents"));
}


/////////////////////////
// Deinitialize controls
void FrameMain::DeInitContents() {
	if (detachedVideo) detachedVideo->Destroy();
	if (stylingAssistant) stylingAssistant->Destroy();
	AssFile::StackReset();
	delete AssFile::top;
	delete EditBox;
	delete videoBox;
	HelpButton::ClearPages();
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
	bool changed = false;
	changed |= toolbar->FindById(Menu_Video_JumpTo)->Enable(isVideo);
	changed |= toolbar->FindById(Menu_Video_Zoom_In)->Enable(isVideo);
	changed |= toolbar->FindById(Menu_Video_Zoom_Out)->Enable(isVideo);
	changed |= ZoomBox->Enable(isVideo);
	changed |= toolbar->FindById(Menu_Subs_Snap_Start_To_Video)->Enable(isVideo && selRows > 0);
	changed |= toolbar->FindById(Menu_Subs_Snap_End_To_Video)->Enable(isVideo && selRows > 0);
	changed |= toolbar->FindById(Menu_Subs_Snap_Video_To_Start)->Enable(isVideo && selRows == 1);
	changed |= toolbar->FindById(Menu_Subs_Snap_Video_To_End)->Enable(isVideo && selRows == 1);
	changed |= toolbar->FindById(Menu_Video_Select_Visible)->Enable(isVideo);
	changed |= toolbar->FindById(Menu_Video_Snap_To_Scene)->Enable(isVideo && selRows > 0);
	changed |= toolbar->FindById(Menu_Video_Shift_To_Frame)->Enable(isVideo && selRows > 0);

	if (changed)
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
	bool isBinary = false;

	// Load
	try {
		// File exists?
		if (isFile) {
			wxFileName fileCheck(filename);
			if (!fileCheck.FileExists()) throw _T("Selected file does not exist.");

			// Make sure that file isn't actually a timecode file
			TextFileReader testSubs(filename,charset);
			charset = testSubs.GetCurrentEncoding();
			isBinary = charset == _T("binary");
			if (!isBinary && testSubs.HasMoreLines()) {
				wxString cur = testSubs.ReadLineFromFile();
				if (cur.Left(10) == _T("# timecode")) {
					LoadVFR(filename);
					Options.SetText(_T("Last open timecodes path"), fileCheck.GetPath());
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
			wxFileName fn(filename);
			StandardPaths::SetPathValue(_T("?script"),fn.GetPath());
			Options.SetText(_T("Last open subtitles path"), fn.GetPath());
		}
		else {
			SubsBox->LoadDefault(AssFile::top);
			StandardPaths::SetPathValue(_T("?script"),_T(""));
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
	if (!isBinary && Options.AsBool(_T("Auto backup")) && origfile.FileExists()) {
		// Get path
		wxString path = Options.AsText(_T("Auto backup path"));
		if (path.IsEmpty()) path = origfile.GetPath();
		wxFileName dstpath(path);
		if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePathMaybeRelative(path, _T("?user/"));
		path += _T("/");
		dstpath.Assign(path);
		if (!dstpath.DirExists()) wxMkdir(path);

		// Save
		wxString backup = path + origfile.GetName() + _T(".ORIGINAL.") + origfile.GetExt();
		wxCopyFile(filename,backup,true);
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

	// Automatic
	if (_showVid == -1) _showVid = (VideoContext::Get()->IsLoaded() && !detachedVideo) ? 1 : 0;
	else if (_showVid == -2) _showVid = showVideo?1:0;
	if (_showAudio == -1) _showAudio = audioBox->loaded ? 1 : 0;
	else if (_showAudio == -2) _showAudio = showAudio?1:0;

	// See if anything changed
	if (_showVid == (showVideo?1:0) && _showAudio == (showAudio?1:0)) return;
	showAudio = _showAudio == 1;
	showVideo = _showVid == 1;

	// Stop
	Freeze();
	VideoContext::Get()->Stop();

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
	if (showVideo) VideoContext::Get()->UpdateDisplays(true);
	Thaw();
}


////////////////////
// Update title bar
void FrameMain::UpdateTitle() {
	// Determine if current subs are modified
	bool subsMod = AssFile::top->IsModified();
	
	// Create ideal title
	wxString newTitle = _T("");
#ifndef __WXMAC__
	if (subsMod) newTitle << _T("* ");
	if (AssFile::top->filename != _T("")) {
		wxFileName file (AssFile::top->filename);
		newTitle << file.GetFullName();
	}
	else newTitle << _("Untitled");
	newTitle << _T(" - Aegisub ") << GetAegisubLongVersionString();
#else
	// Apple HIG says "untitled" should not be capitalised
	// and the window is a document window, it shouldn't contain the app name
	// (The app name is already present in the menu bar)
	if (AssFile::top->filename != _T("")) {
		wxFileName file (AssFile::top->filename);
		newTitle << file.GetFullName();
	}
	else newTitle << _("untitled");
#endif

#ifdef __WXMAC__
	// On Mac, set the mark in the close button
	WindowRef wnd = (WindowRef)GetHandle();
	SetWindowModified(wnd, subsMod);
#endif

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
			curSubsKeyframes != VideoContext::Get()->GetKeyFramesName()
#ifdef WITH_AUTOMATION
			|| !AutoScriptString.IsEmpty() || local_scripts->GetScripts().size() > 0
#endif
			) {
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
				//if (curSubsVideo != _T("")) {
				LoadVideo(curSubsVideo);
				if (VideoContext::Get()->IsLoaded()) {
					VideoContext::Get()->SetAspectRatio(videoAr,videoArValue);
					videoBox->videoDisplay->SetZoomPos(videoZoom-1);
					VideoContext::Get()->JumpToFrame(videoPos);
				}
				//}
			}

			// Keyframes
			if (curSubsKeyframes != _T("")) {
				KeyFrameFile::Load(curSubsKeyframes);
			}

			// Audio
			if (curSubsAudio != audioBox->audioName) {
				if (curSubsAudio == _T("?video")) LoadAudio(_T(""),true);
				else LoadAudio(curSubsAudio);
			}

			// Automation scripts
#ifdef WITH_AUTOMATION
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
					local_scripts->Add(Automation4::ScriptFactory::CreateFromFile(sfnames, true));
				} else {
					wxLogWarning(_T("Automation Script referenced could not be found.\nFilename specified: %s%s\nSearched relative to: %s\nResolved filename: %s"),
						sfnamel.c_str(), sfnames.c_str(), basepath.c_str(), sfname.GetFullPath().c_str());
				}
			}
#endif
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
			if (arType == 4) ar = wxString(_T("c")) + AegiFloatToString(VideoContext::Get()->GetAspectRatioValue());
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
#ifdef WITH_AUTOMATION
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
#endif
	}
}


///////////////
// Loads video
void FrameMain::LoadVideo(wxString file,bool autoload) {
	if (blockVideoLoad) return;
	Freeze();
	VideoContext::Get()->Stop();
	try {
		if (VideoContext::Get()->IsLoaded()) {
			if (VFR_Output.GetFrameRateType() == VFR) {
				if (!autoload) {
					int result = wxMessageBox(_("You have timecodes loaded currently. Would you like to unload them?"), _("Unload timecodes?"), wxYES_NO, this);
					if (result == wxYES) {
						VFR_Output.Unload();
					}
				}
			}
			else {
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

	if (VideoContext::Get()->IsLoaded()) {
		int vidx = VideoContext::Get()->GetWidth(), vidy = VideoContext::Get()->GetHeight();

		// Set zoom level based on video resolution and window size
		int target_zoom = 7; // 100%
		wxSize windowSize = GetSize();
		if (vidx*3 > windowSize.GetX()*2 || vidy*4 > windowSize.GetY()*3)
			target_zoom = 3; // 50%
		if (vidx*3 > windowSize.GetX()*4 || vidy*4 > windowSize.GetY()*6)
			target_zoom = 1; // 25%
		videoBox->videoDisplay->zoomBox->SetSelection(target_zoom);
		videoBox->videoDisplay->SetZoomPos(target_zoom);

		// Check that the video size matches the script video size specified
		int scriptx = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResX"));
		int scripty = SubsBox->ass->GetScriptInfoAsInt(_T("PlayResY"));
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

	DetachVideo(VideoContext::Get()->IsLoaded() && Options.AsBool(_T("Detached Video")));
	Thaw();
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
	#ifdef WITH_AVISYNTH
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


/////////////
// Saves VFR
void FrameMain::SaveVFR(wxString filename) {
	VFR_Output.Save(filename);
}


/////////////
// Open help
void FrameMain::OpenHelp(wxString page) {
	HelpButton::OpenPage(_T("Main"));
}


///////////////////////
// Detach video window
void FrameMain::DetachVideo(bool detach) {
	if (detach) {
		if (!detachedVideo) {
			detachedVideo = new DialogDetachedVideo(this, videoBox->videoDisplay->GetClientSize());
			detachedVideo->Show();
			VideoContext::Get()->UpdateDisplays(true);
		}
	}
	else {
		if (detachedVideo) {
			detachedVideo->Destroy();
			SetDisplayMode(-1,-1);
			detachedVideo = NULL;
		}
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
	std::vector<wxAcceleratorEntry> entry;
	entry.reserve(32);

	// Standard
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global prev frame"),Video_Prev_Frame));
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global next frame"),Video_Next_Frame));
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global focus seek"),Video_Focus_Seek));
	entry.push_back(Hotkeys.GetAccelerator(_T("Grid global prev line"),Grid_Prev_Line));
	entry.push_back(Hotkeys.GetAccelerator(_T("Grid global next line"),Grid_Next_Line));
	entry.push_back(Hotkeys.GetAccelerator(_T("Save Subtitles Alt"),Menu_File_Save_Subtitles));
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global zoom in"),Menu_Video_Zoom_In));
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global zoom out"),Menu_Video_Zoom_Out));
	entry.push_back(Hotkeys.GetAccelerator(_T("Video global play"),Video_Frame_Play));
	entry.push_back(Hotkeys.GetAccelerator(_T("Edit box commit"),Edit_Box_Commit));

	// Medusa
	bool medusaPlay = Options.AsBool(_T("Audio Medusa Timing Hotkeys"));
	if (medusaPlay && audioBox->audioDisplay->loaded) {
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Play"),Medusa_Play));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Stop"),Medusa_Stop));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Play Before"),Medusa_Play_Before));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Play After"),Medusa_Play_After));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Next"),Medusa_Next));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Previous"),Medusa_Prev));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Shift Start Forward"),Medusa_Shift_Start_Forward));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Shift Start Back"),Medusa_Shift_Start_Back));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Shift End Forward"),Medusa_Shift_End_Forward));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Shift End Back"),Medusa_Shift_End_Back));
		entry.push_back(Hotkeys.GetAccelerator(_T("Audio Medusa Enter"),Medusa_Enter));
	}

	// Set table
	wxAcceleratorTable table(entry.size(),&entry[0]);
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
	subsList.Add(_T("sub"));
	subsList.Add(_T("txt"));
	subsList.Add(_T("ttxt"));

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
	audioList.Add(_T("m4a"));

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


/////////////////////////////////
// Check if ASSDraw is available
bool FrameMain::HasASSDraw() {
#ifdef __WINDOWS__
	wxFileName fn(StandardPaths::DecodePath(_T("?data/ASSDraw3.exe")));
	return fn.FileExists();
#else
	return false;
#endif
}
