// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file dialog_fonts_collector.h
/// @see dialog_fonts_collector.cpp
/// @ingroup tools_ui font_collector
///

#ifndef AGI_PRE
#include <wx/dialog.h>
#endif

namespace agi { struct Context; }

class AssFile;
class ScintillaTextCtrl;
class wxButton;
class wxRadioBox;
class wxStaticText;
class wxTextCtrl;
class wxThreadEvent;

/// DOCME
/// @class DialogFontsCollector
/// @brief DOCME
///
/// DOCME
class DialogFontsCollector : public wxDialog {
	AssFile *subs;

	ScintillaTextCtrl *collection_log;
	wxButton *close_btn;
	wxButton *dest_browse_button;
	wxButton *start_btn;
	wxRadioBox *collection_mode;
	wxStaticText *dest_label;
	wxTextCtrl *dest_ctrl;

	void OnStart(wxCommandEvent &);
	void OnBrowse(wxCommandEvent &);
	void OnRadio(wxCommandEvent &e);

	/// Append text to log message from worker thread
	void OnAddText(wxThreadEvent &event);
	/// Collection complete notification from the worker thread to reenable buttons
	void OnCollectionComplete(wxThreadEvent &);

public:
	DialogFontsCollector(agi::Context *c);
};
