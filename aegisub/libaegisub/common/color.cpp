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

#include "../config.h"

#include "libaegisub/color.h"

#include "parser.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace agi {

Color::Color() : r(0), g(0), b(0), a(0) { }

Color::Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
: r(r), g(g), b(b), a(a)
{ }

Color::Color(std::string const& str)
: r(0), g(0), b(0), a(0)
{
	parser::parse(*this, str);
}

std::string Color::GetAssStyleFormatted() const {
	return str(boost::format("&H%02X%02X%02X%02X") % (int)a % (int)b % (int)g % (int)r);
}

std::string Color::GetAssOverrideFormatted() const {
	return str(boost::format("&H%02X%02X%02X&") % (int)b % (int)g % (int)r);
}

std::string Color::GetSsaFormatted() const {
	return boost::lexical_cast<std::string>((a << 24) + (b << 16) + (g << 8) + r);
}

std::string Color::GetHexFormatted() const {
	return str(boost::format("#%02X%02X%02X") % (int)r % (int)g % (int)b);
}

std::string Color::GetRgbFormatted() const {
	return str(boost::format("rgb(%d, %d, %d)") % (int)r % (int)g % (int)b);
}

bool Color::operator==(Color const& col) const {
	return r == col.r && g == col.g && b == col.b && a == col.a;
}

bool Color::operator!=(Color const& col) const {
	return !(*this == col);
}

}
