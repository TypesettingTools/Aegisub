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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file frame_main.h
/// @see frame_main.cpp
/// @ingroup main_ui
///

#ifndef AGI_PRE
#include <vector>

#include <wx/combobox.h>
#include <wx/frame.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#endif


class VideoDisplay;
class VideoSlider;
class VideoZoomSlider;
class SubtitlesGrid;
class SubsEditBox;
class AudioBox;
class VideoBox;
class DialogDetachedVideo;
class DialogStyling;
class AegisubFileDropTarget;

namespace Automation4 { class FeatureMacro; class ScriptManager; }



/// DOCME
/// @class FrameMain
/// @brief DOCME
///
/// DOCME
class FrameMain: public wxFrame {
	friend class AegisubFileDropTarget;
	friend class AegisubApp;
	friend class SubtitlesGrid;

private:

	/// DOCME

	/// DOCME
	bool showVideo,showAudio;

	/// DOCME
	bool HasSelection;

	/// DOCME
	bool menuCreated;

	/// DOCME
	wxTimer AutoSave;

	/// DOCME
	wxTimer StatusClear;


	/// DOCME
	bool blockAudioLoad;

	/// DOCME
	bool blockVideoLoad;


	/// DOCME
	wxPanel *Panel;


	/// DOCME
	wxMenuBar *MenuBar;

	/// DOCME
	wxMenu *fileMenu;

	/// DOCME
	wxMenu *editMenu;

	/// DOCME
	wxMenu *videoMenu;

	/// DOCME
	wxMenu *timingMenu;

	/// DOCME
	wxMenu *subtitlesMenu;

	/// DOCME
	wxMenu *helpMenu;

	/// DOCME
	wxMenu *audioMenu;

	/// DOCME
	wxMenu *viewMenu;

	/// DOCME
	wxMenu *automationMenu;

	/// DOCME
	wxMenu *kanjiTimingMenu;


	/// DOCME
	wxMenu *RecentSubs;

	/// DOCME
	wxMenu *RecentVids;

	/// DOCME
	wxMenu *RecentAuds;

	/// DOCME
	wxMenu *RecentTimecodes;

	/// DOCME
	wxMenu *RecentKeyframes;


	/// DOCME
	wxToolBar *Toolbar;

	/// DOCME
	wxComboBox *ZoomBox;


	/// DOCME
	wxWindow *PreviousFocus;

#ifdef WITH_AUTOMATION

	/// DOCME
	Automation4::ScriptManager *local_scripts;
#endif


	/// DOCME
	std::vector<Automation4::FeatureMacro*> activeMacroItems;
	int AddMacroMenuItems(wxMenu *menu, const std::vector<Automation4::FeatureMacro*> &macros);

	void InitToolbar();
	void InitContents();
	void DeInitContents();

	void OnAutoSave(wxTimerEvent &event);
	void OnStatusClear(wxTimerEvent &event);

	void OnVideoPlay(wxCommandEvent &event);

	void OnOpenRecentSubs (wxCommandEvent &event);
	void OnOpenRecentVideo (wxCommandEvent &event);
	void OnOpenRecentAudio (wxCommandEvent &event);
	void OnOpenRecentTimecodes (wxCommandEvent &event);
	void OnOpenRecentKeyframes (wxCommandEvent &event);

	void OnNewWindow (wxCommandEvent &event);
	void OnCloseWindow (wxCloseEvent &event);
	void OnMenuOpen (wxMenuEvent &event);
	void OnExit(wxCommandEvent &WXUNUSED(event));
	void OnAbout (wxCommandEvent &event);
	void OnLog (wxCommandEvent &event);
	void OnCheckUpdates (wxCommandEvent &event);
	void OnContents (wxCommandEvent &event);
	void OnFiles (wxCommandEvent &event);
	void OnWebsite (wxCommandEvent &event);
	void OnForums (wxCommandEvent &event);
	void OnBugTracker (wxCommandEvent &event);
	void OnIRCChannel (wxCommandEvent &event);

