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

/// @file hotkey.cpp
/// @brief Hotkey handler
/// @ingroup hotkey menu event window

#include <libaegisub/hotkey.h>

#include "include/aegisub/hotkey.h"

#include "libresrc/libresrc.h"
#include "command/command.h"
#include "compat.h"
#include "options.h"

#include <libaegisub/path.h>

#include <boost/range/algorithm/find.hpp>
#include <boost/range/iterator_range.hpp>
#include <wx/intl.h>
#include <wx/msgdlg.h>

namespace {
	const char *added_hotkeys_7035[][5] = {
		{ "audio/play/line", "Audio", "R", nullptr, nullptr },
		{ nullptr }
	};

	const char *added_hotkeys_7070[][5] = {
		{ "edit/color/primary", "Subtitle Edit Box", "Alt", "1", nullptr },
		{ "edit/color/secondary", "Subtitle Edit Box", "Alt", "2", nullptr },
		{ "edit/color/outline", "Subtitle Edit Box", "Alt", "3", nullptr },
		{ "edit/color/shadow", "Subtitle Edit Box", "Alt", "4", nullptr },
		{ nullptr }
	};

	const char *added_hotkeys_shift_back[][5] = {
		{ "edit/line/duplicate/shift_back", "Default", "Ctrl", "Shift", "D" },
		{ nullptr }
	};

	void migrate_hotkeys(const char *added[][5]) {
		agi::hotkey::Hotkey::HotkeyMap hk_map = hotkey::inst->GetHotkeyMap();

		for (size_t i = 0; added[i] && added[i][0]; ++i) {
			std::vector<std::string> keys;
			for (size_t j = 2; j < 5; ++j) {
				if (added[i][j])
					keys.emplace_back(added[i][j]);
			}
			agi::hotkey::Combo combo(added[i][1], added[i][0], keys);

			if (hotkey::inst->HasHotkey(combo.Context(), combo.Str()))
				continue;

			hk_map.insert(make_pair(std::string(added[i][0]), combo));
		}

		hotkey::inst->SetHotkeyMap(hk_map);
	}
}

