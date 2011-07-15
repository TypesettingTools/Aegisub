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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file frame_main.cpp
/// @brief Main window creation and control management
/// @ingroup main_ui

#include "config.h"

#include "frame_main.h"

#ifndef AGI_PRE
#include <wx/clipbrd.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/mimetype.h>
#include <wx/statline.h>
#include <wx/sysopt.h>
#include <wx/tokenzr.h>
#endif

#include <libaegisub/access.h>
#include <libaegisub/log.h>

#include "include/aegisub/context.h"
#include "include/aegisub/menu.h"
#include "include/aegisub/toolbar.h"
#include "include/aegisub/hotkey.h"

#include "ass_file.h"
#include "audio_controller.h"
#include "audio_box.h"
#ifdef WITH_AUTOMATION
#include "auto4_base.h"
#endif
#include "compat.h"
#include "command/command.h"
#include "dialog_detached_video.h"
#include "dialog_search_replace.h"
#include "dialog_styling_assistant.h"
#include "dialog_version_check.h"
#include "drop.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "standard_paths.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "text_file_reader.h"
#include "utils.h"
#include "version.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"
#include "video_slider.h"

enum {
	ID_TOOLBAR_ZOOM_DROPDOWN				= 11001,
	ID_APP_TIMER_AUTOSAVE					= 12001,
	ID_APP_TIMER_STATUSCLEAR				= 12002,
	ID_MENU_AUTOMATION_MACRO				= 13006,
	ID_SASH_MAIN_AUDIO						= 14001
};

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, a, "Aegisub startup log", 0)
#else
#define StartupLog(a)
#endif

static void autosave_timer_changed(wxTimer *timer, const agi::OptionValue &opt);

FrameMain::FrameMain (wxArrayString args)
: wxFrame(0,-1,"",wxDefaultPosition,wxSize(920,700),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
, context(new agi::Context)
, showVideo(true)
, showAudio(true)
, blockVideoLoad(false)
{
	StartupLog("Entering FrameMain constructor");

#ifdef __WXGTK__
/* XXX HACK XXX
 * Gtk just got initialized. And if we're using the SCIM IME,
 * it just did a setlocale(LC_ALL, ""). so, BOOM.
 */
	StartupLog("Setting locale");
 	setlocale(LC_ALL, "");
	setlocale(LC_CTYPE, "C");
	setlocale(LC_NUMERIC, "C");
/* XXX HACK XXX */
#endif

	StartupLog("Initializing context models");
	AssFile::top = context->ass = new AssFile;
	context->ass->AddCommitListener(&FrameMain::UpdateTitle, this);
	context->ass->AddFileOpenListener(&FrameMain::OnSubtitlesOpen, this);
	context->ass->AddFileSaveListener(&FrameMain::UpdateTitle, this);

#ifdef WITH_AUTOMATION
	context->local_scripts = new Automation4::ScriptManager();
#endif

	StartupLog("Initializing context controls");
	context->audioController = new AudioController;
	context->audioController->AddAudioOpenListener(&FrameMain::OnAudioOpen, this);
	context->audioController->AddAudioCloseListener(&FrameMain::OnAudioClose, this);

	// Initialized later due to that the selection controller is currently the subtitles grid
	context->selectionController = 0;

	context->videoController = VideoContext::Get(); // derp
	context->videoController->AddVideoOpenListener(&FrameMain::OnVideoOpen, this);

	StartupLog("Initializing context frames");
	context->parent = this;
	context->previousFocus = 0;
	AegisubApp::Get()->frame = this;

	StartupLog("Binding commands");
	// XXX: This is a hack for now, it will need to be dealt with when other frames are involved.
	Bind(wxEVT_COMMAND_MENU_SELECTED, &FrameMain::cmd_call, this);

#ifdef __WXMAC__
//	Bind(FrameMain::OnAbout, &FrameMain::cmd_call, this, cmd::id("app/about"));
#endif

	StartupLog("Install PNG handler");
	wxImage::AddHandler(new wxPNGHandler);
	wxSafeYield();

	StartupLog("Apply saved Maximized state");
	if (OPT_GET("App/Maximized")->GetBool()) Maximize(true);

	StartupLog("Initialize toolbar");
	InitToolbar();

	StartupLog("Initialize menu bar");
	InitMenu();
	
	StartupLog("Create status bar");
	CreateStatusBar(2);

	StartupLog("Set icon");
#ifdef _WIN32
	SetIcon(wxICON(wxicon));
#else
	wxIcon icon;
	icon.CopyFromBitmap(GETIMAGE(wxicon));
	SetIcon(icon);
#endif

	StartupLog("Create views and inner main window controls");
	context->detachedVideo = 0;
	context->stylingAssistant = 0;
	InitContents();

	StartupLog("Complete context initialization");
	context->videoController->SetContext(context.get());

	StartupLog("Set up Auto Save");
	AutoSave.SetOwner(this, ID_APP_TIMER_AUTOSAVE);
	int time = OPT_GET("App/Auto/Save Every Seconds")->GetInt();
	if (time > 0) {
		AutoSave.Start(time*1000);
	}
	OPT_SUB("App/Auto/Save Every Seconds", autosave_timer_changed, &AutoSave, agi::signal::_1);

	StartupLog("Set up drag/drop target");
	SetDropTarget(new AegisubFileDropTarget(this));

	StartupLog("Load default file");
	context->ass->LoadDefault();

	StartupLog("Load files specified on command line");
	LoadList(args);

	// Version checker
	StartupLog("Possibly perform automatic updates check");
	if (OPT_GET("App/First Start")->GetBool()) {
		OPT_SET("App/First Start")->SetBool(false);
		int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"),wxYES_NO);
		OPT_SET("App/Auto/Check For Updates")->SetBool(result == wxYES);
	}

	PerformVersionCheck(false);

	StartupLog("Display main window");
	Show();
	SetDisplayMode(1, 1);

	StartupLog("Leaving FrameMain constructor");
}

FrameMain::~FrameMain () {
	context->videoController->SetVideo("");
	context->audioController->CloseAudio();
	if (context->detachedVideo) context->detachedVideo->Destroy();
	if (context->stylingAssistant) context->stylingAssistant->Destroy();
	SubsGrid->ClearMaps();
	delete audioBox;
	delete EditBox;
	delete videoBox;
	delete context->ass;
	HelpButton::ClearPages();
	delete context->audioController;
#ifdef WITH_AUTOMATION
	delete context->local_scripts;
#endif
}

void FrameMain::cmd_call(wxCommandEvent& event) {
	int id = event.GetId();
	LOG_D("event/select") << "Id: " << id;
	if (id < cmd::count())
		cmd::call(context.get(), id);
	else if (id >= ID_MENU_AUTOMATION_MACRO)
		OnAutomationMacro(event);
}

void FrameMain::InitToolbar () {
	wxSystemOptions::SetOption("msw.remap", 0);
	Toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL,-1,"Toolbar");

	toolbar::toolbar->GetToolbar("main", Toolbar);

	wxArrayString choices;
	for (int i=1;i<=24;i++) {
		wxString toAdd = wxString::Format("%i",int(i*12.5));
		if (i%2) toAdd += ".5";
		toAdd += "%";
		choices.Add(toAdd);
	}
	ZoomBox = new wxComboBox(Toolbar,ID_TOOLBAR_ZOOM_DROPDOWN,"75%",wxDefaultPosition,wxDefaultSize,choices,wxCB_DROPDOWN);
	Toolbar->AddControl(ZoomBox);
	Toolbar->AddSeparator();

	Toolbar->Realize();
}

