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

#include <main.h>

#include <libaegisub/fs.h>
#include <libaegisub/hotkey.h>

using namespace agi::hotkey;

static const char simple_valid[] = R"raw({
	"Always":{"cmd1":[{"modifiers":["Ctrl"], "key":"C"}]},
	"Default":{"cmd1":[{"modifiers":["Alt"], "key":"C"}], "cmd2":[{"modifiers":["Ctrl"], "key":"C"}]},
	"Other":{"cmd1":[{"modifiers":["Shift"], "key":"C"}], "cmd3":[{"modifiers":[], "key":"Q"}]}
})raw";

static const char simple_valid_new[] = R"raw({
	"Always":{"cmd1":["Ctrl-C"]},
	"Default":{"cmd1":["Alt-C"], "cmd2":["Ctrl-C"]},
	"Other":{"cmd1":["Shift-C"], "cmd3":["Q"]}
})raw";

TEST(lagi_hotkey, simple_valid_default) {
	EXPECT_NO_THROW(Hotkey("", simple_valid));
	EXPECT_NO_THROW(Hotkey("", simple_valid_new));
}

TEST(lagi_hotkey, empty_default) {
	EXPECT_THROW(Hotkey("", ""), std::exception);
}

TEST(lagi_hotkey, scan) {
	Hotkey h("", simple_valid);

	EXPECT_STREQ("cmd1", h.Scan("Always", "Ctrl-C", false).c_str());
	EXPECT_STREQ("cmd2", h.Scan("Default", "Ctrl-C", false).c_str());
	EXPECT_STREQ("cmd2", h.Scan("Other", "Ctrl-C", false).c_str());
	EXPECT_STREQ("cmd2", h.Scan("Nonexistent", "Ctrl-C", false).c_str());

	EXPECT_STREQ("cmd1", h.Scan("Always", "Ctrl-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Default", "Ctrl-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Other", "Ctrl-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Nonexistent", "Ctrl-C", true).c_str());

	EXPECT_STREQ("cmd1", h.Scan("Always", "Alt-C", false).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Default", "Alt-C", false).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Other", "Alt-C", false).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Nonexistent", "Alt-C", false).c_str());

	EXPECT_STREQ("cmd1", h.Scan("Always", "Alt-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Default", "Alt-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Other", "Alt-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Nonexistent", "Alt-C", true).c_str());

	EXPECT_STREQ("", h.Scan("Always", "Shift-C", false).c_str());
	EXPECT_STREQ("", h.Scan("Default", "Shift-C", false).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Other", "Shift-C", false).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "Shift-C", false).c_str());

	EXPECT_STREQ("", h.Scan("Always", "Shift-C", true).c_str());
	EXPECT_STREQ("", h.Scan("Default", "Shift-C", true).c_str());
	EXPECT_STREQ("cmd1", h.Scan("Other", "Shift-C", true).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "Shift-C", true).c_str());

	EXPECT_STREQ("", h.Scan("Always", "Q", false).c_str());
	EXPECT_STREQ("", h.Scan("Default", "Q", false).c_str());
	EXPECT_STREQ("cmd3", h.Scan("Other", "Q", false).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "Q", false).c_str());

	EXPECT_STREQ("", h.Scan("Always", "Q", true).c_str());
	EXPECT_STREQ("", h.Scan("Default", "Q", true).c_str());
	EXPECT_STREQ("cmd3", h.Scan("Other", "Q", true).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "Q", true).c_str());

	EXPECT_STREQ("", h.Scan("Always", "C", false).c_str());
	EXPECT_STREQ("", h.Scan("Default", "C", false).c_str());
	EXPECT_STREQ("", h.Scan("Other", "C", false).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "C", false).c_str());

	EXPECT_STREQ("", h.Scan("Always", "C", true).c_str());
	EXPECT_STREQ("", h.Scan("Default", "C", true).c_str());
	EXPECT_STREQ("", h.Scan("Other", "C", true).c_str());
	EXPECT_STREQ("", h.Scan("Nonexistent", "C", true).c_str());
}

TEST(lagi_hotkey, get_hotkey) {
	Hotkey h("", simple_valid);

	EXPECT_STREQ("Ctrl-C", h.GetHotkey("Always", "cmd1").c_str());
	EXPECT_STREQ("Alt-C", h.GetHotkey("Default", "cmd1").c_str());
	EXPECT_STREQ("Shift-C", h.GetHotkey("Other", "cmd1").c_str());
	EXPECT_STREQ("Alt-C", h.GetHotkey("Nonexistent", "cmd1").c_str());

	EXPECT_STREQ("Ctrl-C", h.GetHotkey("Always", "cmd2").c_str());
	EXPECT_STREQ("Ctrl-C", h.GetHotkey("Default", "cmd2").c_str());
	EXPECT_STREQ("Ctrl-C", h.GetHotkey("Other", "cmd2").c_str());
	EXPECT_STREQ("Ctrl-C", h.GetHotkey("Nonexistent", "cmd2").c_str());

	EXPECT_STREQ("", h.GetHotkey("Always", "cmd3").c_str());
	EXPECT_STREQ("", h.GetHotkey("Default", "cmd3").c_str());
	EXPECT_STREQ("Q", h.GetHotkey("Other", "cmd3").c_str());
	EXPECT_STREQ("", h.GetHotkey("Nonexistent", "cmd3").c_str());

	EXPECT_STREQ("", h.GetHotkey("Always", "cmd4").c_str());
	EXPECT_STREQ("", h.GetHotkey("Default", "cmd4").c_str());
	EXPECT_STREQ("", h.GetHotkey("Other", "cmd4").c_str());
	EXPECT_STREQ("", h.GetHotkey("Nonexistent", "cmd4").c_str());
}

