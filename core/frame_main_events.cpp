// Copyright (c) 2005, Rodrigo Braz Monteiro
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
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/tglbtn.h>
#include <wx/rawbmp.h>
#include "subs_grid.h"
#include "frame_main.h"
#include "video_display.h"
#include "video_slider.h"
#include "video_zoom.h"
#include "video_box.h"
#include "ass_file.h"
#include "dialog_style_manager.h"
#include "dialog_translation.h"
#include "dialog_jumpto.h"
#include "dialog_shift_times.h"
#include "dialog_search_replace.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "dialog_properties.h"
#include "main.h"
#include "fonts_collector.h"
#include "about.h"
#include "automation_gui.h"
#include "dialog_export.h"
#include "audio_box.h"
#include "aspell_wrap.h"
#include "dialog_spellcheck.h"
#include "dialog_selection.h"
#include "dialog_styling_assistant.h"
#include "dialog_resample.h"
#include "audio_display.h"
#include "toggle_bitmap.h"
#include "dialog_hotkeys.h"
#include "dialog_timing_processor.h"
#ifndef NO_FEX
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#include "dialog_fextracker.h"
#endif
#include "dialog_progress.h"
#include "dialog_options.h"
#include "utils.h"


////////////////////
// Menu event table
BEGIN_EVENT_TABLE(FrameMain, wxFrame)
	EVT_TIMER(AutoSave_Timer, FrameMain::OnAutoSave)
	EVT_TIMER(StatusClear_Timer, FrameMain::OnStatusClear)

	EVT_BUTTON(Video_Play, FrameMain::OnVideoPlay)
	EVT_BUTTON(Video_Play_Line, FrameMain::OnVideoPlayLine)
	EVT_BUTTON(Video_Stop, FrameMain::OnVideoStop)
	EVT_TOGGLEBUTTON(Video_Auto_Scroll, FrameMain::OnVideoToggleScroll)

#ifndef NO_FEX
	EVT_BUTTON(Video_Tracker_Menu, FrameMain::OnVideoTrackerMenu)
	EVT_MENU(Video_Track_Points, FrameMain::OnVideoTrackPoints)
	EVT_MENU(Video_Track_Point_Add, FrameMain::OnVideoTrackPointAdd)
	EVT_MENU(Video_Track_Point_Del, FrameMain::OnVideoTrackPointDel)
	EVT_MENU(Video_Track_Movement, FrameMain::OnVideoTrackMovement)
	EVT_BUTTON(Video_Tracker_Menu2, FrameMain::OnVideoTrackerMenu2)
	EVT_MENU(Video_Track_Movement_MoveAll, FrameMain::OnVideoTrackMovementMoveAll)
	EVT_MENU(Video_Track_Movement_MoveOne, FrameMain::OnVideoTrackMovementMoveOne)
	EVT_MENU(Video_Track_Movement_MoveBefore, FrameMain::OnVideoTrackMovementMoveBefore)
	EVT_MENU(Video_Track_Movement_MoveAfter, FrameMain::OnVideoTrackMovementMoveAfter)
	EVT_MENU(Video_Track_Split_Line, FrameMain::OnVideoTrackSplitLine)
	EVT_MENU(Video_Track_Link_File, FrameMain::OnVideoTrackLinkFile)
	EVT_MENU(Video_Track_Movement_Empty, FrameMain::OnVideoTrackMovementEmpty)
