// Copyright (c) 2010-2011, Amar Takhar <verm@aegisub.org>
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

/// @file path.h
/// @brief Common paths.
/// @ingroup libaegisub

#include <libaegisub/exception.h>
#include <libaegisub/scoped_ptr.h>

namespace agi {

DEFINE_BASE_EXCEPTION_NOINNER(PathError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(PathErrorNotFound, PathError, "path/not_found")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PathErrorInvalid, PathError, "path/invalid")
DEFINE_SIMPLE_EXCEPTION_NOINNER(PathErrorInternal, PathError, "path")

class Options;

/// @class Path
// Internal representation of all paths in aegisub.
class Path {
public:
	// For unit testing.
	friend class PathTest;

	/// Constructor
	Path(const std::string &file, const std::string& default_path);

	/// Destructor
	~Path();

	/// @brief Get a path, this is automatically decoded.
	/// @param name Path to get
	/// @return Full path name in UTF-8
	std::string Get(const char *name);

	/// @brief Set a path, this will be automaticalled encoded if a cookie matches.
	/// @param[in] name Path name to save to.
	void Set(const char *name, const std::string &path);

	/// @brief Set a list of paths
	/// @param name Path name.
	/// @param out[out] Map to load list into
	void ListGet(const char *name, std::vector<std::string> &out);

	/// @brief Set a list of paths.
	/// @param name Path name.
	/// @param list List to set.
	void ListSet(const char *name, std::vector<std::string> list);

	/// @brief Get the default 'open' directory when no alternative is available.
	/// @return Directory
	/// This returns several different values based on OS:
	///   Windows: Documents folder
	///   OS X: ~/Documents
	///   Unix: ~ or Documents folder if set in the environment
	std::string Default();

	/// @brief Decode a path
	/// @param path Decode a path in-place.
	void Decode(std::string &path);

	/// Configuration directory
	static std::string Config();

private:
	/// Location of path config file.
	const std::string path_file;

	/// Internal default config.
	const std::string path_default;

	/// @brief Encode a path.
	/// @param path Encode a path in-place.
	///   ^CONFIG   - Configuration directory (not changable)
	///   ^USER     - Users home directory
	///   ^DATA     - Aegisub data files
	///   ^VIDEO    - Last opened video directory
	///   ^SUBTITLE - Last opened subtitle directory
	///   ^AUDIO    - Last opened audio directory
	void Encode(std::string &path);

	/// Options object.
	scoped_ptr<Options> opt;

	/// @brief Locale files
	/// @return Locale location
	/// This is directly assessibly as the Locale directory will never change on any platform.
	std::string Locale();

protected:
	std::string Data();	///< Shared resources
	std::string Doc();	///< Documents
	std::string User();	///< User config directory
	std::string Temp();	///< Temporary storage
};

} // namespace agi
