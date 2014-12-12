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

/// @file charset.h
/// @brief Character set detection and manipulation utilities.
/// @ingroup libaegisub

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>

#include <string>
#include <vector>

namespace agi {
	/// Character set conversion and detection.
	namespace charset {

/// List of detected encodings.
typedef std::vector<std::pair<float, std::string>> CharsetListDetected;

/// @brief Return a complete list of detected character sets ordered by precedence.
/// @param file File to check
/// @return List of possible charsets sorted by probability
CharsetListDetected DetectAll(agi::fs::path const& file);

/// @brief Returns the character set with the highest confidence
/// @param file File to check
/// @return Detected character set.
std::string Detect(agi::fs::path const& file);

	} // namespace util
} // namespace agi