#endif

	EVT_CLOSE(FrameMain::OnCloseWindow)

	EVT_KEY_DOWN(FrameMain::OnKeyDown)

	EVT_MENU_OPEN(FrameMain::OnMenuOpen)
	EVT_MENU_RANGE(Menu_File_Recent,Menu_File_Recent+100, FrameMain::OnOpenRecentSubs)
	EVT_MENU_RANGE(Menu_Video_Recent,Menu_Video_Recent+100, FrameMain::OnOpenRecentVideo)
	EVT_MENU_RANGE(Menu_Audio_Recent,Menu_Audio_Recent+100, FrameMain::OnOpenRecentAudio)

	EVT_MENU(Menu_File_Open, FrameMain::OnOpenProject)
	EVT_MENU(Menu_File_Save, FrameMain::OnSaveProject)
	EVT_MENU(Menu_File_SaveAs, FrameMain::OnSaveProjectAs)
	EVT_MENU(Menu_File_Exit, FrameMain::OnExit)
	EVT_MENU(Menu_File_Open_Video, FrameMain::OnOpenVideo)
	EVT_MENU(Menu_File_Close_Video, FrameMain::OnCloseVideo)
	EVT_MENU(Menu_File_Open_Subtitles, FrameMain::OnOpenSubtitles)
	EVT_MENU(Menu_File_Open_Subtitles_Charset, FrameMain::OnOpenSubtitlesCharset)
	EVT_MENU(Menu_File_New_Subtitles, FrameMain::OnNewSubtitles)
	EVT_MENU(Menu_File_Save_Subtitles, FrameMain::OnSaveSubtitles)
	EVT_MENU(Menu_File_Save_Subtitles_As, FrameMain::OnSaveSubtitlesAs)
	EVT_MENU(Menu_File_Save_Subtitles_With_Charset, FrameMain::OnSaveSubtitlesCharset)
	EVT_MENU(Menu_File_Export_Subtitles, FrameMain::OnExportSubtitles)
	EVT_MENU(Menu_File_Open_VFR, FrameMain::OnOpenVFR)
	EVT_MENU(Menu_File_Close_VFR, FrameMain::OnCloseVFR)

	EVT_MENU(Menu_View_Zoom_50, FrameMain::OnSetZoom50)
	EVT_MENU(Menu_View_Zoom_100, FrameMain::OnSetZoom100)
	EVT_MENU(Menu_View_Zoom_200, FrameMain::OnSetZoom200)
	EVT_COMBOBOX(Toolbar_Zoom_Dropdown, FrameMain::OnSetZoom)
	EVT_MENU(Menu_Video_Zoom_In, FrameMain::OnZoomIn)
	EVT_MENU(Menu_Video_Zoom_Out, FrameMain::OnZoomOut)
	EVT_MENU(Menu_Video_AR_Default, FrameMain::OnSetARDefault)
	EVT_MENU(Menu_Video_AR_Full, FrameMain::OnSetARFull)
	EVT_MENU(Menu_Video_AR_Wide, FrameMain::OnSetARWide)
	EVT_MENU(Menu_Video_AR_235, FrameMain::OnSetAR235)
	EVT_MENU(Menu_Video_AR_Custom, FrameMain::OnSetARCustom)
	EVT_MENU(Menu_Video_JumpTo, FrameMain::OnJumpTo)
	EVT_MENU(Menu_Video_Select_Visible, FrameMain::OnSelectVisible)

	EVT_MENU(Menu_Audio_Open_File, FrameMain::OnOpenAudio)
	EVT_MENU(Menu_Audio_Open_From_Video, FrameMain::OnOpenAudioFromVideo)
	EVT_MENU(Menu_Audio_Close, FrameMain::OnCloseAudio)	

	EVT_MENU(Menu_Edit_Undo, FrameMain::OnUndo)
	EVT_MENU(Menu_Edit_Redo, FrameMain::OnRedo)
	EVT_MENU(Menu_Edit_Cut, FrameMain::OnCut)
	EVT_MENU(Menu_Edit_Copy, FrameMain::OnCopy)
	EVT_MENU(Menu_Edit_Paste, FrameMain::OnPaste)
	EVT_MENU(Menu_Edit_Find, FrameMain::OnFind)
	EVT_MENU(Menu_Edit_Find_Next, FrameMain::OnFindNext)
	EVT_MENU(Menu_Edit_Replace, FrameMain::OnReplace)
	EVT_MENU(Menu_Edit_Shift, FrameMain::OnShift)
	EVT_MENU(Menu_Edit_Select, FrameMain::OnSelect)

	EVT_MENU(Menu_Tools_Properties, FrameMain::OnOpenProperties)
	EVT_MENU(Menu_Tools_Styles_Manager, FrameMain::OnOpenStylesManager)
	EVT_MENU(Menu_Tools_Translation, FrameMain::OnOpenTranslation)
	EVT_MENU(Menu_Tools_SpellCheck, FrameMain::OnOpenSpellCheck)
	EVT_MENU(Menu_Tools_Fonts_Collector, FrameMain::OnOpenFontsCollector)
	EVT_MENU(Menu_Tools_Automation, FrameMain::OnOpenAutomation)
	EVT_MENU(Menu_Tools_Styling, FrameMain::OnOpenStylingAssistant)
	EVT_MENU(Menu_Tools_Resample, FrameMain::OnOpenResample)
	EVT_MENU(Menu_Tools_Timing_Processor, FrameMain::OnOpenTimingProcessor)
	EVT_MENU(Menu_Tools_Hotkeys, FrameMain::OnOpenHotkeys)
	EVT_MENU(Menu_Tools_Options, FrameMain::OnOpenOptions)
	
	EVT_MENU(Menu_Subs_Snap_Start_To_Video, FrameMain::OnSnapSubsStartToVid)
	EVT_MENU(Menu_Subs_Snap_End_To_Video, FrameMain::OnSnapSubsEndToVid)
	EVT_MENU(Menu_Subs_Snap_Video_To_Start, FrameMain::OnSnapVidToSubsStart)
	EVT_MENU(Menu_Subs_Snap_Video_To_End, FrameMain::OnSnapVidToSubsEnd)
	EVT_MENU(Menu_Video_Snap_To_Scene, FrameMain::OnSnapToScene)
	EVT_MENU(Menu_Video_Shift_To_Frame, FrameMain::OnShiftToFrame)

	EVT_MENU(Menu_Help_Contents, FrameMain::OnContents)
	EVT_MENU(Menu_Help_Website, FrameMain::OnWebsite)
	EVT_MENU(Menu_Help_Forums, FrameMain::OnForums)
	EVT_MENU(Menu_Help_BugTracker, FrameMain::OnBugTracker)
	EVT_MENU(Menu_Help_IRCChannel, FrameMain::OnIRCChannel)
	EVT_MENU(Menu_Help_About, FrameMain::OnAbout)

	EVT_MENU(Menu_View_Language, FrameMain::OnChooseLanguage)
	EVT_MENU(Menu_View_Standard, FrameMain::OnViewStandard)
	EVT_MENU(Menu_View_Audio, FrameMain::OnViewAudio)
	EVT_MENU(Menu_View_Video, FrameMain::OnViewVideo)
	EVT_MENU(Menu_View_Subs, FrameMain::OnViewSubs)

	EVT_MENU(Video_Prev_Frame,FrameMain::OnPrevFrame)
	EVT_MENU(Video_Next_Frame,FrameMain::OnNextFrame)
	EVT_MENU(Video_Focus_Seek,FrameMain::OnFocusSeek)
	EVT_MENU(Grid_Next_Line,FrameMain::OnNextLine)
	EVT_MENU(Grid_Prev_Line,FrameMain::OnPrevLine)
	EVT_MENU(Grid_Toggle_Tags,FrameMain::OnToggleTags)

	EVT_MENU(Kana_Game, FrameMain::OnKanaGame)
END_EVENT_TABLE()