namespace hotkey {

agi::hotkey::Hotkey *inst = nullptr;
void init() {
	inst = new agi::hotkey::Hotkey(
		config::path->Decode("?user/hotkey.json"),
		GET_DEFAULT_CONFIG(default_hotkey));

	auto migrations = OPT_GET("App/Hotkey Migrations")->GetListString();

	if (boost::find(migrations, "7035") == end(migrations)) {
		migrate_hotkeys(added_hotkeys_7035);
		migrations.emplace_back("7035");
	}

	if (boost::find(migrations, "7070") == end(migrations)) {
		migrate_hotkeys(added_hotkeys_7070);
		migrations.emplace_back("7070");
	}

	if (boost::find(migrations, "edit/line/duplicate/shift_back") == end(migrations)) {
		migrate_hotkeys(added_hotkeys_shift_back);
		migrations.emplace_back("edit/line/duplicate/shift_back");
	}

	if (boost::find(migrations, "duplicate -> split") == end(migrations)) {
		auto hk_map = hotkey::inst->GetHotkeyMap();
		for (auto const& hotkey : boost::make_iterator_range(hk_map.equal_range("edit/line/duplicate/shift"))) {
			auto combo = agi::hotkey::Combo(hotkey.second.Context(), "edit/line/split/before", hotkey.second.Get());
			hk_map.insert({combo.CmdName(), combo});
		}
		for (auto const& hotkey : boost::make_iterator_range(hk_map.equal_range("edit/line/duplicate/shift_back"))) {
			auto combo = agi::hotkey::Combo(hotkey.second.Context(), "edit/line/split/after", hotkey.second.Get());
			hk_map.insert({combo.CmdName(), combo});
		}

		hk_map.erase("edit/line/duplicate/shift");
		hk_map.erase("edit/line/duplicate/shift_back");

		hotkey::inst->SetHotkeyMap(hk_map);
		migrations.emplace_back("duplicate -> split");
	}

	OPT_SET("App/Hotkey Migrations")->SetListString(std::move(migrations));
}

void clear() {
	delete inst;
}

static std::vector<std::string> keycode_names;

static void init_keycode_names();

static std::string const& keycode_name(int code) {
	if (keycode_names.empty())
		init_keycode_names();

	if (static_cast<size_t>(code) > keycode_names.size()) {
		static std::string str;
		return str;
	}

	return keycode_names[code];
}

std::string keypress_to_str(int key_code, int modifier) {
	std::string combo;
	if ((modifier != wxMOD_NONE)) {
		if ((modifier & wxMOD_CMD) != 0) combo.append("Ctrl-");
		if ((modifier & wxMOD_ALT) != 0) combo.append("Alt-");
		if ((modifier & wxMOD_SHIFT) != 0) combo.append("Shift-");
	}

	combo += keycode_name(key_code);

	return combo;
}

bool check(std::string const& context, agi::Context *c, int key_code, int modifier) {
	std::string combo = keypress_to_str(key_code, modifier);
	if (combo.empty()) return false;

	std::string command = inst->Scan(context, combo, OPT_GET("Audio/Medusa Timing Hotkeys")->GetBool());
	if (!command.empty()) {
		cmd::call(command, c);
		return true;
	}
	return false;
}

bool check(std::string const& context, agi::Context *c, wxKeyEvent &evt) {
	try {
		if (!hotkey::check(context, c, evt.GetKeyCode(), evt.GetModifiers())) {
			evt.Skip();
			return false;
		}
		return true;
	}
	catch (cmd::CommandNotFound const& e) {
		wxMessageBox(to_wx(e.GetChainedMessage()), _("Invalid command name for hotkey"),
			wxOK | wxICON_ERROR | wxCENTER | wxSTAY_ON_TOP);
		return true;
	}
}

std::vector<std::string> get_hotkey_strs(std::string const& context, std::string const& command) {
	return inst->GetHotkeys(context, command);
}

std::string get_hotkey_str_first(std::string const& context, std::string const& command) {
	return inst->GetHotkey(context, command);
}

static inline void set_kc(std::vector<std::string> &vec, int code, std::string const& str) {
	if (static_cast<size_t>(code) >= vec.size()) vec.resize(code * 2, "");
	vec[code] = str;
}

static void init_keycode_names() {
	char str[] = { 0, 0 };
	for (char i = 33; i < 127; ++i) {
		str[0] = i;
		set_kc(keycode_names, i, str);
	}
	set_kc(keycode_names, WXK_BACK, "Backspace");
	set_kc(keycode_names, WXK_TAB, "Tab");
	set_kc(keycode_names, WXK_RETURN, "Enter");
	set_kc(keycode_names, WXK_ESCAPE, "Escape");
	set_kc(keycode_names, WXK_SPACE, "Space");
	set_kc(keycode_names, WXK_DELETE, "Delete");
	set_kc(keycode_names, WXK_SHIFT, "Shift");
	set_kc(keycode_names, WXK_ALT, "Alt");
	set_kc(keycode_names, WXK_CONTROL, "Control");
	set_kc(keycode_names, WXK_PAUSE, "Pause");
	set_kc(keycode_names, WXK_END, "End");
	set_kc(keycode_names, WXK_HOME, "Home");
	set_kc(keycode_names, WXK_LEFT, "Left");
	set_kc(keycode_names, WXK_UP, "Up");
	set_kc(keycode_names, WXK_RIGHT, "Right");
	set_kc(keycode_names, WXK_DOWN, "Down");
	set_kc(keycode_names, WXK_PRINT, "Print");
	set_kc(keycode_names, WXK_INSERT, "Insert");
	set_kc(keycode_names, WXK_NUMPAD0, "KP_0");
	set_kc(keycode_names, WXK_NUMPAD1, "KP_1");
	set_kc(keycode_names, WXK_NUMPAD2, "KP_2");
	set_kc(keycode_names, WXK_NUMPAD3, "KP_3");
	set_kc(keycode_names, WXK_NUMPAD4, "KP_4");
	set_kc(keycode_names, WXK_NUMPAD5, "KP_5");
	set_kc(keycode_names, WXK_NUMPAD6, "KP_6");
	set_kc(keycode_names, WXK_NUMPAD7, "KP_7");
	set_kc(keycode_names, WXK_NUMPAD8, "KP_8");
	set_kc(keycode_names, WXK_NUMPAD9, "KP_9");
	set_kc(keycode_names, WXK_MULTIPLY, "Asterisk");
	set_kc(keycode_names, WXK_ADD, "Plus");
	set_kc(keycode_names, WXK_SUBTRACT, "Hyphen");
	set_kc(keycode_names, WXK_DECIMAL, "Period");
	set_kc(keycode_names, WXK_DIVIDE, "Slash");
	set_kc(keycode_names, WXK_F1, "F1");
	set_kc(keycode_names, WXK_F2, "F2");
	set_kc(keycode_names, WXK_F3, "F3");
	set_kc(keycode_names, WXK_F4, "F4");
	set_kc(keycode_names, WXK_F5, "F5");
	set_kc(keycode_names, WXK_F6, "F6");
	set_kc(keycode_names, WXK_F7, "F7");
	set_kc(keycode_names, WXK_F8, "F8");
	set_kc(keycode_names, WXK_F9, "F9");
	set_kc(keycode_names, WXK_F10, "F10");
	set_kc(keycode_names, WXK_F11, "F11");
	set_kc(keycode_names, WXK_F12, "F12");
	set_kc(keycode_names, WXK_F13, "F13");
	set_kc(keycode_names, WXK_F14, "F14");
	set_kc(keycode_names, WXK_F15, "F15");
	set_kc(keycode_names, WXK_F16, "F16");
	set_kc(keycode_names, WXK_F17, "F17");
	set_kc(keycode_names, WXK_F18, "F18");
	set_kc(keycode_names, WXK_F19, "F19");
	set_kc(keycode_names, WXK_F20, "F20");
	set_kc(keycode_names, WXK_F21, "F21");
	set_kc(keycode_names, WXK_F22, "F22");
	set_kc(keycode_names, WXK_F23, "F23");
	set_kc(keycode_names, WXK_F24, "F24");
	set_kc(keycode_names, WXK_NUMLOCK, "Num_Lock");
	set_kc(keycode_names, WXK_SCROLL, "Scroll_Lock");
	set_kc(keycode_names, WXK_PAGEUP, "PageUp");
	set_kc(keycode_names, WXK_PAGEDOWN, "PageDown");
	set_kc(keycode_names, WXK_NUMPAD_SPACE, "KP_Space");
	set_kc(keycode_names, WXK_NUMPAD_TAB, "KP_Tab");
	set_kc(keycode_names, WXK_NUMPAD_ENTER, "KP_Enter");
	set_kc(keycode_names, WXK_NUMPAD_F1, "KP_F1");
	set_kc(keycode_names, WXK_NUMPAD_F2, "KP_F2");
	set_kc(keycode_names, WXK_NUMPAD_F3, "KP_F3");
	set_kc(keycode_names, WXK_NUMPAD_F4, "KP_F4");
	set_kc(keycode_names, WXK_NUMPAD_HOME, "KP_Home");
	set_kc(keycode_names, WXK_NUMPAD_LEFT, "KP_Left");
	set_kc(keycode_names, WXK_NUMPAD_UP, "KP_Up");
	set_kc(keycode_names, WXK_NUMPAD_RIGHT, "KP_Right");
	set_kc(keycode_names, WXK_NUMPAD_DOWN, "KP_Down");
	set_kc(keycode_names, WXK_NUMPAD_PAGEUP, "KP_PageUp");
	set_kc(keycode_names, WXK_NUMPAD_PAGEDOWN, "KP_PageDown");
	set_kc(keycode_names, WXK_NUMPAD_END, "KP_End");
	set_kc(keycode_names, WXK_NUMPAD_BEGIN, "KP_Begin");
	set_kc(keycode_names, WXK_NUMPAD_INSERT, "KP_insert");
	set_kc(keycode_names, WXK_NUMPAD_DELETE, "KP_Delete");
	set_kc(keycode_names, WXK_NUMPAD_EQUAL, "KP_Equal");
	set_kc(keycode_names, WXK_NUMPAD_MULTIPLY, "KP_Multiply");
	set_kc(keycode_names, WXK_NUMPAD_ADD, "KP_Add");
	set_kc(keycode_names, WXK_NUMPAD_SUBTRACT, "KP_Subtract");
	set_kc(keycode_names, WXK_NUMPAD_DECIMAL, "KP_Decimal");
	set_kc(keycode_names, WXK_NUMPAD_DIVIDE, "KP_Divide");
}

} // namespace hotkey
