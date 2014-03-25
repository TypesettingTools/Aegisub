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

/// @file frame_main.cpp
/// @brief Main window creation and control management
/// @ingroup main_ui

#include "config.h"

#include "frame_main.h"

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include "include/aegisub/context.h"
#include "include/aegisub/menu.h"
#include "include/aegisub/toolbar.h"
#include "include/aegisub/hotkey.h"

#include "ass_file.h"
#include "audio_controller.h"
#include "audio_box.h"
#include "auto4_base.h"
#include "base_grid.h"
#include "compat.h"
#include "command/command.h"
#include "dialog_detached_video.h"
#include "dialog_manager.h"
#include "dialog_version_check.h"
#include "help_button.h"
#include "initial_line_state.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "options.h"
#include "search_replace_engine.h"
#include "subs_controller.h"
#include "subs_edit_box.h"
#include "subs_edit_ctrl.h"
#include "utils.h"
#include "version.h"
#include "video_box.h"
#include "video_context.h"
#include "video_display.h"
#include "video_provider_manager.h"
#include "video_slider.h"

#include <boost/algorithm/string/predicate.hpp>

#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/msgdlg.h>
#include <wx/statline.h>
#include <wx/sysopt.h>

enum {
	ID_APP_TIMER_STATUSCLEAR				= 12002
};

#ifdef WITH_STARTUPLOG
#define StartupLog(a) MessageBox(0, a, "Aegisub startup log", 0)
#else
#define StartupLog(a) LOG_I("frame_main/init") << a
#endif

wxDEFINE_EVENT(FILE_LIST_DROPPED, wxThreadEvent);

static void get_files_to_load(wxArrayString const& list, std::string &subs, std::string &audio, std::string &video) {
	// Keep these lists sorted

	// Video formats
	const wxString videoList[] = {
		"asf",
		"avi",
		"avs",
		"d2v",
		"m2ts",
		"m4v",
		"mkv",
		"mov",
		"mp4",
		"mpeg",
		"mpg",
		"ogm",
		"rm",
		"rmvb",
		"ts",
		"webm"
		"wmv",
		"y4m",
		"yuv"
	};

	// Subtitle formats
	const wxString subsList[] = {
		"ass",
		"srt",
		"ssa",
		"sub",
		"ttxt",
		"txt"
	};

	// Audio formats
	const wxString audioList[] = {
		"aac",
		"ac3",
		"ape",
		"dts",
		"flac",
		"m4a",
		"mka",
		"mp3",
		"ogg",
		"w64",
		"wav",
		"wma"
	};

	// Scan list
	for (wxFileName file : list) {
		if (file.IsRelative()) file.MakeAbsolute();
		if (!file.FileExists()) continue;

		wxString ext = file.GetExt().Lower();

		if (subs.empty() && std::binary_search(std::begin(subsList), std::end(subsList), ext))
			subs = from_wx(file.GetFullPath());
		if (video.empty() && std::binary_search(std::begin(videoList), std::end(videoList), ext))
			video = from_wx(file.GetFullPath());
		if (audio.empty() && std::binary_search(std::begin(audioList), std::end(audioList), ext))
			audio = from_wx(file.GetFullPath());
	}
}

/// Handle files drag and dropped onto Aegisub
class AegisubFileDropTarget : public wxFileDropTarget {
	FrameMain *parent;
public:
	AegisubFileDropTarget(FrameMain *parent) : parent(parent) { }
	bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames) override {
		std::string subs, audio, video;
		get_files_to_load(filenames, subs, audio, video);

		if (subs.empty() && audio.empty() && video.empty())
			return false;

		auto evt = new wxThreadEvent(FILE_LIST_DROPPED);
		evt->SetPayload(filenames);
		parent->QueueEvent(evt);
		return true;
	}
};

FrameMain::FrameMain()
: wxFrame(nullptr, -1, "", wxDefaultPosition, wxSize(920,700), wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN)
, context(agi::util::make_unique<agi::Context>())
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
	// When run from an app bundle, LC_CTYPE defaults to "C", which breaks on
	// anything involving unicode and in some cases number formatting.
	// The right thing to do here would be to query CoreFoundation for the user's
	// locale and add .UTF-8 to that, but :effort:
	LOG_D("locale") << setlocale(LC_ALL, nullptr);
	setlocale(LC_CTYPE, "en_US.UTF-8");
	LOG_D("locale") << setlocale(LC_ALL, nullptr);