////////////////////////
// Menu is being opened
void FrameMain::OnMenuOpen (wxMenuEvent &event) {
	// Get menu
	//Freeze();
	wxMenu *curMenu = event.GetMenu();

	// File menu
	if (curMenu == fileMenu) {
		// Wipe recent
		int count = RecentSubs->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentSubs->Destroy(RecentSubs->FindItemByPosition(i));
		}

		// Rebuild recent
		int added = 0;
		wxString n;
		wxArrayString entries = Options.GetRecentList(_T("Recent sub"));
		for (size_t i=0;i<entries.Count();i++) {
			n = wxString::Format(_T("%i"),i+1);
			if (i < 9) n = _T("&") + n;
			wxFileName shortname(entries[i]);
			wxString filename = shortname.GetFullName();
			RecentSubs->Append(Menu_File_Recent+i,n + _T(" ") + filename);
			added++;
		}
		if (added == 0) RecentSubs->Append(Menu_File_Recent,_T("Empty"))->Enable(false);
	}

	// View menu
	else if (curMenu == viewMenu) {
		// Flags
		bool aud = audioBox->audioDisplay->loaded;
		bool vid = videoBox->videoDisplay->loaded;

		// Set states
		MenuBar->Enable(Menu_View_Audio,aud);
		MenuBar->Enable(Menu_View_Video,vid);
		MenuBar->Enable(Menu_View_Standard,aud && vid);

		// Select option
		if (curMode == 0) MenuBar->Check(Menu_View_Subs,true);
		if (curMode == 1) MenuBar->Check(Menu_View_Video,true);
		if (curMode == 2) MenuBar->Check(Menu_View_Standard,true);
		if (curMode == 3) MenuBar->Check(Menu_View_Audio,true);
	}

	// Video menu
	else if (curMenu == videoMenu) {
		bool state = videoBox->videoDisplay->loaded;

		// Rebuild icons
		RebuildMenuItem(videoMenu,Menu_Video_JumpTo,wxBITMAP(jumpto_button),wxBITMAP(jumpto_disable_button),state);
		RebuildMenuItem(videoMenu,Menu_Subs_Snap_Video_To_Start,wxBITMAP(video_to_substart),wxBITMAP(video_to_substart_disable),state);
		RebuildMenuItem(videoMenu,Menu_Subs_Snap_Video_To_End,wxBITMAP(video_to_subend),wxBITMAP(video_to_subend_disable),state);
		RebuildMenuItem(videoMenu,Menu_Subs_Snap_Start_To_Video,wxBITMAP(substart_to_video),wxBITMAP(substart_to_video_disable),state);
		RebuildMenuItem(videoMenu,Menu_Subs_Snap_End_To_Video,wxBITMAP(subend_to_video),wxBITMAP(subend_to_video_disable),state);
		RebuildMenuItem(videoMenu,Menu_Video_Snap_To_Scene,wxBITMAP(snap_subs_to_scene),wxBITMAP(snap_subs_to_scene_disable),state);
		RebuildMenuItem(videoMenu,Menu_Video_Shift_To_Frame,wxBITMAP(shift_to_frame),wxBITMAP(shift_to_frame_disable),state);

		// Set states
		MenuBar->Enable(Menu_View_Zoom_50,state);
		MenuBar->Enable(Menu_View_Zoom_100,state);
		MenuBar->Enable(Menu_View_Zoom_200,state);
		MenuBar->Enable(Menu_File_Close_Video,state);
		MenuBar->Enable(Menu_Video_AR_Default,state);
		MenuBar->Enable(Menu_Video_AR_Full,state);
		MenuBar->Enable(Menu_Video_AR_Wide,state);
		MenuBar->Enable(Menu_Video_AR_235,state);
		MenuBar->Enable(Menu_Video_AR_Custom,state);
		MenuBar->Enable(Menu_File_Close_VFR,VFR_Output.GetFrameRateType() == VFR); //fix me, wrong?

		// Set AR radio
		int arType = videoBox->videoDisplay->GetAspectRatioType();
		MenuBar->Check(Menu_Video_AR_Default,false);
		MenuBar->Check(Menu_Video_AR_Full,false);
		MenuBar->Check(Menu_Video_AR_Wide,false);
		MenuBar->Check(Menu_Video_AR_235,false);
		MenuBar->Check(Menu_Video_AR_Custom,false);
		switch (arType) {
			case 0: MenuBar->Check(Menu_Video_AR_Default,true); break;
			case 1: MenuBar->Check(Menu_Video_AR_Full,true); break;
			case 2: MenuBar->Check(Menu_Video_AR_Wide,true); break;
			case 3: MenuBar->Check(Menu_Video_AR_235,true); break;
			case 4: MenuBar->Check(Menu_Video_AR_Custom,true); break;
		}

		// Wipe recent
		int count = RecentVids->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentVids->Destroy(RecentVids->FindItemByPosition(i));
		}

		// Rebuild recent
		int added = 0;
		wxString n;
		wxArrayString entries = Options.GetRecentList(_T("Recent vid"));
		for (size_t i=0;i<entries.Count();i++) {
			n = wxString::Format(_T("%i"),i+1);
			if (i < 9) n = _T("&") + n;
			wxFileName shortname(entries[i]);
			wxString filename = shortname.GetFullName();
			RecentVids->Append(Menu_Video_Recent+i,n + _T(" ") + filename);
			added++;
		}
		if (added == 0) RecentVids->Append(Menu_Video_Recent,_T("Empty"))->Enable(false);
	}

	// Audio menu
	else if (curMenu == audioMenu) {
		bool state = audioBox->loaded;
		bool vidstate = videoBox->videoDisplay->loaded;

		MenuBar->Enable(Menu_Audio_Open_From_Video,vidstate);
		MenuBar->Enable(Menu_Audio_Close,state);

		// Wipe recent
		int count = RecentAuds->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentAuds->Destroy(RecentAuds->FindItemByPosition(i));
		}

		// Rebuild recent
		int added = 0;
		wxString n;
		wxArrayString entries = Options.GetRecentList(_T("Recent aud"));
		for (size_t i=0;i<entries.Count();i++) {
			n = wxString::Format(_T("%i"),i+1);
			if (i < 9) n = _T("&") + n;
			wxFileName shortname(entries[i]);
			wxString filename = shortname.GetFullName();
			RecentAuds->Append(Menu_Audio_Recent+i,n + _T(" ") + filename);
			added++;
		}
		if (added == 0) RecentAuds->Append(Menu_Audio_Recent,_T("Empty"))->Enable(false);
	}

	// Edit menu
	else if (curMenu == editMenu) {
		// Undo state
		RebuildMenuItem(editMenu,Menu_Edit_Undo,wxBITMAP(undo_button),wxBITMAP(undo_disable_button),!AssFile::IsUndoStackEmpty());
		RebuildMenuItem(editMenu,Menu_Edit_Redo,wxBITMAP(redo_button),wxBITMAP(redo_disable_button),!AssFile::IsRedoStackEmpty());

		// Copy/cut/paste
		wxArrayInt sels = SubsBox->GetSelection();
		bool state = (sels.Count() > 0);
		RebuildMenuItem(editMenu,Menu_Edit_Cut,wxBITMAP(cut_button),wxBITMAP(cut_disable_button),state);
		RebuildMenuItem(editMenu,Menu_Edit_Copy,wxBITMAP(copy_button),wxBITMAP(copy_disable_button),state);
		RebuildMenuItem(editMenu,Menu_Edit_Paste,wxBITMAP(paste_button),wxBITMAP(paste_disable_button),state);
	}

	//Thaw();
}


