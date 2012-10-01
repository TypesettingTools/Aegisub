// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file audio_karaoke.h
/// @see audio_karaoke.cpp
/// @ingroup audio_ui
///

#ifndef AGI_PRE
#include <set>
#include <vector>

#include <wx/bitmap.h>
#include <wx/window.h>
#endif

#include <libaegisub/scoped_ptr.h>
#include <libaegisub/signal.h>

#include "selection_controller.h"

class AssDialogue;
class AssKaraoke;
class wxButton;

namespace agi { struct Context; }

/// @class AudioKaraoke
/// @brief Syllable split and join UI for karaoke
///
/// This class has two main responsibilities: the syllable split/join UI, and
/// the karaoke mode controller. The split/join UI consists of the dialogue
/// line with spaces and lines at each syllable split point. Clicking on a line
/// removes that \k tag; clicking anywhere else inserts a new \k tag with
/// interpolated duration. Added or removed splits are not autocommitted and
/// must be explicitly accepted or rejected. This is for two reasons:
///   1. It's easy for a stray click on the split/join bar to go unnoticed,
///      making autocommitting somewhat error-prone.
///   2. When a line with zero \k tags is activated, it's automatically split
///      at each space. This clearly should not automatically update the line
///      (changing the active selection should never directly change the file
///      itself), so there must be a notion of pending splits.
///
/// As the karaoke controller, it owns the AssKaraoke instance shared by this
/// class and the karaoke timing controller, and is responsible for switching
/// between timing controllers when entering and leaving karaoke mode. Ideally
/// the creation of the dialogue timing controller should probably be done
/// elsewhere, but there currently isn't any particularly appropriate place and
/// it's not worth caring about. The KaraokeController duties should perhaps be
/// split off into its own class, but at the moment they're insignificant
/// enough that it's not worth it.
///
/// The shared AssKaraoke instance is primarily to improve the handling of
/// pending splits. When a split is added removed, or a line is autosplit,
/// the audio display immediately reflects the changes, but the file is not
/// actually updated until the line is committed (which if auto-commit timing
/// changes is on, will happen as soon as the user adjusts the timing of the
/// new syllable).
class AudioKaraoke : public wxWindow, private SelectionListener<AssDialogue> {
	agi::Context *c; ///< Project context
	agi::signal::Connection file_changed; ///< File changed slot
	agi::signal::Connection audio_opened; ///< Audio opened connection
	agi::signal::Connection audio_closed; ///< Audio closed connection

	/// Currently active dialogue line
	AssDialogue *active_line;
	/// Karaoke data
	agi::scoped_ptr<AssKaraoke> kara;

	/// Current line's stripped text with spaces added between each syllable
	wxString spaced_text;

	/// spaced_text + syl_lines rendered to a bitmap
	wxBitmap rendered_line;

	/// Indexes in spaced_text which are the beginning of syllables
	std::vector<int> syl_start_points;

	/// x coordinate in pixels of the separator lines of each syllable
	std::vector<int> syl_lines;

	/// Left x coordinate of each character in spaced_text in pixels
	std::vector<int> char_x;

	int scroll_x;
	int scroll_dir;
	wxTimer scroll_timer;

	int char_height; ///< Maximum character height in pixels
	int char_width; ///< Maximum character width in pixels
	int mouse_pos; ///< Last x coordinate of the mouse

	wxFont split_font; ///< Font used in the split/join interface

	bool enabled; ///< Is karaoke mode enabled?

	wxButton *accept_button; ///< Accept pending splits button
	wxButton *cancel_button; ///< Revert pending changes

	wxWindow *split_area; ///< The split/join window

	/// Load syllable data from the currently active line
	void LoadFromLine();
	/// Cache presentational data from the loaded syllable data
	void SetDisplayText();

	/// Helper function for context menu creation
	void AddMenuItem(wxMenu &menu, wxString const& tag, wxString const& help, wxString const& selected);
	/// Set the karaoke tags for the selected syllables to the indicated one
	void SetTagType(wxString new_type);

	/// Prerender the current line along with syllable split lines
	void RenderText();

	/// Refresh the area of the display around a single character
	/// @param pos Index in spaced_text
	void LimitedRefresh(int pos);

	/// Reset all pending split information and return to normal mode
	void CancelSplit();
	/// Apply any pending split information to the syllable data and return to normal mode
	void AcceptSplit();

	void OnActiveLineChanged(AssDialogue *new_line);
	void OnContextMenu(wxContextMenuEvent&);
	void OnEnableButton(wxCommandEvent &evt);
	void OnFileChanged(int type);
	void OnMouse(wxMouseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnSelectedSetChanged(Selection const&, Selection const&) { }
	void OnAudioOpened();
	void OnAudioClosed();
	void OnScrollTimer(wxTimerEvent &event);

public:
	/// Constructor
	/// @param parent Parent window
	/// @param c Project context
	AudioKaraoke(wxWindow *parent, agi::Context *c);
	/// Destructor
	~AudioKaraoke();

	/// Is karaoke mode currently enabled?
	bool IsEnabled() const { return enabled; }

	/// Enable or disable karaoke mode
	void SetEnabled(bool enable);
};
