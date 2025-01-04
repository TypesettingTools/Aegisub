// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/

#include "libaegisub/path.h"

#include "libaegisub/fs.h"

#include <boost/range/distance.hpp>

namespace {
constexpr std::string_view tokens[] = {
	"?audio",
	"?data",
	"?dictionary",
	"?local",
	"?script",
	"?temp",
	"?user",
	"?video",
	"?state"
};

int find_token(std::string_view str) {
	if (str.size() < 5 || str[0] != '?') return -1;
	int idx;
	switch (str[1] + str[4]) {
	case 'a' + 'i': idx = 0; break;
	case 'd' + 'a': idx = 1; break;
	case 'd' + 't': idx = 2; break;
	case 'l' + 'a': idx = 3; break;
	case 's' + 'i': idx = 4; break;
	case 't' + 'p': idx = 5; break;
	case 'u' + 'r': idx = 6; break;
	case 'v' + 'e': idx = 7; break;
	case 's' + 't': idx = 8; break;
	default: return -1;
	}
	return str.starts_with(tokens[idx]) ? idx : -1;
}
int checked_find_token(std::string_view str) {
	int idx = find_token(str);
	if (idx == -1) throw agi::InternalError("Bad token: " + std::string(str));
	return idx;
}
}

namespace agi {

Path::Path() {
	static_assert(sizeof(paths) / sizeof(paths[0]) == sizeof(tokens) / sizeof(tokens[0]),
		"Token and path arrays need to be the same size");
	FillPlatformSpecificPaths();
}

fs::path Path::Decode(std::string_view path) const {
	int idx = find_token(path);
	if (idx == -1 || paths[idx].empty())
		return fs::path(path).make_preferred();
	path = path.substr(tokens[idx].size());
	if (path.size() && path[0] == '/') path.remove_prefix(1);
	if (path.empty()) return paths[idx];
	return (paths[idx]/path).make_preferred();
}

fs::path Path::MakeRelative(fs::path const& path, std::string_view token) const {
	return MakeRelative(path, paths[checked_find_token(token)]);
}

fs::path Path::MakeRelative(fs::path const& path, fs::path const& base) const {
	if (path.empty() || base.empty()) return path;

	const auto str = path.string();
	if (str.starts_with("?dummy") || str.starts_with("dummy-audio:"))
		return path;

	// Paths on different volumes can't be made relative to each other
	if (path.has_root_name() && path.root_name() != base.root_name())
		return path.string();

	auto path_it = path.begin();
	auto ref_it = base.begin();
	for (; path_it != path.end() && ref_it != base.end() && *path_it == *ref_it; ++path_it, ++ref_it) ;

	agi::fs::path result;
	for (; ref_it != base.end(); ++ref_it)
		result /= "..";
	for (; path_it != path.end(); ++path_it)
		result /= *path_it;

	return result;
}

fs::path Path::MakeAbsolute(fs::path path, std::string_view token) const {
	if (path.empty()) return path;
	int idx = checked_find_token(token);

	path.make_preferred();
	const auto str = path.string();
	if (str.starts_with("?dummy") || str.starts_with("dummy-audio:"))
		return path;
	return (paths[idx].empty() || path.is_absolute()) ? path : paths[idx]/path;
}

std::string Path::Encode(fs::path const& path) const {
	// Find the shortest encoding of path made relative to each token
	std::string shortest = path.string();
	size_t length = boost::distance(path);
	for (size_t i = 0; i < paths.size(); ++i) {
		if (paths[i].empty()) continue;

		const auto p = MakeRelative(path, tokens[i]);
		const size_t d = boost::distance(p);
		if (d < length) {
			length = d;
			shortest = (tokens[i]/p).string();
		}
	}

	return shortest;
}

void Path::SetToken(std::string_view token_name, fs::path const& token_value) {
	int idx = checked_find_token(token_name);

	if (token_value.empty())
		paths[idx] = token_value;
	else if (!token_value.is_absolute())
		paths[idx].clear();
	else {
		paths[idx] = token_value;
		paths[idx].make_preferred();
		if (fs::FileExists(paths[idx]))
			paths[idx] = paths[idx].parent_path();
	}
}

}
