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


#ifndef FRAME_MAIN_H

/// DOCME
#define FRAME_MAIN_H


///////////////////
// Include headers
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


////////////////////
// Class prototypes
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

/// DOCME
namespace Automation4 { class FeatureMacro; class ScriptManager; };



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

	/// DOCME
	wxLogWindow *LogWindow;

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
	void OnKeyDown(wxKeyEvent &event);

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
	void OnCheckUpdates (wxCommandEvent &event);
	void OnContents (wxCommandEvent &event);
	void OnWebsite (wxCommandEvent &event);
	void OnForums (wxCommandEvent &event);
	void OnBugTracker (wxCommandEvent &event);
	void OnIRCChannel (wxCommandEvent &event);

	void OnNewSubtitles (wxCommandEvent &event);
	void OnOpenSubtitles (wxCommandEvent &event);
	void OnOpenSubtitlesCharset (wxCommandEvent &event);
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
	void OnPickAssociations (wxCommandEvent &event);
	void OnViewStandard (wxCommandEvent &event);
	void OnViewVideo (wxCommandEvent &event);
	void OnViewAudio (wxCommandEvent &event);
	void OnViewSubs (wxCommandEvent &event);

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
	void OnSort (wxCommandEvent &event);
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

	void OnOpenOptions (wxCommandEvent &event);
	void OnOpenLog (wxCommandEvent &event);
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


////////////////
// Menu Entries
enum {

	/// DOCME
	Menu_File_New = 200,

	/// DOCME
	Menu_File_Open,

	/// DOCME
	Menu_File_Save,

	/// DOCME
	Menu_File_SaveAs,

	/// DOCME
	Menu_File_Close,

	/// DOCME
	Menu_File_Open_Video,

	/// DOCME
	Menu_File_Close_Video,

	/// DOCME
	Menu_File_Open_Subtitles,

	/// DOCME
	Menu_File_Open_Subtitles_Charset,

	/// DOCME
	Menu_File_New_Subtitles,

	/// DOCME
	Menu_File_Save_Subtitles,

	/// DOCME
	Menu_File_Save_Subtitles_As,

	/// DOCME
	Menu_File_Save_Subtitles_With_Charset,

	/// DOCME
	Menu_File_Export_Subtitles,

	/// DOCME
	Menu_File_Open_VFR,

	/// DOCME
	Menu_File_Save_VFR,

	/// DOCME
	Menu_File_Close_VFR,

	/// DOCME
	Menu_File_New_Window,

	/// DOCME
	Menu_File_Exit,


	/// DOCME
	Menu_File_Recent_Subs_Parent,

	/// DOCME
	Menu_File_Recent_Vids_Parent,

	/// DOCME
	Menu_File_Recent_Auds_Parent,

	/// DOCME
	Menu_File_Recent_Timecodes_Parent,

	/// DOCME
	Menu_File_Recent_Keyframes_Parent,


	/// DOCME
	Menu_Video_JumpTo,

	/// DOCME
	Menu_View_Zoom,

	/// DOCME
	Menu_View_Zoom_50,

	/// DOCME
	Menu_View_Zoom_100,

	/// DOCME
	Menu_View_Zoom_200,

	/// DOCME
	Menu_Video_Zoom_In,

	/// DOCME
	Menu_Video_Zoom_Out,

	/// DOCME
	Menu_Video_Load_Keyframes,

	/// DOCME
	Menu_Video_Save_Keyframes,

	/// DOCME
	Menu_Video_Close_Keyframes,

	/// DOCME
	Toolbar_Zoom_Dropdown,

	/// DOCME
	Menu_Video_AR,

	/// DOCME
	Menu_Video_AR_Default,

	/// DOCME
	Menu_Video_AR_Full,

	/// DOCME
	Menu_Video_AR_Wide,

	/// DOCME
	Menu_Video_AR_235,

	/// DOCME
	Menu_Video_AR_Custom,

	/// DOCME
	Menu_Video_Select_Visible,

	/// DOCME
	Menu_Video_Play,

	/// DOCME
	Menu_Video_Detach,

	/// DOCME
	Menu_Video_Dummy,

	/// DOCME
	Menu_Video_Overscan,

	/// DOCME
	Menu_Video_Details,


	/// DOCME
	Menu_Audio_Open_File,

	/// DOCME
	Menu_Audio_Open_From_Video,

	/// DOCME
	Menu_Audio_Close,
#ifdef _DEBUG

	/// DOCME
	Menu_Audio_Open_Dummy,

