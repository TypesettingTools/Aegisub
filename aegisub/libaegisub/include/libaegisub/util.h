// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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
// $Id$

/// @file util.h
/// @brief Public interface for general utilities.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <stdint.h>

#include <string>
#include <algorithm>
#endif // LAGI_PRE

#include <libaegisub/access.h>
#include <libaegisub/types.h>

namespace agi {
	namespace util {


	/// Whether the path is a file or directory.
	enum PathType {
		TypeFile,		///< File
		TypeDir			///< Directory
	};

	// Calculate midpoint from a list of Integers
	template<typename T> inline T mid(T a, T b, T c) { return std::max(a, std::min(b, c)); }

	// Get the parent directory of a path.
	// @param path Path to process.
	const std::string DirName(const std::string& path);

	// Rename a file.
	// @param from Source.
	// @param to   Destination.
	void Rename(const std::string& from, const std::string& to);

	// Get time sutable for logging mechinisms.
	// @param tv timeval
	void time_log(agi_timeval &tv);

	// Make all alphabetic characters lowercase.
	// @param str Input string
	void str_lower(std::string &str);

	// Convert a string to Integer.
	// @param str Input string
	int strtoi(std::string &str);

	/// Check for amount of free space on a Path.
	// @param path[in] Path to check
	// @param type     PathType (default is TypeDir)
	uint64_t freespace(std::string &path, PathType type=TypeDir);

	struct delete_ptr {
		template<class T>
		void operator()(T* ptr) const {
			delete ptr;
		}
	};
	template<class T>
	void delete_clear(T& container) {
		std::for_each(container.begin(), container.end(), delete_ptr());
		container.clear();
	}

	} // namespace util
} // namespace agi
