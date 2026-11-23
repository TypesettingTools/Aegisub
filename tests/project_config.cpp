#include "../src/project_properties.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef GetMessage
#endif

namespace {
void Check(bool condition, const char *message) {
	if (!condition) throw std::runtime_error(message);
}

void Write(agi::fs::path const& filename, std::string const& data) {
	std::filesystem::create_directories(filename.parent_path());
	agi::io::Save(filename).Get() << data;
}

void CheckHidden(agi::fs::path const& filename) {
#ifdef _WIN32
	auto attributes = GetFileAttributesW(filename.c_str());
	Check(attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_HIDDEN), "saved sidecars must be hidden on Windows");
#endif
}

std::string Read(agi::fs::path const& filename) {
	std::ifstream file(filename);
	return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void CheckLegacyPrecedence(agi::fs::path const& root) {
	auto directory = root / "episode";
	auto subtitle = directory / "episode.ass";
	auto sidecar = ProjectProperties::ConfigFilename(subtitle);
	Write(subtitle, "");
	std::filesystem::create_directories(root / ".git");
	Write(directory / ".aegiconf-other.ass", "Active Line: 99\n");

	ProjectProperties legacy;
	legacy.automation_scripts = "legacy.lua";
	legacy.export_filters = "legacy filter";
	legacy.export_encoding = "legacy encoding";
	legacy.style_storage = "legacy styles";
	legacy.audio_file = "?video";
	legacy.video_file = "legacy.mkv";
	legacy.timecodes_file = "legacy.timecodes.txt";
	legacy.keyframes_file = "legacy.keyframes.txt";
	legacy.automation_settings["legacy script"] = "legacy settings";
	legacy.video_zoom = 0.5;
	legacy.ar_value = 1.5;
	legacy.scroll_position = 10;
	legacy.active_row = 11;
	legacy.ar_mode = 2;
	legacy.video_position = 12;
	legacy.audio_from_video = true;
	legacy.inferred_files.insert("Video File");

	auto matches = [](ProjectProperties const& actual, ProjectProperties const& expected) {
		return actual.Serialize(true, true) == expected.Serialize(true, true) &&
		       actual.audio_from_video == expected.audio_from_video &&
		       actual.inferred_files == expected.inferred_files;
	};

	auto properties = legacy;
	properties.LoadConfig(subtitle);
	Check(matches(properties, legacy), "without matching configs, all legacy settings must remain unchanged");

	for (auto const& contents : {"", "; comment only\n", "Unknown: ignored\n"}) {
		Write(sidecar, contents);
		properties = legacy;
		properties.LoadConfig(subtitle);
		Check(matches(properties, ProjectProperties{}), "any per-file config must discard every legacy setting, even with no recognized directives");
	}

	std::filesystem::remove(sidecar);
	Write(root / ".aegiconf", "");
	properties = legacy;
	properties.LoadConfig(subtitle);
	Check(matches(properties, ProjectProperties{}), "an empty ancestor config must discard every legacy setting");

	Write(root / ".aegiconf", "Export Encoding: UTF-16\nActive Line: 6\n");
	Write(directory / ".aegiconf", "Active Line: 3\n");
	ProjectProperties expected;
	expected.export_encoding = "UTF-16";
	expected.active_row = 3;
	properties = legacy;
	properties.LoadConfig(subtitle);
	Check(matches(properties, expected), "configs must cascade over defaults without inheriting unspecified legacy settings");

	Write(sidecar, "Video Position: 20\n");
	expected.video_position = 20;
	properties = legacy;
	properties.LoadConfig(subtitle);
	Check(matches(properties, expected), "per-file configs must preserve earlier config values rather than resetting the cascade");
}

void Run(agi::fs::path const& root) {
	auto child = root / "season";
	auto subtitle = child / "episode.ass";
	Write(subtitle, "");
	std::filesystem::create_directories(root / ".git");
	Write(root / ".aegiconf", "Video File: parent.mkv\nAudio File: ?video\nActive Line: 8\nVideo AR Value: 1.5\n");
	Write(child / ".aegiconf", "Active Line: 3\n");
	ProjectProperties properties;
	properties.video_file = "legacy.mkv";
	properties.LoadConfig(subtitle);
	Check(properties.video_file == "parent.mkv", "inherited explicit paths must remain relative for the project loader and override legacy paths");
	Check(properties.active_row == 3 && properties.ar_value == 1.5, "child config must cascade over parent");
	Check(properties.audio_file == "?video" && properties.audio_from_video, "audio must retain the video directive");

	auto sidecar = ProjectProperties::ConfigFilename(subtitle);
	Write(sidecar, "Active Line: 0\nVideo File: local.mkv\n");
	properties.LoadConfig(subtitle);
	Check(properties.active_row == 0 && properties.video_file == "local.mkv", "per-file config must override directory config, including explicit zero");
	Write(child / ".aegiconf-other.ass", "Active Line: 99\n");
	properties.LoadConfig(subtitle);
	Check(properties.active_row == 0, "a sibling subtitle's config must not be loaded");

	std::filesystem::remove(sidecar);
	Write(child / "video_b.mkv", "");
	Write(child / "video_a.mkv", "");
	Write(child / "audio.flac", "");
	Write(child / "keyframes.txt", "");
	Write(root / ".aegiconf", "Video File Pattern: video_*.mkv\nAudio File Pattern: *.flac\nKeyframes File Pattern: key*.txt\nTimecodes File: timecodes.txt\n");
	Write(root / "video_0_decoy.mkv", "");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "video_a.mkv").generic_string(), "inherited video patterns must search the subtitle directory, not the config directory");
	Check(properties.audio_file == (child / "audio.flac").generic_string() &&
	      properties.keyframes_file == (child / "keyframes.txt").generic_string() &&
	      properties.timecodes_file == "timecodes.txt", "all inherited media paths and patterns must use the subtitle directory");
	auto other_episode = root / "episode02" / "episode02.ass";
	Write(other_episode, "");
	Write(other_episode.parent_path() / "video_episode02.mkv", "");
	ProjectProperties other;
	other.LoadConfig(other_episode);
	Check(other.video_file == (other_episode.parent_path() / "video_episode02.mkv").generic_string(), "one shared pattern must select media separately for each episode");
	Write(child / ".aegiconf", "Video File Pattern: missing*.mkv\nVideo File Pattern: video_*.mkv\nAudio File Pattern: *.flac\nKeyframes File Pattern: key*.txt\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "video_a.mkv").generic_string(), "patterns must try in order and select a deterministic match");
	Check(properties.audio_file == (child / "audio.flac").generic_string(), "audio patterns must expand");
	Check(properties.keyframes_file == (child / "keyframes.txt").generic_string(), "keyframe patterns must expand");
	Check(properties.inferred_files.size() == 3, "pattern-derived paths must be tracked");
	properties.SaveConfig(subtitle, true);
	CheckHidden(sidecar);
	auto saved = Read(sidecar);
	Check(saved.find("Video File:") == std::string::npos && saved.find("Audio File:") == std::string::npos && saved.find("Keyframes File:") == std::string::npos, "saving must not freeze inferred paths");
	Check(saved.find("Video File Pattern:") == std::string::npos, "saving must not copy parent patterns");
	ProjectProperties reopened;
	reopened.LoadConfig(subtitle);
	Check(reopened.video_file == properties.video_file, "saving and reopening must retain inherited patterns");

	Write(sidecar, "  ; keep this comment  \nUnknown: untouched\nOther File Pattern: preserve*\nVideo File Pattern: video_b*\nVideo File Pattern: video_a*\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "video_b.mkv").generic_string(), "pattern line order must take priority over filename order");
	properties.SaveConfig(subtitle, true);
	saved = Read(sidecar);
	Check(saved.find("  ; keep this comment  ") != std::string::npos && saved.find("Unknown: untouched") != std::string::npos && saved.find("Other File Pattern: preserve*") != std::string::npos, "saving must preserve comments and unknown directives");
	Check(saved.find("video_b*") < saved.find("video_a*"), "saving must preserve pattern order");
	properties.SaveConfig(subtitle, true);
	Check(Read(sidecar) == saved, "repeated saves must not accumulate blank lines or alter preserved whitespace");
	CheckHidden(sidecar);

	properties.video_file = "chosen.mkv";
	properties.inferred_files.erase("Video File");
	properties.active_row = 0;
	properties.automation_settings["script"] = "settings";
	properties.SaveConfig(subtitle, false);
	saved = Read(sidecar);
	Check(saved.find("Video File: chosen.mkv") != std::string::npos && saved.find("Video File Pattern:") == std::string::npos, "a manual selection must replace per-file patterns");
	Check(saved.find("Active Line:") == std::string::npos, "UI state saving can be disabled");
	reopened.LoadConfig(subtitle);
	Check(reopened.automation_settings["script"] == "settings", "automation settings must round trip");

	Write(sidecar, "Video File Pattern: no_match*\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file.empty() && properties.inferred_files.count("Video File"), "unmatched child patterns must suppress parent candidates");
	Write(sidecar, "Video File Pattern: video_*\nVideo File: explicit.mkv\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == "explicit.mkv" && !properties.inferred_files.count("Video File"), "explicit child paths must override patterns");

	Write(child / "媒体" / "épisode01.mkv", "");
	Write(sidecar, "Video File Pattern: 媒*/épisode??.mkv\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "媒体" / "épisode01.mkv").generic_string(), "Unicode names and wildcard directory components must match");
	Write(sidecar, "Video File Pattern: ?ideo_a.mkv\n");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "video_a.mkv").generic_string(), "a leading question mark in a pattern must be a wildcard");

	Write(sidecar, "");
	properties.LoadConfig(subtitle);
	Check(properties.video_file == (child / "video_a.mkv").generic_string(), "an empty per-file config must preserve inherited settings");

	Write(sidecar, "\xEF\xBB\xBF" "Audio File: ?video\r\nVideo AR Mode: invalid\r\n");
	properties.ar_mode = 2;
	properties.LoadConfig(subtitle);
	Check(properties.audio_from_video && properties.ar_mode == 0, "UTF-8 BOM and CRLF must be handled, and malformed numbers must not restore legacy values");
	auto lines = properties.Serialize(true, false);
	Check(std::find(lines.begin(), lines.end(), "Audio File: ?video") != lines.end(), "video audio references must not become explicit media paths");
	auto legacy = properties.Serialize(false, true);
	Check(std::find(legacy.begin(), legacy.end(), "Video File: " + properties.video_file) != legacy.end(), "legacy and recovery serialization must include resolved paths");

	for (bool git_file : {false, true}) {
		auto repository = root / (git_file ? "worktree" : "repository");
		auto nested_subtitle = repository / "episode" / "test.ass";
		Write(nested_subtitle, "");
		if (git_file) Write(repository / ".git", "gitdir: elsewhere\n");
		else std::filesystem::create_directories(repository / ".git");
		Write(repository / ".aegiconf", "Active Line: 12\n");
		ProjectProperties bounded;
		bounded.LoadConfig(nested_subtitle);
		Check(bounded.active_row == 12, "config in the Git boundary directory must be included");
		Check(bounded.video_file.empty() && bounded.timecodes_file.empty(), "config discovery must not cross a .git directory or file");
	}
}
}

int main() {
	agi::dispatch::Init([](agi::dispatch::Thunk) {});
	agi::log::LogSink sink;
	agi::log::log = &sink;
	auto root = agi::fs::Canonicalize(agi::fs::path(std::filesystem::temp_directory_path())) /
		("aegisub-config-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	try {
		Run(root);
		CheckLegacyPrecedence(root / "legacy-precedence");
		std::filesystem::remove_all(root);
		std::cout << "Project config tests passed\n";
		return 0;
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << '\n';
	}
	catch (agi::Exception const& e) {
		std::cerr << e.GetMessage() << '\n';
	}
	std::filesystem::remove_all(root);
	return 1;
}