///////////////////////////////
// Open recent subs menu entry
void FrameMain::OnOpenRecentSubs(wxCommandEvent &event) {
	int number = event.GetId()-Menu_File_Recent;
	wxString key = _T("Recent sub #") + wxString::Format(_T("%i"),number+1);
	LoadSubtitles(Options.AsText(key));
}


////////////////////////////////
// Open recent video menu entry
void FrameMain::OnOpenRecentVideo(wxCommandEvent &event) {
	int number = event.GetId()-Menu_Video_Recent;
	wxString key = _T("Recent vid #") + wxString::Format(_T("%i"),number+1);
	LoadVideo(Options.AsText(key));
}


////////////////////////////////
// Open recent audio menu entry
void FrameMain::OnOpenRecentAudio(wxCommandEvent &event) {
	int number = event.GetId()-Menu_Audio_Recent;
	wxString key = _T("Recent aud #") + wxString::Format(_T("%i"),number+1);
	LoadAudio(Options.AsText(key));
}


////////
// Exit
void FrameMain::OnExit(wxCommandEvent& WXUNUSED(event)) {
	Close();
}


//////////////////
// Open about box
void FrameMain::OnAbout(wxCommandEvent &event) {
	AboutScreen About(this);
	About.ShowModal();
}


////////////////////
// Open help topics
void FrameMain::OnContents(wxCommandEvent& WXUNUSED(event)) {
	OpenHelp(_T(""));
}


////////////////
// Open website
void FrameMain::OnWebsite(wxCommandEvent& WXUNUSED(event)) {
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("html"));
	if (type) {
		wxString command = type->GetOpenCommand(_T("http://www.aegisub.net/"));
		if (!command.empty()) wxExecute(command);
	}
}


///////////////
// Open forums
void FrameMain::OnForums(wxCommandEvent& WXUNUSED(event)) {
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("html"));
	if (type) {
		wxString command = type->GetOpenCommand(_T("http://www.malakith.net/aegisub/"));
		if (!command.empty()) wxExecute(command);
	}
}


///////////////////
// Open bugtracker
void FrameMain::OnBugTracker(wxCommandEvent& WXUNUSED(event)) {
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("html"));
	if (type) {
		wxString command = type->GetOpenCommand(_T("http://www.malakith.net/aegibug/"));
		if (!command.empty()) wxExecute(command);
	}
}


////////////////////
// Open IRC channel
void FrameMain::OnIRCChannel(wxCommandEvent& WXUNUSED(event)) {
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("html"));
	if (type) {
		wxString command = type->GetOpenCommand(_T("irc://irc.chatsociety.net/aegisub"));
		if (!command.empty()) wxExecute(command);
	}
}


////////////////
// Open project
void FrameMain::OnOpenProject(wxCommandEvent& WXUNUSED(event)) {
	// TODO
	//wxString filename = wxFileSelector(_T("Open file"),_T(""),_T(""),_T(""),_T("Aegisub Project (*.vsa)|*.vsa|All Files (*.*)|*.*"),wxOPEN | wxFILE_MUST_EXIST);
}


////////////////
// Save project
void FrameMain::OnSaveProject(wxCommandEvent& WXUNUSED(event)) {
	// TODO: Maybe? Perhaps autosave is better
}


///////////////////
// Save project as
void FrameMain::OnSaveProjectAs(wxCommandEvent& WXUNUSED(event)) {
	// TODO: Read above note
	wxString filename = wxFileSelector(_("Save file"),_T(""),_T(""),_T(""),_T("Aegisub Project (*.vsa)|*.vsa|All Files (*.*)|*.*"),wxSAVE | wxOVERWRITE_PROMPT);
}


