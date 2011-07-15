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

class AegisubFileDropTarget;
class AssFile;
class AudioBox;
class AudioController;
class AudioProvider;
class DialogDetachedVideo;
class DialogStyling;
class SubsEditBox;
class SubtitlesGrid;
class VideoBox;
class VideoDisplay;
class VideoSlider;
class VideoZoomSlider;

namespace agi { struct Context; }
namespace Automation4 { class FeatureMacro; class ScriptManager; }

/// DOCME
/// @class FrameMain
/// @brief DOCME
///
/// DOCME
class FrameMain: public wxFrame {
	friend class AegisubFileDropTarget;

public:
	/// Set the status bar text
	/// @param text New status bar text
	/// @param ms Time in milliseconds that the message should be visible
	void StatusTimeout(wxString text,int ms=10000);
	/// @brief Set the video and audio display visibility
	/// @param video -1: leave unchanged; 0: hide; 1: show
	/// @param audio -1: leave unchanged; 0: hide; 1: show
	void SetDisplayMode(int showVid, int showAudio);
	void LoadSubtitles(wxString filename,wxString charset="");
	void DetachVideo(bool detach=true);
	void LoadVFR(wxString filename);

	agi::scoped_ptr<agi::Context> context;

private:
    // XXX: Make Freeze()/Thaw() noops on GTK, this seems to be buggy
#ifdef __WXGTK__
    void Freeze(void) {}
    void Thaw(void) {}
#endif
	void cmd_call(wxCommandEvent& event);

	bool showVideo;       ///< Is the video display shown?
	bool showAudio;       ///< Is the audio display shown?
	wxTimer AutoSave;     ///< Autosave timer
	wxTimer StatusClear;  ///< Status bar timeout timer
	/// Block video loading; used when both video and subtitles are opened at
	/// the same time, so that the video associated with the subtitles (if any)
	/// isn't loaded
	bool blockVideoLoad;

	wxPanel *Panel;
	wxToolBar *Toolbar;  ///< The main toolbar
	wxComboBox *ZoomBox; ///< The video zoom dropdown in the main toolbar
	std::vector<Automation4::FeatureMacro*> activeMacroItems;

	int AddMacroMenuItems(wxMenu *menu, const std::vector<Automation4::FeatureMacro*> &macros);

	void InitToolbar();
	void InitContents();
	void InitMenu();

	bool LoadList(wxArrayString list);
	void UpdateTitle();

	void OnKeyDown(wxKeyEvent &event);
	void OnMenuOpen (wxMenuEvent &event);

	void OnAudioBoxResize(wxSashEvent &event);
	/// @brief Autosave the currently open file, if any
	void OnAutoSave(wxTimerEvent &event);
	void OnStatusClear(wxTimerEvent &event);
	void OnCloseWindow (wxCloseEvent &event);
	/// @brief General handler for all Automation-generated menu items
	void OnAutomationMacro(wxCommandEvent &event);

	/// Close the currently open subs, asking the user if they want to save if there are unsaved changes
	/// @param enableCancel Should the user be able to cancel the close?
	int TryToCloseSubs(bool enableCancel=true);

	void RebuildRecentList(const char *root_command, const char *mru_name);

	// AudioControllerAudioEventListener implementation
	void OnAudioOpen(AudioProvider *provider);
	void OnAudioClose();

	void OnVideoOpen();

	void OnSubtitlesOpen();
	void OnSubtitlesSave();


	SubtitlesGrid *SubsGrid; ///< The subtitle editing area
	SubsEditBox *EditBox;    ///< The subtitle editing textbox
	wxSashWindow *audioSash; ///< Sash for resizing the audio area
	AudioBox *audioBox;      ///< The audio area
	VideoBox *videoBox;      ///< The video area

	wxBoxSizer *MainSizer;  ///< Arranges things from top to bottom in the window
	wxBoxSizer *TopSizer;   ///< Arranges video box and tool box from left to right
	wxBoxSizer *ToolsSizer; ///< Arranges audio and editing areas top to bottom

public:
	FrameMain(wxArrayString args);
	~FrameMain();

	void UpdateToolbar();
	bool HasASSDraw();

	DECLARE_EVENT_TABLE()
};
