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

/// @file hotkey.cpp
/// @brief Hotkey handler
/// @ingroup hotkey menu event window

#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <memory>
#include <vector>
#endif

#include <libaegisub/log.h>
#include <libaegisub/hotkey.h>

#include "include/aegisub/hotkey.h"

#include "include/aegisub/toolbar.h"
#include "libresrc/libresrc.h"
#include "command/command.h"
#include "frame_main.h"
#include "main.h"

namespace hotkey {

static std::vector<std::string> keycode_names;

static std::string const& get_keycode_name(int code);
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

bool check(std::string const& context, int key_code, wchar_t key_char, int modifier) {
	std::string combo;
	if ((modifier != wxMOD_NONE)) {
		if ((modifier & wxMOD_CMD) != 0) combo.append("Ctrl-");
		if ((modifier & wxMOD_ALT) != 0) combo.append("Alt-");
		if ((modifier & wxMOD_SHIFT) != 0) combo.append("Shift-");
	}

	combo += keycode_name(key_code);
	if (combo.empty()) return false;

	std::string command;
	if (agi::hotkey::hotkey->Scan(context, combo, OPT_GET("Audio/Medusa Timing Hotkeys")->GetBool(), command) == 0) {
		/// The bottom line should be removed after all the hotkey commands are fixed.
		/// This is to avoid pointless exceptions.
		if (command.find("/") != std::string::npos) {
			(*cmd::get(command))(wxGetApp().frame->context.get());
			return true;
		}
	}
	return false;
}

std::vector<std::string> get_hotkey_strs(std::string const& context, std::string const& command) {
	return agi::hotkey::hotkey->GetHotkeys(context, command);
}

std::string get_hotkey_str_first(std::string const& context, std::string const& command) {
	std::vector<std::string> strs = get_hotkey_strs(context, command);
	return strs.empty() ? "" : strs.front();
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
	set_kc(keycode_names, WXK_TAB, "Tab");
	set_kc(keycode_names, WXK_RETURN, "Return");
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
	set_kc(keycode_names, WXK_NUMPAD_ENTER, "KP_Return");
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


} // namespace toolbar

