// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

#pragma once

#include <string>
#include <vector>

/// File extensions (including the leading dot) handled by compiled providers.
std::vector<std::string> GetAudioFileExtensions();
std::vector<std::string> GetVideoFileExtensions();

/// Turn an extension list into the pattern portion of a wx file-dialog filter.
std::string MakeWildcard(std::vector<std::string> const& extensions);
