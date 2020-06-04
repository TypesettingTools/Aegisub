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

/// @file menu.h
/// @brief Dynamic menu and toolbar generator.
/// @ingroup menu toolbar

#include <memory>
#include <string>

#include <libaegisub/exception.h>

namespace agi { struct Context; }

class wxFrame;
class wxMenu;
class wxMenuBar;
class wxWindow;

/*
ID allocation for menu items:

... - wxID_ANY(-1), wxID_LOWEST(4999) - wxID_HIGHEST(5999)    Reserved by wxWidgets, see documentation for wxID_HIGHEST

(wxID_HIGHEST + 1) + 2000 ~ (wxID_HIGHEST + 1) + 2014    Grid column list, see base_grid.cpp
(wxID_HIGHEST + 1) + 3000 ~ (wxID_HIGHEST + 1) + 3001    Context menu, see timeedit_ctrl.cpp
(wxID_HIGHEST + 1) + 4000 ~ (wxID_HIGHEST + 1) + 7999    Context menu, see subs_edit_ctrl.cpp
(wxID_HIGHEST + 1) + 8000 ~ (wxID_HIGHEST + 1) + 8019    Grid context menu, see base_grid.cpp
(wxID_HIGHEST + 1) + 9000 ~ (wxID_HIGHEST + 1) + 9004    Video context menu, see video_display.cpp
(wxID_HIGHEST + 1) + 10000 ~ (wxID_HIGHEST + 1) + 10999  Main menu
*/

namespace menu {
	DEFINE_EXCEPTION(Error, agi::Exception);
	DEFINE_EXCEPTION(UnknownMenu, Error);
	DEFINE_EXCEPTION(InvalidMenu, Error);

	/// @brief Get the menu with the specified name as a wxMenuBar
	/// @param name Name of the menu
	///
	/// Throws:
	///     UnknownMenu if no menu with the given name was found
	///     BadMenu if there is a menu with the given name, but it is invalid
	void GetMenuBar(std::string const& name, wxFrame *window, int id_base, agi::Context *c);

	/// @brief Get the menu with the specified name as a wxMenu
	/// @param name Name of the menu
	///
	/// Throws:
	///     UnknownMenu if no menu with the given name was found
	///     BadMenu if there is a menu with the given name, but it is invalid
	std::unique_ptr<wxMenu> GetMenu(std::string const& name, int id_base, agi::Context *c);

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
