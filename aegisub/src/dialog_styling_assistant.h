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

/// @file dialog_styling_assistant.h
/// @see dialog_styling_assistant.cpp
/// @ingroup tools_ui
///

#include "selection_controller.h"

#ifndef AGI_PRE
#include <wx/dialog.h>
#include <wx/event.h>
#endif

#include <libaegisub/scoped_ptr.h>

namespace agi { struct Context; }
class AssDialogue;
class PersistLocation;
class wxButton;
class wxCheckBox;
class wxListBox;
class wxTextCtrl;

/// DOCME
/// @class DialogStyling
/// @brief DOCME
///
/// DOCME
class DialogStyling : public wxDialog, public SelectionListener<AssDialogue> {
	agi::Context *c;

	wxButton *play_audio;
	wxButton *play_video;
	wxCheckBox *auto_seek;
	wxListBox *style_list;
	wxTextCtrl *current_line_text;
	wxTextCtrl *style_name;

	void OnActivate(wxActivateEvent &evt);
	void OnKeyDown(wxKeyEvent &evt);
	void OnListClicked(wxCommandEvent &evt);
	void OnListDoubleClicked(wxCommandEvent &evt);
	void OnPlayAudioButton(wxCommandEvent &evt);
	void OnPlayVideoButton(wxCommandEvent &evt);
	void OnShow(wxShowEvent &evt);
	void OnStyleBoxModified(wxCommandEvent &evt);

	void OnActiveLineChanged(AssDialogue *);
	void OnSelectedSetChanged(Selection const&, Selection const&) { }

	AssDialogue *active_line;

	agi::scoped_ptr<PersistLocation> persist;

public:
	void Commit(bool next);

	DialogStyling(agi::Context *context);
	~DialogStyling();
};
