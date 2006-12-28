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
#include "setup.h"
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
#include "ass_dialogue.h"
#include "dialog_style_manager.h"
#include "dialog_translation.h"
#include "dialog_jumpto.h"
#include "dialog_shift_times.h"
#include "dialog_search_replace.h"
#include "vfr.h"
#include "subs_edit_box.h"
#include "options.h"
#include "dialog_properties.h"
#include "dialog_attachments.h"
#include "main.h"
#include "dialog_fonts_collector.h"
#include "dialog_about.h"
#include "dialog_export.h"
#include "audio_box.h"
#include "dialog_selection.h"
#include "dialog_styling_assistant.h"
#include "dialog_resample.h"
#include "audio_display.h"
#include "toggle_bitmap.h"
#include "dialog_hotkeys.h"
#include "dialog_timing_processor.h"
#if USE_FEXTRACKER == 1
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexTrackingFeature.h"
#include "../FexTrackerSource/FexMovement.h"
#include "dialog_fextracker.h"
#endif
#include "dialog_progress.h"
#include "dialog_options.h"
#include "utils.h"
#include "auto4_base.h"
#include "dialog_automation.h"


////////////////////
// Menu event table
BEGIN_EVENT_TABLE(FrameMain, wxFrame)
	EVT_TIMER(AutoSave_Timer, FrameMain::OnAutoSave)
	EVT_TIMER(StatusClear_Timer, FrameMain::OnStatusClear)

	EVT_BUTTON(Video_Play, FrameMain::OnVideoPlay)
	EVT_BUTTON(Video_Play_Line, FrameMain::OnVideoPlayLine)
	EVT_BUTTON(Video_Stop, FrameMain::OnVideoStop)
	EVT_TOGGLEBUTTON(Video_Auto_Scroll, FrameMain::OnVideoToggleScroll)

#if USE_FEXTRACKER == 1
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
	EVT_MENU_RANGE(Menu_File_Recent,Menu_File_Recent+99, FrameMain::OnOpenRecentSubs)
	EVT_MENU_RANGE(Menu_Video_Recent,Menu_Video_Recent+99, FrameMain::OnOpenRecentVideo)
	EVT_MENU_RANGE(Menu_Audio_Recent,Menu_Audio_Recent+99, FrameMain::OnOpenRecentAudio)
	EVT_MENU_RANGE(Menu_Timecodes_Recent,Menu_Timecodes_Recent+99, FrameMain::OnOpenRecentTimecodes)
	EVT_MENU_RANGE(Menu_Keyframes_Recent,Menu_Keyframes_Recent+99, FrameMain::OnOpenRecentKeyframes)
	EVT_MENU_RANGE(Menu_Automation_Macro,Menu_Automation_Macro+99, FrameMain::OnAutomationMacro)

	EVT_MENU_RANGE(MENU_GRID_START+1,MENU_GRID_END-1,FrameMain::OnGridEvent)
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
	EVT_MENU(Menu_Video_Load_Keyframes, FrameMain::OnOpenKeyframes)
	EVT_MENU(Menu_Video_Save_Keyframes, FrameMain::OnSaveKeyframes)
	EVT_MENU(Menu_Video_Close_Keyframes, FrameMain::OnCloseKeyframes)

	EVT_MENU(Menu_View_Zoom_50, FrameMain::OnSetZoom50)
	EVT_MENU(Menu_View_Zoom_100, FrameMain::OnSetZoom100)
	EVT_MENU(Menu_View_Zoom_200, FrameMain::OnSetZoom200)
	EVT_COMBOBOX(Toolbar_Zoom_Dropdown, FrameMain::OnSetZoom)
	EVT_MENU(Video_Play, FrameMain::OnVideoPlay)
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
	EVT_MENU(Menu_Edit_Paste_Over, FrameMain::OnPasteOver)
	EVT_MENU(Menu_Edit_Find, FrameMain::OnFind)
	EVT_MENU(Menu_Edit_Find_Next, FrameMain::OnFindNext)
	EVT_MENU(Menu_Edit_Replace, FrameMain::OnReplace)
	EVT_MENU(Menu_Edit_Shift, FrameMain::OnShift)
	EVT_MENU(Menu_Edit_Select, FrameMain::OnSelect)
	EVT_MENU(Menu_Edit_Sort, FrameMain::OnSort)

	EVT_MENU(Menu_Tools_Properties, FrameMain::OnOpenProperties)
	EVT_MENU(Menu_Tools_Styles_Manager, FrameMain::OnOpenStylesManager)
	EVT_MENU(Menu_Tools_Attachments, FrameMain::OnOpenAttachments)
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
	EVT_MENU(Edit_Box_Commit,FrameMain::OnEditBoxCommit)

	EVT_MENU(Kana_Game, FrameMain::OnKanaGame)

