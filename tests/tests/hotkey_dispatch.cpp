#include <gtest/gtest.h>

#include "include/aegisub/hotkey.h"
#include "command/command.h"
#include "libresrc/libresrc.h"
#include "options.h"

#include <libaegisub/hotkey.h>

// Exercise the application's dispatcher without opening media or modifying subtitles.
namespace {
std::vector<std::string> called_commands;
}

namespace config {
agi::Options *opt = nullptr;
agi::Path *path = nullptr;
agi::MRUManager *mru = nullptr;
}

namespace cmd {
void call(std::string_view name, agi::Context *) {
	called_commands.emplace_back(name);
}
}

class HotkeyDispatch : public testing::Test {
	agi::Options options{"", R"({"Audio":{"Medusa Timing Hotkeys":false}})", agi::Options::FLUSH_SKIP};
	agi::hotkey::Hotkey hotkeys{"", GET_DEFAULT_CONFIG(default_hotkey)};

protected:
	void SetUp() override {
		config::opt = &options;
		hotkey::inst = &hotkeys;
		called_commands.clear();
	}

	void TearDown() override {
		hotkey::inst = nullptr;
		config::opt = nullptr;
	}
};

TEST_F(HotkeyDispatch, AudioCommandsConsumeEventAfterGridMiss) {
	// Use the shipped shortcuts, with no duplicated bindings in Subtitle Grid.
	struct PlaybackKey { int key; const char *command; };
	const PlaybackKey keys[] = {
		{'S', "audio/play/selection"},
		{WXK_SPACE, "audio/play/selection"},
		{'D', "audio/play/selection/end"},
		{'Q', "audio/play/selection/before"},
		{'W', "audio/play/selection/after"},
		{'E', "audio/play/selection/begin"},
		{'R', "audio/play/line"},
		{'T', "audio/play/to_end"},
		{'B', "audio/play/toggle"},
		{'H', "audio/stop"},
	};
	for (auto const& key : keys) {
		SCOPED_TRACE(key.command);
		called_commands.clear();
		wxEvtHandler handler;
		handler.Bind(wxEVT_CHAR_HOOK, [](wxKeyEvent &event) {
			EXPECT_FALSE(hotkey::check("Subtitle Grid", nullptr, event));
			EXPECT_TRUE(event.GetSkipped());
			EXPECT_TRUE(hotkey::check("Audio", nullptr, event));
		});
		wxKeyEvent event(wxEVT_CHAR_HOOK);
		event.m_keyCode = key.key;
		EXPECT_TRUE(handler.ProcessEvent(event));
		EXPECT_FALSE(event.GetSkipped());
		EXPECT_EQ((std::vector<std::string>{key.command}), called_commands);
	}
}

TEST_F(HotkeyDispatch, UnboundKeyContinuesThroughBothContexts) {
	wxKeyEvent event(wxEVT_CHAR_HOOK);
	event.m_keyCode = WXK_F24;
	EXPECT_FALSE(hotkey::check("Subtitle Grid", nullptr, event));
	EXPECT_FALSE(hotkey::check("Audio", nullptr, event));
	EXPECT_TRUE(event.GetSkipped());
	EXPECT_TRUE(called_commands.empty());
}

TEST_F(HotkeyDispatch, DirectAudioBindingConsumesEvent) {
	wxKeyEvent event(wxEVT_CHAR_HOOK);
	event.m_keyCode = 'S';
	EXPECT_TRUE(hotkey::check("Audio", nullptr, event));
	EXPECT_FALSE(event.GetSkipped());
	EXPECT_EQ((std::vector<std::string>{"audio/play/selection"}), called_commands);
}

TEST_F(HotkeyDispatch, GridBindingKeepsPriorityOverAudio) {
	wxKeyEvent event(wxEVT_CHAR_HOOK);
	event.m_keyCode = WXK_RIGHT;
	if (!hotkey::check("Subtitle Grid", nullptr, event))
		hotkey::check("Audio", nullptr, event);
	EXPECT_FALSE(event.GetSkipped());
	EXPECT_EQ((std::vector<std::string>{"video/frame/next"}), called_commands);
}
