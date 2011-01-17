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

/// @file menu.cpp
/// @brief Dynamic menu and toolbar generator.
/// @ingroup menu

#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <memory>

#include <wx/menuitem.h>
#endif

#include "include/aegisub/menu.h"

#include <libaegisub/io.h>
#include <libaegisub/json.h>
#include <libaegisub/log.h>

#include "include/aegisub/hotkey.h"
#include "command/command.h"
#include "libresrc/libresrc.h"
#include "main.h"

namespace menu {

menu::Menu *menu;

Menu::Menu() {

	main_menu = new wxMenuBar();

	std::istringstream *stream = new std::istringstream(GET_DEFAULT_CONFIG(default_menu));
	json::UnknownElement menu_root = agi::json_util::parse(stream);

	LOG_D("menu/init") << "Generating Menus";
	json::Object object = menu_root;

	for (json::Object::const_iterator index(object.Begin()); index != object.End(); index++) {
		const json::Object::Member& member = *index;
//		const json::UnknownElement& element = member.element;

		const json::Array& array = member.element;
		BuildMenu(member.name, array);

	}
}

Menu::~Menu() {}


wxMenu* Menu::GetMenu(std::string name) {

	MTMap::iterator index;

	if ((index = map.find(name)) != map.end()) {
		return index->second;
	}

	LOG_E("menu/invalid") << "Invalid index name: " << name;
	throw MenuInvalidName("Unknown index");
}




wxMenu* Menu::BuildMenu(std::string name, const json::Array& array, int submenu) {
	wxMenu *menu = new wxMenu();


	for (json::Array::const_iterator index(array.Begin()); index != array.End(); index++) {
		std::string name_sub = name;

		const json::Object& obj = *index;
		const json::Number& type_number = obj["type"];
		int type = type_number.Value();

		if (type == Menu::Spacer) {
			menu->AppendSeparator();
			continue;
		}


		const json::String& command = obj["command"];
		std::string name_submenu = name_sub + "/" + command.Value();


		std::string cmd_name = type == Menu::Submenu ? name_submenu : command.Value();
		cmd::Command *cmd = cmd::get(cmd_name);

		wxString display = cmd->StrMenu() + "\t" + hotkey::get_hotkey_str_first("Default", cmd_name);
		wxString descr = cmd->StrHelp();

		switch (type) {
			case Menu::Option: {
				wxMenuItem *menu_item = new wxMenuItem(menu, cmd::id(command.Value()), display, descr, wxITEM_NORMAL);
				menu->Append(menu_item);
			}
			break;

			case Menu::Check: {
				menu->AppendCheckItem(cmd::id(command.Value()), display, descr);
			}
			break;

			case Menu::Radio: {
				menu->AppendRadioItem(cmd::id(command.Value()), display, descr);
			}
			break;

			case Menu::Recent: {
	 			wxMenu *menu_new = new wxMenu();
				wxMenuItem *menu_item = new wxMenuItem(menu, cmd::id(command.Value()), display, descr, wxITEM_NORMAL, menu_new);
				menu->Append(menu_item);
				map.insert(MTPair(command.Value(), menu_new));

			}
			break;

			case Menu::Submenu: {

				const json::Array& arr = obj["contents"];

	 			wxMenu *menu_new = BuildMenu(name_sub.append("/").append(command), arr, 1);
				map.insert(MTPair(name_submenu, menu_new));

				if (submenu) {
					wxMenuItem *menu_item = new wxMenuItem(menu, cmd::id(name_sub), display, descr, wxITEM_NORMAL, menu_new);
					menu->Append(menu_item);
				} else {
					main_menu->Append(menu_new, display);
				}
			}
			break;
		}

	} // for index

	return menu;
}

} // namespace menu
