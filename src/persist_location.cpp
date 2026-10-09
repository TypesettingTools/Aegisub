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

/// @file persist_location.cpp
/// @see persist_location.h
/// @ingroup utility

#include "persist_location.h"

#include "options.h"

#include <wx/display.h>
#include <wx/toplevel.h>

PersistLocation::PersistLocation(wxTopLevelWindow *window, std::string options_prefix, bool size_too)
: x_opt(OPT_SET(options_prefix + "/Last/X"))
, y_opt(OPT_SET(options_prefix + "/Last/Y"))
, w_opt(size_too ? OPT_SET(options_prefix + "/Last/Width") : nullptr)
, h_opt(size_too ? OPT_SET(options_prefix + "/Last/Height") : nullptr)
, maximize_opt(OPT_SET(options_prefix + "/Maximized"))
, window(window)
{
	int x = x_opt->GetInt();
	int y = y_opt->GetInt();
	if (x == -1 && y == -1)
		window->CenterOnParent();
	else {
		// First move to the saved place so that it ends up on the right monitor
		window->Move(x, y);

		if (size_too && w_opt->GetInt() > 0 && h_opt->GetInt() > 0)
			window->SetSize(w_opt->GetInt(), h_opt->GetInt());

		int display_index = wxDisplay::GetFromWindow(window);

		// If it's moved offscreen center on the parent and try again
		if (display_index == wxNOT_FOUND) {
			window->CenterOnParent();
			display_index = wxDisplay::GetFromWindow(window);
		}

		// If it's still offscreen just leave it where it is
		if (display_index != wxNOT_FOUND) {
			wxRect display_area = wxDisplay(display_index).GetClientArea();
			wxSize window_size = window->GetSize();

			// Ensure that the top-left corner is onscreen
			if (x < display_area.x) x = display_area.x;
			if (y < display_area.y) y = display_area.y;

			// Ensure that the bottom-right corner is onscreen as long as doing so
			// wouldn't force the top-left corner offscreen
			if (x + window_size.x > display_area.GetRight())
				x = std::max(display_area.x, display_area.GetRight() - window_size.x);
			if (y + window_size.y > display_area.GetBottom())
				y = std::max(display_area.y, display_area.GetBottom() - window_size.y);

			window->Move(x, y);
		}
	}

	window->Bind(wxEVT_MOVE, &PersistLocation::OnMove, this);
	window->Bind(wxEVT_SIZE, &PersistLocation::OnSize, this);
	if ((window->GetWindowStyle() & wxMAXIMIZE_BOX) && maximize_opt->GetBool())
		window->Maximize();
}

PersistLocation::~PersistLocation() {
	window->Unbind(wxEVT_MOVE, &PersistLocation::OnMove, this);
	window->Unbind(wxEVT_SIZE, &PersistLocation::OnSize, this);
}

bool PersistLocation::IsTransient() const {
	return window->IsMaximized() || window->IsFullScreen() || window->IsIconized();
}

void PersistLocation::SavePosition() {
	if (IsTransient()) return;
	wxPoint pos = window->GetPosition();
	x_opt->SetInt(pos.x);
	y_opt->SetInt(pos.y);
}

void PersistLocation::OnMove(wxMoveEvent &e) {
	SavePosition();
	e.Skip();
}

void PersistLocation::OnSize(wxSizeEvent &e) {
	maximize_opt->SetBool(window->IsMaximized());
	// Resizing from the bottom-right doesn't send a move event, so save the
	// position too in case the window is still at its default location
	SavePosition();
	if (w_opt && !IsTransient()) {
		w_opt->SetInt(window->GetSize().GetWidth());
		h_opt->SetInt(window->GetSize().GetHeight());
	}
	e.Skip();
}