	void OnNewSubtitles (wxCommandEvent &event);
	void OnOpenSubtitles (wxCommandEvent &event);
	void OnOpenSubtitlesCharset (wxCommandEvent &event);
	void OnOpenSubtitlesVideo (wxCommandEvent &event);
	void OnSaveSubtitles (wxCommandEvent &event);
	void OnSaveSubtitlesAs (wxCommandEvent &event);
	void OnSaveSubtitlesCharset (wxCommandEvent &event);
	void OnExportSubtitles (wxCommandEvent &event);
	void OnOpenVideo (wxCommandEvent &event);
	void OnCloseVideo (wxCommandEvent &event);
	void OnOpenVFR (wxCommandEvent &event);
	void OnSaveVFR (wxCommandEvent &event);
	void OnCloseVFR (wxCommandEvent &event);
	void OnOpenKeyframes (wxCommandEvent &event);
	void OnCloseKeyframes (wxCommandEvent &event);
	void OnSaveKeyframes (wxCommandEvent &event);

	void OnZoomIn (wxCommandEvent &event);
	void OnZoomOut (wxCommandEvent &event);
	void OnSetZoom50 (wxCommandEvent &event);
	void OnSetZoom100 (wxCommandEvent &event);
	void OnSetZoom200 (wxCommandEvent &event);
	void OnSetZoom (wxCommandEvent &event);
	void OnSetARDefault (wxCommandEvent &event);
	void OnSetARWide (wxCommandEvent &event);
	void OnSetARFull (wxCommandEvent &event);
	void OnSetAR235 (wxCommandEvent &event);
	void OnSetARCustom (wxCommandEvent &event);
	void OnDetachVideo (wxCommandEvent &event);
	void OnDummyVideo (wxCommandEvent &event);
	void OnOverscan (wxCommandEvent &event);

	void OnOpenAudio (wxCommandEvent &event);
	void OnOpenAudioFromVideo (wxCommandEvent &event);
	void OnCloseAudio (wxCommandEvent &event);
#ifdef _DEBUG
	void OnOpenDummyAudio(wxCommandEvent &event);
	void OnOpenDummyNoiseAudio(wxCommandEvent &event);
#endif

	void OnChooseLanguage (wxCommandEvent &event);
	void OnViewStandard (wxCommandEvent &event);
	void OnViewVideo (wxCommandEvent &event);
	void OnViewAudio (wxCommandEvent &event);
	void OnViewSubs (wxCommandEvent &event);
	void OnSetTags (wxCommandEvent &event);

	void OnUndo (wxCommandEvent &event);
	void OnRedo (wxCommandEvent &event);
	void OnCut (wxCommandEvent &event);
	void OnCopy (wxCommandEvent &event);
	void OnPaste (wxCommandEvent &event);
	void OnPasteOver (wxCommandEvent &event);
	void OnDelete (wxCommandEvent &event);
	void OnFind (wxCommandEvent &event);
	void OnFindNext (wxCommandEvent &event);
	void OnReplace (wxCommandEvent &event);
	void OnJumpTo (wxCommandEvent &event);
	void OnShift (wxCommandEvent &event);
	void OnSortStart (wxCommandEvent &event);
	void OnSortEnd (wxCommandEvent &event);
	void OnSortStyle (wxCommandEvent &event);
	void OnEditBoxCommit (wxCommandEvent &event);
	void OnOpenProperties (wxCommandEvent &event);
	void OnOpenStylesManager (wxCommandEvent &event);
	void OnOpenAttachments (wxCommandEvent &event);
	void OnOpenTranslation (wxCommandEvent &event);
	void OnOpenSpellCheck (wxCommandEvent &event);
	void OnOpenFontsCollector (wxCommandEvent &event);
	void OnSnapSubsStartToVid (wxCommandEvent &event);
	void OnSnapSubsEndToVid (wxCommandEvent &event);
	void OnSnapVidToSubsStart (wxCommandEvent &event);
	void OnSnapVidToSubsEnd (wxCommandEvent &event);
	void OnSnapToScene (wxCommandEvent &event);
	void OnShiftToFrame (wxCommandEvent &event);
	void OnSelectVisible (wxCommandEvent &event);
	void OnSelect (wxCommandEvent &event);
	void OnOpenStylingAssistant (wxCommandEvent &event);
	void OnOpenResample (wxCommandEvent &event);
	void OnOpenTimingProcessor (wxCommandEvent &event);
	void OnOpenKanjiTimer (wxCommandEvent &event);
	void OnOpenVideoDetails (wxCommandEvent &event);
	void OnOpenASSDraw (wxCommandEvent &event);

