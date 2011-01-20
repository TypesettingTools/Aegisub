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
#include "selection_controller.h"
#include "audio_controller.h"
#include "audio_box.h"
#ifdef WITH_AUTOMATION
#include "auto4_base.h"
#endif
#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#endif
#include "compat.h"
#include "command/command.h"
#include "dialog_detached_video.h"
#include "dialog_search_replace.h"
#include "dialog_styling_assistant.h"
#include "dialog_version_check.h"
#include "drop.h"
#include "frame_main.h"
#include "help_button.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "standard_paths.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "subs_grid.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"
#include "version.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"
#include "video_slider.h"


#ifdef WITH_STARTUPLOG

/// DOCME
#define StartupLog(a) MessageBox(0, a, _T("Aegisub startup log"), 0)
#else

/// DOCME
#define StartupLog(a)
#endif

static void autosave_timer_changed(wxTimer *timer, const agi::OptionValue &opt);

FrameMain::FrameMain (wxArrayString args)
: wxFrame ((wxFrame*)NULL,-1,_T(""),wxDefaultPosition,wxSize(920,700),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
, context(new agi::Context)
, showVideo(true)
, showAudio(true)
, HasSelection(false)
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
	context->ass->AddCommitListener(&FrameMain::OnSubtitlesCommit, this);
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
	int count = cmd::count();
	for (int i = 0; i < count; i++) {
		Bind(wxEVT_COMMAND_MENU_SELECTED, &FrameMain::cmd_call, this, i);
	}

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

	//ShowFullScreen(true,wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
	StartupLog("Leaving FrameMain constructor");
}

/// @brief FrameMain destructor 
FrameMain::~FrameMain () {
	context->videoController->SetVideo("");
	context->audioController->CloseAudio();
	DeInitContents();
	delete context->audioController;
#ifdef WITH_AUTOMATION
	delete context->local_scripts;
#endif
}

void FrameMain::cmd_call(wxCommandEvent& event) {
	int id = event.GetId();
	LOG_D("event/select") << "Id: " << id;
	cmd::call(context.get(), id);
}

/// @brief Initialize toolbar 
void FrameMain::InitToolbar () {
	// Create toolbar
	wxSystemOptions::SetOption(_T("msw.remap"), 0);
	Toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL,-1,_T("Toolbar"));

	toolbar::toolbar->GetToolbar("main", Toolbar);

	wxArrayString choices;
	for (int i=1;i<=24;i++) {
		wxString toAdd = wxString::Format(_T("%i"),int(i*12.5));
		if (i%2) toAdd += _T(".5");
		toAdd += _T("%");
		choices.Add(toAdd);
	}
	ZoomBox = new wxComboBox(Toolbar,ID_TOOLBAR_ZOOM_DROPDOWN,_T("75%"),wxDefaultPosition,wxDefaultSize,choices,wxCB_DROPDOWN);
	Toolbar->AddControl(ZoomBox);
	Toolbar->AddSeparator();

	// Update
	Toolbar->Realize();
}


/// @brief Initialize menu bar 
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


/// @brief Initialize contents 
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
	context->videoBox->videoSlider->grid = SubsGrid;
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
	//MainSizer->SetSizeHints(Panel);
	//SetSizer(MainSizer);

	StartupLog("Perform layout");
	Layout();
	StartupLog("Set focus to edting box");
	EditBox->TextEdit->SetFocus();
	StartupLog("Leaving InitContents");
}

/// @brief Deinitialize controls 
void FrameMain::DeInitContents() {
	if (context->detachedVideo) context->detachedVideo->Destroy();
	if (context->stylingAssistant) context->stylingAssistant->Destroy();
	SubsGrid->ClearMaps();
	delete audioBox;
	delete EditBox;
	delete videoBox;
	delete context->ass;
	HelpButton::ClearPages();
}