//////////////
// Open video
void FrameMain::OnOpenVideo(wxCommandEvent& WXUNUSED(event)) {
	wxString path = Options.AsText(_T("Last open video path"));
	wxString filename = wxFileSelector(_("Open video file"),path,_T(""),_T(""),_T("Recommended Formats (*.avi,*.avs,*.d2v)|*.avi;*.avs;*.d2v|Other supported formats (*.mkv,*.ogm,*.mp4,*.mpeg,*.mpg,*.vob)|*.mkv;*.ogm;*.mp4;*.mpeg;*.mpg;*.vob|All Files (*.*)|*.*"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadVideo(filename);
		Options.SetText(_T("Last open video path"), filename);
	}
}


///////////////
// Close video
void FrameMain::OnCloseVideo(wxCommandEvent& WXUNUSED(event)) {
	LoadVideo(_T(""));
}


//////////////
// Open Audio
void FrameMain::OnOpenAudio (wxCommandEvent& WXUNUSED(event)) {
	wxString path = Options.AsText(_T("Last open audio path"));
	wxString filename = wxFileSelector(_("Open audio file"),path,_T(""),_T(""),_T("Audio Formats (*.wav,*.mp3,*.ogg,*.flac,*.mp4,*.ac3,*.aac,*.mka)|*.wav;*.mp3;*.ogg;*.flac;*.mp4;*.ac3;*.aac;*.mka|All files (*.*)|*.*"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadAudio(filename);
		Options.SetText(_T("Last open audio path"), filename);
	}
}


void FrameMain::OnOpenAudioFromVideo (wxCommandEvent& WXUNUSED(event)) {
	LoadAudio(_T(""),true);
}


void FrameMain::OnCloseAudio (wxCommandEvent& WXUNUSED(event)) {
	LoadAudio(_T(""));
}


//////////////////
// Open subtitles
void FrameMain::OnOpenSubtitles(wxCommandEvent& WXUNUSED(event)) {
	wxString path = Options.AsText(_T("Last open subtitles path"));	
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),_T("All Supported Types (*.ass,*.ssa,*.srt,*.txt)|*.ass;*.ssa;*.srt;*.txt|Advanced Substation Alpha (*.ass)|*.ass|Substation Alpha (*.ssa)|*.ssa|SubRip (*.srt)|*.srt|Plain-text (*.txt)|*.txt"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadSubtitles(filename);
		wxFileName filepath(filename);
		Options.SetText(_T("Last open subtitles path"), filepath.GetPath());
	}
}


////////////////////////////////////////
// Open subtitles with specific charset
void FrameMain::OnOpenSubtitlesCharset(wxCommandEvent& WXUNUSED(event)) {
	// Initialize charsets
	wxArrayString choices = GetEncodings();
	wxString path = Options.AsText(_T("Last open subtitles path"));

	// Get options and load
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),_T("All Supported Types (*.ass,*.ssa,*.srt,*.txt)|*.ass;*.ssa;*.srt;*.txt|Advanced Substation Alpha (*.ass)|*.ass|Substation Alpha (*.ssa)|*.ssa|SubRip (*.srt)|*.srt|Plain-text (*.txt)|*.txt"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		wxString charset = wxGetSingleChoice(_("Choose charset code:"), _("Charset"),choices,this,-1, -1,true,250,200);
		if (!charset.empty()) {
			LoadSubtitles(filename,charset);
		}
		Options.SetText(_T("Last open subtitles path"), filename);
	}
}


/////////////////////
// Save subtitles as
void FrameMain::OnSaveSubtitlesAs(wxCommandEvent& WXUNUSED(event)) {
	SaveSubtitles(true);
}


//////////////////
// Save subtitles
void FrameMain::OnSaveSubtitles(wxCommandEvent& WXUNUSED(event)) {
	SaveSubtitles(false);
}


////////////////////////////////////////
// Save subtitles with specific charset
void FrameMain::OnSaveSubtitlesCharset(wxCommandEvent& WXUNUSED(event)) {
	SaveSubtitles(true,true);
}


///////////////////
// Close subtitles
void FrameMain::OnNewSubtitles(wxCommandEvent& WXUNUSED(event)) {
	LoadSubtitles(_T(""));
}


////////////////////
// Export subtitles
void FrameMain::OnExportSubtitles(wxCommandEvent & WXUNUSED(event)) {
	//wxString filename = wxFileSelector(_T("Export subtitles file"),_T(""),_T(""),_T(""),_T("Advanced Substation Alpha (*.ass)|*.ass"),wxSAVE | wxOVERWRITE_PROMPT);
	//if (!filename.empty()) {
	//	AssFile::top->Export(filename);
	//}
	DialogExport exporter(this);
	exporter.ShowModal();
}


