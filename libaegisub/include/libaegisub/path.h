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

#include <libaegisub/fs_fwd.h>

#include <array>
#include <boost/filesystem/path.hpp>

namespace agi {
/// Class for handling everything path-related in Aegisub
class Path {
	/// Token -> Path map
	std::array<fs::path, 8> paths;

	/// Platform-specific code to fill in the default paths, called in the constructor
	void FillPlatformSpecificPaths();

public:
	/// Constructor
	Path();

	/// Decode and normalize a path which may begin with a registered token
	/// @param path Path which is either already absolute or begins with a token
	/// @return Absolute path
	fs::path Decode(std::string const& path) const;

	/// If path is relative, make it absolute relative to the token's path
	/// @param path A possibly relative path
	/// @param token Token containing base path for resolving relative paths
	/// @return Absolute path if `path` is absolute or `token` is set, `path` otherwise
	/// @throws InternalError if `token` is not a valid token name
	fs::path MakeAbsolute(fs::path path, std::string const& token) const;

	/// If `token` is set, make `path` relative to it
	/// @param path An absolute path
	/// @param token Token name to make `path` relative to
	/// @return A path relative to `token`'s value if `token` is set, `path` otherwise
	/// @throws InternalError if `token` is not a valid token name
	fs::path MakeRelative(fs::path const& path, std::string const& token) const;
	fs::path MakeRelative(fs::path const& path, const char *token) const { return MakeRelative(path, std::string(token)); }

	/// Make `path` relative to `base`, if possible
	/// @param path An absolute path
	/// @param base Base path to make `path` relative to
	/// @return A path relative to `base`'s value if possible, or `path` otherwise
	fs::path MakeRelative(fs::path const& path, fs::path const& base) const;

	/// Encode an absolute path to begin with a token if there are any applicable
	/// @param path Absolute path to encode
	/// @return path untouched, or with some portion of the beginning replaced with a token
	std::string Encode(fs::path const& path) const;

	/// Set a prefix token to use for encoding and decoding paths
	/// @param token_name A single word token beginning with '?'
	/// @param token_value An absolute path to a directory or file
	/// @throws InternalError if `token` is not a valid token name
	void SetToken(const char *token_name, fs::path const& token_value);
};

}
