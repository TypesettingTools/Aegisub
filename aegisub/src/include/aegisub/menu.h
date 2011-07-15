// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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
#include <map>

#include <wx/menu.h>
#endif

#include <libaegisub/exception.h>

namespace json { class Array; }

namespace menu {
DEFINE_BASE_EXCEPTION_NOINNER(MenuError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(MenuJsonValueArray, MenuError, "menu/value/array")
DEFINE_SIMPLE_EXCEPTION_NOINNER(MenuJsonValueSingle, MenuError, "menu/value")
DEFINE_SIMPLE_EXCEPTION_NOINNER(MenuJsonValueNull, MenuError, "menu/value")
DEFINE_SIMPLE_EXCEPTION_NOINNER(MenuInvalidName, MenuError, "menu/invalid")

class Menu;
extern Menu *menu;

class Menu {
public:
	Menu();
	~Menu();
	wxMenuBar* GetMainMenu() { return main_menu; }
	wxMenu* GetMenu(std::string name);

private:

	typedef std::map<std::string, wxMenu*> MTMap;
	typedef std::pair<std::string, wxMenu*> MTPair;

	enum MenuTypes {
		Option = 1,
		Check = 2,
		Radio = 3,
		Submenu = 4,
		Recent = 5,
		Spacer = 100
	};

	wxMenuBar *main_menu;
	MTMap map;

	wxMenu* BuildMenu(std::string name, const json::Array& array, int submenu=0);

};

} // namespace menu
