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

#include <libaegisub/line_iterator.h>
#include <libaegisub/charset_conv.h>

#include <boost/algorithm/string/case_conv.hpp>

namespace agi {

line_iterator_base::line_iterator_base(std::istream &stream, const char *encoding)
: stream(&stream)
{
	std::string_view e = encoding;
	if (e != "utf-8" && e != "UTF-8") {
		agi::charset::IconvWrapper c("utf-8", encoding);
		c.Convert("\r", cr);
		width = c.Convert("\n", lf);
		conv = std::make_shared<agi::charset::IconvWrapper>(encoding, "utf-8");
		assert(width != 0);
	}
}

bool line_iterator_base::getline(std::string &str) {
	if (!stream) return false;
	if (!stream->good()) {
		stream = nullptr;
		return false;
	}

	if (width == 1) {
		std::getline(*stream, str);
		if (str.size() && str.back() == '\r')
			str.pop_back();
	}
	else {
		for (;;) {
			std::array<char, 4> buf = {0, 0, 0, 0};
			std::streamsize read = stream->rdbuf()->sgetn(buf.data(), width);
			if (read < (std::streamsize)width) {
				for (int i = 0; i < read; i++) {
					str += buf[i];
				}
				stream->setstate(std::ios::eofbit);
				break;
			}
			if (buf == cr) continue;
			if (buf == lf) break;
			for (int i = 0; i < read; i++) {
				str += buf[i];
			}
		}
	}

	if (conv) {
		std::string tmp;
		conv->Convert(str, tmp);
		str = std::move(tmp);
	}

	return true;
}
}