	void OnOpenPreferences (wxCommandEvent &event);
	void OnGridEvent (wxCommandEvent &event);

	void OnOpenAutomation (wxCommandEvent &event);
	void OnAutomationMacro(wxCommandEvent &event);

	void OnNextFrame(wxCommandEvent &event);
	void OnPrevFrame(wxCommandEvent &event);
	void OnFocusSeek(wxCommandEvent &event);
	void OnNextLine(wxCommandEvent &event);
	void OnPrevLine(wxCommandEvent &event);
	void OnToggleTags(wxCommandEvent &event);

	void OnMedusaPlay(wxCommandEvent &event);
	void OnMedusaStop(wxCommandEvent &event);
	void OnMedusaShiftStartForward(wxCommandEvent &event);
	void OnMedusaShiftStartBack(wxCommandEvent &event);
	void OnMedusaShiftEndForward(wxCommandEvent &event);
	void OnMedusaShiftEndBack(wxCommandEvent &event);
	void OnMedusaPlayBefore(wxCommandEvent &event);
	void OnMedusaPlayAfter(wxCommandEvent &event);
	void OnMedusaEnter(wxCommandEvent &event);
	void OnMedusaNext(wxCommandEvent &event);
	void OnMedusaPrev(wxCommandEvent &event);

	void LoadVideo(wxString filename,bool autoload=false);
	void LoadAudio(wxString filename,bool FromVideo=false);
	void LoadVFR(wxString filename);
	void SaveVFR(wxString filename);
	void LoadSubtitles(wxString filename,wxString charset=_T(""));
	bool SaveSubtitles(bool saveas=false,bool withCharset=false);
	int TryToCloseSubs(bool enableCancel=true);

	void RebuildRecentList(wxString listName,wxMenu *menu,int startID);
	void SynchronizeProject(bool FromSubs=false);

public:

	/// DOCME
	SubtitlesGrid *SubsBox;

	/// DOCME
	SubsEditBox *EditBox;

	/// DOCME
	AudioBox *audioBox;

	/// DOCME
	VideoBox *videoBox;

	/// DOCME
	DialogDetachedVideo *detachedVideo;

	/// DOCME
	DialogStyling *stylingAssistant;


	/// DOCME
	wxBoxSizer *MainSizer;

	/// DOCME
	wxBoxSizer *TopSizer;

	/// DOCME
	wxBoxSizer *BottomSizer;

	/// DOCME
	wxBoxSizer *ToolSizer;

	FrameMain (wxArrayString args);
	~FrameMain ();

	bool LoadList(wxArrayString list);
	static void OpenHelp(wxString page=_T(""));
	void UpdateTitle();
	void StatusTimeout(wxString text,int ms=10000);
	void DetachVideo(bool detach=true);

	void SetAccelerators();
	void InitMenu();
	void UpdateToolbar();
	void SetDisplayMode(int showVid,int showAudio);
	
	void SetUndoRedoDesc();
	bool HasASSDraw();

	DECLARE_EVENT_TABLE()
};


/// Menu entry event ids
enum {
	Menu_File_New = 200,
	Menu_File_Open,
	Menu_File_Save,
	Menu_File_SaveAs,
	Menu_File_Close,
	Menu_File_Open_Video,
	Menu_File_Close_Video,
	Menu_File_Open_Subtitles,
	Menu_File_Open_Subtitles_Charset,
	Menu_File_Open_Subtitles_From_Video,
	Menu_File_New_Subtitles,
	Menu_File_Save_Subtitles,
	Menu_File_Save_Subtitles_As,
	Menu_File_Save_Subtitles_With_Charset,
	Menu_File_Export_Subtitles,
	Menu_File_Open_VFR,
	Menu_File_Save_VFR,
	Menu_File_Close_VFR,
	Menu_File_New_Window,
	Menu_File_Exit,

	Menu_File_Recent_Subs_Parent,
	Menu_File_Recent_Vids_Parent,
	Menu_File_Recent_Auds_Parent,
	Menu_File_Recent_Timecodes_Parent,
	Menu_File_Recent_Keyframes_Parent,