#ifdef __WXMAC__
   EVT_MENU(wxID_ABOUT, FrameMain::OnAbout)
   EVT_MENU(wxID_EXIT, FrameMain::OnExit)
#endif
END_EVENT_TABLE()


////////////////////////////////
// Redirect grid events to grid
void FrameMain::OnGridEvent (wxCommandEvent &event) {
	SubsBox->AddPendingEvent(event);
}


////////////////////////
// Menu is being opened
void FrameMain::OnMenuOpen (wxMenuEvent &event) {
	// Get menu
	//Freeze();
	wxMenu *curMenu = event.GetMenu();

	// Start by cleaning up in macro menu items
	for (int i = 0; i < activeMacroItems.size(); i++) {
		wxMenu *p = 0;
		wxMenuItem *it = MenuBar->FindItem(Menu_Automation_Macro + i, &p);
		if (it)
			p->Delete(it);
	}
	activeMacroItems.clear();

	// File menu
	if (curMenu == fileMenu) {
		// Wipe recent
		int count = (int)RecentSubs->GetMenuItemCount();
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

		// Set states
		MenuBar->Enable(Menu_View_Zoom,state);
		MenuBar->Enable(Menu_View_Zoom_50,state);
		MenuBar->Enable(Menu_View_Zoom_100,state);
		MenuBar->Enable(Menu_View_Zoom_200,state);
		MenuBar->Enable(Menu_File_Close_Video,state);
		MenuBar->Enable(Menu_Video_AR,state);
		MenuBar->Enable(Menu_Video_AR_Default,state);
		MenuBar->Enable(Menu_Video_AR_Full,state);
		MenuBar->Enable(Menu_Video_AR_Wide,state);
		MenuBar->Enable(Menu_Video_AR_235,state);
		MenuBar->Enable(Menu_Video_AR_Custom,state);
		MenuBar->Enable(Menu_File_Close_VFR,VFR_Output.GetFrameRateType() == VFR);
		MenuBar->Enable(Menu_Video_Close_Keyframes,videoBox->videoDisplay->OverKeyFramesLoaded());
		MenuBar->Enable(Menu_Video_Save_Keyframes,videoBox->videoDisplay->KeyFramesLoaded());

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
		int count = (int)RecentVids->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentVids->Destroy(RecentVids->FindItemByPosition(i));
		}
		count = (int)RecentTimecodes->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentTimecodes->Destroy(RecentTimecodes->FindItemByPosition(i));
		}
		count = (int)RecentKeyframes->GetMenuItemCount();
		for (int i=count;--i>=0;) {
			RecentKeyframes->Destroy(RecentKeyframes->FindItemByPosition(i));
		}

		// Rebuild recent videos
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

		// Rebuild recent timecodes
		added = 0;
		entries = Options.GetRecentList(_T("Recent timecodes"));
		for (size_t i=0;i<entries.Count();i++) {
			n = wxString::Format(_T("%i"),i+1);
			if (i < 9) n = _T("&") + n;
			wxFileName shortname(entries[i]);
			wxString filename = shortname.GetFullName();
			RecentTimecodes->Append(Menu_Timecodes_Recent+i,n + _T(" ") + filename);
			added++;
		}
		if (added == 0) RecentTimecodes->Append(Menu_Timecodes_Recent,_T("Empty"))->Enable(false);

		// Rebuild recent Keyframes
		added = 0;
		entries = Options.GetRecentList(_T("Recent Keyframes"));
		for (size_t i=0;i<entries.Count();i++) {
			n = wxString::Format(_T("%i"),i+1);
			if (i < 9) n = _T("&") + n;
			wxFileName shortname(entries[i]);
			wxString filename = shortname.GetFullName();
			RecentKeyframes->Append(Menu_Keyframes_Recent+i,n + _T(" ") + filename);
			added++;
		}
		if (added == 0) RecentKeyframes->Append(Menu_Keyframes_Recent,_T("Empty"))->Enable(false);
	}

	// Audio menu
	else if (curMenu == audioMenu) {
		bool state = audioBox->loaded;
		bool vidstate = videoBox->videoDisplay->loaded;

		MenuBar->Enable(Menu_Audio_Open_From_Video,vidstate);
		MenuBar->Enable(Menu_Audio_Close,state);

		// Wipe recent
		int count = (int)RecentAuds->GetMenuItemCount();
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

	// Subtitles menu
	else if (curMenu == subtitlesMenu) {
		// Variables
		bool continuous;
		wxArrayInt sels = SubsBox->GetSelection(&continuous);
		int count = sels.Count();
		bool state,state2;

		// Entries
		state = count > 0;
		MenuBar->Enable(MENU_INSERT_BEFORE,state);
		MenuBar->Enable(MENU_INSERT_AFTER,state);
		MenuBar->Enable(MENU_SPLIT_BY_KARAOKE,state);
		RebuildMenuItem(subtitlesMenu,MENU_DELETE,wxBITMAP(delete_button),wxBITMAP(delete_disable_button),state);
		state2 = count > 0 && videoBox->videoDisplay->loaded;
		MenuBar->Enable(MENU_INSERT_BEFORE_VIDEO,state2);
		MenuBar->Enable(MENU_INSERT_AFTER_VIDEO,state2);
		MenuBar->Enable(Menu_Subtitles_Insert,state);
		state = count > 0 && continuous;
		MenuBar->Enable(MENU_DUPLICATE,state);
		state = count > 0 && continuous && VFR_Output.IsLoaded();
		MenuBar->Enable(MENU_DUPLICATE_NEXT_FRAME,state);
		state = count == 2;
		MenuBar->Enable(MENU_SWAP,state);
		state = count >= 2 && continuous;
		MenuBar->Enable(MENU_JOIN_CONCAT,state);
		MenuBar->Enable(MENU_JOIN_REPLACE,state);
		MenuBar->Enable(MENU_JOIN_AS_KARAOKE,state);
		MenuBar->Enable(Menu_Subtitles_Join,state);
		state = count == 2 && continuous;
		MenuBar->Enable(MENU_1_12_RECOMBINE,state);
		MenuBar->Enable(MENU_12_2_RECOMBINE,state);
		state2 = count == 3 && continuous;
		MenuBar->Enable(MENU_1_12_2_RECOMBINE,state2);
		MenuBar->Enable(Menu_Subtitles_Recombine,state || state2);
	}

	// Timing menu
	else if (curMenu == timingMenu) {
		// Variables
		bool continuous;
		wxArrayInt sels = SubsBox->GetSelection(&continuous);
		int count = sels.Count();

		// Video related
		bool state = videoBox->videoDisplay->loaded;
		RebuildMenuItem(timingMenu,Menu_Subs_Snap_Start_To_Video,wxBITMAP(substart_to_video),wxBITMAP(substart_to_video_disable),state);
		RebuildMenuItem(timingMenu,Menu_Subs_Snap_End_To_Video,wxBITMAP(subend_to_video),wxBITMAP(subend_to_video_disable),state);
		RebuildMenuItem(timingMenu,Menu_Video_Snap_To_Scene,wxBITMAP(snap_subs_to_scene),wxBITMAP(snap_subs_to_scene_disable),state);
		RebuildMenuItem(timingMenu,Menu_Video_Shift_To_Frame,wxBITMAP(shift_to_frame),wxBITMAP(shift_to_frame_disable),state);

		// Other
		state = count >= 2 && continuous;
		MenuBar->Enable(MENU_ADJOIN,state);
		MenuBar->Enable(MENU_ADJOIN2,state);
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
		MenuBar->Enable(Menu_Edit_Paste_Over,state);
	}

	// Automation menu
	else if (curMenu == automationMenu) {
		AddMacroMenuItems(automationMenu, wxGetApp().global_scripts->GetMacros(Automation4::MACROMENU_ALL));
		AddMacroMenuItems(automationMenu, local_scripts->GetMacros(Automation4::MACROMENU_ALL));
	}

	//Thaw();
}


