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

#include <libaegisub/cajun/elements.h>
#include <libaegisub/fs.h>

namespace agi::json_util {

/// Parse a JSON stream.
/// @param stream JSON stream to parse
/// @return json::UnknownElement
json::UnknownElement parse(std::istream &stream);

/// Parse a json stream, with default handler.
/// @param file Path to JSON file.
/// @param default_config Default config file to load in case of nonexistent file
/// @return json::UnknownElement
json::UnknownElement file(agi::fs::path const& file, std::string_view default_config);

}
