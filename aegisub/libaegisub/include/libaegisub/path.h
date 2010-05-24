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

/// @file path.h
/// @brief Common paths.
/// @ingroup libaegisub

#ifndef AGI_PRE
#endif

#include <libaegisub/option.h>

namespace agi {

/// @class Path
// Internal representation of all paths in aegisub.
class Path {
public:

	/// Constructor
	Path(const std::string &file, const std::string& default_path);

	/// Destructor
	~Path();

private:
	/// Location of path config file.
	const std::string path_file;

	/// Internal default config.
	const std::string path_default;

	/// Options object.
	Options *opt;

	const char *Data();		///< Shared resources
	const char *Doc();		///< Documents
	const char *User();		///< User config directory
	const char *Locale();	///< Locale files
	const char *Temp();		///< Temporary storage
};

} // namespace agi