//////////////////////////////
// Macro menu creation helper
void FrameMain::AddMacroMenuItems(wxMenu *menu, const std::vector<Automation4::FeatureMacro*> &macros) {
	if (macros.empty()) {
		return;
	}

	int id = activeMacroItems.size();;
	for (std::vector<Automation4::FeatureMacro*>::const_iterator i = macros.begin(); i != macros.end(); ++i) {
		wxMenuItem * m = menu->Append(Menu_Automation_Macro + id, (*i)->GetName(), (*i)->GetDescription());
		m->Enable((*i)->Validate(SubsBox->ass, SubsBox->GetAbsoluteSelection(), SubsBox->GetFirstSelRow()));
		activeMacroItems.push_back(*i);
		id++;
	}
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
// Open recent timecodes entry
void FrameMain::OnOpenRecentTimecodes(wxCommandEvent &event) {
	int number = event.GetId()-Menu_Timecodes_Recent;
	wxString key = _T("Recent timecodes #") + wxString::Format(_T("%i"),number+1);
	LoadVFR(Options.AsText(key));
}


////////////////////////////////
// Open recent Keyframes entry
void FrameMain::OnOpenRecentKeyframes(wxCommandEvent &event) {
	int number = event.GetId()-Menu_Keyframes_Recent;
	wxString key = _T("Recent Keyframes #") + wxString::Format(_T("%i"),number+1);
	LoadKeyframes(Options.AsText(key));
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


//////////////
// Open video
void FrameMain::OnOpenVideo(wxCommandEvent& WXUNUSED(event)) {
	wxString path = Options.AsText(_T("Last open video path"));
	wxString filename = wxFileSelector(_("Open video file"),path,_T(""),_T(""),_T("Recommended Formats (*.avi,*.avs,*.d2v)|*.avi;*.avs;*.d2v|Other supported formats (*.mkv,*.ogm,*.mp4,*.mpeg,*.mpg,*.vob)|*.mkv;*.ogm;*.mp4;*.mpeg;*.mpg;*.vob|All Files (*.*)|*.*"),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadVideo(filename);
		Options.SetText(_T("Last open video path"), filename);
		Options.Save();
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
		Options.Save();
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
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		LoadSubtitles(filename);
		wxFileName filepath(filename);
		Options.SetText(_T("Last open subtitles path"), filepath.GetPath());
		Options.Save();
	}
}


////////////////////////////////////////
// Open subtitles with specific charset
void FrameMain::OnOpenSubtitlesCharset(wxCommandEvent& WXUNUSED(event)) {
	// Initialize charsets
	wxArrayString choices = GetEncodings();
	wxString path = Options.AsText(_T("Last open subtitles path"));

	// Get options and load
	wxString filename = wxFileSelector(_("Open subtitles file"),path,_T(""),_T(""),AssFile::GetWildcardList(0),wxOPEN | wxFILE_MUST_EXIST);
	if (!filename.empty()) {
		wxString charset = wxGetSingleChoice(_("Choose charset code:"), _("Charset"),choices,this,-1, -1,true,250,200);
		if (!charset.empty()) {
			LoadSubtitles(filename,charset);
		}
		Options.SetText(_T("Last open subtitles path"), filename);
		Options.Save();
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
		Options.Save();
	}
}


//////////////////
// Close VFR tags
void FrameMain::OnCloseVFR(wxCommandEvent &event) {
	LoadVFR(_T(""));
}


//////////////////
// Open keyframes
void FrameMain::OnOpenKeyframes (wxCommandEvent &event) {
	// Pick file
	wxString path = Options.AsText(_T("Last open keyframes path"));
	wxString filename = wxFileSelector(_T("Select the Keyframes file to open"),path,_T(""),_T(".txt"),_T("Text files (*.txt)|*.txt"),wxFILE_MUST_EXIST | wxOPEN);
	if (filename.IsEmpty()) return;
	Options.SetText(_T("Last open keyframes path"),filename);
	Options.Save();

	// Load
	LoadKeyframes(filename);
}


///////////////////
// Close keyframes
void FrameMain::OnCloseKeyframes (wxCommandEvent &event) {
	videoBox->videoDisplay->CloseOverKeyFrames();
}


//////////////////
// Save keyframes
void FrameMain::OnSaveKeyframes (wxCommandEvent &event) {
	// Pick file
	wxString path = Options.AsText(_T("Last open keyframes path"));
	wxString filename = wxFileSelector(_T("Select the Keyframes file to open"),path,_T(""),_T("*.key.txt"),_T("Text files (*.txt)|*.txt"),wxOVERWRITE_PROMPT | wxSAVE);
	if (filename.IsEmpty()) return;
	Options.SetText(_T("Last open keyframes path"),filename);
	Options.Save();

	// Save
	SaveKeyframes(filename);
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


////////////////////
// Open attachments
void FrameMain::OnOpenAttachments(wxCommandEvent& WXUNUSED(event)) {
	videoBox->videoDisplay->Stop();
	DialogAttachments attachments(this);
	attachments.ShowModal();
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
	wxMessageBox(_T("TODO!"),_T("Spellchecker"));
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
	DialogAutomation dlg(this, local_scripts);
	dlg.ShowModal();
}


///////////////////////////////////////////////////////////
// General handler for all Automation-generated menu items
void FrameMain::OnAutomationMacro (wxCommandEvent &event) {
	AssFile *oldtop = AssFile::top;
	activeMacroItems[event.GetId()-Menu_Automation_Macro]->Process(SubsBox->ass, SubsBox->GetAbsoluteSelection(), SubsBox->GetFirstSelRow(), this);
	// check if modifications were made and put on undo stack
	SubsBox->LoadFromAss(AssFile::top, true, true);
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
		wxArrayInt keyframes = videoBox->videoDisplay->GetKeyFrames();
		size_t n = keyframes.Count();
		bool found = false;
		for (size_t i=0;i<n;i++) {
			frame = keyframes[i];

			if (frame == curFrame) {
				prev = frame;
				if (i < n-1) next = keyframes[i+1];
				else next = videoBox->videoDisplay->length;
				found = true;
				break;
			}

			if (frame > curFrame) {
				if (i != 0) prev = keyframes[i-1];
				else prev = 0;
				next = frame;
				found = true;
				break;
			}
		}

		// Last section?
		if (!found) {
			if (n > 0) prev = keyframes[n-1];
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
		size_t n=sels.Count();
		if (n == 0) return;

		// Get shifting in ms
		AssDialogue *cur = SubsBox->GetDialogue(sels[0]);
		if (!cur) return;
		int shiftBy = VFR_Output.GetTimeAtFrame(videoBox->videoDisplay->frame_n,true) - cur->Start.GetMS();

		// Update
		for (size_t i=0;i<n;i++) {
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
	wxString value = wxGetTextFromUser(_("Enter aspect ratio in either decimal (e.g. 2.35) or fractional (e.g. 16:9) form. Enter a value like 853x480 to set a specific resolution."),_("Enter aspect ratio"),FloatToString(videoBox->videoDisplay->GetAspectRatioValue()));
	
	if (value.IsEmpty()) return;
	value.MakeLower();

	// Process text
	double numval = 0.0;
	if (value.Freq(_T(':')) == 1) {
doNormalAR:
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
	else if (value.Freq(_T('x')) == 1) {
		int pos = value.Find(_T('x'));
		wxString width = value.Left(pos);
		wxString height = value.Mid(pos+1);
		if (width.IsNumber() && height.IsNumber()) {
			double w,h;
			width.ToDouble(&w);
			height.ToDouble(&h);
			if (h != 0) {
				numval = w/h;
				videoBox->videoDisplay->SetZoom(h / videoBox->videoDisplay->h);
			}
		}
	}
	else if (value.Freq(_T(':')) < 1) {
		value.Append(_T(":1"));
		goto doNormalAR;
	}

	// Sanity check
	if (numval < 0.5 || numval > 5.0) wxMessageBox(_("Invalid value! Aspect ratio must be between 0.5 and 5.0."),_("Invalid Aspect Ratio"),wxICON_ERROR);

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
	if (FindFocus() == EditBox->TextEdit) {
		EditBox->TextEdit->Cut();
		return;
	}
	SubsBox->CutLines(SubsBox->GetSelection());
}

void FrameMain::OnCopy (wxCommandEvent &event) {
	if (FindFocus() == EditBox->TextEdit) {
		EditBox->TextEdit->Copy();
		return;
	}
	SubsBox->CopyLines(SubsBox->GetSelection());
}

void FrameMain::OnPaste (wxCommandEvent &event) {
	if (FindFocus() == EditBox->TextEdit) {
		EditBox->TextEdit->Paste();
		return;
	}
	SubsBox->PasteLines(SubsBox->GetFirstSelRow());
}


//////////////
// Paste over
void FrameMain::OnPasteOver (wxCommandEvent &event) {
	SubsBox->PasteLines(SubsBox->GetFirstSelRow(),true);
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


//////////////////
// Sort subtitles
void FrameMain::OnSort (wxCommandEvent &event) {
	// Ensure that StartMS is set properly
	AssEntry *curEntry;
	AssDialogue *curDiag;
	int startMS = -1;
	for (std::list<AssEntry*>::iterator cur = AssFile::top->Line.begin(); cur != AssFile::top->Line.end(); cur++) {
		curEntry = *cur;
		curDiag = AssEntry::GetAsDialogue(curEntry);
		if (curDiag) startMS = curDiag->Start.GetMS();
		curEntry->StartMS = startMS;
	}

	// Sort
	AssFile::top->Line.sort(LessByPointedToValue<AssEntry>());
	AssFile::top->FlagAsModified();
	SubsBox->UpdateMaps();
	SubsBox->CommitChanges();
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
		if (AssFile::top->loaded && AssFile::top->CanSave()) {
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


/////////////////////////////
// Commit Edit Box's changes
void FrameMain::OnEditBoxCommit(wxCommandEvent &event) {
	// Find focus
	wxWindow *focus = FindFocus();

	// Is the text edit
	if (focus == EditBox->TextEdit) {
		EditBox->CommitText();
		SubsBox->ass->FlagAsModified();
		SubsBox->CommitChanges();
	}

	// Other window
	else {
		wxKeyEvent keyevent;
		keyevent.m_keyCode = WXK_RETURN;
		keyevent.m_controlDown = true;
		keyevent.SetEventType(wxEVT_KEY_DOWN);
		focus->AddPendingEvent(keyevent);
	}
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
