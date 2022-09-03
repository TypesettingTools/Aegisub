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

#include <fstream>

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

	EXPECT_EQ("cmd1", h.Scan("Always", "Ctrl-C", false));
	EXPECT_EQ("cmd2", h.Scan("Default", "Ctrl-C", false));
	EXPECT_EQ("cmd2", h.Scan("Other", "Ctrl-C", false));
	EXPECT_EQ("cmd2", h.Scan("Nonexistent", "Ctrl-C", false));

	EXPECT_EQ("cmd1", h.Scan("Always", "Ctrl-C", true));
	EXPECT_EQ("cmd1", h.Scan("Default", "Ctrl-C", true));
	EXPECT_EQ("cmd1", h.Scan("Other", "Ctrl-C", true));
	EXPECT_EQ("cmd1", h.Scan("Nonexistent", "Ctrl-C", true));

	EXPECT_EQ("cmd1", h.Scan("Always", "Alt-C", false));
	EXPECT_EQ("cmd1", h.Scan("Default", "Alt-C", false));
	EXPECT_EQ("cmd1", h.Scan("Other", "Alt-C", false));
	EXPECT_EQ("cmd1", h.Scan("Nonexistent", "Alt-C", false));

	EXPECT_EQ("cmd1", h.Scan("Always", "Alt-C", true));
	EXPECT_EQ("cmd1", h.Scan("Default", "Alt-C", true));
	EXPECT_EQ("cmd1", h.Scan("Other", "Alt-C", true));
	EXPECT_EQ("cmd1", h.Scan("Nonexistent", "Alt-C", true));

	EXPECT_EQ("", h.Scan("Always", "Shift-C", false));
	EXPECT_EQ("", h.Scan("Default", "Shift-C", false));
	EXPECT_EQ("cmd1", h.Scan("Other", "Shift-C", false));
	EXPECT_EQ("", h.Scan("Nonexistent", "Shift-C", false));

	EXPECT_EQ("", h.Scan("Always", "Shift-C", true));
	EXPECT_EQ("", h.Scan("Default", "Shift-C", true));
	EXPECT_EQ("cmd1", h.Scan("Other", "Shift-C", true));
	EXPECT_EQ("", h.Scan("Nonexistent", "Shift-C", true));

	EXPECT_EQ("", h.Scan("Always", "Q", false));
	EXPECT_EQ("", h.Scan("Default", "Q", false));
	EXPECT_EQ("cmd3", h.Scan("Other", "Q", false));
	EXPECT_EQ("", h.Scan("Nonexistent", "Q", false));

	EXPECT_EQ("", h.Scan("Always", "Q", true));
	EXPECT_EQ("", h.Scan("Default", "Q", true));
	EXPECT_EQ("cmd3", h.Scan("Other", "Q", true));
	EXPECT_EQ("", h.Scan("Nonexistent", "Q", true));

	EXPECT_EQ("", h.Scan("Always", "C", false));
	EXPECT_EQ("", h.Scan("Default", "C", false));
	EXPECT_EQ("", h.Scan("Other", "C", false));
	EXPECT_EQ("", h.Scan("Nonexistent", "C", false));

	EXPECT_EQ("", h.Scan("Always", "C", true));
	EXPECT_EQ("", h.Scan("Default", "C", true));
	EXPECT_EQ("", h.Scan("Other", "C", true));
	EXPECT_EQ("", h.Scan("Nonexistent", "C", true));
}

TEST(lagi_hotkey, get_hotkey) {
	Hotkey h("", simple_valid);

	EXPECT_EQ("Ctrl-C", h.GetHotkey("Always", "cmd1"));
	EXPECT_EQ("Alt-C", h.GetHotkey("Default", "cmd1"));
	EXPECT_EQ("Shift-C", h.GetHotkey("Other", "cmd1"));
	EXPECT_EQ("Alt-C", h.GetHotkey("Nonexistent", "cmd1"));

	EXPECT_EQ("Ctrl-C", h.GetHotkey("Always", "cmd2"));
	EXPECT_EQ("Ctrl-C", h.GetHotkey("Default", "cmd2"));
	EXPECT_EQ("Ctrl-C", h.GetHotkey("Other", "cmd2"));
	EXPECT_EQ("Ctrl-C", h.GetHotkey("Nonexistent", "cmd2"));

	EXPECT_EQ("", h.GetHotkey("Always", "cmd3"));
	EXPECT_EQ("", h.GetHotkey("Default", "cmd3"));
	EXPECT_EQ("Q", h.GetHotkey("Other", "cmd3"));
	EXPECT_EQ("", h.GetHotkey("Nonexistent", "cmd3"));

	EXPECT_EQ("", h.GetHotkey("Always", "cmd4"));
	EXPECT_EQ("", h.GetHotkey("Default", "cmd4"));
	EXPECT_EQ("", h.GetHotkey("Other", "cmd4"));
	EXPECT_EQ("", h.GetHotkey("Nonexistent", "cmd4"));
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

		EXPECT_EQ("cmd1", h.Scan("Always", "C", false));
		EXPECT_EQ("cmd2", h.Scan("Default", "Shift-C", false));
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
	EXPECT_EQ("Ctrl-C", c.Str());
	EXPECT_EQ("cmd2", c.CmdName());
	EXPECT_EQ("Default", c.Context());
}

TEST(lagi_hotkey,  old_format_is_backed_up_before_migrating) {
	{
		std::ofstream tmp("data/hotkey_tmp");
		tmp.write(simple_valid, sizeof(simple_valid));
	}

	{
		Hotkey h("data/hotkey_tmp", "{}");
		h.SetHotkeyMap(h.GetHotkeyMap());
	}
	{
		std::ifstream tmp("data/hotkey_tmp.3_1");
		ASSERT_TRUE(tmp.good());
		char buff[sizeof(simple_valid)];
		tmp.read(buff, sizeof(buff));
		ASSERT_TRUE(memcmp(buff, simple_valid, sizeof(buff)) == 0);
	}

	agi::fs::Remove("data/hotkey_tmp.3_1");
}
