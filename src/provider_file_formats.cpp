// Copyright (c) 2026, Aegisub contributors
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted.

#include "provider_file_formats.h"

std::string MakeWildcard(std::vector<std::string> const& extensions) {
	std::string result;
	for (auto const& extension : extensions) {
		if (!result.empty()) result += ';';
		result += '*' + extension;
	}
	return result;
}
