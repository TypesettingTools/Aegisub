// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <string>

namespace agi {
struct Color;

namespace parser {
/// Try to parse a string as a color
/// @param[out] dst Color struct to populate with the parsed result
/// @param str String to parse
/// @return Was the string successfully parsed as a color?
///
/// If this function returns false, the contents of dst is undefined. It
/// may contain partially-parsed garbage.
///
/// This function supports the following formats:
///  * SSA colors (i.e. a decimal number representing a bgr value)
///  * ASS override and style formats (a (a)bgr hex number possibly with
///    some ampersands and an H somewhere)
///  * CSS-style #rrggbb and #rgb
///  * CSS-style rgb(r,g,b)
///
/// CSS's rgb(r%,g%,b%) format is not currently supported.
bool parse(Color& dst, std::string const& str);
} // namespace parser
} // namespace agi