#endif

	StartupLog("Initializing context models");
	memset(context.get(), 0, sizeof(*context));
	context->ass = new AssFile;

	StartupLog("Initializing context controls");
	context->subsController = new SubsController(context.get());
	context->ass->AddCommitListener(&FrameMain::UpdateTitle, this);
	context->subsController->AddFileOpenListener(&FrameMain::OnSubtitlesOpen, this);
	context->subsController->AddFileSaveListener(&FrameMain::UpdateTitle, this);

	context->audioController = new AudioController(context.get());
	context->audioController->AddAudioOpenListener(&FrameMain::OnAudioOpen, this);
	context->audioController->AddAudioCloseListener(&FrameMain::OnAudioClose, this);

	context->local_scripts = new Automation4::LocalScriptManager(context.get());

	// Initialized later due to that the selection controller is currently the subtitles grid
	context->selectionController = nullptr;

	context->videoController = VideoContext::Get(); // derp
	context->videoController->AddVideoOpenListener(&FrameMain::OnVideoOpen, this);

	StartupLog("Initializing context frames");
	context->parent = this;
	context->previousFocus = nullptr;

	StartupLog("Install PNG handler");
	wxImage::AddHandler(new wxPNGHandler);

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
	context->dialog = new DialogManager;
	InitContents();
	OPT_SUB("Video/Detached/Enabled", &FrameMain::OnVideoDetach, this, agi::signal::_1);

	StartupLog("Complete context initialization");
	context->videoController->SetContext(context.get());

	StartupLog("Set up drag/drop target");
	SetDropTarget(new AegisubFileDropTarget(this));

	StartupLog("Load default file");
	context->subsController->Close();

	StartupLog("Display main window");
	AddFullScreenButton(this);
	Show();
	SetDisplayMode(1, 1);

	// Version checker
	StartupLog("Possibly perform automatic updates check");
	if (OPT_GET("App/First Start")->GetBool()) {
		OPT_SET("App/First Start")->SetBool(false);
#ifdef WITH_UPDATE_CHECKER
		int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"), wxYES_NO | wxCENTER);
		OPT_SET("App/Auto/Check For Updates")->SetBool(result == wxYES);
		try {
			config::opt->Flush();
		}
		catch (agi::fs::FileSystemError const& e) {
			wxMessageBox(to_wx(e.GetMessage()), "Error saving config file", wxOK | wxICON_ERROR | wxCENTER);
		}
#endif
	}

#ifdef WITH_UPDATE_CHECKER
	PerformVersionCheck(false);
#endif

	Bind(FILE_LIST_DROPPED, &FrameMain::OnFilesDropped, this);

	StartupLog("Leaving FrameMain constructor");
}

/// @brief Delete everything but @a keep and its parents
/// @param window Root window to delete the children of
/// @param keep Window to keep alive
/// @return Was @a keep found?
static bool delete_children(wxWindow *window, wxWindow *keep) {
	bool found = false;
	while (window->GetChildren().size() > (size_t)found) {
		auto it = window->GetChildren().begin();

		if (*it == keep)
			found = true;

		if (found) {
			if (++it != window->GetChildren().end())
				(*it)->wxWindowBase::Destroy();
		}
		else if (!delete_children(*it, keep))
			(*it)->wxWindowBase::Destroy();
		else
			found = true;
	}
	return found;
}

FrameMain::~FrameMain () {
	wxGetApp().frame = nullptr;

	context->videoController->SetVideo("");
	context->audioController->CloseAudio();

	// SubsGrid needs to be deleted last due to being the selection
	// controller, but everything else needs to be deleted before the context
	// is cleaned up
	delete_children(this, SubsGrid);

	delete context->ass;
	HelpButton::ClearPages();
	delete context->audioController;
	delete context->local_scripts;
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
		SetToolBar(nullptr);
		delete old_tb;
		Layout();
	}
}

