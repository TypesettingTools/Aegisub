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

line_iterator_base::line_iterator_base(std::istream &stream, std::string encoding)
: stream(&stream)
{
	boost::to_lower(encoding);
	if (encoding != "utf-8") {
		agi::charset::IconvWrapper c("utf-8", encoding.c_str());
		c.Convert("\r", 1, reinterpret_cast<char *>(&cr), sizeof(int));
		c.Convert("\n", 1, reinterpret_cast<char *>(&lf), sizeof(int));
		width = c.RequiredBufferSize("\n");
		conv = std::make_shared<agi::charset::IconvWrapper>(encoding.c_str(), "utf-8");
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
		union {
			int32_t chr;
			char buf[4];
		} u;

		for (;;) {
			u.chr = 0;
			std::streamsize read = stream->rdbuf()->sgetn(u.buf, width);
			if (read < (std::streamsize)width) {
				for (int i = 0; i < read; i++) {
					str += u.buf[i];
				}
				stream->setstate(std::ios::eofbit);
				break;
			}
			if (u.chr == cr) continue;
			if (u.chr == lf) break;
			for (int i = 0; i < read; i++) {
				str += u.buf[i];
			}
		}
	}

	if (conv.get()) {
		std::string tmp;
		conv->Convert(str, tmp);
		str = std::move(tmp);
	}

	return true;
}
}