/////////////////
// Open VFR tags
void FrameMain::OnOpenVFR(wxCommandEvent &event) {
	wxString path = Options.AsText(_T("Last open timecodes path"));
	wxString filename = wxFileSelector(_("Open timecodes file"),path,_T(""),_T(""),_T("All Supported Types (*.txt)|*.txt|All Files (*.*)|*.*"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadVFR(filename);
		Options.SetText(_T("Last open timecodes path"), filename);
	}
}


//////////////////
// Close VFR tags
void FrameMain::OnCloseVFR(wxCommandEvent &event) {
	LoadVFR(_T(""));
}


///////////////
// Zoom levels
void FrameMain::OnSetZoom50(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->zoomBox->SetSelection(3);
	videoBox->videoDisplay->SetZoomPos(3);
}

void FrameMain::OnSetZoom100(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->zoomBox->SetSelection(7);
	videoBox->videoDisplay->SetZoomPos(7);
}

void FrameMain::OnSetZoom200(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->zoomBox->SetSelection(15);
	videoBox->videoDisplay->SetZoomPos(15);
}

void FrameMain::OnZoomIn (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->zoomBox->SetSelection(videoBox->videoDisplay->zoomBox->GetSelection()+1);
	videoBox->videoDisplay->SetZoomPos(videoBox->videoDisplay->zoomBox->GetSelection());
}

void FrameMain::OnZoomOut (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	int selTo = videoBox->videoDisplay->zoomBox->GetSelection()-1;
	if (selTo < 0) selTo = 0;
	videoBox->videoDisplay->zoomBox->SetSelection(selTo);
	videoBox->videoDisplay->SetZoomPos(videoBox->videoDisplay->zoomBox->GetSelection());
}

void FrameMain::OnSetZoom(wxCommandEvent &event) {
	//videoBox->videoDisplay->SetZoomPos(event.GetValue());
	videoBox->videoDisplay->SetZoomPos(videoBox->videoDisplay->zoomBox->GetSelection());
}


///////////////////////
// Open jump to dialog
void FrameMain::OnJumpTo(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	if (videoBox->videoDisplay->loaded) {
		DialogJumpTo JumpTo(this,videoBox->videoDisplay);
		JumpTo.ShowModal();
		videoBox->videoSlider->SetFocus();
	}
}


/////////////////////
// Open shift dialog
void FrameMain::OnShift(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	DialogShiftTimes Shift(this,SubsBox,videoBox->videoDisplay);
	Shift.ShowModal();
}


///////////////////
// Open properties
void FrameMain::OnOpenProperties (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogProperties Properties(this, videoBox->videoDisplay);
	int res = Properties.ShowModal();
	if (res) {
		SubsBox->CommitChanges();
	}
}


///////////////////////
// Open styles manager
void FrameMain::OnOpenStylesManager(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	DialogStyleManager StyleManager(this,SubsBox);
	StyleManager.ShowModal();
	EditBox->UpdateGlobals();
	SubsBox->CommitChanges();
}


//////////////////////////////
// Open translation assistant
void FrameMain::OnOpenTranslation(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	int start = SubsBox->GetFirstSelRow();
	if (start == -1) start = 0;
	DialogTranslation Trans(this,AssFile::top,SubsBox,start,true);
	Trans.ShowModal();
}


//////////////////////
// Open Spell Checker
void FrameMain::OnOpenSpellCheck (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	#ifndef NO_SPELLCHECKER
	wxArrayInt selList = SubsBox->GetSelection();
	if (selList.GetCount() == 1){
		AssDialogue * a = SubsBox->GetDialogue(selList.Item(0));
		if (a->Text.IsEmpty()){
			wxMessageDialog Question(this, _T(
				"You've selected a single row with no text. Instead would you like to check the entire document?"),
				_T("Single Row Selection"),
				wxICON_QUESTION | wxSTAY_ON_TOP | wxYES_NO | wxYES_DEFAULT,
				wxDefaultPosition);

			int a = Question.ShowModal();
			if (a == wxID_YES){
				selList.Clear();
				selList.Add(-1);
				}
		}
	}
	DialogSpellCheck SpellCheck(this,AssFile::top, &selList, SubsBox);
	SpellCheck.ShowModal();

	SubsBox->CommitChanges();    
	#endif
}


////////////////////////
// Open Fonts Collector
void FrameMain::OnOpenFontsCollector (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogFontsCollector Collector(this);
	Collector.ShowModal();
}


/////////////////////////////
// Open Resolution Resampler
void FrameMain::OnOpenResample (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogResample diag(this, SubsBox);
	diag.ShowModal();
}


/////////////////////////////////////
// Open Timing post-processor dialog
void FrameMain::OnOpenTimingProcessor (wxCommandEvent &event) {
	DialogTimingProcessor timing(this,SubsBox);
	timing.ShowModal();
}


///////////////////////
// Open Hotkeys dialog
void FrameMain::OnOpenHotkeys (wxCommandEvent &event) {
	DialogHotkeys keys(this);
	keys.ShowModal();
}


///////////////////////
// Open Options dialog
void FrameMain::OnOpenOptions (wxCommandEvent &event) {
	DialogOptions options(this);
	options.ShowModal();
}


///////////////////
// Open Automation
void FrameMain::OnOpenAutomation (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogAutomationManager *automan = new DialogAutomationManager(this, SubsBox);
	automan->ShowModal();
	delete automan;
}


//////////////////////
// Snap subs to video
void FrameMain::OnSnapSubsStartToVid (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		wxArrayInt sel = SubsBox->GetSelection();
		if (sel.Count() > 0) {
			wxCommandEvent dummy;
			SubsBox->SetSubsToVideo(true);
		}
	}
}

void FrameMain::OnSnapSubsEndToVid (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		wxArrayInt sel = SubsBox->GetSelection();
		if (sel.Count() > 0) {
			wxCommandEvent dummy;
			SubsBox->SetSubsToVideo(false);
		}
	}
}


//////////////////////
// Jump video to subs
void FrameMain::OnSnapVidToSubsStart (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		wxArrayInt sel = SubsBox->GetSelection();
		if (sel.Count() > 0) {
			wxCommandEvent dummy;
			SubsBox->SetVideoToSubs(true);
		}
	}
}

void FrameMain::OnSnapVidToSubsEnd (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		wxArrayInt sel = SubsBox->GetSelection();
		if (sel.Count() > 0) {
			wxCommandEvent dummy;
			SubsBox->SetVideoToSubs(false);
		}
	}
}


/////////////////
// Snap to scene
void FrameMain::OnSnapToScene (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		// Get frames
		wxArrayInt sel = SubsBox->GetSelection();
		int curFrame = videoBox->videoDisplay->frame_n;
		int prev = 0;
		int next = 0;
		int frame = 0;
		size_t n = videoBox->videoDisplay->KeyFrames.Count();
		bool found = false;
		for (size_t i=0;i<n;i++) {
			frame = videoBox->videoDisplay->KeyFrames[i];

			if (frame == curFrame) {
				prev = frame;
				if (i < n-1) next = videoBox->videoDisplay->KeyFrames[i+1];
				else next = videoBox->videoDisplay->length;
				found = true;
				break;
			}

			if (frame > curFrame) {
				if (i != 0) prev = videoBox->videoDisplay->KeyFrames[i-1];
				else prev = 0;
				next = frame;
				found = true;
				break;
			}
		}

		// Last section?
		if (!found) {
			if (n > 0) prev = videoBox->videoDisplay->KeyFrames[n-1];
			else prev = 0;
			next = videoBox->videoDisplay->length;
		}

		// Get times
		int start_ms = VFR_Output.GetTimeAtFrame(prev,true);
		int end_ms = VFR_Output.GetTimeAtFrame(next-1,false);
		AssDialogue *cur;

		// Update rows
		for (size_t i=0;i<sel.Count();i++) {
			cur = SubsBox->GetDialogue(sel[i]);
			cur->Start.SetMS(start_ms);
			cur->End.SetMS(end_ms);
			cur->UpdateData();
		}

		// Commit
		SubsBox->ass->FlagAsModified();
		SubsBox->CommitChanges();
	}
}


