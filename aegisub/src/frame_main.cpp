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

#include <libaegisub/log.h>

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
{
	StartupLog(_T("Entering FrameMain constructor"));
	temp_context.parent = this;

	// Bind all commands.
	// XXX: This is a hack for now, it will need to be dealt with when other frames are involved.
	int count = cmd::count();
	for (int i = 0; i < count; i++) {
		Bind(wxEVT_COMMAND_MENU_SELECTED, &FrameMain::cmd_call, this, i);
    }

#ifdef __WXMAC__
//	Bind(FrameMain::OnAbout, &FrameMain::cmd_call, this, cmd::id("app/about"));
#endif




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

	// Initialize flags
	HasSelection = false;
	menuCreated = false;
	blockVideoLoad = false;

	StartupLog(_T("Install PNG handler"));
	// Create PNG handler
	wxPNGHandler *png = new wxPNGHandler;
	wxImage::AddHandler(png);

	wxSafeYield();

	// Storage for subs-file-local scripts
#ifdef WITH_AUTOMATION
	StartupLog(_T("Create local Automation script manager"));
	local_scripts = new Automation4::ScriptManager();
	temp_context.local_scripts = local_scripts;
#endif

	// Contexts and controllers
	audioController = new AudioController;
	temp_context.audioController = audioController;
	audioController->AddAudioOpenListener(&FrameMain::OnAudioOpen, this);
	audioController->AddAudioCloseListener(&FrameMain::OnAudioClose, this);

	// Create menu and tool bars
	StartupLog(_T("Apply saved Maximized state"));
	if (OPT_GET("App/Maximized")->GetBool()) Maximize(true);
	StartupLog(_T("Initialize toolbar"));
	InitToolbar();
	StartupLog(_T("Initialize menu bar"));
	InitMenu();
	
	// Create status bar
	StartupLog(_T("Create status bar"));
	CreateStatusBar(2);

	// Set icon
	StartupLog(_T("Set icon"));
#ifdef _WIN32
	SetIcon(wxICON(wxicon));
#else
	wxIcon icon;
	icon.CopyFromBitmap(GETIMAGE(wxicon));
	SetIcon(icon);
#endif

	// Contents
	showVideo = true;
	showAudio = true;
	detachedVideo = NULL;
	stylingAssistant = NULL;
	temp_context.stylingAssistant = stylingAssistant;
	StartupLog(_T("Initialize inner main window controls"));
	InitContents();

	// Set autosave timer
	StartupLog(_T("Set up Auto Save"));
	AutoSave.SetOwner(this, ID_APP_TIMER_AUTOSAVE);
	int time = OPT_GET("App/Auto/Save Every Seconds")->GetInt();
	if (time > 0) {
		AutoSave.Start(time*1000);
	}
	OPT_SUB("App/Auto/Save Every Seconds", autosave_timer_changed, &AutoSave, agi::signal::_1);


	PreviousFocus = NULL;						// Artifact from old hotkey removal not sure what it does.
	temp_context.PreviousFocus = PreviousFocus;	// Artifact from old hotkey removal not sure what it does.

	// Set drop target
	StartupLog(_T("Set up drag/drop target"));
	SetDropTarget(new AegisubFileDropTarget(this));

	// Parse arguments
	StartupLog(_T("Initialize empty file"));
	LoadSubtitles(_T(""));
	StartupLog(_T("Load files specified on command line"));
	LoadList(args);

	// Version checker
	StartupLog(_T("Possibly perform automatic updates check"));
	if (OPT_GET("App/First Start")->GetBool()) {
		OPT_SET("App/First Start")->SetBool(false);
		int result = wxMessageBox(_("Do you want Aegisub to check for updates whenever it starts? You can still do it manually via the Help menu."),_("Check for updates?"),wxYES_NO);
		OPT_SET("App/Auto/Check For Updates")->SetBool(result == wxYES);
	}

	PerformVersionCheck(false);

	StartupLog(_T("Display main window"));
	Show();
	Freeze();
	SetDisplayMode(1, 1);
	Thaw();

	//ShowFullScreen(true,wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
	StartupLog(_T("Leaving FrameMain constructor"));
}