void FrameMain::InitContents() {
	StartupLog("Create background panel");
	wxPanel *Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	StartupLog("Create subtitles grid");
	context->subsGrid = SubsGrid = new BaseGrid(Panel, context.get(), wxDefaultSize, wxWANTS_CHARS | wxSUNKEN_BORDER);
	context->selectionController = context->subsGrid;
	context->search = new SearchReplaceEngine(context.get());
	context->initialLineState = new InitialLineState(context.get());

	StartupLog("Create video box");
	videoBox = new VideoBox(Panel, false, context.get());

	StartupLog("Create audio box");
	context->audioBox = audioBox = new AudioBox(Panel, context.get());

	StartupLog("Create subtitle editing box");
	auto EditBox = new SubsEditBox(Panel, context.get());

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
	StartupLog("Leaving InitContents");
}

void FrameMain::SetDisplayMode(int video, int audio) {
	if (!IsShownOnScreen()) return;

	bool sv = false, sa = false;

	if (video == -1) sv = showVideo;
	else if (video)  sv = context->videoController->IsLoaded() && !context->dialog->Get<DialogDetachedVideo>();

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
	if (context->subsController->IsModified()) newTitle << "* ";
	newTitle << context->subsController->Filename().filename().wstring();

#ifndef __WXMAC__
	newTitle << " - Aegisub " << GetAegisubLongVersionString();
#endif

#if defined(__WXMAC__) && !defined(__LP64__)
	// On Mac, set the mark in the close button
	OSXSetModified(context->subsController->IsModified());
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

	if (OPT_GET("Video/Detached/Enabled")->GetBool() && !context->dialog->Get<DialogDetachedVideo>())
		cmd::call("video/detach", context.get());
	Thaw();

	if (!blockAudioLoad && OPT_GET("Video/Open Audio")->GetBool() && context->audioController->GetAudioURL() != context->videoController->GetVideoName()) {
		try {
			context->audioController->OpenAudio(context->videoController->GetVideoName());
		}
		catch (agi::UserCancelException const&) { }
		// Opening a video with no audio data isn't an error, so just log
		// and move on
		catch (agi::fs::FileSystemError const&) {
			LOG_D("video/open/audio") << "File " << context->videoController->GetVideoName() << " found by video provider but not audio provider";
		}
		catch (agi::AudioDataNotFoundError const& e) {
			LOG_D("video/open/audio") << "File " << context->videoController->GetVideoName() << " has no audio data: " << e.GetChainedMessage();
		}
		catch (agi::AudioOpenError const& err) {
			wxMessageBox(to_wx(err.GetMessage()), "Error loading audio", wxOK | wxICON_ERROR | wxCENTER);
		}
	}
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

void FrameMain::OnFilesDropped(wxThreadEvent &evt) {
	LoadList(evt.GetPayload<wxArrayString>());
}

bool FrameMain::LoadList(wxArrayString list) {
	std::string audio, video, subs;
	get_files_to_load(list, subs, audio, video);

	blockVideoLoad = !video.empty();
	blockAudioLoad = !audio.empty();

	// Load files
	if (subs.size())
		context->subsController->Load(subs);

	if (blockVideoLoad) {
		blockVideoLoad = false;
		context->videoController->SetVideo(video);
	}

	if (blockAudioLoad) {
		blockAudioLoad = false;
		try {
			context->audioController->OpenAudio(audio);
		} catch (agi::UserCancelException const&) { }
	}

	bool loaded_any = subs.size() || audio.size() || video.size();
	if (loaded_any)
		Refresh(false);

	return loaded_any;
}

BEGIN_EVENT_TABLE(FrameMain, wxFrame)
	EVT_TIMER(ID_APP_TIMER_STATUSCLEAR, FrameMain::OnStatusClear)

	EVT_CLOSE(FrameMain::OnCloseWindow)

	EVT_CHAR_HOOK(FrameMain::OnKeyDown)
	EVT_MOUSEWHEEL(FrameMain::OnMouseWheel)
END_EVENT_TABLE()

void FrameMain::OnCloseWindow(wxCloseEvent &event) {
	wxEventBlocker blocker(this, wxEVT_CLOSE_WINDOW);

	context->videoController->Stop();
	context->audioController->Stop();

	// Ask user if he wants to save first
	if (context->subsController->TryToClose(event.CanVeto()) == wxCANCEL) {
		event.Veto();
		return;
	}

	delete context->dialog;
	context->dialog = nullptr;

	// Store maximization state
	OPT_SET("App/Maximized")->SetBool(IsMaximized());

	Destroy();
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
	auto vc = context->videoController;

	/// @todo figure out how to move this to the relevant controllers without
	///       prompting for each file loaded/unloaded

	// Load stuff from the new script
	auto video     = config::path->MakeAbsolute(context->ass->GetScriptInfo("Video File"), "?script");
	auto vfr       = config::path->MakeAbsolute(context->ass->GetScriptInfo("VFR File"), "?script");
	auto keyframes = config::path->MakeAbsolute(context->ass->GetScriptInfo("Keyframes File"), "?script");
	auto audio     = config::path->MakeAbsolute(context->ass->GetScriptInfo("Audio URI"), "?script");

	bool videoChanged     = !blockVideoLoad && video != vc->GetVideoName();
	bool timecodesChanged = vfr != vc->GetTimecodesName();
	bool keyframesChanged = keyframes != vc->GetKeyFramesName();
	bool audioChanged     = !blockAudioLoad && audio != context->audioController->GetAudioURL();

	// Check if there is anything to change
	int autoLoadMode = OPT_GET("App/Auto/Load Linked Files")->GetInt();
	if (autoLoadMode == 0 || (!videoChanged && !timecodesChanged && !keyframesChanged && !audioChanged)) {
		SetDisplayMode(1, 1);
		return;
	}

	if (autoLoadMode == 2) {
		if (wxMessageBox(_("Do you want to load/unload the associated files?"), _("(Un)Load files?"), wxYES_NO | wxCENTRE, this) != wxYES) {
			SetDisplayMode(1, 1);
			if (vc->IsLoaded() && vc->GetProvider()->GetColorSpace() != context->ass->GetScriptInfo("YCbCr Matrix"))
				vc->Reload();
			return;
		}
	}

	if (audioChanged)
		blockAudioLoad = true;

	// Video
	if (videoChanged) {
		vc->SetVideo(video);
		if (vc->IsLoaded()) {
			vc->JumpToFrame(context->ass->GetUIStateAsInt("Video Position"));

			std::string arString = context->ass->GetUIState("Video Aspect Ratio");
			if (boost::starts_with(arString, "c")) {
				double ar = 0.;
				agi::util::try_parse(arString.substr(1), &ar);
				vc->SetAspectRatio(ar);
			}
			else {
				int ar = 0;
				if (agi::util::try_parse(arString, &ar) && ar >= 0 && ar < 4)
					vc->SetAspectRatio((AspectRatio)ar);
			}

			double videoZoom = 0.;
			if (agi::util::try_parse(context->ass->GetUIState("Video Zoom Percent"), &videoZoom))
				context->videoDisplay->SetZoom(videoZoom);
		}
	}
	else if (vc->IsLoaded() && vc->GetProvider()->GetColorSpace() != context->ass->GetScriptInfo("YCbCr Matrix"))
		vc->Reload();

	vc->LoadTimecodes(vfr);
	vc->LoadKeyframes(keyframes);

	// Audio
	if (audioChanged) {
		blockAudioLoad = false;
		try {
			if (audio.empty())
				context->audioController->CloseAudio();
			else
				context->audioController->OpenAudio(audio);
		}
		catch (agi::UserCancelException const&) { }
		catch (agi::fs::FileSystemError const& err) {
			wxMessageBox(to_wx(err.GetMessage()), "Error opening audio", wxOK | wxICON_ERROR | wxCENTER, this);
		}
	}

	SetDisplayMode(1, 1);
}

void FrameMain::OnKeyDown(wxKeyEvent &event) {
	hotkey::check("Main Frame", context.get(), event);
}

void FrameMain::OnMouseWheel(wxMouseEvent &evt) {
	ForwardMouseWheelEvent(this, evt);
}