//////////////////
// Shift to frame
void FrameMain::OnShiftToFrame (wxCommandEvent &event) {
	if (videoBox->videoDisplay->loaded) {
		// Get selection
		wxArrayInt sels = SubsBox->GetSelection();
		int n=sels.Count();
		if (n == 0) return;

		// Get shifting in ms
		AssDialogue *cur = SubsBox->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = VFR_Output.GetTimeAtFrame(videoBox->videoDisplay->frame_n,true) - cur->Start.GetMS();

		// Update
		for (int i=0;i<n;i++) {
			cur = SubsBox->GetDialogue(sels[i]);
			if (cur) {
				cur->Start.SetMS(cur->Start.GetMS()+shiftBy);
				cur->End.SetMS(cur->End.GetMS()+shiftBy);
				cur->UpdateData();
			}
		}

		// Commit
		SubsBox->ass->FlagAsModified();
		SubsBox->CommitChanges();
	}
}


////////
// Undo
void FrameMain::OnUndo(wxCommandEvent& WXUNUSED(event)) {
	// Block if it's on a editbox
	//wxWindow *focused = wxWindow::FindFocus();
	//if (focused && focused->IsKindOf(CLASSINFO(wxTextCtrl))) return;

	videoBox->videoDisplay->Stop();
	AssFile::StackPop();
	SubsBox->LoadFromAss(AssFile::top,true);
	AssFile::Popping = false;
}


////////
// Redo
void FrameMain::OnRedo(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	AssFile::StackRedo();
	SubsBox->LoadFromAss(AssFile::top,true);
	AssFile::Popping = false;
}


////////
// Find
void FrameMain::OnFind(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	Search.OpenDialog(false);
}


/////////////
// Find next
void FrameMain::OnFindNext(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	Search.FindNext();
}


//////////////////
// Find & replace
void FrameMain::OnReplace(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	Search.OpenDialog(true);
}


//////////////////////////////////
// Change aspect ratio to default
void FrameMain::OnSetARDefault (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->SetAspectRatio(0);
	SetDisplayMode(-1);
}


/////////////////////////////////////
// Change aspect ratio to fullscreen
void FrameMain::OnSetARFull (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->SetAspectRatio(1);
	SetDisplayMode(-1);
}


/////////////////////////////////////
// Change aspect ratio to widescreen
void FrameMain::OnSetARWide (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->SetAspectRatio(2);
	SetDisplayMode(-1);
}


///////////////////////////////
// Change aspect ratio to 2:35
void FrameMain::OnSetAR235 (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	videoBox->videoDisplay->SetAspectRatio(3);
	SetDisplayMode(-1);
}


/////////////////////////////////////////
// Change aspect ratio to a custom value
void FrameMain::OnSetARCustom (wxCommandEvent &event) {
	// Get text
	videoBox->videoDisplay->Stop();
	wxString value = wxGetTextFromUser(_T("Enter aspect ratio in either decimal (e.g. 2.35) or fractional (e.g. 16:9) form:"),_T("Enter aspect ratio"),FloatToString(videoBox->videoDisplay->GetAspectRatioValue()));

	// Process text
	double numval = 0.0;
	value.Replace(_T(","),_T("."));
	if (value.Freq(_T('.')) == 1) {
		value.ToDouble(&numval);
	}
	else if (value.Freq(_T(':')) == 1) {
		int pos = value.Find(_T(':'));
		wxString num = value.Left(pos);
		wxString denum = value.Mid(pos+1);
		if (num.IsNumber() && denum.IsNumber()) {
			double a,b;
			num.ToDouble(&a);
			denum.ToDouble(&b);
			if (b != 0) numval = a/b;
		}
	}

	// Sanity check
	if (numval < 0.5 || numval > 5.0) wxMessageBox(_T("Invalid value! Aspect ratio must be between 0.5 and 5.0."),_T("Invalid Aspect Ratio"),wxICON_ERROR);

	// Set value
	else {
		videoBox->videoDisplay->SetAspectRatio(4,numval);
		SetDisplayMode(-1);
	}
}


////////////////////////////////////
// Window is attempted to be closed
void FrameMain::OnCloseWindow (wxCloseEvent &event) {
	// Stop audio and video
	videoBox->videoDisplay->Stop();
	audioBox->audioDisplay->Stop();

	// Ask user if he wants to save first
	bool canVeto = event.CanVeto();
	int result = TryToCloseSubs(canVeto);

	// Abort/destroy
	if (canVeto) {
		if (result == wxCANCEL) event.Veto();
		else Destroy();
	}
	else Destroy();
}


//////////////////
// Cut/copy/paste
void FrameMain::OnCut (wxCommandEvent &event) {
	SubsBox->CutLines(SubsBox->GetSelection());
}

void FrameMain::OnCopy (wxCommandEvent &event) {
	SubsBox->CopyLines(SubsBox->GetSelection());
}

void FrameMain::OnPaste (wxCommandEvent &event) {
	SubsBox->PasteLines(SubsBox->GetFirstSelRow());
}


////////////////////////
// Select visible lines
void FrameMain::OnSelectVisible (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	SubsBox->SelectVisible();
}


//////////////////////
// Open select dialog
void FrameMain::OnSelect (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogSelection select(this, SubsBox);
	select.ShowModal();
}


//////////////////////////
// Open styling assistant
void FrameMain::OnOpenStylingAssistant (wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
	DialogStyling styling(this,SubsBox);
	styling.ShowModal();
}