void FrameMain::InitMenu() {

#ifdef __WXMAC__
	// Make sure special menu items are placed correctly on Mac
//	wxApp::s_macAboutMenuItemId = Menu_Help_About;
//	wxApp::s_macExitMenuItemId = Menu_File_Exit;
//	wxApp::s_macPreferencesMenuItemId = Menu_Tools_Options;
//	wxApp::s_macHelpMenuTitleName = _("&Help");
#endif

	SetMenuBar(menu::menu->GetMainMenu());
}

void FrameMain::InitContents() {
	StartupLog("Create background panel");
	Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	StartupLog("Create video box");
	context->videoBox = videoBox = new VideoBox(Panel, false, ZoomBox, context.get());
	wxBoxSizer *videoSizer = new wxBoxSizer(wxVERTICAL);
	videoSizer->Add(videoBox , 0, wxEXPAND);
	videoSizer->AddStretchSpacer(1);

	StartupLog("Create subtitles grid");
	context->subsGrid = SubsGrid = new SubtitlesGrid(Panel,context.get(),wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,"Subs grid");
	context->selectionController = context->subsGrid;
	Search.context = context.get();

	StartupLog("Create tool area splitter window");
	audioSash = new wxSashWindow(Panel, ID_SASH_MAIN_AUDIO, wxDefaultPosition, wxDefaultSize, wxSW_3D|wxCLIP_CHILDREN);
	wxBoxSizer *audioSashSizer = new wxBoxSizer(wxHORIZONTAL);
	audioSash->SetSashVisible(wxSASH_BOTTOM, true);

	StartupLog("Create audio box");
	context->audioBox = audioBox = new AudioBox(audioSash, context.get());
	audioSashSizer->Add(audioBox, 1, wxEXPAND);
	audioSash->SetSizer(audioSashSizer);
	audioBox->Fit();
	audioSash->SetMinimumSizeY(audioBox->GetSize().GetHeight());

	StartupLog("Create subtitle editing box");
	context->editBox = EditBox = new SubsEditBox(Panel, context.get());

	StartupLog("Arrange main sizers");
	ToolsSizer = new wxBoxSizer(wxVERTICAL);
	ToolsSizer->Add(audioSash, 0, wxEXPAND);
	ToolsSizer->Add(EditBox, 1, wxEXPAND);
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(videoSizer, 0, wxEXPAND, 0);
	TopSizer->Add(ToolsSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(new wxStaticLine(Panel),0,wxEXPAND | wxALL,0);
	MainSizer->Add(TopSizer,0,wxEXPAND | wxALL,0);
	MainSizer->Add(SubsGrid,1,wxEXPAND | wxALL,0);
	Panel->SetSizer(MainSizer);

	StartupLog("Perform layout");
	Layout();
	StartupLog("Set focus to edting box");
	EditBox->TextEdit->SetFocus();
	StartupLog("Leaving InitContents");
}

static void validate_toolbar(wxToolBar *toolbar, const char *command, const agi::Context *context) {
	toolbar->FindById(cmd::id(command))->Enable(cmd::get(command)->Validate(context));
}

void FrameMain::UpdateToolbar() {
	wxToolBar* toolbar = GetToolBar();
	const agi::Context *c = context.get();
	ZoomBox->Enable(context->videoController->IsLoaded() && !context->detachedVideo);

	validate_toolbar(toolbar, "video/jump", c);
	validate_toolbar(toolbar, "video/zoom/in", c);
	validate_toolbar(toolbar, "video/zoom/out", c);

	validate_toolbar(toolbar, "video/jump/start", c);
	validate_toolbar(toolbar, "video/jump/end", c);

	validate_toolbar(toolbar, "time/snap/start_video", c);
	validate_toolbar(toolbar, "time/snap/end_video", c);

	validate_toolbar(toolbar, "subtitle/select/visible", c);
	validate_toolbar(toolbar, "time/snap/scene", c);
	validate_toolbar(toolbar, "time/snap/frame", c);
	toolbar->Realize();
}

void FrameMain::LoadSubtitles(wxString filename,wxString charset) {
	if (context->ass->loaded) {
		if (TryToCloseSubs() == wxCANCEL) return;
	}

	try {
		// Make sure that file isn't actually a timecode file
		try {
			TextFileReader testSubs(filename,charset);
			wxString cur = testSubs.ReadLineFromFile();
			if (cur.Left(10) == "# timecode") {
				context->videoController->LoadTimecodes(filename);
				return;
			}
		}
		catch (...) {
			// if trying to load the file as timecodes fails it's fairly
			// safe to assume that it is in fact not a timecode file
		}

		context->ass->Load(filename,charset);
	}
	catch (agi::acs::AcsNotFound const&) {
		wxMessageBox(filename + " not found.", "Error", wxOK | wxICON_ERROR, NULL);
		config::mru->Remove("Subtitle", STD_STR(filename));
		return;
	}
	catch (const wchar_t *err) {
		wxMessageBox(wxString(err), "Error", wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (wxString err) {
		wxMessageBox(err, "Error", wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (...) {
		wxMessageBox("Unknown error", "Error", wxOK | wxICON_ERROR, NULL);
		return;
	}
}

int FrameMain::TryToCloseSubs(bool enableCancel) {
	if (context->ass->IsModified()) {
		int flags = wxYES_NO;
		if (enableCancel) flags |= wxCANCEL;
		int result = wxMessageBox(_("Save before continuing?"), _("Unsaved changes"), flags,this);
		if (result == wxYES) {
			(*cmd::get("subtitle/save"))(context.get());
			// If it fails saving, return cancel anyway
			return context->ass->IsModified() ? wxCANCEL : wxYES;
		}
		return result;
	}
	else {
		return wxYES;
	}
}

void FrameMain::SetDisplayMode(int video, int audio) {
	if (!IsShownOnScreen()) return;

	bool sv = false, sa = false;

	if (video == -1) sv = showVideo;
	else if (video)  sv = context->videoController->IsLoaded() && !context->detachedVideo;

	if (audio == -1) sa = showAudio;
	else if (audio)  sa = context->audioController->IsAudioOpen();

	// See if anything changed
	if (sv == showVideo && sa == showAudio) return;

	showVideo = sv;
	showAudio = sa;

	bool didFreeze = !IsFrozen();
	if (didFreeze) Freeze();

	context->videoController->Stop();

	TopSizer->Show(videoBox, showVideo, true);
	ToolsSizer->Show(audioSash, showAudio, true);

	UpdateToolbar();
	MainSizer->CalcMin();
	MainSizer->RecalcSizes();
	MainSizer->Layout();
	Layout();

	if (didFreeze) Thaw();
}

void FrameMain::UpdateTitle() {
	wxString newTitle;
	if (context->ass->IsModified()) newTitle << "* ";
	if (context->ass->filename.empty()) {
		// Apple HIG says "untitled" should not be capitalised
		// and the window is a document window, it shouldn't contain the app name
		// (The app name is already present in the menu bar)
#ifndef __WXMAC__
		newTitle << _("Untitled");
#else
		newTitle << _("untitled");
#endif
	}
	else {
		wxFileName file (context->ass->filename);
		newTitle << file.GetFullName();
	}

#ifndef __WXMAC__
	newTitle << " - Aegisub " << GetAegisubLongVersionString();
#endif

#if defined(__WXMAC__) && !defined(__LP64__)
	// On Mac, set the mark in the close button
	OSXSetModified(context->ass->IsModified());
#endif

	if (GetTitle() != newTitle) SetTitle(newTitle);
}

void FrameMain::OnVideoOpen() {
	if (!context->videoController->IsLoaded()) {
		SetDisplayMode(0, -1);
		DetachVideo(false);
		return;
	}

	Freeze();
	int vidx = context->videoController->GetWidth(),
		vidy = context->videoController->GetHeight();

	// Set zoom level based on video resolution and window size
	double zoom = videoBox->videoDisplay->GetZoom();
	wxSize windowSize = GetSize();
	if (vidx*3*zoom > windowSize.GetX()*4 || vidy*4*zoom > windowSize.GetY()*6)
		videoBox->videoDisplay->SetZoom(zoom * .25);
	else if (vidx*3*zoom > windowSize.GetX()*2 || vidy*4*zoom > windowSize.GetY()*3)
		videoBox->videoDisplay->SetZoom(zoom * .5);

	// Check that the video size matches the script video size specified
	int scriptx = context->ass->GetScriptInfoAsInt("PlayResX");
	int scripty = context->ass->GetScriptInfoAsInt("PlayResY");
	if (scriptx != vidx || scripty != vidy) {
		switch (OPT_GET("Video/Check Script Res")->GetInt()) {
			case 1:
				// Ask to change on mismatch
				if (wxMessageBox(wxString::Format(_("The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\nVideo resolution:\t%d x %d\nScript resolution:\t%d x %d\n\nChange subtitles resolution to match video?"), vidx, vidy, scriptx, scripty), _("Resolution mismatch"), wxYES_NO, this) != wxYES)
					break;
				// Fallthrough to case 2
			case 2:
				// Always change script res
				context->ass->SetScriptInfo("PlayResX", wxString::Format("%d", vidx));
				context->ass->SetScriptInfo("PlayResY", wxString::Format("%d", vidy));
				context->ass->Commit(_("Change script resolution"));
				break;
			case 0:
			default:
				// Never change
				break;
		}
	}

	SetDisplayMode(1,-1);

	DetachVideo(OPT_GET("Video/Detached/Enabled")->GetBool());
	Thaw();
}

void FrameMain::LoadVFR(wxString filename) {
	if (filename.empty()) {
		context->videoController->CloseTimecodes();
	}
	else {
		context->videoController->LoadTimecodes(filename);
	}
}

void FrameMain::DetachVideo(bool detach) {
	if (detach) {
		if (!context->detachedVideo) {
			context->detachedVideo = new DialogDetachedVideo(this, context.get(), videoBox->videoDisplay->GetClientSize());
			context->detachedVideo->Show();
		}
	}
	else if (context->detachedVideo) {
		context->detachedVideo->Destroy();
		context->detachedVideo = 0;
		SetDisplayMode(1,-1);
	}
	UpdateToolbar();
}

void FrameMain::StatusTimeout(wxString text,int ms) {
	SetStatusText(text,1);
	StatusClear.SetOwner(this, ID_APP_TIMER_STATUSCLEAR);
	StatusClear.Start(ms,true);
}

bool FrameMain::LoadList(wxArrayString list) {
	wxArrayString List;
	for (size_t i=0;i<list.Count();i++) {
		wxFileName file(list[i]);
		if (file.IsRelative()) file.MakeAbsolute();
		if (file.FileExists()) List.Add(file.GetFullPath());
	}

	// Video formats
	wxArrayString videoList;
	videoList.Add("avi");
	videoList.Add("mkv");
	videoList.Add("mp4");
	videoList.Add("d2v");
	videoList.Add("mpg");
	videoList.Add("mpeg");
	videoList.Add("ogm");
	videoList.Add("avs");
	videoList.Add("wmv");
	videoList.Add("asf");
	videoList.Add("mov");
	videoList.Add("rm");
	videoList.Add("y4m");
	videoList.Add("yuv");

	// Subtitle formats
	wxArrayString subsList;
	subsList.Add("ass");
	subsList.Add("ssa");
	subsList.Add("srt");
	subsList.Add("sub");
	subsList.Add("txt");
	subsList.Add("ttxt");

	// Audio formats
	wxArrayString audioList;
	audioList.Add("wav");
	audioList.Add("mp3");
	audioList.Add("ogg");
	audioList.Add("wma");
	audioList.Add("ac3");
	audioList.Add("aac");
	audioList.Add("mpc");
	audioList.Add("ape");
	audioList.Add("flac");
	audioList.Add("mka");
	audioList.Add("m4a");

	// Scan list
	wxString audio;
	wxString video;
	wxString subs;
	wxString ext;
	for (size_t i=0;i<List.Count();i++) {
		wxFileName file(List[i]);
		ext = file.GetExt().Lower();

		if (subs.empty() && subsList.Index(ext) != wxNOT_FOUND) subs = List[i];
		if (video.empty() && videoList.Index(ext) != wxNOT_FOUND) video = List[i];
		if (audio.empty() && audioList.Index(ext) != wxNOT_FOUND) audio = List[i];
	}

	blockVideoLoad = !video.empty();

	// Load files
	if (subs.size()) {
		LoadSubtitles(subs);
	}
	if (blockVideoLoad) {
		blockVideoLoad = false;
		context->videoController->SetVideo(video);
	}
	if (!audio.empty())
		context->audioController->OpenAudio(audio);

	return subs.size() || audio.size() || video.size();
}

bool FrameMain::HasASSDraw() {
#ifdef __WINDOWS__
	wxFileName fn(StandardPaths::DecodePath("?data/ASSDraw3.exe"));
	return fn.FileExists();
#else
	return false;
#endif
}

static void autosave_timer_changed(wxTimer *timer, const agi::OptionValue &opt) {
	int freq = opt.GetInt();
	if (freq <= 0) {
		timer->Stop();
	}
	else {
		timer->Start(freq * 1000);
	}
}
BEGIN_EVENT_TABLE(FrameMain, wxFrame)
	EVT_TIMER(ID_APP_TIMER_AUTOSAVE, FrameMain::OnAutoSave)
	EVT_TIMER(ID_APP_TIMER_STATUSCLEAR, FrameMain::OnStatusClear)

	EVT_CLOSE(FrameMain::OnCloseWindow)

	EVT_SASH_DRAGGED(ID_SASH_MAIN_AUDIO, FrameMain::OnAudioBoxResize)

	EVT_MENU_OPEN(FrameMain::OnMenuOpen)
	EVT_KEY_DOWN(FrameMain::OnKeyDown)

#ifdef __WXMAC__
//   EVT_MENU(wxID_ABOUT, FrameMain::OnAbout)
//   EVT_MENU(wxID_EXIT, FrameMain::OnExit)
#endif
END_EVENT_TABLE()

void FrameMain::RebuildRecentList(const char *root_command, const char *mru_name) {
	wxMenu *menu = menu::menu->GetMenu(root_command);

	int count = (int)menu->GetMenuItemCount();
	for (int i=count;--i>=0;) {
		menu->Destroy(menu->FindItemByPosition(i));
	}

	const agi::MRUManager::MRUListMap *map_list = config::mru->Get(mru_name);
	if (map_list->empty()) {
		menu->Append(-1, _("Empty"))->Enable(false);
		return;
	}

	int i = 0;
	for (agi::MRUManager::MRUListMap::const_iterator it = map_list->begin(); it != map_list->end(); ++it) {
		std::stringstream ss;
		ss << root_command;
		ss << "/";
		ss << i;

		wxFileName shortname(lagi_wxString(it->second));

		menu->Append(cmd::id(ss.str()),
			wxString::Format("%s%d %s", i <= 9 ? "&" : "", i + 1, shortname.GetFullName()));
		++i;
	}
}

static void validate(wxMenuBar *menu, const agi::Context *c, const char *command) {
	menu->Enable(cmd::id(command), cmd::get(command)->Validate(c));
}

void FrameMain::OnMenuOpen (wxMenuEvent &event) {
	wxMenuBar *MenuBar = menu::menu->GetMainMenu();

	MenuBar->Freeze();
	wxMenu *curMenu = event.GetMenu();

	// File menu
	if (curMenu == menu::menu->GetMenu("main/file")) {
		RebuildRecentList("recent/subtitle", "Subtitle");
		validate(MenuBar, context.get(), "subtitle/open/video");
	}

	// View menu
	else if (curMenu == menu::menu->GetMenu("main/view")) {
		if (!showVideo && !showAudio) MenuBar->Check(cmd::id("app/display/subs"),true);
		else if (showVideo && !showAudio) MenuBar->Check(cmd::id("app/display/video_subs"),true);
		else if (showAudio && showVideo) MenuBar->Check(cmd::id("app/display/full"),true);
		else MenuBar->Check(cmd::id("app/display/audio_subs"),true);

		int sub_grid = OPT_GET("Subtitle/Grid/Hide Overrides")->GetInt();
		if (sub_grid == 1) MenuBar->Check(cmd::id("grid/tags/show"), true);
		if (sub_grid == 2) MenuBar->Check(cmd::id("grid/tags/simplify"), true);
		if (sub_grid == 3) MenuBar->Check(cmd::id("grid/tags/hide"), true);
	}
	// Video menu
	else if (curMenu == menu::menu->GetMenu("main/video")) {
		validate(MenuBar, context.get(), "timecode/save");
		validate(MenuBar, context.get(), "timecode/close");
		validate(MenuBar, context.get(), "keyframe/close");
		validate(MenuBar, context.get(), "keyframe/save");

		int arType = context->videoController->GetAspectRatioType();
		MenuBar->Check(cmd::id("video/aspect/default"),false);
		MenuBar->Check(cmd::id("video/aspect/full"),false);
		MenuBar->Check(cmd::id("video/aspect/wide"),false);
		MenuBar->Check(cmd::id("video/aspect/cinematic"),false);
		MenuBar->Check(cmd::id("video/aspect/custom"),false);
		switch (arType) {
			case 0: MenuBar->Check(cmd::id("video/aspect/default"),true); break;
			case 1: MenuBar->Check(cmd::id("video/aspect/full"),true); break;
			case 2: MenuBar->Check(cmd::id("video/aspect/wide"),true); break;
			case 3: MenuBar->Check(cmd::id("video/aspect/cinematic"),true); break;
			case 4: MenuBar->Check(cmd::id("video/aspect/custom"),true); break;
		}

		MenuBar->Check(cmd::id("video/show_overscan"),OPT_GET("Video/Overscan Mask")->GetBool());

		RebuildRecentList("recent/video", "Video");
		RebuildRecentList("recent/timecode", "Timecodes");
		RebuildRecentList("recent/keyframe", "Keyframes");
	}

	// Audio menu
	else if (curMenu == menu::menu->GetMenu("main/audio")) {
		validate(MenuBar, context.get(), "audio/open/video");
		validate(MenuBar, context.get(), "audio/close");
		RebuildRecentList("recent/audio", "Audio");
	}

	// Subtitles menu
	else if (curMenu == menu::menu->GetMenu("main/subtitle")) {
		validate(MenuBar, context.get(), "main/subtitle/insert lines");
		validate(MenuBar, context.get(), "edit/line/duplicate");
		validate(MenuBar, context.get(), "edit/line/duplicate/shift");
		validate(MenuBar, context.get(), "edit/line/swap");
		validate(MenuBar, context.get(), "edit/line/join/concatenate");
		validate(MenuBar, context.get(), "edit/line/join/keep_first");
		validate(MenuBar, context.get(), "edit/line/join/as_karaoke");
		validate(MenuBar, context.get(), "main/subtitle/join lines");
		validate(MenuBar, context.get(), "edit/line/recombine");
	}

	// Timing menu
	else if (curMenu == menu::menu->GetMenu("main/timing")) {
		validate(MenuBar, context.get(), "time/snap/start_video");
		validate(MenuBar, context.get(), "time/snap/end_video");
		validate(MenuBar, context.get(), "time/snap/scene");
		validate(MenuBar, context.get(), "time/frame/current");

		validate(MenuBar, context.get(), "time/continuous/start");
		validate(MenuBar, context.get(), "time/continuous/end");
	}

	// Edit menu
	else if (curMenu == menu::menu->GetMenu("main/edit")) {
		wxMenu *editMenu = menu::menu->GetMenu("main/edit");

		// Undo state
		wxString undo_text = wxString::Format("%s %s\t%s",
			cmd::get("edit/undo")->StrMenu(),
			context->ass->GetUndoDescription(),
			hotkey::get_hotkey_str_first("Default", "edit/undo"));
		wxMenuItem *item = editMenu->FindItem(cmd::id("edit/undo"));
		item->SetItemLabel(undo_text);
		item->Enable(!context->ass->IsUndoStackEmpty());

		// Redo state
		wxString redo_text = wxString::Format("%s %s\t%s",
			cmd::get("edit/redo")->StrMenu(),
			context->ass->GetRedoDescription(),
			hotkey::get_hotkey_str_first("Default", "edit/redo"));
		item = editMenu->FindItem(cmd::id("edit/redo"));
		item->SetItemLabel(redo_text);
		item->Enable(!context->ass->IsRedoStackEmpty());

		validate(MenuBar, context.get(), "edit/line/cut");
		validate(MenuBar, context.get(), "edit/line/copy");
		validate(MenuBar, context.get(), "edit/line/paste");
		validate(MenuBar, context.get(), "edit/line/paste/over");
	}

	// Automation menu
#ifdef WITH_AUTOMATION
	else if (curMenu == menu::menu->GetMenu("main/automation")) {
		wxMenu *automationMenu = menu::menu->GetMenu("main/automation");

		// Remove old macro items
		for (unsigned int i = 0; i < activeMacroItems.size(); i++) {
			wxMenu *p = 0;
			wxMenuItem *it = MenuBar->FindItem(ID_MENU_AUTOMATION_MACRO + i, &p);
			if (it)
				p->Delete(it);
		}
		activeMacroItems.clear();

		// Add new ones
		int added = 0;
		added += AddMacroMenuItems(automationMenu, wxGetApp().global_scripts->GetMacros());
		added += AddMacroMenuItems(automationMenu, context->local_scripts->GetMacros());

		// If none were added, show a ghosted notice
		if (added == 0) {
			automationMenu->Append(ID_MENU_AUTOMATION_MACRO, _("No Automation macros loaded"))->Enable(false);
			activeMacroItems.push_back(0);
		}
	}
#endif

	MenuBar->Thaw();
}

int FrameMain::AddMacroMenuItems(wxMenu *menu, const std::vector<Automation4::FeatureMacro*> &macros) {
#ifdef WITH_AUTOMATION
	if (macros.empty()) {
		return 0;
	}

	int id = activeMacroItems.size();;
	for (std::vector<Automation4::FeatureMacro*>::const_iterator i = macros.begin(); i != macros.end(); ++i) {
		wxMenuItem * m = menu->Append(ID_MENU_AUTOMATION_MACRO + id, (*i)->GetName(), (*i)->GetDescription());
		m->Enable((*i)->Validate(context->ass, SubsGrid->GetAbsoluteSelection(), SubsGrid->GetFirstSelRow()));
		activeMacroItems.push_back(*i);
		id++;
	}

	return macros.size();
#else
	return 0;
#endif
}

void FrameMain::OnAutomationMacro (wxCommandEvent &event) {
#ifdef WITH_AUTOMATION
	SubsGrid->BeginBatch();
	// First get selection data
	std::vector<int> selected_lines = SubsGrid->GetAbsoluteSelection();
	int first_sel = SubsGrid->GetFirstSelRow();
	// Run the macro...
	activeMacroItems[event.GetId()-ID_MENU_AUTOMATION_MACRO]->Process(context->ass, selected_lines, first_sel, this);
	SubsGrid->SetSelectionFromAbsolute(selected_lines);
	SubsGrid->EndBatch();
#endif
}

void FrameMain::OnCloseWindow (wxCloseEvent &event) {
	// Stop audio and video
	context->videoController->Stop();
	context->audioController->Stop();

	// Ask user if he wants to save first
	bool canVeto = event.CanVeto();
	int result = TryToCloseSubs(canVeto);

	// Store maximization state
	OPT_SET("App/Maximized")->SetBool(IsMaximized());

	// Abort/destroy
	if (canVeto) {
		if (result == wxCANCEL) event.Veto();
		else Destroy();
	}
	else Destroy();
}

void FrameMain::OnAutoSave(wxTimerEvent &) try {
		if (context->ass->loaded && context->ass->IsModified()) {
			wxFileName origfile(context->ass->filename);
			wxString path = lagi_wxString(OPT_GET("Path/Auto/Save")->GetString());
			if (path.IsEmpty()) path = origfile.GetPath();
			wxFileName dstpath(path);
		if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePathMaybeRelative(path, "?user/");
			dstpath.AssignDir(path);
			if (!dstpath.DirExists()) wxMkdir(path);

			wxString name = origfile.GetName();
		if (name.empty()) {
				dstpath.SetFullName("Untitled.AUTOSAVE.ass");
			}
			else {
			dstpath.SetFullName(name + ".AUTOSAVE.ass");
			}

			context->ass->Save(dstpath.GetFullPath(),false,false);

		StatusTimeout(_("File backup saved as \"") + dstpath.GetFullPath() + "\".");
		}
	}
	catch (const agi::Exception& err) {
		StatusTimeout(lagi_wxString("Exception when attempting to autosave file: " + err.GetMessage()));
	}
	catch (wxString err) {
	StatusTimeout("Exception when attempting to autosave file: " + err);
	}
	catch (const wchar_t *err) {
	StatusTimeout("Exception when attempting to autosave file: " + wxString(err));
	}
	catch (...) {
	StatusTimeout("Unhandled exception when attempting to autosave file.");
}

void FrameMain::OnStatusClear(wxTimerEvent &) {
	SetStatusText("",1);
}

void FrameMain::OnAudioBoxResize(wxSashEvent &event) {
	if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
		return;

	wxRect rect = event.GetDragRect();

	if (rect.GetHeight() < audioSash->GetMinimumSizeY())
		rect.SetHeight(audioSash->GetMinimumSizeY());

	audioBox->SetMinSize(wxSize(-1, rect.GetHeight()));
	Panel->Layout();
	Refresh();
}

void FrameMain::OnAudioOpen(AudioProvider *provider) {
	SetDisplayMode(-1, 1);
}

void FrameMain::OnAudioClose() {
	SetDisplayMode(-1, 0);
}

void FrameMain::OnSubtitlesOpen() {
	UpdateTitle();

	/// @todo figure out how to move this to the relevant controllers without
	///       prompting for each file loaded/unloaded

	// Load stuff from the new script
	wxString curSubsVideo = DecodeRelativePath(context->ass->GetScriptInfo("Video File"),context->ass->filename);
	wxString curSubsVFR = DecodeRelativePath(context->ass->GetScriptInfo("VFR File"),context->ass->filename);
	wxString curSubsKeyframes = DecodeRelativePath(context->ass->GetScriptInfo("Keyframes File"),context->ass->filename);
	wxString curSubsAudio = DecodeRelativePath(context->ass->GetScriptInfo("Audio URI"),context->ass->filename);
	wxString AutoScriptString = context->ass->GetScriptInfo("Automation Scripts");

	// Check if there is anything to change
	int autoLoadMode = OPT_GET("App/Auto/Load Linked Files")->GetInt();
	bool doLoad = false;
	if (curSubsAudio != context->audioController->GetAudioURL() ||
		curSubsVFR != context->videoController->GetTimecodesName() ||
		curSubsVideo != context->videoController->videoName ||
		curSubsKeyframes != context->videoController->GetKeyFramesName()
#ifdef WITH_AUTOMATION
		|| !AutoScriptString.IsEmpty() || context->local_scripts->GetScripts().size() > 0
#endif
		)
	{
		if (autoLoadMode == 1) {
			doLoad = true;
		}
		else if (autoLoadMode == 2) {
			doLoad = wxMessageBox(_("Do you want to load/unload the associated files?"), _("(Un)Load files?"), wxYES_NO) == wxYES;
		}
	}

	if (doLoad) {
		// Video
		if (!blockVideoLoad && curSubsVideo != context->videoController->videoName) {
			context->videoController->SetVideo(curSubsVideo);
			if (context->videoController->IsLoaded()) {
					long videoPos = 0;
					long videoAr = 0;
					double videoArValue = 0.0;
					double videoZoom = 0.;

					context->ass->GetScriptInfo("Video Position").ToLong(&videoPos);
					context->ass->GetScriptInfo("Video Zoom Percent").ToDouble(&videoZoom);
					wxString arString = context->ass->GetScriptInfo("Video Aspect Ratio");
					if (arString.Left(1) == "c") {
						videoAr = 4;
						arString = arString.Mid(1);
						arString.ToDouble(&videoArValue);
					}
					else if (arString.IsNumber()) {
						arString.ToLong(&videoAr);
					}

				context->videoController->SetAspectRatio(videoAr,videoArValue);
				videoBox->videoDisplay->SetZoom(videoZoom);
				context->videoController->JumpToFrame(videoPos);
			}
		}

		context->videoController->LoadTimecodes(curSubsVFR);
		context->videoController->LoadKeyframes(curSubsKeyframes);

		// Audio
		if (curSubsAudio != context->audioController->GetAudioURL()) {
			context->audioController->OpenAudio(curSubsAudio);
		}

		// Automation scripts
#ifdef WITH_AUTOMATION
		context->local_scripts->RemoveAll();
		wxStringTokenizer tok(AutoScriptString, "|", wxTOKEN_STRTOK);
		wxFileName assfn(context->ass->filename);
		wxString autobasefn(lagi_wxString(OPT_GET("Path/Automation/Base")->GetString()));
		while (tok.HasMoreTokens()) {
			wxString sfnames = tok.GetNextToken().Trim(true).Trim(false);
			wxString sfnamel = sfnames.Left(1);
			sfnames.Remove(0, 1);
			wxString basepath;
			if (sfnamel == "~") {
				basepath = assfn.GetPath();
			} else if (sfnamel == "$") {
				basepath = autobasefn;
			} else if (sfnamel == "/") {
				basepath = "";
			} else {
				wxLogWarning("Automation Script referenced with unknown location specifier character.\nLocation specifier found: %s\nFilename specified: %s",
					sfnamel.c_str(), sfnames.c_str());
				continue;
			}
			wxFileName sfname(sfnames);
			sfname.MakeAbsolute(basepath);
			if (sfname.FileExists()) {
				sfnames = sfname.GetFullPath();
				context->local_scripts->Add(Automation4::ScriptFactory::CreateFromFile(sfnames, true));
			} else {
				wxLogWarning("Automation Script referenced could not be found.\nFilename specified: %s%s\nSearched relative to: %s\nResolved filename: %s",
					sfnamel.c_str(), sfnames.c_str(), basepath.c_str(), sfname.GetFullPath().c_str());
			}
		}
#endif
	}

	// Display
	SetDisplayMode(1,1);
}

void FrameMain::OnSubtitlesSave() {
	UpdateTitle();

	// Store Automation script data
	// Algorithm:
	// 1. If script filename has Automation Base Path as a prefix, the path is relative to that (ie. "$")
	// 2. Otherwise try making it relative to the ass filename
	// 3. If step 2 failed, or absolute path is shorter than path relative to ass, use absolute path ("/")
	// 4. Otherwise, use path relative to ass ("~")
#ifdef WITH_AUTOMATION
	wxString scripts_string;
	wxString autobasefn(lagi_wxString(OPT_GET("Path/Automation/Base")->GetString()));

	const std::vector<Automation4::Script*> &scripts = context->local_scripts->GetScripts();
	for (unsigned int i = 0; i < scripts.size(); i++) {
		Automation4::Script *script = scripts[i];

		if (i != 0)
			scripts_string += "|";

		wxString autobase_rel, assfile_rel;
		wxString scriptfn(script->GetFilename());
		autobase_rel = MakeRelativePath(scriptfn, autobasefn);
		assfile_rel = MakeRelativePath(scriptfn, context->ass->filename);

		if (autobase_rel.size() <= scriptfn.size() && autobase_rel.size() <= assfile_rel.size()) {
			scriptfn = "$" + autobase_rel;
		} else if (assfile_rel.size() <= scriptfn.size() && assfile_rel.size() <= autobase_rel.size()) {
			scriptfn = "~" + assfile_rel;
		} else {
			scriptfn = "/" + wxFileName(scriptfn).GetFullPath(wxPATH_UNIX);
		}

		scripts_string += scriptfn;
	}
	context->ass->SetScriptInfo("Automation Scripts", scripts_string);
#endif
}

void FrameMain::OnKeyDown(wxKeyEvent &event) {
	if (!hotkey::check("Main Frame", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		event.Skip();
}