/// @brief FrameMain destructor 
FrameMain::~FrameMain () {
	VideoContext::Get()->SetVideo(_T(""));
	audioController->CloseAudio();
	DeInitContents();
	delete audioController;
#ifdef WITH_AUTOMATION
	delete local_scripts;
#endif
}



void FrameMain::cmd_call(wxCommandEvent& event) {
	int id = event.GetId();
	LOG_D("event/select") << "Id: " << id;
	cmd::call(&temp_context, id);
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
	AssFile::top = ass = new AssFile;
	temp_context.ass = ass;
	ass->AddCommitListener(&FrameMain::OnSubtitlesFileChanged, this);

	// Set a background panel
	StartupLog(_T("Create background panel"));
	Panel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL | wxCLIP_CHILDREN);

	// Video area;
	StartupLog(_T("Create video box"));
	videoBox = new VideoBox(Panel, false, ZoomBox, ass);
	temp_context.videoBox = videoBox;
	temp_context.videoContext = VideoContext::Get();
	temp_context.videoContext->audio = audioController;
	wxBoxSizer *videoSizer = new wxBoxSizer(wxVERTICAL);
	videoSizer->Add(videoBox, 0, wxEXPAND);
	videoSizer->AddStretchSpacer(1);

	// Subtitles area
	StartupLog(_T("Create subtitles grid"));
	SubsGrid = new SubtitlesGrid(this,Panel,-1,ass,wxDefaultPosition,wxSize(600,100),wxWANTS_CHARS | wxSUNKEN_BORDER,_T("Subs grid"));
	temp_context.SubsGrid = SubsGrid;
	videoBox->videoSlider->grid = SubsGrid;
	temp_context.videoContext->grid = SubsGrid;
	Search.grid = SubsGrid;

	// Tools area
	StartupLog(_T("Create tool area splitter window"));
	audioSash = new wxSashWindow(Panel, ID_SASH_MAIN_AUDIO, wxDefaultPosition, wxDefaultSize, wxSW_3D|wxCLIP_CHILDREN);
	wxBoxSizer *audioSashSizer = new wxBoxSizer(wxHORIZONTAL);
	audioSash->SetSashVisible(wxSASH_BOTTOM, true);

	// Audio area
	StartupLog(_T("Create audio box"));
	audioBox = new AudioBox(audioSash, audioController, SubsGrid, ass);
	temp_context.audioBox = audioBox;
	audioBox->frameMain = this;
	audioSashSizer->Add(audioBox, 1, wxEXPAND);
	audioSash->SetSizer(audioSashSizer);
	audioBox->Fit();
	audioSash->SetMinimumSizeY(audioBox->GetSize().GetHeight());

	// Editing area
	StartupLog(_T("Create subtitle editing box"));
	EditBox = new SubsEditBox(Panel,SubsGrid);
	temp_context.EditBox = EditBox;

	// Set sizers/hints
	StartupLog(_T("Arrange main sizers"));
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

	// Set display
	StartupLog(_T("Perform layout"));
	Layout();
	StartupLog(_T("Set focus to edting box"));
	EditBox->TextEdit->SetFocus();
	StartupLog(_T("Leaving InitContents"));
}

/// @brief Deinitialize controls 
void FrameMain::DeInitContents() {
	if (detachedVideo) detachedVideo->Destroy();
	if (stylingAssistant) stylingAssistant->Destroy();
	SubsGrid->ClearMaps();
	delete audioBox;
	delete EditBox;
	delete videoBox;
	delete ass;
	HelpButton::ClearPages();
	VideoContext::Get()->audio = NULL;
}