///////////////
// Auto backup
void FrameMain::OnAutoSave(wxTimerEvent &event) {
	// Auto Save
	try {
		if (AssFile::top->loaded && AssFile::top->IsASS) {
			// Set path
			wxFileName origfile(AssFile::top->filename);
			wxString path = Options.AsText(_T("Auto save path"));
			if (path.IsEmpty()) path = origfile.GetPath();
			wxFileName dstpath(path);
			if (!dstpath.IsAbsolute()) path = AegisubApp::folderName + path;
			path += _T("/");
			dstpath.Assign(path);
			if (!dstpath.DirExists()) wxMkdir(path);

			// Save
			wxString backup = path + origfile.GetName() + _T(".AUTOSAVE.") + origfile.GetExt();
			AssFile::top->Save(backup,false,false);

			// Set status bar
			StatusTimeout(_("File backup saved as \"") + backup + _T("\"."));
		}
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


///////////////////
// Clear statusbar
void FrameMain::OnStatusClear(wxTimerEvent &event) {
	SetStatusText(_T(""),1);
}


////////////
// Key down
void FrameMain::OnKeyDown(wxKeyEvent &event) {
	audioBox->audioDisplay->AddPendingEvent(event);
	event.Skip();
}


/////////////////////
// Next frame hotkey
void FrameMain::OnNextFrame(wxCommandEvent &event) {
	videoBox->videoSlider->NextFrame();
}


/////////////////////////
// Previous frame hotkey
void FrameMain::OnPrevFrame(wxCommandEvent &event) {
	videoBox->videoSlider->PrevFrame();
}


///////////////////////////////////////////////////
// Toggle focus between seek bar and whatever else
void FrameMain::OnFocusSeek(wxCommandEvent &event) {
	wxWindow *curFocus = wxWindow::FindFocus();
	if (curFocus == videoBox->videoSlider) {
		if (PreviousFocus) PreviousFocus->SetFocus();
	}
	else {
		PreviousFocus = curFocus;
		videoBox->videoSlider->SetFocus();
	}
}


////////////////////////
// Previous line hotkey
void FrameMain::OnPrevLine(wxCommandEvent &event) {
	int next = EditBox->linen-1;
	if (next < 0) return;
	SubsBox->SelectRow(next);
	SubsBox->MakeCellVisible(next,0);
	EditBox->SetToLine(next);
}


////////////////////
// Next line hotkey
void FrameMain::OnNextLine(wxCommandEvent &event) {
	int nrows = SubsBox->GetRows();
	int next = EditBox->linen+1;
	if (next >= nrows) return;
	SubsBox->SelectRow(next);
	SubsBox->MakeCellVisible(next,0);
	EditBox->SetToLine(next);
}


//////////////////////////////////
// Cycle through tag hiding modes
void FrameMain::OnToggleTags(wxCommandEvent &event) {
	// Read value
	int tagMode = Options.AsInt(_T("Grid hide overrides"));

	// Cycle to next
	if (tagMode < 0 || tagMode > 2) tagMode = 1;
	else {
		tagMode = (tagMode+1)%3;
	}

	// Show on status bar
	wxString message = _T("ASS Override Tag mode set to ");
	if (tagMode == 0) message += _T("show full tags.");
	if (tagMode == 1) message += _T("simplify tags.");
	if (tagMode == 2) message += _T("hide tags.");
	StatusTimeout(message,10000);

	// Set option
	Options.SetInt(_T("Grid hide overrides"),tagMode);
	Options.Save();

	// Refresh grid
	SubsBox->Refresh(false);
}


//////////////
// Play video
void FrameMain::OnVideoPlay(wxCommandEvent &event) {
	videoBox->videoDisplay->Play();
}


///////////////////
// Play video line
void FrameMain::OnVideoPlayLine(wxCommandEvent &event) {
	videoBox->videoDisplay->PlayLine();
}


//////////////
// Stop video
void FrameMain::OnVideoStop(wxCommandEvent &event) {
	videoBox->videoDisplay->Stop();
}


/////////////////////
// Toggle autoscroll
void FrameMain::OnVideoToggleScroll(wxCommandEvent &event) {
	Options.SetBool(_T("Sync video with subs"),videoBox->AutoScroll->GetValue());
	Options.Save();
}


///////////////////////////////
// Choose a different language
void FrameMain::OnChooseLanguage (wxCommandEvent &event) {
	// Get language
	AegisubApp *app = (AegisubApp*) wxTheApp;
	int old = app->locale.curCode;
	int newCode = app->locale.PickLanguage();

	// Is OK?
	if (newCode != -1) {
		// Set code
		Options.SetInt(_T("Locale Code"),newCode);
		Options.Save();

		// Language actually changed?
		if (newCode != old) {
			// Ask to restart program
			int result = wxMessageBox(_T("Aegisub needs to be restarted so that the new language can be applied. Restart now?"),_T("Restart Aegisub?"),wxICON_QUESTION | wxYES_NO);
			if (result == wxYES) {
				// Restart Aegisub
				if (Close()) wxExecute(AegisubApp::fullPath);
			}
		}
	}
}


/////////////////
// View standard
void FrameMain::OnViewStandard (wxCommandEvent &event) {
	if (!audioBox->audioDisplay->loaded || !videoBox->videoDisplay->loaded) return;
	SetDisplayMode(2);
}


//////////////
// View video
void FrameMain::OnViewVideo (wxCommandEvent &event) {
	if (!videoBox->videoDisplay->loaded) return;
	SetDisplayMode(1);
}


//////////////
// View audio
void FrameMain::OnViewAudio (wxCommandEvent &event) {
	if (!audioBox->audioDisplay->loaded) return;
	SetDisplayMode(3);
}


/////////////
// View subs
void FrameMain::OnViewSubs (wxCommandEvent &event) {
	SetDisplayMode(0);
}


/////////////
// Kana game
void FrameMain::OnKanaGame(wxCommandEvent &event) {
	wxMessageBox(_T("TODO"));
}
