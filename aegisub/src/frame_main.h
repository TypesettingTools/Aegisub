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
#include <wx/sashwin.h>
#include <wx/timer.h>
#endif

#include <libaegisub/scoped_ptr.h>

class AssFile;
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
class AudioController;
class AudioProvider;

namespace agi { struct Context; }
namespace Automation4 { class FeatureMacro; class ScriptManager; }


enum SubMenuID {
	ID_TOOLBAR_ZOOM_DROPDOWN				= 11001,
	ID_APP_TIMER_AUTOSAVE					= 12001,
	ID_APP_TIMER_STATUSCLEAR				= 12002,
	ID_MENU_AUTOMATION_MACRO				= 13006,
	ID_SASH_MAIN_AUDIO						= 14001
};

/// DOCME
/// @class FrameMain
/// @brief DOCME
///
/// DOCME
class FrameMain: public wxFrame {
	friend class AegisubFileDropTarget;

public:
	void StatusTimeout(wxString text,int ms=10000);
	void SetDisplayMode(int showVid,int showAudio);
	void LoadSubtitles(wxString filename,wxString charset="");
	void DetachVideo(bool detach=true);
	void LoadVFR(wxString filename);

	agi::scoped_ptr<agi::Context> context;

private:
	void cmd_call(wxCommandEvent& event);

	/// DOCME

	/// DOCME
	bool showVideo,showAudio;

	/// DOCME
	bool HasSelection;

	/// DOCME
	wxTimer AutoSave;

	/// DOCME
	wxTimer StatusClear;


	/// DOCME
	bool blockVideoLoad;


	/// DOCME
	wxPanel *Panel;

	/// DOCME
	wxToolBar *Toolbar;

	/// DOCME
	wxComboBox *ZoomBox;

	/// DOCME
	std::vector<Automation4::FeatureMacro*> activeMacroItems;
	int AddMacroMenuItems(wxMenu *menu, const std::vector<Automation4::FeatureMacro*> &macros);

	void InitToolbar();
	void InitContents();
	void DeInitContents();

	bool LoadList(wxArrayString list);
	void UpdateTitle();

	void InitMenu();

	void SetUndoRedoDesc();

	void OnKeyDown(wxKeyEvent &event);
	void OnGridEvent (wxCommandEvent &event);
	void OnMenuOpen (wxMenuEvent &event);

	void OnAudioBoxResize(wxSashEvent &event);
	void OnAutoSave(wxTimerEvent &event);
	void OnStatusClear(wxTimerEvent &event);
	void OnCloseWindow (wxCloseEvent &event);
	void OnAutomationMacro(wxCommandEvent &event);

	int TryToCloseSubs(bool enableCancel=true);

	void RebuildRecentList(const char *root_command, const char *mru_name);

	// AudioControllerAudioEventListener implementation
	void OnAudioOpen(AudioProvider *provider);
	void OnAudioClose();

	void OnVideoOpen();

	void OnSubtitlesCommit();
	void OnSubtitlesOpen();
	void OnSubtitlesSave();


	/// The subtitle editing area
	SubtitlesGrid *SubsGrid;

	/// The subtitle editing textbox
	SubsEditBox *EditBox;

	/// Sash for resizing the audio area
	wxSashWindow *audioSash;

	/// The audio area
	AudioBox *audioBox;

	/// The video area
	VideoBox *videoBox;

	/// Arranges things from top to bottom in the window
	wxBoxSizer *MainSizer;

	/// Arranges video box and tool box from left to right
	wxBoxSizer *TopSizer;

	/// Arranges audio and editing areas top to bottom
	wxBoxSizer *ToolsSizer;

public:
	FrameMain(wxArrayString args);
	~FrameMain();

	void UpdateToolbar();
	static void OpenHelp(wxString page=_T(""));
	bool HasASSDraw();

	DECLARE_EVENT_TABLE()
};
