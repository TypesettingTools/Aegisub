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
#endif

#include <libaegisub/io.h>
#include <libaegisub/json.h>
#include <libaegisub/log.h>
#include <libaegisub/hotkey.h>

#include "include/aegisub/hotkey.h"

#include "include/aegisub/toolbar.h"
#include "libresrc/libresrc.h"
#include "command/command.h"
#include "frame_main.h"
#include "main.h"

namespace hotkey {

typedef std::pair<int, const char *> KCNamePair;

KCNameMap kc_name_map;


void check(std::string context, int key_code, wchar_t key_char, int modifier) {
	std::string combo;
	if ((modifier != wxMOD_NONE)) {
		if ((modifier & wxMOD_CMD) != 0) combo.append("Ctrl-");
		if ((modifier & wxMOD_ALT) != 0) combo.append("Alt-");
		if ((modifier & wxMOD_SHIFT) != 0) combo.append("Shift-");
	}

	if ((key_char != 0)
		&& (key_code != WXK_BACK)
		&& (key_code != WXK_RETURN)
		&& (key_code != WXK_ESCAPE)
		&& (key_code != WXK_SPACE)
		&& (key_code != WXK_DELETE)) {
		combo.append(wxString::Format("%c", key_char));
	} else if (keycode_name(key_code, combo) == 1) {
		std::stringstream ss;
		ss << key_code;
		combo.append(ss.str());
	}

	std::string command;
	if (agi::hotkey::hotkey->Scan(context, combo, command) == 0) {
		/// The bottom line should be removed after all the hotkey commands are fixed.
		/// This is to avoid pointless exceptions.
		if (command.find("/") != std::string::npos)
			(*cmd::get(command))(&wxGetApp().frame->temp_context);
	}

}


bool keycode_name(const int &code, std::string &combo) {
	KCNameMap::iterator index;

	if ((index = kc_name_map.find(code)) != kc_name_map.end()) {
		combo.append(index->second);
		return 0;
	}
	return 1;
}


void keycode_name_map_init() {
	kc_name_map.insert(KCNamePair(WXK_TAB, "Tab"));
	kc_name_map.insert(KCNamePair(WXK_RETURN, "Return"));
	kc_name_map.insert(KCNamePair(WXK_ESCAPE, "Escape"));
	kc_name_map.insert(KCNamePair(WXK_SPACE, "Space"));
	kc_name_map.insert(KCNamePair(WXK_DELETE, "Delete"));
	kc_name_map.insert(KCNamePair(WXK_SHIFT, "Shift"));
	kc_name_map.insert(KCNamePair(WXK_ALT, "Alt"));
	kc_name_map.insert(KCNamePair(WXK_CONTROL, "Control"));
	kc_name_map.insert(KCNamePair(WXK_PAUSE, "Pause"));
	kc_name_map.insert(KCNamePair(WXK_END, "End"));
	kc_name_map.insert(KCNamePair(WXK_HOME, "Home"));
	kc_name_map.insert(KCNamePair(WXK_LEFT, "Left"));
	kc_name_map.insert(KCNamePair(WXK_UP, "Up"));
	kc_name_map.insert(KCNamePair(WXK_RIGHT, "Right"));
	kc_name_map.insert(KCNamePair(WXK_DOWN, "Down"));
	kc_name_map.insert(KCNamePair(WXK_PRINT, "Print"));
	kc_name_map.insert(KCNamePair(WXK_INSERT, "Insert"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD0, "KP_0"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD1, "KP_1"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD2, "KP_2"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD3, "KP_3"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD4, "KP_4"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD5, "KP_5"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD6, "KP_6"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD7, "KP_7"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD8, "KP_8"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD9, "KP_9"));
	kc_name_map.insert(KCNamePair(WXK_MULTIPLY, "Astrisk"));
	kc_name_map.insert(KCNamePair(WXK_ADD, "Plus"));
	kc_name_map.insert(KCNamePair(WXK_SUBTRACT, "Hyphen"));
	kc_name_map.insert(KCNamePair(WXK_DECIMAL, "Period"));
	kc_name_map.insert(KCNamePair(WXK_DIVIDE, "Slash"));
	kc_name_map.insert(KCNamePair(WXK_F1, "F1"));
	kc_name_map.insert(KCNamePair(WXK_F2, "F2"));
	kc_name_map.insert(KCNamePair(WXK_F3, "F3"));
	kc_name_map.insert(KCNamePair(WXK_F4, "F4"));
	kc_name_map.insert(KCNamePair(WXK_F5, "F5"));
	kc_name_map.insert(KCNamePair(WXK_F6, "F6"));
	kc_name_map.insert(KCNamePair(WXK_F7, "F7"));
	kc_name_map.insert(KCNamePair(WXK_F8, "F8"));
	kc_name_map.insert(KCNamePair(WXK_F9, "F9"));
	kc_name_map.insert(KCNamePair(WXK_F10, "F10"));
	kc_name_map.insert(KCNamePair(WXK_F11, "F11"));
	kc_name_map.insert(KCNamePair(WXK_F12, "F12"));
	kc_name_map.insert(KCNamePair(WXK_F13, "F13"));
	kc_name_map.insert(KCNamePair(WXK_F14, "F14"));
	kc_name_map.insert(KCNamePair(WXK_F15, "F15"));
	kc_name_map.insert(KCNamePair(WXK_F16, "F16"));
	kc_name_map.insert(KCNamePair(WXK_F17, "F17"));
	kc_name_map.insert(KCNamePair(WXK_F18, "F18"));
	kc_name_map.insert(KCNamePair(WXK_F19, "F19"));
	kc_name_map.insert(KCNamePair(WXK_F20, "F20"));
	kc_name_map.insert(KCNamePair(WXK_F21, "F21"));
	kc_name_map.insert(KCNamePair(WXK_F22, "F22"));
	kc_name_map.insert(KCNamePair(WXK_F23, "F23"));
	kc_name_map.insert(KCNamePair(WXK_F24, "F24"));
	kc_name_map.insert(KCNamePair(WXK_NUMLOCK, "Num_Lock"));
	kc_name_map.insert(KCNamePair(WXK_SCROLL, "Scroll_Lock"));
	kc_name_map.insert(KCNamePair(WXK_PAGEUP, "PageUp"));
	kc_name_map.insert(KCNamePair(WXK_PAGEDOWN, "PageDown"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_SPACE, "KP_Space"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_TAB, "KP_Tab"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_ENTER, "KP_Return"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_F1, "KP_F1"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_F2, "KP_F2"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_F3, "KP_F3"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_F4, "KP_F4"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_HOME, "KP_Home"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_LEFT, "KP_Left"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_UP, "KP_Up"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_RIGHT, "KP_Right"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_DOWN, "KP_Down"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_PAGEUP, "KP_PageUp"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_PAGEDOWN, "KP_PageDown"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_END, "KP_End"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_BEGIN, "KP_Begin"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_INSERT, "KP_insert"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_DELETE, "KP_Delete"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_EQUAL, "KP_Equal"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_MULTIPLY, "KP_Multiply"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_ADD, "KP_Add"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_SUBTRACT, "KP_Subtract"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_DECIMAL, "KP_Decimal"));
	kc_name_map.insert(KCNamePair(WXK_NUMPAD_DIVIDE, "KP_Divide"));

}

} // namespace toolbar

