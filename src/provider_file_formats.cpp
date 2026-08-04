// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

#include "provider_file_formats.h"

#include <algorithm>
#include <initializer_list>

namespace {
void Add(std::vector<std::string>& formats, std::initializer_list<const char *> added) {
	formats.insert(formats.end(), added.begin(), added.end());
}

void Finish(std::vector<std::string>& formats) {
	std::sort(formats.begin(), formats.end());
	formats.erase(std::unique(formats.begin(), formats.end()), formats.end());
}
}

std::vector<std::string> GetVideoFileExtensions() {
	// YUV4MPEG is always available.
	std::vector<std::string> formats = {".y4m", ".yuv"};
#ifdef WITH_FFMS2
	Add(formats, {".asf", ".avi", ".avs", ".d2v", ".h264", ".hevc", ".m2ts",
	              ".m4v", ".mkv", ".mov", ".mp4", ".mpeg", ".mpg", ".ogm", ".ts",
	              ".webm", ".wmv"});
#endif
#ifdef WITH_AVISYNTH
	// Other formats may work through installed source filters, but these are the
	// formats which the built-in AviSynth provider handles explicitly.
	Add(formats, {".avi", ".avs", ".d2v"});
#endif
	Finish(formats);
	return formats;
}

std::vector<std::string> GetAudioFileExtensions() {
	// The PCM provider is always available.
	std::vector<std::string> formats = {".wav"};
#ifdef WITH_FFMS2
	Add(formats, {".aac", ".ac3", ".ape", ".asf", ".avi", ".avs", ".d2v", ".dts",
	              ".eac3", ".flac", ".m2ts", ".m4a", ".m4v", ".mka", ".mkv", ".mov",
	              ".mp3", ".mp4", ".mpeg", ".mpg", ".ogg", ".ogm", ".opus", ".ts",
	              ".w64", ".webm", ".wma", ".wmv"});
#endif
#ifdef WITH_AVISYNTH
	Add(formats, {".avi", ".avs"});
#endif
	Finish(formats);
	return formats;
}

std::string MakeWildcard(std::vector<std::string> const& extensions) {
	std::string result;
	for (auto const& extension : extensions) {
		if (!result.empty()) result += ';';
		result += '*' + extension;
	}
	return result;
}
