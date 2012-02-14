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
#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/statline.h>
#include <wx/sysopt.h>
#include <wx/tokenzr.h>
#endif

#include <libaegisub/log.h>

#include "include/aegisub/context.h"
#include "include/aegisub/menu.h"
#include "include/aegisub/toolbar.h"
#include "include/aegisub/hotkey.h"

#include "ass_file.h"
#include "audio_controller.h"
#include "audio_box.h"
#include "auto4_base.h"
#include "compat.h"
#include "command/command.h"
#include "dialog_search_replace.h"
#include "dialog_version_check.h"
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
	ID_APP_TIMER_AUTOSAVE					= 12001,
	ID_APP_TIMER_STATUSCLEAR				= 12002,
	ID_SASH_MAIN_AUDIO						= 14001
};

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, a, "Aegisub startup log", 0)
#else
#define StartupLog(a) LOG_I("frame_main/init") << a
#endif

static void autosave_timer_changed(wxTimer *timer);

/// Handle files drag and dropped onto Aegisub
class AegisubFileDropTarget : public wxFileDropTarget {
	FrameMain *parent;
public:
	AegisubFileDropTarget(FrameMain *parent) : parent(parent) { }
	bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames) {
		return parent->LoadList(filenames);
	}
};

FrameMain::FrameMain (wxArrayString args)
: wxFrame(0,-1,"",wxDefaultPosition,wxSize(920,700),wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
, context(new agi::Context)
, showVideo(true)
, showAudio(true)
, blockVideoLoad(false)
{
	StartupLog("Entering FrameMain constructor");

#ifdef __WXGTK__
	// XXX HACK XXX
	// We need to set LC_ALL to "" here for input methods to work reliably.
	setlocale(LC_ALL, "");

	// However LC_NUMERIC must be "C", otherwise some parsing fails.
	setlocale(LC_NUMERIC, "C");
#endif
#ifdef __APPLE__
	// Apple's wprintf() and family breaks with CTYPE set to "C"
	setlocale(LC_CTYPE, "");
#endif

	StartupLog("Initializing context models");
	AssFile::top = context->ass = new AssFile;
	context->ass->AddCommitListener(&FrameMain::UpdateTitle, this);
	context->ass->AddFileOpenListener(&FrameMain::OnSubtitlesOpen, this);
	context->ass->AddFileSaveListener(&FrameMain::UpdateTitle, this);

	context->local_scripts = new Automation4::LocalScriptManager(context.get());

	StartupLog("Initializing context controls");
	context->audioController = new AudioController(context.get());
	context->audioController->AddAudioOpenListener(&FrameMain::OnAudioOpen, this);
	context->audioController->AddAudioCloseListener(&FrameMain::OnAudioClose, this);

	// Initialized later due to that the selection controller is currently the subtitles grid
	context->selectionController = 0;

	context->videoController = VideoContext::Get(); // derp
	context->videoController->AddVideoOpenListener(&FrameMain::OnVideoOpen, this);

	StartupLog("Initializing context frames");
	context->parent = this;
	context->previousFocus = 0;
	wxGetApp().frame = this;

#ifdef __WXMAC__
//	Bind(FrameMain::OnAbout, &FrameMain::cmd_call, this, cmd::id("app/about"));
#endif

	StartupLog("Install PNG handler");
	wxImage::AddHandler(new wxPNGHandler);
#ifndef __APPLE__
	wxSafeYield();
#endif

	StartupLog("Apply saved Maximized state");
	if (OPT_GET("App/Maximized")->GetBool()) Maximize(true);

	StartupLog("Initialize toolbar");
	InitToolbar();

	StartupLog("Initialize menu bar");
	menu::GetMenuBar("main", this, context.get());
	
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
	OPT_SUB("Video/Detached/Enabled", &FrameMain::OnVideoDetach, this, agi::signal::_1);

	StartupLog("Complete context initialization");
	context->videoController->SetContext(context.get());

	StartupLog("Set up Auto Save");
	AutoSave.SetOwner(this, ID_APP_TIMER_AUTOSAVE);
	autosave_timer_changed(&AutoSave);
	OPT_SUB("App/Auto/Save", autosave_timer_changed, &AutoSave);
	OPT_SUB("App/Auto/Save Every Seconds", autosave_timer_changed, &AutoSave);

	StartupLog("Set up drag/drop target");
	SetDropTarget(new AegisubFileDropTarget(this));

	StartupLog("Load default file");
	context->ass->LoadDefault();

	StartupLog("Display main window");
	Show();
	SetDisplayMode(1, 1);

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

	StartupLog("Leaving FrameMain constructor");
}

FrameMain::~FrameMain () {
	// Because the subs grid is the selection controller, it needs to stay
	// alive significantly longer than the other child controls
	SubsGrid->Reparent(0);
	SubsGrid->Hide();

	context->videoController->SetVideo("");
	context->audioController->CloseAudio();

	// Ensure the children get destroyed before the project context is destroyed
	DestroyChildren();
	wxTheApp->ProcessPendingEvents();

	delete context->ass;
	HelpButton::ClearPages();
	delete context->audioController;
	delete context->local_scripts;

	SubsGrid->Destroy();
}

void FrameMain::InitToolbar() {
	wxSystemOptions::SetOption("msw.remap", 0);
	OPT_SUB("App/Show Toolbar", &FrameMain::EnableToolBar, this);
	EnableToolBar(*OPT_GET("App/Show Toolbar"));
}

void FrameMain::EnableToolBar(agi::OptionValue const& opt) {
	if (opt.GetBool()) {
		if (!GetToolBar()) {
			toolbar::AttachToolbar(this, "main", context.get(), "Default");
			GetToolBar()->Realize();
		}
	}
	else if (wxToolBar *old_tb = GetToolBar()) {
		SetToolBar(0);
		delete old_tb;
		Layout();
	}
}

void FrameMain::InitContents() {
	StartupLog("Create background panel");
	wxPanel *Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	StartupLog("Create subtitles grid");
	context->subsGrid = SubsGrid = new SubtitlesGrid(Panel,context.get(),wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,"Subs grid");
	context->selectionController = context->subsGrid;
	Search.context = context.get();

	StartupLog("Create video box");
	videoBox = new VideoBox(Panel, false, context.get());

	StartupLog("Create audio box");
	context->audioBox = audioBox = new AudioBox(Panel, context.get());

	StartupLog("Create subtitle editing box");
	SubsEditBox *EditBox = new SubsEditBox(Panel, context.get());
	context->editBox = EditBox->TextEdit;

	StartupLog("Arrange main sizers");
	ToolsSizer = new wxBoxSizer(wxVERTICAL);
	ToolsSizer->Add(audioBox, 0, wxEXPAND);
	ToolsSizer->Add(EditBox, 1, wxEXPAND);
	TopSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(videoBox, 0, wxEXPAND, 0);
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
	catch (agi::FileNotFoundError const&) {
		wxMessageBox(filename + " not found.", "Error", wxOK | wxICON_ERROR, NULL);
		config::mru->Remove("Subtitle", STD_STR(filename));
		return;
	}
	catch (const char *err) {
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
		int result = wxMessageBox(wxString::Format(_("Do you want to save changes to %s?"), GetScriptFileName()), _("Unsaved changes"), flags, this);
		if (result == wxYES) {
			cmd::call("subtitle/save", context.get());
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
	ToolsSizer->Show(audioBox, showAudio, true);

	MainSizer->CalcMin();
	MainSizer->RecalcSizes();
	MainSizer->Layout();
	Layout();

	if (didFreeze) Thaw();
}

void FrameMain::UpdateTitle() {
	wxString newTitle;
	if (context->ass->IsModified()) newTitle << "* ";
	newTitle << GetScriptFileName();

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
		return;
	}

	Freeze();
	int vidx = context->videoController->GetWidth(),
		vidy = context->videoController->GetHeight();

	// Set zoom level based on video resolution and window size
	double zoom = context->videoDisplay->GetZoom();
	wxSize windowSize = GetSize();
	if (vidx*3*zoom > windowSize.GetX()*4 || vidy*4*zoom > windowSize.GetY()*6)
		context->videoDisplay->SetZoom(zoom * .25);
	else if (vidx*3*zoom > windowSize.GetX()*2 || vidy*4*zoom > windowSize.GetY()*3)
		context->videoDisplay->SetZoom(zoom * .5);

	SetDisplayMode(1,-1);

	if (OPT_GET("Video/Detached/Enabled")->GetBool() && !context->detachedVideo)
		cmd::call("video/detach", context.get());
	Thaw();
}

void FrameMain::OnVideoDetach(agi::OptionValue const& opt) {
	if (opt.GetBool())
		SetDisplayMode(0, -1);
	else if (context->videoController->IsLoaded())
		SetDisplayMode(1, -1);
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
	videoList.Add("asf");
	videoList.Add("avi");
	videoList.Add("avs");
	videoList.Add("d2v");
	videoList.Add("m2ts");
	videoList.Add("mkv");
	videoList.Add("mov");
	videoList.Add("mp4");
	videoList.Add("mpeg");
	videoList.Add("mpg");
	videoList.Add("ogm");
	videoList.Add("rm");
	videoList.Add("rmvb");
	videoList.Add("wmv");
	videoList.Add("ts");
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
	audioList.Add("aac");
	audioList.Add("ac3");
	audioList.Add("ape");
	audioList.Add("dts");
	audioList.Add("flac");
	audioList.Add("m4a");
	audioList.Add("mka");
	audioList.Add("mp3");
	audioList.Add("ogg");
	audioList.Add("w64");
	audioList.Add("wav");
	audioList.Add("wma");

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

static void autosave_timer_changed(wxTimer *timer) {
	int freq = OPT_GET("App/Auto/Save Every Seconds")->GetInt();
	if (freq > 0 && OPT_GET("App/Auto/Save")->GetBool())
		timer->Start(freq * 1000);
	else
		timer->Stop();
}
BEGIN_EVENT_TABLE(FrameMain, wxFrame)
	EVT_TIMER(ID_APP_TIMER_AUTOSAVE, FrameMain::OnAutoSave)
	EVT_TIMER(ID_APP_TIMER_STATUSCLEAR, FrameMain::OnStatusClear)

	EVT_CLOSE(FrameMain::OnCloseWindow)

	EVT_KEY_DOWN(FrameMain::OnKeyDown)
	EVT_MOUSEWHEEL(FrameMain::OnMouseWheel)

#ifdef __WXMAC__
//   EVT_MENU(wxID_ABOUT, FrameMain::OnAbout)
//   EVT_MENU(wxID_EXIT, FrameMain::OnExit)
#endif
END_EVENT_TABLE()

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
	wxString fn = context->ass->AutoSave();
	if (!fn.empty())
		StatusTimeout(wxString::Format(_("File backup saved as \"%s\"."), fn));
}
catch (const agi::Exception& err) {
	StatusTimeout(lagi_wxString("Exception when attempting to autosave file: " + err.GetMessage()));
}
catch (wxString err) {
	StatusTimeout("Exception when attempting to autosave file: " + err);
}
catch (const char *err) {
	StatusTimeout("Exception when attempting to autosave file: " + wxString(err));
}
catch (...) {
	StatusTimeout("Unhandled exception when attempting to autosave file.");
}

void FrameMain::OnStatusClear(wxTimerEvent &) {
	SetStatusText("",1);
}

void FrameMain::OnAudioOpen(AudioProvider *) {
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
	wxString curSubsAudio = context->ass->GetScriptInfo("Audio URI");

	// Check if there is anything to change
	int autoLoadMode = OPT_GET("App/Auto/Load Linked Files")->GetInt();
	bool doLoad = false;
	if (curSubsAudio != context->audioController->GetAudioURL() ||
		curSubsVFR != context->videoController->GetTimecodesName() ||
		curSubsVideo != context->videoController->GetVideoName() ||
		curSubsKeyframes != context->videoController->GetKeyFramesName()
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
		if (!blockVideoLoad && curSubsVideo != context->videoController->GetVideoName()) {
			context->videoController->SetVideo(curSubsVideo);
			if (context->videoController->IsLoaded()) {
				context->videoController->JumpToFrame(context->ass->GetScriptInfoAsInt("Video Position"));

				long videoAr = 0;
				double videoArValue = 0.;
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

				double videoZoom = 0.;
				if (context->ass->GetScriptInfo("Video Zoom Percent").ToDouble(&videoZoom))
					context->videoDisplay->SetZoom(videoZoom);
			}
		}

		context->videoController->LoadTimecodes(curSubsVFR);
		context->videoController->LoadKeyframes(curSubsKeyframes);

		// Audio
		if (curSubsAudio != context->audioController->GetAudioURL()) {
			try {
				if (!curSubsAudio)
					context->audioController->CloseAudio();
				else
					context->audioController->OpenAudio(curSubsAudio);
			}
			catch (agi::UserCancelException const&) { }
		}
	}

	// Display
	SetDisplayMode(1,1);
}

void FrameMain::OnKeyDown(wxKeyEvent &event) {
	if (!hotkey::check("Main Frame", context.get(), event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers()))
		event.Skip();
}

void FrameMain::OnMouseWheel(wxMouseEvent &evt) {
	ForwardMouseWheelEvent(this, evt);
}

wxString FrameMain::GetScriptFileName() const {
	if (context->ass->filename.empty()) {
		// Apple HIG says "untitled" should not be capitalised
		// and the window is a document window, it shouldn't contain the app name
		// (The app name is already present in the menu bar)
#ifndef __WXMAC__
		return _("Untitled");
#else
		return _("untitled");
#endif
	}
	else {
		wxFileName file (context->ass->filename);
		return file.GetFullName();
	}
}
