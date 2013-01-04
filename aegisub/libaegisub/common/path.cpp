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

/// @file path.cpp
/// @brief Platform-independent path code
/// @ingroup libaegisub

#include "../config.h"

#include "libaegisub/path.h"

#include "libaegisub/fs.h"
#include "libaegisub/util.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/distance.hpp>

namespace {
	template<class T, class U>
	typename T::const_iterator last_less_than(T const& cont, U const& value) {
		auto it = cont.upper_bound(value);
		if (it != cont.begin())
			--it;
		return it;
	}
}

namespace agi {

Path::Path() {
	tokens["?user"];
	tokens["?local"];
	tokens["?data"];
	tokens["?temp"];
	tokens["?dictionary"];
	tokens["?docs"];

	FillPlatformSpecificPaths();

	tokens["?audio"];
	tokens["?script"];
	tokens["?video"];
}

fs::path Path::Decode(std::string const& path) const {
	const auto it = last_less_than(tokens, path);

	if (!it->second.empty() && boost::starts_with(path, it->first))
		return (it->second/path.substr(it->first.size())).make_preferred();
	return fs::path(path).make_preferred();
}

fs::path Path::MakeRelative(fs::path const& path, std::string const& token) const {
	const auto it = tokens.find(token);
	if (it == tokens.end()) throw agi::InternalError("Bad token: " + token, 0);

	return MakeRelative(path, it->second);
}

fs::path Path::MakeRelative(fs::path const& path, fs::path const& base) const {
	if (path.empty() || base.empty()) return path;

	const auto str = path.string();
	if (boost::starts_with(str, "?dummy") || boost::starts_with(str, "dummy-audio:"))
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

fs::path Path::MakeAbsolute(fs::path path, std::string const& token) const {
	const auto it = tokens.find(token);
	if (it == tokens.end()) throw agi::InternalError("Bad token: " + token, 0);
	if (path.empty()) return path;

	path.make_preferred();
	const auto str = path.string();
	if (boost::starts_with(str, "?dummy") || boost::starts_with(str, "dummy-audio:"))
		return path;
	return (it->second.empty() || path.is_absolute()) ? path : it->second/path;
}

std::string Path::Encode(fs::path const& path) const {
	// Find the shortest encoding of path made relative to each token
	std::string shortest = path.string();
	size_t length = boost::distance(path);
	for (auto const& tok : tokens) {
		if (tok.second.empty()) continue;

		const auto p = MakeRelative(path, tok.first);
		const size_t d = boost::distance(p);
		if (d < length) {
			length = d;
			shortest = (tok.first/p).string();
		}
	}

	return shortest;
}

void Path::SetToken(std::string const& token_name, fs::path const& token_value) {
	const auto it = tokens.find(token_name);
	if (it == tokens.end()) throw agi::InternalError("Bad token: " + token_name, 0);

	if (token_value.empty())
		it->second = token_value;
	else if (!token_value.is_absolute())
		it->second.clear();
	else {
		it->second = token_value;
		it->second.make_preferred();
		if (fs::FileExists(it->second))
			it->second = it->second.parent_path();
	}

	paths.clear();
	for (auto const& tok : tokens) {
		if (!tok.second.empty())
			paths[tok.second] = tok.first;
	}
}

}
