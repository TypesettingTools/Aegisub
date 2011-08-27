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
// $Id$

/// @file menu.h
/// @brief Dynamic menu and toolbar generator.
/// @ingroup menu toolbar

#ifndef AGI_PRE
#include <string>
#endif

#include <libaegisub/exception.h>

namespace agi { struct Context; }

class wxMenu;
class wxMenuBar;

namespace menu {
	DEFINE_BASE_EXCEPTION_NOINNER(Error, agi::Exception)
	DEFINE_SIMPLE_EXCEPTION_NOINNER(UnknownMenu, Error, "menu/unknown")
	DEFINE_SIMPLE_EXCEPTION_NOINNER(InvalidMenu, Error, "menu/invalid")

	/// @brief Get the menu with the specified name as a wxMenuBar
	/// @param name Name of the menu
	///
	/// Throws:
	///     UnknownMenu if no menu with the given name was found
	///     BadMenu if there is a menu with the given name, but it is invalid
	void GetMenuBar(std::string const& name, wxFrame *window, agi::Context *c);

	/// @brief Get the menu with the specified name as a wxMenu
	/// @param name Name of the menu
	///
	/// Throws:
	///     UnknownMenu if no menu with the given name was found
	///     BadMenu if there is a menu with the given name, but it is invalid
	wxMenu *GetMenu(std::string const& name, agi::Context *c);

	/// @brief Open a popup menu at the mouse
	/// @param menu Menu to open
	/// @param parent_window Parent window for the menu; cannot be NULL
	///
	/// This function should be used rather than wxWindow::PopupMenu due to
	/// that PopupMenu does not trigger menu open events and triggers update
	/// ui events on the opening window rather than the menu for some bizarre
	/// reason
	void OpenPopupMenu(wxMenu *menu, wxWindow *parent_window);
}
