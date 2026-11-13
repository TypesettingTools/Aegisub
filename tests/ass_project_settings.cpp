// Copyright (c) 2026, Aegisub contributors
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

#include "../src/ass_attachment.h"
#include "../src/ass_dialogue.h"
#include "../src/ass_file.h"
#include "../src/ass_info.h"
#include "../src/ass_style.h"
#include "../src/options.h"
#include "../src/subtitle_format_ass.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/log.h>
#include <libaegisub/vfr.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace config {
	agi::Options *opt;
	agi::MRUManager *mru;
	agi::Path *path;
}

namespace {
void Check(bool condition, const char *message) {
	if (!condition) throw std::runtime_error(message);
}

void Write(agi::fs::path const& path, std::string const& contents) {
	std::ofstream(path) << contents;
}

std::string Read(agi::fs::path const& path) {
	std::ifstream file(path);
	return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

const char *events = "[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\nDialogue: 0,0:00:00.00,0:00:01.00,Default,,0,0,0,,Original\n";

void Run(agi::fs::path const& root) {
	std::filesystem::create_directories(root / ".git");
	agi::Options options(root / "options.json", R"({"App":{"Save Project Settings in Subtitle File":false,"Save UI State":false}})", agi::Options::FLUSH_SKIP);
	config::opt = &options;
	AssSubtitleFormat format;
	agi::vfr::Framerate fps;
	auto input = root / "original.ass";
	auto output = root / "saved.ass";

	std::string garbage = "[aegisub project garbage]\n; Preserve this comment\n  Video File: old.mkv  \nActive Line: 8\nVideo AR Mode: invalid\nUnknown Setting: untouched\n\n";
	Write(input, "[Script Info]\nScriptType: v4.00+\n  Audio URI: old.flac  \n\n" + garbage + events);
	AssFile file;
	format.ReadFile(&file, input, fps, "utf-8");
	Check(file.Properties.video_file == "old.mkv", "legacy project settings must still load");
	Check(file.OriginalProperties.video_file == "old.mkv", "parsed in-file settings must be captured at load time");
	auto original_settings = file.OriginalProperties.Serialize(true, true);

	Write(root / ".aegiconf", "Video File: sidecar.mkv\nActive Line: 2\n");
	file.Properties.LoadConfig(input);
	Check(file.Properties.video_file == "sidecar.mkv" && file.Properties.audio_file.empty(), "configs must replace the active legacy settings");
	file.Properties.video_file = "current.mkv";
	file.Properties.audio_file = "current.flac";
	file.Properties.active_row = 3;
	file.Events.front().Text = "Edited";
	format.WriteFile(&file, output, fps, "utf-8");
	auto saved = Read(output);
	Check(saved.find("Video File: old.mkv") != std::string::npos && saved.find("Active Line: 8") != std::string::npos, "disabled in-file saving must retain parsed load-time settings, including UI state");
	Check(saved.find("Audio File: old.flac") != std::string::npos && saved.find("Audio URI:") == std::string::npos, "legacy settings must normalize into the project garbage section");
	Check(saved.find("Unknown Setting:") == std::string::npos && saved.find("; Preserve this comment") == std::string::npos && saved.find("Video AR Mode: invalid") == std::string::npos, "unparsed metadata must follow the normal ASS parsing behavior");
	Check(saved.find("current.mkv") == std::string::npos && saved.find("Edited") != std::string::npos, "only subtitle edits, not active project settings, should update the ASS file");

	file.Properties.SaveConfig(output, true);
	Check(Read(ProjectProperties::ConfigFilename(output)).find("Video File: current.mkv") != std::string::npos, "sidecar saving must use current settings rather than the preserved snapshot");

	AssFile reloaded;
	format.ReadFile(&reloaded, output, fps, "utf-8");
	Check(reloaded.OriginalProperties.Serialize(true, true) == original_settings, "saving and reloading must preserve the original parsed values");
	format.WriteFile(&reloaded, output, fps, "utf-8");
	Check(Read(output) == saved, "repeated saves must leave preserved project garbage unchanged");

	AssFile copied(file);
	Check(copied.OriginalProperties.Serialize(true, true) == original_settings && copied.Properties.video_file == "current.mkv", "file copies must retain both original and active project settings");
	AssFile swapped;
	swapped.swap(copied);
	format.WriteFile(&swapped, output, fps, "utf-8");
	Check(Read(output).find("Video File: old.mkv") != std::string::npos, "swapping files must retain the loaded settings snapshot");

	options.Get("App/Save Project Settings in Subtitle File")->SetBool(true);
	options.Get("App/Save UI State")->SetBool(true);
	format.WriteFile(&file, output, fps, "utf-8");
	saved = Read(output);
	Check(saved.find("Video File: current.mkv") != std::string::npos && saved.find("Active Line: 3") != std::string::npos, "enabled in-file saving must serialize current project settings");
	Check(saved.find("old.mkv") == std::string::npos && saved.find("Unknown Setting:") == std::string::npos, "enabled in-file saving must replace the old snapshot");

	options.Get("App/Save Project Settings in Subtitle File")->SetBool(false);
	format.WriteFile(&file, output, fps, "utf-8");
	Check(Read(output).find("Video File: old.mkv") != std::string::npos, "the preserved snapshot must remain the load-time state even after earlier saves");

	format.WriteRecoveryFile(&file, output);
	Check(Read(output).find("Video File: current.mkv") != std::string::npos, "recovery files must contain current project settings");
	format.ExportFile(&file, output, fps, "utf-8");
	Check(Read(output).find("[Aegisub Project Garbage]") == std::string::npos && Read(output).find("Audio File:") == std::string::npos, "exports must omit preserved project settings");

	for (auto const& section : {std::string(), std::string("[Aegisub Project Garbage]\n")}) {
		Write(input, "[Script Info]\nScriptType: v4.00+\n" + section + events);
		AssFile fresh;
		format.ReadFile(&fresh, input, fps, "utf-8");
		Check(fresh.OriginalProperties.Serialize(true, true).empty(), "empty or absent sections must both produce an empty snapshot");
		fresh.Properties.video_file = "current.mkv";
		format.WriteFile(&fresh, output, fps, "utf-8");
		Check(Read(output).find("[Aegisub Project Garbage]") == std::string::npos, "an empty saved snapshot must not produce a project garbage section");
		Check(Read(output).find("current.mkv") == std::string::npos, "disabled in-file saving must not populate an empty or absent section");
	}

	Write(input, "[Script Info]\nScriptType: v4.00+\nVideo File: legacy.mkv\n\n" + std::string(events));
	AssFile legacy_only;
	format.ReadFile(&legacy_only, input, fps, "utf-8");
	legacy_only.Properties.video_file = "current.mkv";
	format.WriteFile(&legacy_only, output, fps, "utf-8");
	saved = Read(output);
	Check(saved.find("[Aegisub Project Garbage]\nVideo File: legacy.mkv") != std::string::npos, "legacy-only files must migrate their loaded values into a project garbage section");
	Check(saved.find("current.mkv") == std::string::npos, "legacy migration must not replace loaded values with current settings");
	AssFile migrated;
	format.ReadFile(&migrated, output, fps, "utf-8");
	format.WriteFile(&migrated, output, fps, "utf-8");
	Check(Read(output) == saved, "legacy-only migration must be stable across subsequent saves");

}
}

int main() {
	agi::dispatch::Init([](agi::dispatch::Thunk) {});
	agi::log::LogSink sink;
	agi::log::log = &sink;
	auto root = agi::fs::path(std::filesystem::temp_directory_path()) /
		("aegisub-inline-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
	try {
		Run(root);
		std::filesystem::remove_all(root);
		std::cout << "ASS project settings tests passed\n";
		return 0;
	}
	catch (agi::Exception const& e) {
		std::cerr << e.GetMessage() << '\n';
	}
	catch (std::exception const& e) {
		std::cerr << e.what() << '\n';
	}
	std::filesystem::remove_all(root);
	return 1;
}