	Menu_Video_JumpTo,
	Menu_View_Zoom,
	Menu_View_Zoom_50,
	Menu_View_Zoom_100,
	Menu_View_Zoom_200,
	Menu_Video_Zoom_In,
	Menu_Video_Zoom_Out,
	Menu_Video_Load_Keyframes,
	Menu_Video_Save_Keyframes,
	Menu_Video_Close_Keyframes,
	Toolbar_Zoom_Dropdown,
	Menu_Video_AR,
	Menu_Video_AR_Default,
	Menu_Video_AR_Full,
	Menu_Video_AR_Wide,
	Menu_Video_AR_235,
	Menu_Video_AR_Custom,
	Menu_Video_Select_Visible,
	Menu_Video_Play,
	Menu_Video_Detach,
	Menu_Video_Dummy,
	Menu_Video_Overscan,
	Menu_Video_Details,

	Menu_Audio_Open_File,
	Menu_Audio_Open_From_Video,
	Menu_Audio_Close,
#ifdef _DEBUG
	Menu_Audio_Open_Dummy,
	Menu_Audio_Open_Dummy_Noise,
#endif

	Menu_Edit_Select,
	Menu_Edit_Undo,
	Menu_Edit_Redo,
	Menu_Edit_Find,
	Menu_Edit_Find_Next,
	Menu_Edit_Replace,
	Menu_Edit_Shift,
	Menu_Edit_Cut,
	Menu_Edit_Copy,
	Menu_Edit_Paste,
	Menu_Edit_Paste_Over,
	Menu_Edit_Delete,

	Menu_View_Language,
	Menu_View_Standard,
	Menu_View_Audio,
	Menu_View_Video,
	Menu_View_Subs,
	Menu_View_FullTags,
	Menu_View_ShortTags,
	Menu_View_NoTags,

	Menu_Subtitles_Join,
	Menu_Subtitles_Recombine,
	Menu_Subtitles_Insert,
	
	Menu_Subtitles_Sort_Start,
	Menu_Subtitles_Sort_End,
	Menu_Subtitles_Sort_Style,
	Menu_Tools_Properties,
	Menu_Tools_Styles_Manager,
	Menu_Tools_Attachments,
	Menu_Tools_Translation,
	Menu_Tools_SpellCheck,
	Menu_Tools_Fonts_Collector,
	Menu_Tools_Automation,
	Menu_Tools_Styling,
	Menu_Tools_Resample,
	Menu_Tools_Timing_Processor,
	Menu_Tools_Kanji_Timer,
	Menu_Tools_Options,
	Menu_Tools_ASSDraw,

	Menu_Help_Contents,
	Menu_Help_Files,
	Menu_Help_IRCChannel,
	Menu_Help_Website,
	Menu_Help_Forums,
	Menu_Help_BugTracker,
	Menu_Help_Check_Updates,
	Menu_Help_About,

	Menu_Help_Log,
	Menu_Subs_Snap_Start_To_Video,
	Menu_Subs_Snap_End_To_Video,
	Menu_Subs_Snap_Video_To_Start,
	Menu_Subs_Snap_Video_To_End,
	Menu_Video_Snap_To_Scene,
	Menu_Video_Shift_To_Frame,

	AutoSave_Timer,
	StatusClear_Timer,

	Video_Next_Frame,
	Video_Prev_Frame,
	Video_Focus_Seek,
	Grid_Next_Line,
	Grid_Prev_Line,
	Grid_Toggle_Tags,

	Edit_Box_Commit,

	Video_Frame_Play,

	Medusa_Play,
	Medusa_Stop,
	Medusa_Shift_Start_Forward,
	Medusa_Shift_Start_Back,
	Medusa_Shift_End_Forward,
	Medusa_Shift_End_Back,
	Medusa_Play_Before,
	Medusa_Play_After,
	Medusa_Next,
	Medusa_Prev,
	Medusa_Enter,

	Menu_File_Recent = 2000,
	Menu_Video_Recent = 2200,
	Menu_Audio_Recent = 2400,
	Menu_Timecodes_Recent = 2500,
	Menu_Keyframes_Recent = 2600,
	Menu_Automation_Macro = 2700
};