/// @brief Update toolbar 
void FrameMain::UpdateToolbar() {
	// Collect flags
	bool isVideo = context->videoController->IsLoaded();
	HasSelection = true;
	int selRows = SubsGrid->GetNumberSelection();

	// Update
	wxToolBar* toolbar = GetToolBar();
	toolbar->FindById(cmd::id("video/jump"))->Enable(isVideo);
	toolbar->FindById(cmd::id("video/zoom/in"))->Enable(isVideo && !context->detachedVideo);
	toolbar->FindById(cmd::id("video/zoom/out"))->Enable(isVideo && !context->detachedVideo);
	ZoomBox->Enable(isVideo && !context->detachedVideo);

	toolbar->FindById(cmd::id("video/jump/start"))->Enable(isVideo && selRows > 0);
	toolbar->FindById(cmd::id("video/jump/end"))->Enable(isVideo && selRows > 0);

	toolbar->FindById(cmd::id("time/snap/start_video"))->Enable(isVideo && selRows == 1);
	toolbar->FindById(cmd::id("time/snap/end_video"))->Enable(isVideo && selRows == 1);

	toolbar->FindById(cmd::id("subtitle/select/visible"))->Enable(isVideo);
	toolbar->FindById(cmd::id("time/snap/scene"))->Enable(isVideo && selRows > 0);
	toolbar->FindById(cmd::id("time/snap/frame"))->Enable(isVideo && selRows > 0);
	toolbar->Realize();
}