/// @brief Update toolbar 
void FrameMain::UpdateToolbar() {
	// Collect flags
	bool isVideo = VideoContext::Get()->IsLoaded();
	HasSelection = true;
	int selRows = SubsGrid->GetNumberSelection();

	// Update
	wxToolBar* toolbar = GetToolBar();
	toolbar->FindById(cmd::id("video/jump"))->Enable(isVideo);
	toolbar->FindById(cmd::id("video/zoom/in"))->Enable(isVideo && !detachedVideo);
	toolbar->FindById(cmd::id("video/zoom/out"))->Enable(isVideo && !detachedVideo);
	ZoomBox->Enable(isVideo && !detachedVideo);

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
void FrameMain::LoadSubtitles (wxString filename,wxString charset) {
	// First check if there is some loaded
	if (ass && ass->loaded) {
		if (TryToCloseSubs() == wxCANCEL) return;
	}

	// Setup
	bool isFile = !filename.empty();

	// Load
	try {
		// File exists?
		if (isFile) {
			wxFileName fileCheck(filename);
			if (!fileCheck.FileExists()) {
				throw agi::FileNotFoundError(STD_STR(filename));
			}

			// Make sure that file isn't actually a timecode file
			try {
				TextFileReader testSubs(filename,charset);
				wxString cur = testSubs.ReadLineFromFile();
				if (cur.Left(10) == _T("# timecode")) {
					LoadVFR(filename);
					OPT_SET("Path/Last/Timecodes")->SetString(STD_STR(fileCheck.GetPath()));
					return;
				}
			}
			catch (...) {
				// if trying to load the file as timecodes fails it's fairly
				// safe to assume that it is in fact not a timecode file
			}
		}

		// Proceed into loading
		SubsGrid->ClearMaps();
		if (isFile) {
			ass->Load(filename,charset);
			if (SubsGrid->GetRows()) {
				SubsGrid->SetActiveLine(SubsGrid->GetDialogue(0));
				SubsGrid->SelectRow(0);
			}
			wxFileName fn(filename);
			StandardPaths::SetPathValue(_T("?script"),fn.GetPath());
			OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(fn.GetPath()));
		}
		else {
			SubsGrid->LoadDefault();
			StandardPaths::SetPathValue(_T("?script"),_T(""));
		}
		SubsGrid->SetColumnWidths();
	}
	catch (agi::FileNotFoundError const&) {
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

	// Save copy
	wxFileName origfile(filename);
	if (ass->CanSave() && OPT_GET("App/Auto/Backup")->GetBool() && origfile.FileExists()) {
		// Get path
		wxString path = lagi_wxString(OPT_GET("Path/Auto/Backup")->GetString());
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

/// @brief Save subtitles 
/// @param saveas      
/// @param withCharset 
/// @return 
bool FrameMain::SaveSubtitles(bool saveas,bool withCharset) {
	// Try to get filename from file
	wxString filename;
	if (saveas == false && ass->CanSave()) filename = ass->filename;

	// Failed, ask user
	if (filename.IsEmpty()) {
		VideoContext::Get()->Stop();
		wxString path = lagi_wxString(OPT_GET("Path/Last/Subtitles")->GetString());
		wxFileName origPath(ass->filename);
		filename =  wxFileSelector(_("Save subtitles file"),path,origPath.GetName() + _T(".ass"),_T("ass"),AssFile::GetWildcardList(1),wxFD_SAVE | wxFD_OVERWRITE_PROMPT,this);
	}

	// Actually save
	if (!filename.empty()) {
		// Store path
		wxFileName filepath(filename);
		OPT_SET("Path/Last/Subtitles")->SetString(STD_STR(filepath.GetPath()));

		// Fix me, ghetto hack for correct relative path generation in SynchronizeProject()
		ass->filename = filename;

		// Synchronize
		SynchronizeProject();

		// Get charset
		wxString charset = _T("");
		if (withCharset) {
			charset = wxGetSingleChoice(_("Choose charset code:"), _T("Charset"),agi::charset::GetEncodingsList<wxArrayString>(),this,-1, -1,true,250,200);
			if (charset.IsEmpty()) return false;
		}

		// Save
		try {
			ass->Save(filename,true,true,charset);
			UpdateTitle();
		}
		catch (const agi::Exception& err) {
			wxMessageBox(lagi_wxString(err.GetMessage()), "Error", wxOK | wxICON_ERROR, NULL);
			return false;
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

/// @brief Try to close subtitles 
/// @param enableCancel 
/// @return 
int FrameMain::TryToCloseSubs(bool enableCancel) {
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

/// @brief Set the video and audio display visibilty
/// @param video -1: leave unchanged; 0: hide; 1: show
/// @param audio -1: leave unchanged; 0: hide; 1: show
void FrameMain::SetDisplayMode(int video, int audio) {
	if (!IsShownOnScreen()) return;

	bool sv = false, sa = false;

	if (video == -1) sv = showVideo;
	else if (video)  sv = VideoContext::Get()->IsLoaded() && !detachedVideo;

	if (audio == -1) sa = showAudio;
	else if (audio)  sa = audioController->IsAudioOpen();

	// See if anything changed
	if (sv == showVideo && sa == showAudio) return;

	showVideo = sv;
	showAudio = sa;

	bool didFreeze = !IsFrozen();
	if (didFreeze) Freeze();

	VideoContext::Get()->Stop();

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
	// Determine if current subs are modified
	bool subsMod = ass->IsModified();
	
	// Create ideal title
	wxString newTitle = _T("");
#ifndef __WXMAC__
	if (subsMod) newTitle << _T("* ");
	if (ass->filename != _T("")) {
		wxFileName file (ass->filename);
		newTitle << file.GetFullName();
	}
	else newTitle << _("Untitled");
	newTitle << _T(" - Aegisub ") << GetAegisubLongVersionString();
#else
	// Apple HIG says "untitled" should not be capitalised
	// and the window is a document window, it shouldn't contain the app name
	// (The app name is already present in the menu bar)
	if (ass->filename != _T("")) {
		wxFileName file (ass->filename);
		newTitle << file.GetFullName();
	}
	else newTitle << _("untitled");
#endif

#if defined(__WXMAC__) && !defined(__LP64__)
	// On Mac, set the mark in the close button
	OSXSetModified(subsMod);
#endif

	// Get current title
	wxString curTitle = GetTitle();
	if (curTitle != newTitle) SetTitle(newTitle);
}

/// @brief Updates subs with video/whatever data 
/// @param fromSubs 
void FrameMain::SynchronizeProject(bool fromSubs) {
	// Retrieve data from subs
	if (fromSubs) {
		// Reset the state
		long videoPos = 0;
		long videoAr = 0;
		double videoArValue = 0.0;
		double videoZoom = 0.;

		// Get AR
		wxString arString = ass->GetScriptInfo(_T("Video Aspect Ratio"));
		if (arString.Left(1) == _T("c")) {
			videoAr = 4;
			arString = arString.Mid(1);
			arString.ToDouble(&videoArValue);
		}
		else if (arString.IsNumber()) arString.ToLong(&videoAr);

		// Get new state info
		ass->GetScriptInfo(_T("Video Position")).ToLong(&videoPos);
		ass->GetScriptInfo(_T("Video Zoom Percent")).ToDouble(&videoZoom);
		wxString curSubsVideo = DecodeRelativePath(ass->GetScriptInfo(_T("Video File")),ass->filename);
		wxString curSubsVFR = DecodeRelativePath(ass->GetScriptInfo(_T("VFR File")),ass->filename);
		wxString curSubsKeyframes = DecodeRelativePath(ass->GetScriptInfo(_T("Keyframes File")),ass->filename);
		wxString curSubsAudio = DecodeRelativePath(ass->GetScriptInfo(_T("Audio URI")),ass->filename);
		wxString AutoScriptString = ass->GetScriptInfo(_T("Automation Scripts"));

		// Check if there is anything to change
		int autoLoadMode = OPT_GET("App/Auto/Load Linked Files")->GetInt();
		bool hasToLoad = false;
		if (curSubsAudio !=audioController->GetAudioURL() ||
			curSubsVFR != VideoContext::Get()->GetTimecodesName() ||
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
			// Video
			if (curSubsVideo != VideoContext::Get()->videoName) {
				LoadVideo(curSubsVideo);
				if (VideoContext::Get()->IsLoaded()) {
					VideoContext::Get()->SetAspectRatio(videoAr,videoArValue);
					videoBox->videoDisplay->SetZoom(videoZoom);
					VideoContext::Get()->JumpToFrame(videoPos);
				}
			}

			VideoContext::Get()->LoadTimecodes(curSubsVFR);
			VideoContext::Get()->LoadKeyframes(curSubsKeyframes);

			// Audio
			if (curSubsAudio != audioController->GetAudioURL()) {
				audioController->OpenAudio(curSubsAudio);
			}

			// Automation scripts
#ifdef WITH_AUTOMATION
			local_scripts->RemoveAll();
			wxStringTokenizer tok(AutoScriptString, _T("|"), wxTOKEN_STRTOK);
			wxFileName assfn(ass->filename);
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
					local_scripts->Add(Automation4::ScriptFactory::CreateFromFile(sfnames, true));
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

	// Store data on ass
	else {
		// Setup
		wxString seekpos = _T("0");
		wxString ar = _T("0");
		wxString zoom = _T("6");
		if (VideoContext::Get()->IsLoaded()) {
			seekpos = wxString::Format(_T("%i"),VideoContext::Get()->GetFrameN());
			zoom = wxString::Format(_T("%f"),videoBox->videoDisplay->GetZoom());

			int arType = VideoContext::Get()->GetAspectRatioType();
			if (arType == 4) ar = wxString(_T("c")) + AegiFloatToString(VideoContext::Get()->GetAspectRatioValue());
			else ar = wxString::Format(_T("%i"),arType);
		}
		
		// Store audio data
		ass->SetScriptInfo(_T("Audio URI"),MakeRelativePath(audioController->GetAudioURL(),ass->filename));

		// Store video data
		ass->SetScriptInfo(_T("Video File"),MakeRelativePath(VideoContext::Get()->videoName,ass->filename));
		ass->SetScriptInfo(_T("Video Aspect Ratio"),ar);
		ass->SetScriptInfo(_T("Video Zoom Percent"),zoom);
		ass->SetScriptInfo(_T("Video Position"),seekpos);
		ass->SetScriptInfo(_T("VFR File"),MakeRelativePath(VideoContext::Get()->GetTimecodesName(),ass->filename));
		ass->SetScriptInfo(_T("Keyframes File"),MakeRelativePath(VideoContext::Get()->GetKeyFramesName(),ass->filename));

		// Store Automation script data
		// Algorithm:
		// 1. If script filename has Automation Base Path as a prefix, the path is relative to that (ie. "$")
		// 2. Otherwise try making it relative to the ass filename
		// 3. If step 2 failed, or absolut path is shorter than path relative to ass, use absolute path ("/")
		// 4. Otherwise, use path relative to ass ("~")
#ifdef WITH_AUTOMATION
		wxString scripts_string;
		wxString autobasefn(lagi_wxString(OPT_GET("Path/Automation/Base")->GetString()));

		const std::vector<Automation4::Script*> &scripts = local_scripts->GetScripts();
		for (unsigned int i = 0; i < scripts.size(); i++) {
			Automation4::Script *script = scripts[i];

			if (i != 0)
				scripts_string += _T("|");

			wxString autobase_rel, assfile_rel;
			wxString scriptfn(script->GetFilename());
			autobase_rel = MakeRelativePath(scriptfn, autobasefn);
			assfile_rel = MakeRelativePath(scriptfn, ass->filename);

			if (autobase_rel.size() <= scriptfn.size() && autobase_rel.size() <= assfile_rel.size()) {
				scriptfn = _T("$") + autobase_rel;
			} else if (assfile_rel.size() <= scriptfn.size() && assfile_rel.size() <= autobase_rel.size()) {
				scriptfn = _T("~") + assfile_rel;
			} else {
				scriptfn = _T("/") + wxFileName(scriptfn).GetFullPath(wxPATH_UNIX);
			}

			scripts_string += scriptfn;
		}
		ass->SetScriptInfo(_T("Automation Scripts"), scripts_string);
#endif
	}
}

/// @brief Loads video 
/// @param file     
/// @param autoload 
void FrameMain::LoadVideo(wxString file,bool autoload) {
	if (blockVideoLoad) return;
	Freeze();
	try {
		VideoContext::Get()->SetVideo(file);
	}
	catch (const wchar_t *error) {
		wxMessageBox(error, _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}
	catch (...) {
		wxMessageBox(_T("Unknown error"), _T("Error opening video file"), wxOK | wxICON_ERROR, this);
	}

	if (VideoContext::Get()->IsLoaded()) {
		int vidx = VideoContext::Get()->GetWidth(), vidy = VideoContext::Get()->GetHeight();

		// Set zoom level based on video resolution and window size
		double zoom = videoBox->videoDisplay->GetZoom();
		wxSize windowSize = GetSize();
		if (vidx*3*zoom > windowSize.GetX()*4 || vidy*4*zoom > windowSize.GetY()*6)
			videoBox->videoDisplay->SetZoom(zoom * .25);
		else if (vidx*3*zoom > windowSize.GetX()*2 || vidy*4*zoom > windowSize.GetY()*3)
			videoBox->videoDisplay->SetZoom(zoom * .5);

		// Check that the video size matches the script video size specified
		int scriptx = SubsGrid->ass->GetScriptInfoAsInt(_T("PlayResX"));
		int scripty = SubsGrid->ass->GetScriptInfoAsInt(_T("PlayResY"));
		if (scriptx != vidx || scripty != vidy) {
			switch (OPT_GET("Video/Check Script Res")->GetInt()) {
				case 1:
					// Ask to change on mismatch
					if (wxMessageBox(wxString::Format(_("The resolution of the loaded video and the resolution specified for the subtitles don't match.\n\nVideo resolution:\t%d x %d\nScript resolution:\t%d x %d\n\nChange subtitles resolution to match video?"), vidx, vidy, scriptx, scripty), _("Resolution mismatch"), wxYES_NO, this) != wxYES)
						break;
					// Fallthrough to case 2
				case 2:
					// Always change script res
					SubsGrid->ass->SetScriptInfo(_T("PlayResX"), wxString::Format(_T("%d"), vidx));
					SubsGrid->ass->SetScriptInfo(_T("PlayResY"), wxString::Format(_T("%d"), vidy));
					SubsGrid->ass->Commit(_("Change script resolution"));
					break;
				case 0:
				default:
					// Never change
					break;
			}
		}
	}

	SetDisplayMode(1,-1);

	DetachVideo(VideoContext::Get()->IsLoaded() && OPT_GET("Video/Detached/Enabled")->GetBool());
	Thaw();
}

void FrameMain::LoadVFR(wxString filename) {
	if (filename.empty()) {
		VideoContext::Get()->CloseTimecodes();
	}
	else {
		VideoContext::Get()->LoadTimecodes(filename);
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
		if (!detachedVideo) {
			detachedVideo = new DialogDetachedVideo(this, videoBox->videoDisplay->GetClientSize());
			temp_context.detachedVideo = detachedVideo;
			detachedVideo->Show();
		}
	}
	else if (detachedVideo) {
		detachedVideo->Destroy();
		detachedVideo = NULL;
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
	blockVideoLoad = (video != _T(""));

	// Load files
	if (subs != _T("")) {
		LoadSubtitles(subs);
	}
	if (blockVideoLoad) {
		blockVideoLoad = false;
		LoadVideo(video);
	}
	if (!audio.IsEmpty())
		audioController->OpenAudio(audio);

	// Result
	return ((subs != _T("")) || (audio != _T("")) || (video != _T("")));
}


/// @brief Sets the descriptions for undo/redo 
void FrameMain::SetUndoRedoDesc() {
	wxMenu *editMenu = menu::menu->GetMenu("main/edit");
	editMenu->SetHelpString(0,_T("Undo ")+ass->GetUndoDescription());
	editMenu->SetHelpString(1,_T("Redo ")+ass->GetRedoDescription());
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
void FrameMain::RebuildRecentList(wxString listName,wxMenu *menu,int startID) {
	// Wipe previous list
	int count = (int)menu->GetMenuItemCount();
	for (int i=count;--i>=0;) {
		menu->Destroy(menu->FindItemByPosition(i));
	}

	// Rebuild
	int added = 0;
	wxString n;
	wxArrayString entries = lagi_MRU_wxAS(listName);
	for (size_t i=0;i<entries.Count();i++) {
		n = wxString::Format(_T("%ld"),i+1);
		if (i < 9) n = _T("&") + n;
		wxFileName shortname(entries[i]);
		wxString filename = shortname.GetFullName();
		menu->Append(startID+i,n + _T(" ") + filename);
		added++;
	}

	// Nothing added, add an empty placeholder
	if (added == 0) menu->Append(startID,_("Empty"))->Enable(false);
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
		RebuildRecentList(_T("Subtitle"),menu::menu->GetMenu("recent/subtitle"), cmd::id("recent/subtitle"));

		MenuBar->Enable(cmd::id("subtitle/open/video"),VideoContext::Get()->HasSubtitles());
	}

	// View menu
	else if (curMenu == menu::menu->GetMenu("main/view")) {
		// Flags
		bool aud = audioController->IsAudioOpen();
		bool vid = VideoContext::Get()->IsLoaded() && !detachedVideo;

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
		bool state = VideoContext::Get()->IsLoaded();
		bool attached = state && !detachedVideo;

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
		MenuBar->Enable(cmd::id("timecode/save"),VideoContext::Get()->TimecodesLoaded());
		MenuBar->Enable(cmd::id("timecode/close"),VideoContext::Get()->OverTimecodesLoaded());
		MenuBar->Enable(cmd::id("keyframe/close"),VideoContext::Get()->OverKeyFramesLoaded());
		MenuBar->Enable(cmd::id("keyframe/save"),VideoContext::Get()->KeyFramesLoaded());
		MenuBar->Enable(cmd::id("video/details"),state);
		MenuBar->Enable(cmd::id("video/show_overscan"),state);

		// Set AR radio
		int arType = VideoContext::Get()->GetAspectRatioType();
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
		RebuildRecentList(_T("Video"),menu::menu->GetMenu("recent/video"), cmd::id("recent/video"));
		RebuildRecentList(_T("Timecodes"),menu::menu->GetMenu("recent/timecode"), cmd::id("recent/timecode"));
		RebuildRecentList(_T("Keyframes"),menu::menu->GetMenu("recent/keyframe"), cmd::id("recent/keyframe"));
	}

	// Audio menu
	else if (curMenu == menu::menu->GetMenu("main/audio")) {
		bool state = audioController->IsAudioOpen();
		bool vidstate = VideoContext::Get()->IsLoaded();

		MenuBar->Enable(cmd::id("audio/open/video"),vidstate);
		MenuBar->Enable(cmd::id("audio/close"),state);

		// Rebuild recent
		RebuildRecentList(_T("Audio"),menu::menu->GetMenu("recent/audio"), cmd::id("recent/audio"));
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
		state2 = count > 0 && VideoContext::Get()->IsLoaded();
		MenuBar->Enable(cmd::id("subtitle/insert/before/videotime"),state2);
		MenuBar->Enable(cmd::id("subtitle/insert/after/videotime"),state2);
		MenuBar->Enable(cmd::id("main/subtitle/insert lines"),state);
		state = count > 0 && continuous;
		MenuBar->Enable(cmd::id("edit/line/duplicate"),state);
		state = count > 0 && continuous && VideoContext::Get()->TimecodesLoaded();
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
		bool state = VideoContext::Get()->IsLoaded();
		MenuBar->Enable(cmd::id("time/snap/start_video"),state);
		MenuBar->Enable(cmd::id("time/snap/end_video"),state);
		MenuBar->Enable(cmd::id("time/snap/scene"),state);
		MenuBar->Enable(cmd::id("time/frame/current"),state);

		// Other
		state = count >= 2 && continuous;
		MenuBar->Enable(cmd::id("time/continous/start"),state);
		MenuBar->Enable(cmd::id("time/continous/end"),state);
	}

	// Edit menu
	else if (curMenu == menu::menu->GetMenu("main/edit")) {
		wxMenu *editMenu = menu::menu->GetMenu("main/edit");

		// Undo state
		wxMenuItem *item;
//H		wxString undo_text = _("&Undo") + wxString(_T(" ")) + ass->GetUndoDescription() + wxString(_T("\t")) + Hotkeys.GetText(_T("Undo"));
// The bottom line needs to be fixed for the new hotkey system
		wxString undo_text = _("&Undo") + wxString(_T(" ")) + ass->GetUndoDescription() + wxString(_T("\t")) + _T("Undo");
		item = editMenu->FindItem(cmd::id("edit/undo"));
		item->SetItemLabel(undo_text);
		item->Enable(!ass->IsUndoStackEmpty());

		// Redo state
//H		wxString redo_text = _("&Redo") + wxString(_T(" ")) + ass->GetRedoDescription() + wxString(_T("\t")) + Hotkeys.GetText(_T("Redo"));
// Same as above.
		wxString redo_text = _("&Redo") + wxString(_T(" ")) + ass->GetRedoDescription() + wxString(_T("\t")) + _T("Redo");
		item = editMenu->FindItem(cmd::id("edit/redo"));
		item->SetItemLabel(redo_text);
		item->Enable(!ass->IsRedoStackEmpty());

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
		added += AddMacroMenuItems(automationMenu, local_scripts->GetMacros());

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
		m->Enable((*i)->Validate(SubsGrid->ass, SubsGrid->GetAbsoluteSelection(), SubsGrid->GetFirstSelRow()));
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
	activeMacroItems[event.GetId()-ID_MENU_AUTOMATION_MACRO]->Process(SubsGrid->ass, selected_lines, first_sel, this);
	SubsGrid->SetSelectionFromAbsolute(selected_lines);
	SubsGrid->EndBatch();
#endif
}


/// @brief Window is attempted to be closed
/// @param event
void FrameMain::OnCloseWindow (wxCloseEvent &event) {
	// Stop audio and video
	VideoContext::Get()->Stop();
	audioController->Stop();

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
		if (ass->loaded && ass->IsModified()) {
			// Set path
			wxFileName origfile(ass->filename);
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

			ass->Save(dstpath.GetFullPath(),false,false);

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

void FrameMain::OnSubtitlesFileChanged() {
	if (OPT_GET("App/Auto/Save on Every Change")->GetBool()) {
		if (ass->IsModified() && !ass->filename.empty()) SaveSubtitles(false);
	}

	UpdateTitle();
}

void FrameMain::OnKeyDown(wxKeyEvent &event) {
	hotkey::check("Main Frame", event.GetKeyCode(), event.GetUnicodeKey(), event.GetModifiers());
}

