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

/// @file persist_location.cpp
/// @see persist_location.h
/// @ingroup utility

#include "config.h"

#include "persist_location.h"

#include "main.h"

#ifndef AGI_PRE
#include <wx/dialog.h>
#include <wx/display.h>
#endif

PersistLocation::PersistLocation(wxDialog *dialog, std::string options_prefix, bool size_too)
: x_opt(OPT_SET(options_prefix + "/Last/X"))
, y_opt(OPT_SET(options_prefix + "/Last/Y"))
, w_opt(size_too ? OPT_SET(options_prefix + "/Last/Width") : 0)
, h_opt(size_too ? OPT_SET(options_prefix + "/Last/Height") : 0)
, maximize_opt(OPT_SET(options_prefix + "/Maximized"))
, dialog(dialog)
{
	int x = x_opt->GetInt();
	int y = y_opt->GetInt();
	if (x == -1 && y == -1)
		dialog->CenterOnParent();
	else {
		// First move to the saved place so that it ends up on the right monitor
		dialog->Move(x, y);

		if (size_too && w_opt->GetInt() > 0 && h_opt->GetInt() > 0)
			dialog->SetSize(w_opt->GetInt(), h_opt->GetInt());

		int display_index = wxDisplay::GetFromWindow(dialog);

		// If it's moved offscreen center on the parent and try again
		if (display_index == wxNOT_FOUND) {
			dialog->CenterOnParent();
			display_index = wxDisplay::GetFromWindow(dialog);
		}

		// If it's still offscreen just give up
		if (display_index == wxNOT_FOUND) return;

		wxRect display_area = wxDisplay(display_index).GetClientArea();
		wxSize dialog_size = dialog->GetSize();

		// Ensure that the top-left corner is onscreen
		if (x < display_area.x) x = display_area.x;
		if (y < display_area.y) y = display_area.y;

		// Ensure that the bottom-right corner is onscreen as long as doing so
		// wouldn't force the top-left corner offscreen
		if (x + dialog_size.x > display_area.GetRight())
			x = std::max(display_area.x, display_area.GetRight() - dialog_size.x);
		if (y + dialog_size.y > display_area.GetBottom())
			y = std::max(display_area.y, display_area.GetBottom() - dialog_size.y);

		dialog->Move(x, y);
	}

	dialog->Bind(wxEVT_MOVE, &PersistLocation::OnMove, this);

	dialog->Bind(wxEVT_SIZE, &PersistLocation::OnSize, this);
	if ((dialog->GetWindowStyle() & wxMAXIMIZE_BOX) && maximize_opt->GetBool())
		dialog->Maximize();
}

void PersistLocation::OnMove(wxMoveEvent &e) {
	wxPoint pos = dialog->GetPosition();
	x_opt->SetInt(pos.x);
	y_opt->SetInt(pos.y);
	e.Skip();
}

void PersistLocation::OnSize(wxSizeEvent &e) {
	maximize_opt->SetBool(dialog->IsMaximized());
	if (w_opt) {
		w_opt->SetInt(dialog->GetSize().GetWidth());
		h_opt->SetInt(dialog->GetSize().GetHeight());
	}
	e.Skip();
}