/// @brief Open subtitles 
/// @param filename 
/// @param charset  
void FrameMain::LoadSubtitles(wxString filename,wxString charset) {
	if (context->ass->loaded) {
		if (TryToCloseSubs() == wxCANCEL) return;
	}

	try {
		// Make sure that file isn't actually a timecode file
		try {
			TextFileReader testSubs(filename,charset);
			wxString cur = testSubs.ReadLineFromFile();
			if (cur.Left(10) == _T("# timecode")) {
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
		wxMessageBox(filename + L" not found.", L"Error", wxOK | wxICON_ERROR, NULL);
		config::mru->Remove("Subtitle", STD_STR(filename));
		return;
	}
	catch (const wchar_t *err) {
		wxMessageBox(wxString(err), _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (wxString err) {
		wxMessageBox(err, _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error"), wxOK | wxICON_ERROR, NULL);
		return;
	}
}

/// @brief Try to close subtitles 
/// @param enableCancel 
/// @return 
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

/// @brief Set the video and audio display visibilty
/// @param video -1: leave unchanged; 0: hide; 1: show
/// @param audio -1: leave unchanged; 0: hide; 1: show
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

	// Set display
	TopSizer->Show(videoBox, showVideo, true);
	ToolsSizer->Show(audioSash, showAudio, true);

	// Update
	UpdateToolbar();
	MainSizer->CalcMin();
	MainSizer->RecalcSizes();
	MainSizer->Layout();
	Layout();

	if (didFreeze) Thaw();
}

/// @brief Update title bar 
void FrameMain::UpdateTitle() {
	wxString newTitle;
	if (context->ass->IsModified()) newTitle << _T("* ");
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

/// @brief Open help 
void FrameMain::OpenHelp(wxString) {
	HelpButton::OpenPage(_T("Main"));
}

/// @brief Detach video window 
/// @param detach 
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

/// @brief Sets status and clear after n milliseconds
/// @param text 
/// @param ms   
void FrameMain::StatusTimeout(wxString text,int ms) {
	SetStatusText(text,1);
	StatusClear.SetOwner(this, ID_APP_TIMER_STATUSCLEAR);
	StatusClear.Start(ms,true);
}


/// @brief Load list of files 
/// @param list 
/// @return 
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
	videoList.Add(_T("y4m"));
	videoList.Add(_T("yuv"));

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

	// Set blocking
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

	// Result
	return subs.size() || audio.size() || video.size();
}


/// @brief Sets the descriptions for undo/redo 
void FrameMain::SetUndoRedoDesc() {
	wxMenu *editMenu = menu::menu->GetMenu("main/edit");
	editMenu->SetHelpString(0,_T("Undo ")+context->ass->GetUndoDescription());
	editMenu->SetHelpString(1,_T("Redo ")+context->ass->GetRedoDescription());
}

/// @brief Check if ASSDraw is available 
bool FrameMain::HasASSDraw() {
#ifdef __WINDOWS__
	wxFileName fn(StandardPaths::DecodePath(_T("?data/ASSDraw3.exe")));
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
//	EVT_MENU(cmd::id("subtitle/new"), FrameMain::cmd_call)
//	EVT_MENU(cmd::id("subtitle/open"), FrameMain::cmd_call)
//	EVT_MENU(cmd::id("subtitle/save"), FrameMain::cmd_call)

//	EVT_MENU_RANGE(MENU_GRID_START+1,MENU_GRID_END-1,FrameMain::OnGridEvent)
//	EVT_COMBOBOX(Toolbar_Zoom_Dropdown, FrameMain::OnSetZoom)
//	EVT_TEXT_ENTER(Toolbar_Zoom_Dropdown, FrameMain::OnSetZoom)

#ifdef __WXMAC__
//   EVT_MENU(wxID_ABOUT, FrameMain::OnAbout)
//   EVT_MENU(wxID_EXIT, FrameMain::OnExit)
#endif
END_EVENT_TABLE()


/// @brief Redirect grid events to grid 
/// @param event 
void FrameMain::OnGridEvent (wxCommandEvent &event) {
	SubsGrid->GetEventHandler()->ProcessEvent(event);
}

/// @brief Rebuild recent list 
/// @param listName 
/// @param menu     
/// @param startID  
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

/// @brief Menu is being opened 
/// @param event 
void FrameMain::OnMenuOpen (wxMenuEvent &event) {
	// Get menu
	wxMenuBar *MenuBar = menu::menu->GetMainMenu();

	MenuBar->Freeze();
	wxMenu *curMenu = event.GetMenu();

	// File menu
	if (curMenu == menu::menu->GetMenu("main/file")) {
		// Rebuild recent
		RebuildRecentList("recent/subtitle", "Subtitle");

		MenuBar->Enable(cmd::id("subtitle/open/video"),context->videoController->HasSubtitles());
	}

	// View menu
	else if (curMenu == menu::menu->GetMenu("main/view")) {
		// Flags
		bool aud = context->audioController->IsAudioOpen();
		bool vid = context->videoController->IsLoaded() && !context->detachedVideo;

		// Set states
		MenuBar->Enable(cmd::id("app/display/audio_subs"),aud);
		MenuBar->Enable(cmd::id("app/display/video_subs"),vid);
		MenuBar->Enable(cmd::id("app/display/full"),aud && vid);

		// Select option
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
		bool state = context->videoController->IsLoaded();
		bool attached = state && !context->detachedVideo;

		// Set states
		MenuBar->Enable(cmd::id("video/jump"),state);
		MenuBar->Enable(cmd::id("video/jump/start"),state);
		MenuBar->Enable(cmd::id("video/jump/end"),state);
		MenuBar->Enable(cmd::id("main/video/set zoom"), attached);
		MenuBar->Enable(cmd::id("video/zoom/50"),attached);
		MenuBar->Enable(cmd::id("video/zoom/100"),attached);
		MenuBar->Enable(cmd::id("video/zoom/200"),attached);
		MenuBar->Enable(cmd::id("video/close"),state);
		MenuBar->Enable(cmd::id("main/video/override ar"),attached);
		MenuBar->Enable(cmd::id("video/aspect/default"),attached);
		MenuBar->Enable(cmd::id("video/aspect/full"),attached);
		MenuBar->Enable(cmd::id("video/aspect/wide"),attached);
		MenuBar->Enable(cmd::id("video/aspect/cinematic"),attached);
		MenuBar->Enable(cmd::id("video/aspect/custom"),attached);
		MenuBar->Enable(cmd::id("video/detach"),state);
		MenuBar->Enable(cmd::id("timecode/save"),context->videoController->TimecodesLoaded());
		MenuBar->Enable(cmd::id("timecode/close"),context->videoController->OverTimecodesLoaded());
		MenuBar->Enable(cmd::id("keyframe/close"),context->videoController->OverKeyFramesLoaded());
		MenuBar->Enable(cmd::id("keyframe/save"),context->videoController->KeyFramesLoaded());
		MenuBar->Enable(cmd::id("video/details"),state);
		MenuBar->Enable(cmd::id("video/show_overscan"),state);

		// Set AR radio
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

		// Set overscan mask
		MenuBar->Check(cmd::id("video/show_overscan"),OPT_GET("Video/Overscan Mask")->GetBool());

		// Rebuild recent lists
		RebuildRecentList("recent/video", "Video");
		RebuildRecentList("recent/timecode", "Timecodes");
		RebuildRecentList("recent/keyframe", "Keyframes");
	}

	// Audio menu
	else if (curMenu == menu::menu->GetMenu("main/audio")) {
		bool state = context->audioController->IsAudioOpen();
		bool vidstate = context->videoController->IsLoaded();

		MenuBar->Enable(cmd::id("audio/open/video"),vidstate);
		MenuBar->Enable(cmd::id("audio/close"),state);

		// Rebuild recent
		RebuildRecentList("recent/audio", "Audio");
	}

	// Subtitles menu
	else if (curMenu == menu::menu->GetMenu("main/subtitle")) {
		// Variables
		bool continuous;
		wxArrayInt sels = SubsGrid->GetSelection(&continuous);
		int count = sels.Count();
		bool state,state2;

		// Entries
		state = count > 0;
		MenuBar->Enable(cmd::id("subtitle/insert/before"),state);
		MenuBar->Enable(cmd::id("subtitle/insert/after"),state);
		MenuBar->Enable(cmd::id("edit/line/split/by_karaoke"),state);
		MenuBar->Enable(cmd::id("edit/line/delete"),state);
		state2 = count > 0 && context->videoController->IsLoaded();
		MenuBar->Enable(cmd::id("subtitle/insert/before/videotime"),state2);
		MenuBar->Enable(cmd::id("subtitle/insert/after/videotime"),state2);
		MenuBar->Enable(cmd::id("main/subtitle/insert lines"),state);
		state = count > 0 && continuous;
		MenuBar->Enable(cmd::id("edit/line/duplicate"),state);
		state = count > 0 && continuous && context->videoController->TimecodesLoaded();
		MenuBar->Enable(cmd::id("edit/line/duplicate/shift"),state);
		state = count == 2;
		MenuBar->Enable(cmd::id("edit/line/swap"),state);
		state = count >= 2 && continuous;
		MenuBar->Enable(cmd::id("edit/line/join/concatenate"),state);
		MenuBar->Enable(cmd::id("edit/line/join/keep_first"),state);
		MenuBar->Enable(cmd::id("edit/line/join/as_karaoke"),state);
		MenuBar->Enable(cmd::id("main/subtitle/join lines"),state);
		state = (count == 2 || count == 3) && continuous;
		MenuBar->Enable(cmd::id("edit/line/recombine"),state);
	}

	// Timing menu
	else if (curMenu == menu::menu->GetMenu("main/timing")) {
		// Variables
		bool continuous;
		wxArrayInt sels = SubsGrid->GetSelection(&continuous);
		int count = sels.Count();

		// Video related
		bool state = context->videoController->IsLoaded();
		MenuBar->Enable(cmd::id("time/snap/start_video"),state);
		MenuBar->Enable(cmd::id("time/snap/end_video"),state);
		MenuBar->Enable(cmd::id("time/snap/scene"),state);
		MenuBar->Enable(cmd::id("time/frame/current"),state);

		// Other
		state = count >= 2 && continuous;
		MenuBar->Enable(cmd::id("time/continuous/start"),state);
		MenuBar->Enable(cmd::id("time/continuous/end"),state);
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

		// Copy/cut/paste
		wxArrayInt sels = SubsGrid->GetSelection();
		bool can_copy = (sels.Count() > 0);

		bool can_paste = true;
		if (wxTheClipboard->Open()) {
			can_paste = wxTheClipboard->IsSupported(wxDF_TEXT);
			wxTheClipboard->Close();
		}

		MenuBar->Enable(cmd::id("edit/line/cut"),can_copy);
		MenuBar->Enable(cmd::id("edit/line/copy"),can_copy);
		MenuBar->Enable(cmd::id("edit/line/paste"),can_paste);
		MenuBar->Enable(cmd::id("edit/line/paste/over"),can_copy&&can_paste);
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

/// @brief Macro menu creation helper 
/// @param menu   
/// @param macros 
/// @return 
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

/// @brief General handler for all Automation-generated menu items
/// @param event 
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


/// @brief Window is attempted to be closed
/// @param event
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

/// @brief Autosave the currently open file, if any
void FrameMain::OnAutoSave(wxTimerEvent &) {
	try {
		if (context->ass->loaded && context->ass->IsModified()) {
			// Set path
			wxFileName origfile(context->ass->filename);
			wxString path = lagi_wxString(OPT_GET("Path/Auto/Save")->GetString());
			if (path.IsEmpty()) path = origfile.GetPath();
			wxFileName dstpath(path);
			if (!dstpath.IsAbsolute()) path = StandardPaths::DecodePathMaybeRelative(path, _T("?user/"));
			dstpath.AssignDir(path);
			if (!dstpath.DirExists()) wxMkdir(path);

			wxString name = origfile.GetName();
			if (name.IsEmpty()) {
				dstpath.SetFullName("Untitled.AUTOSAVE.ass");
			}
			else {
				dstpath.SetFullName(name + L".AUTOSAVE.ass");
			}

			context->ass->Save(dstpath.GetFullPath(),false,false);

			// Set status bar
			StatusTimeout(_("File backup saved as \"") + dstpath.GetFullPath() + _T("\"."));
		}
	}
	catch (const agi::Exception& err) {
		StatusTimeout(lagi_wxString("Exception when attempting to autosave file: " + err.GetMessage()));
	}
	catch (wxString err) {
		StatusTimeout(_T("Exception when attempting to autosave file: ") + err);
	}
	catch (const wchar_t *err) {
		StatusTimeout(_T("Exception when attempting to autosave file: ") + wxString(err));
	}
	catch (...) {
		StatusTimeout(_T("Unhandled exception when attempting to autosave file."));
	}
}

/// @brief Clear statusbar 
void FrameMain::OnStatusClear(wxTimerEvent &) {
	SetStatusText(_T(""),1);
}

void FrameMain::OnAudioBoxResize(wxSashEvent &event)
{
	if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
		return;

	wxRect rect = event.GetDragRect();

	if (rect.GetHeight() < audioSash->GetMinimumSizeY())
		rect.SetHeight(audioSash->GetMinimumSizeY());

	audioBox->SetMinSize(wxSize(-1, rect.GetHeight()));
	Panel->Layout();
	Refresh();
}

void FrameMain::OnAudioOpen(AudioProvider *provider)
{
	SetDisplayMode(-1, 1);
}

void FrameMain::OnAudioClose()
{
	SetDisplayMode(-1, 0);
}

void FrameMain::OnSubtitlesCommit() {
	if (OPT_GET("App/Auto/Save on Every Change")->GetBool()) {
		if (context->ass->IsModified() && context->ass->CanSave())
			(*cmd::get("subtitle/save"))(context.get());
	}

	UpdateTitle();
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
		wxStringTokenizer tok(AutoScriptString, _T("|"), wxTOKEN_STRTOK);
		wxFileName assfn(context->ass->filename);
		wxString autobasefn(lagi_wxString(OPT_GET("Path/Automation/Base")->GetString()));
		while (tok.HasMoreTokens()) {
			wxString sfnames = tok.GetNextToken().Trim(true).Trim(false);
			wxString sfnamel = sfnames.Left(1);
			sfnames.Remove(0, 1);
			wxString basepath;
			if (sfnamel == _T("~")) {
				basepath = assfn.GetPath();
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
				context->local_scripts->Add(Automation4::ScriptFactory::CreateFromFile(sfnames, true));
			} else {
				wxLogWarning(_T("Automation Script referenced could not be found.\nFilename specified: %s%s\nSearched relative to: %s\nResolved filename: %s"),
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
			scripts_string += _T("|");

		wxString autobase_rel, assfile_rel;
		wxString scriptfn(script->GetFilename());
		autobase_rel = MakeRelativePath(scriptfn, autobasefn);
		assfile_rel = MakeRelativePath(scriptfn, context->ass->filename);

		if (autobase_rel.size() <= scriptfn.size() && autobase_rel.size() <= assfile_rel.size()) {
			scriptfn = _T("$") + autobase_rel;
		} else if (assfile_rel.size() <= scriptfn.size() && assfile_rel.size() <= autobase_rel.size()) {
			scriptfn = _T("~") + assfile_rel;
		} else {
			scriptfn = _T("/") + wxFileName(scriptfn).GetFullPath(wxPATH_UNIX);
		}

		scripts_string += scriptfn;
	}
	context->ass->SetScriptInfo(_T("Automation Scripts"), scripts_string);
#endif

}

void FrameMain::OnKeyDown(wxKeyEvent &event) {
	if (!hotkey::check("Main Frame", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		event.Skip();
}