TEST(lagi_hotkey, get_hotkeys) {
	Hotkey h("", simple_valid);

	EXPECT_EQ(2, h.GetHotkeys("Always", "cmd1").size());
	EXPECT_EQ(2, h.GetHotkeys("Default", "cmd1").size());
	EXPECT_EQ(3, h.GetHotkeys("Other", "cmd1").size());
	EXPECT_EQ(2, h.GetHotkeys("Nonexistent", "cmd1").size());

	EXPECT_EQ(1, h.GetHotkeys("Always", "cmd2").size());
	EXPECT_EQ(1, h.GetHotkeys("Default", "cmd2").size());
	EXPECT_EQ(1, h.GetHotkeys("Other", "cmd2").size());
	EXPECT_EQ(1, h.GetHotkeys("Nonexistent", "cmd2").size());

	EXPECT_EQ(0, h.GetHotkeys("Always", "cmd3").size());
	EXPECT_EQ(0, h.GetHotkeys("Default", "cmd3").size());
	EXPECT_EQ(1, h.GetHotkeys("Other", "cmd3").size());
	EXPECT_EQ(0, h.GetHotkeys("Nonexistent", "cmd3").size());

	EXPECT_EQ(0, h.GetHotkeys("Always", "cmd4").size());
	EXPECT_EQ(0, h.GetHotkeys("Default", "cmd4").size());
	EXPECT_EQ(0, h.GetHotkeys("Other", "cmd4").size());
	EXPECT_EQ(0, h.GetHotkeys("Nonexistent", "cmd4").size());
}

TEST(lagi_hotkey, has_hotkey) {
	Hotkey h("", simple_valid);
	EXPECT_TRUE(h.HasHotkey("Always", "Ctrl-C"));
	EXPECT_FALSE(h.HasHotkey("Always", "Alt-C"));
}

TEST(lagi_hotkey, get_hotkeys_dedups) {
	Hotkey h("", "{\"Always\":{\"cmd1\":[{\"modifiers\":[\"Ctrl\"], \"key\":\"C\"}]},\"Default\":{\"cmd1\":[{\"modifiers\":[\"Ctrl\"], \"key\":\"C\"}]}}");
	EXPECT_EQ(1, h.GetHotkeys("Always", "cmd1").size());
}

static void insert_combo(Hotkey::HotkeyMap &hm, const char *ctx, const char *cmd, const char *keys) {
	hm.insert(make_pair(std::string(cmd), Combo(ctx, cmd, keys)));
}

TEST(lagi_hotkey, set_hotkey_map) {
	agi::fs::Remove("data/hotkey_tmp");
	{
		Hotkey h("data/hotkey_tmp", "{}");

		Hotkey::HotkeyMap hm = h.GetHotkeyMap();
		EXPECT_EQ(0, hm.size());

		insert_combo(hm, "Always", "cmd1", "C");
		insert_combo(hm, "Default", "cmd2", "Shift-C");

		bool listener_called = false;
		h.AddHotkeyChangeListener([&] { listener_called = true; });

		h.SetHotkeyMap(hm);
		EXPECT_TRUE(listener_called);

		EXPECT_STREQ("cmd1", h.Scan("Always", "C", false).c_str());
		EXPECT_STREQ("cmd2", h.Scan("Default", "Shift-C", false).c_str());
	}

	EXPECT_EQ(2, Hotkey("data/hotkey_tmp", "{}").GetHotkeyMap().size());
}

TEST(lagi_hotkey, combo_stuff) {
	Hotkey::HotkeyMap hm = Hotkey("", simple_valid).GetHotkeyMap();

	Hotkey::HotkeyMap::const_iterator it, end;
	std::tie(it, end) = hm.equal_range("cmd1");
	EXPECT_EQ(3, std::distance(it, end));

	std::tie(it, end) = hm.equal_range("cmd2");
	ASSERT_EQ(1, std::distance(it, end));

	Combo c = it->second;
	EXPECT_STREQ("Ctrl-C", c.Str().c_str());
	EXPECT_STREQ("cmd2", c.CmdName().c_str());
	EXPECT_STREQ("Default", c.Context().c_str());
}
