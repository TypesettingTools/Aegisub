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

/// @file subs_edit_box.h
/// @see subs_edit_box.cpp
/// @ingroup main_ui
///

#ifndef AGI_PRE
#include <deque>
#include <map>
#include <vector>

#include <wx/panel.h>
#include <wx/timer.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

#include "selection_controller.h"

namespace agi { namespace vfr { class Framerate; } }
namespace agi { struct Context; }
class AssDialogue;
class AssTime;
class SubsTextEditCtrl;
class TextSelectionController;
class TimeEdit;
class wxButton;
class wxCheckBox;
class wxComboBox;
class wxRadioButton;
class wxSizer;
class wxSpinCtrl;
class wxStyledTextCtrl;
class wxStyledTextEvent;
class wxTextCtrl;

template<class Base> class Placeholder;

/// DOCME
/// @class SubsEditBox
/// @brief Main subtitle edit box
///
/// Controls the text edit and all surrounding controls
class SubsEditBox : public wxPanel {
	enum TimeField {
		TIME_START = 0,
		TIME_END,
		TIME_DURATION
	};

	std::deque<agi::signal::Connection> connections;

	/// Currently active dialogue line
	AssDialogue *line;
	/// Last seen grid selection
	SubtitleSelection sel;

	/// Are the buttons currently split into two lines?
	bool splitLineMode;
	/// Are the controls currently enabled?
	bool controlState;

	agi::Context *c;

	agi::signal::Connection file_changed_slot;

	// Box controls
	wxCheckBox *CommentBox;
	wxComboBox *StyleBox;
	Placeholder<wxComboBox> *ActorBox;
	TimeEdit *StartTime;
	TimeEdit *EndTime;
	TimeEdit *Duration;
	wxSpinCtrl *Layer;
	wxTextCtrl *MarginL;
	wxTextCtrl *MarginR;
	wxTextCtrl *MarginV;
	Placeholder<wxComboBox> *Effect;
	wxRadioButton *ByTime;
	wxRadioButton *ByFrame;

	wxSizer *TopSizer;
	wxSizer *MiddleBotSizer;
	wxSizer *MiddleSizer;
	wxSizer *BottomSizer;

	void SetControlsState(bool state);
	/// @brief Update times of selected lines
	/// @param field Field which changed
	void CommitTimes(TimeField field);
	/// @brief Commits the current edit box contents
	/// @param desc Undo description to use
	void CommitText(wxString const& desc);

	/// Last commit ID for undo coalescing
	int commitId;

	/// Last used commit message to avoid coalescing different types of changes
	wxString lastCommitType;

	/// Last field to get a time commit, as they all have the same commit message
	int lastTimeCommitType;

	/// Timer to stop coalescing changes after a break with no edits
	wxTimer undoTimer;

	/// The start and end times of the selected lines without changes made to
	/// avoid negative durations, so that they can be restored if future changes
	/// eliminate the negative durations
	std::map<AssDialogue *, std::pair<AssTime, AssTime> > initialTimes;

	// Constructor helpers
	wxTextCtrl *MakeMarginCtrl(wxString const& tooltip, void (SubsEditBox::*handler)(wxCommandEvent&));
	TimeEdit *MakeTimeCtrl(bool end, wxString const& tooltip, void (SubsEditBox::*handler)(wxCommandEvent&));
	void MakeButton(const char *cmd_name);
	wxComboBox *MakeComboBox(wxString const& initial_text, int style, void (SubsEditBox::*handler)(wxCommandEvent&), wxString const& tooltip);
	wxRadioButton *MakeRadio(wxString const& text, bool start, wxString const& tooltip);

	void OnChange(wxStyledTextEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	void OnActiveLineChanged(AssDialogue *new_line);
	void OnSelectedSetChanged(const SubtitleSelection &, const SubtitleSelection &);

	void OnFrameTimeRadio(wxCommandEvent &event);
	void OnStyleChange(wxCommandEvent &event);
	void OnActorChange(wxCommandEvent &event);
	void OnLayerEnter(wxCommandEvent &event);
	void OnStartTimeChange(wxCommandEvent &);
	void OnEndTimeChange(wxCommandEvent &);
	void OnDurationChange(wxCommandEvent &);
	void OnMarginLChange(wxCommandEvent &);
	void OnMarginRChange(wxCommandEvent &);
	void OnMarginVChange(wxCommandEvent &);
	void OnCommentChange(wxCommandEvent &);
	void OnEffectChange(wxCommandEvent &);
	void OnSize(wxSizeEvent &event);
	void OnUndoTimer(wxTimerEvent&);

	void SetPlaceholderCtrl(wxControl *ctrl, wxString const& value);

	/// @brief Set a field in each selected line to a specified value
	/// @param set   Callable which does the setting
	/// @param value Value to pass to set
	/// @param desc  Undo description to use
	/// @param type  Commit type to use
	/// @param amend Coalesce sequences of commits of the same type
	template<class T, class setter>
	void SetSelectedRows(setter set, T value, wxString desc, int type, bool amend = false);

	/// @brief Set a field in each selected line to a specified value
	/// @param field Field to set
	/// @param value Value to set the field to
	/// @param desc  Undo description to use
	/// @param type  Commit type to use
	/// @param amend Coalesce sequences of commits of the same type
	template<class T>
	void SetSelectedRows(T AssDialogue::*field, T value, wxString desc, int type, bool amend = false);

	/// @brief Reload the current line from the file
	/// @param type AssFile::CommitType
	void OnCommit(int type);

	/// Regenerate a dropdown list with the unique values of a dialogue field
	void PopulateList(wxComboBox *combo, wxString AssDialogue::*field);

	/// @brief Enable or disable frame timing mode
	void UpdateFrameTiming(agi::vfr::Framerate const& fps);

	/// Call a command the restore focus to the edit box
	void CallCommand(const char *cmd_name);

	SubsTextEditCtrl *TextEdit;
	agi::scoped_ptr<TextSelectionController> textSelectionController;

public:
	/// @brief Constructor
	/// @param parent Parent window
	SubsEditBox(wxWindow *parent, agi::Context *context);
	~SubsEditBox();
};