	/// DOCME
	Menu_Audio_Open_Dummy_Noise,
#endif


	/// DOCME
	Menu_Edit_Select,

	/// DOCME
	Menu_Edit_Undo,

	/// DOCME
	Menu_Edit_Redo,

	/// DOCME
	Menu_Edit_Sort,

	/// DOCME
	Menu_Edit_Find,

	/// DOCME
	Menu_Edit_Find_Next,

	/// DOCME
	Menu_Edit_Replace,

	/// DOCME
	Menu_Edit_Shift,

	/// DOCME
	Menu_Edit_Cut,

	/// DOCME
	Menu_Edit_Copy,

	/// DOCME
	Menu_Edit_Paste,

	/// DOCME
	Menu_Edit_Paste_Over,

	/// DOCME
	Menu_Edit_Delete,


	/// DOCME
	Menu_View_Language,

	/// DOCME
	Menu_View_Associations,

	/// DOCME
	Menu_View_Standard,

	/// DOCME
	Menu_View_Audio,

	/// DOCME
	Menu_View_Video,

	/// DOCME
	Menu_View_Subs,


	/// DOCME
	Menu_Subtitles_Join,

	/// DOCME
	Menu_Subtitles_Recombine,

	/// DOCME
	Menu_Subtitles_Insert,


	/// DOCME
	Menu_Tools_Properties,

	/// DOCME
	Menu_Tools_Styles_Manager,

	/// DOCME
	Menu_Tools_Attachments,

	/// DOCME
	Menu_Tools_Translation,

	/// DOCME
	Menu_Tools_SpellCheck,

	/// DOCME
	Menu_Tools_Fonts_Collector,

	/// DOCME
	Menu_Tools_Automation,

	/// DOCME
	Menu_Tools_Styling,

	/// DOCME
	Menu_Tools_Resample,

	/// DOCME
	Menu_Tools_Timing_Processor,

	/// DOCME
	Menu_Tools_Kanji_Timer,

	/// DOCME
	Menu_Tools_Options,

	/// DOCME
	Menu_Tools_Log,

	/// DOCME
	Menu_Tools_ASSDraw,


	/// DOCME
	Menu_Help_Contents,

	/// DOCME
	Menu_Help_IRCChannel,

	/// DOCME
	Menu_Help_Website,

	/// DOCME
	Menu_Help_Forums,

	/// DOCME
	Menu_Help_BugTracker,

	/// DOCME
	Menu_Help_Check_Updates,

	/// DOCME
	Menu_Help_About,


	/// DOCME
	Menu_Subs_Snap_Start_To_Video,

	/// DOCME
	Menu_Subs_Snap_End_To_Video,

	/// DOCME
	Menu_Subs_Snap_Video_To_Start,

	/// DOCME
	Menu_Subs_Snap_Video_To_End,

	/// DOCME
	Menu_Video_Snap_To_Scene,

	/// DOCME
	Menu_Video_Shift_To_Frame,


	/// DOCME
	AutoSave_Timer,

	/// DOCME
	StatusClear_Timer,


	/// DOCME
	Video_Next_Frame,

	/// DOCME
	Video_Prev_Frame,

	/// DOCME
	Video_Focus_Seek,

	/// DOCME
	Grid_Next_Line,

	/// DOCME
	Grid_Prev_Line,

	/// DOCME
	Grid_Toggle_Tags,

	/// DOCME
	Edit_Box_Commit,


	/// DOCME
	Video_Frame_Play,


	/// DOCME
	Medusa_Play,

	/// DOCME
	Medusa_Stop,

	/// DOCME
	Medusa_Shift_Start_Forward,

	/// DOCME
	Medusa_Shift_Start_Back,

	/// DOCME
	Medusa_Shift_End_Forward,

	/// DOCME
	Medusa_Shift_End_Back,

	/// DOCME
	Medusa_Play_Before,

	/// DOCME
	Medusa_Play_After,

	/// DOCME
	Medusa_Next,

	/// DOCME
	Medusa_Prev,

	/// DOCME
	Medusa_Enter,


	/// DOCME
	Menu_File_Recent = 2000,

	/// DOCME
	Menu_Video_Recent = 2200,

	/// DOCME
	Menu_Audio_Recent = 2400,

	/// DOCME
	Menu_Timecodes_Recent = 2500,

	/// DOCME
	Menu_Keyframes_Recent = 2600,

	/// DOCME
	Menu_Automation_Macro = 2700
};


#endif


