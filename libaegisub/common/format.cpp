// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/format.h>

#include <libaegisub/fs_fwd.h>

#include <boost/filesystem/path.hpp>
#include <codecvt>

namespace {
template<typename Char>
int actual_len(int max_len, const Char *value) {
	int len = 0;
	while (value[len] && (max_len <= 0 || len < max_len)) ++len;
	return len;
}

template<typename Char>
int actual_len(int max_len, std::basic_string<Char> value) {
	if (max_len > 0 && static_cast<size_t>(max_len) < value.size())
		return max_len;
	return value.size();
}

template<typename Char>
void do_write_str(std::basic_ostream<Char>& out, const Char *str, int len) {
	out.write(str, len);
}

void do_write_str(std::ostream& out, const wchar_t *str, int len) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	auto u8 = utf8_conv.to_bytes(str, str + len);
	out.write(u8.data(), u8.size());
}

void do_write_str(std::wostream& out, const char *str, int len) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	auto wc = utf8_conv.from_bytes(str, str + len);
	out.write(wc.data(), wc.size());
}

template<typename Char>
struct format_parser {
	agi::format_detail::formatter_state<Char> &s;

	void read_and_append_up_to_next_specifier() {
		for (std::streamsize len = 0; ; ++len) {
			// Ran out of format specifiers; not an error due to that
			// translated strings may not need them all
			if (!s.fmt[len]) {
				s.out.write(s.fmt, len);
				s.fmt += len;
				return;
			}

			if (s.fmt[len] == '%') {
				if (s.fmt[len + 1] == '%') {
					s.out.write(s.fmt, len);
					s.fmt += len + 1;
					len = 0;
					continue;
				}

				s.out.write(s.fmt, len);
				s.fmt += len;
				break;
			}
		}
	}

	int read_int() {
		int i = 0;
		for (; *s.fmt_cur >= '0' && *s.fmt_cur <= '9'; ++s.fmt_cur)
			i = 10 * i + (*s.fmt_cur - '0');
		return i;
	}

	void parse_flags() {
		for (; ; ++s.fmt_cur) {
			switch (*s.fmt_cur) {
			// Not supported: ' ' (add a space before positive numers to align with negative)
			case '#':
				s.out.setf(std::ios::showpoint | std::ios::showbase);
				continue;
			case '0':
				// overridden by left alignment ('-' flag)
				if (!(s.out.flags() & std::ios::left)) {
					// Use internal padding so that numeric values are
					// formatted correctly, eg -00010 rather than 000-10
					s.out.fill('0');
					s.out.setf(std::ios::internal, std::ios::adjustfield);
				}
				continue;
			case '-':
				s.out.fill(' ');
				s.out.setf(std::ios::left, std::ios::adjustfield);
				continue;
			case '+':
				s.out.setf(std::ios::showpos);
				continue;
			}
			break;
		}
	}

	void parse_width() {
		if (*s.fmt_cur >= '0' && *s.fmt_cur <= '9')
			s.width = read_int();
		else if (*s.fmt_cur == '*') {
			s.read_width = true;
			s.pending = true;
			++s.fmt_cur;
		}
	}

	void parse_precision() {
		if (*s.fmt_cur != '.') return;
		++s.fmt_cur;

		// Ignoring negative precision because it's dumb and pointless
		if (*s.fmt_cur >= '0' && *s.fmt_cur <= '9')
			s.precision = read_int();
		else if (*s.fmt_cur == '*') {
			s.read_precision = true;
			s.pending = true;
			++s.fmt_cur;
		}
		else
			s.precision = 0;
	}

	void parse_length_modifiers() {
		// Where "parse" means "skip" since we don't need them
		for (Char c = *s.fmt_cur;
			c == 'l' || c == 'h' || c == 'L' || c == 'j' || c == 'z' || c == 't';
			c = *++s.fmt_cur);
	}

	void parse_format_specifier() {
		s.width = 0;
		s.precision = -1;
		s.out.fill(' ');
		s.out.unsetf(
			std::ios::adjustfield |
			std::ios::basefield   |
			std::ios::boolalpha   |
			std::ios::floatfield  |
			std::ios::showbase    |
			std::ios::showpoint   |
			std::ios::showpos     |
			std::ios::uppercase);

		// Don't touch fmt until the specifier is fully applied so that if we
		// have insufficient arguments it'll get passed through to the output
		s.fmt_cur = s.fmt + 1;

		parse_flags();
		parse_width();
		parse_precision();
		parse_length_modifiers();
	}
};
}

namespace agi {

template<typename StreamChar, typename Char>
void writer<StreamChar, const Char *>::write(std::basic_ostream<StreamChar>& out, int max_len, const Char *value) {
	do_write_str(out, value, actual_len(max_len, value));
}

template<typename StreamChar, typename Char>
void writer<StreamChar, std::basic_string<Char>>::write(std::basic_ostream<StreamChar>& out, int max_len, std::basic_string<Char> const& value) {
	do_write_str(out, value.data(), actual_len(max_len, value));
}

template struct writer<char, const char *>;
template struct writer<char, const wchar_t *>;
template struct writer<wchar_t, const char *>;
template struct writer<wchar_t, const wchar_t *>;
template struct writer<char, std::string>;
template struct writer<char, std::wstring>;
template struct writer<wchar_t, std::string>;
template struct writer<wchar_t, std::wstring>;

namespace format_detail {

template<typename Char>
bool formatter<Char>::parse_next() {
	format_parser<Char> parser{*static_cast<formatter_state<Char> *>(this)};
	parser.read_and_append_up_to_next_specifier();
	if (!*this->fmt) return false;
	parser.parse_format_specifier();
	return true;
}

template<typename Char>
formatter<Char>::~formatter() {
	// Write remaining formatting string
	for (std::streamsize len = 0; ; ++len) {
		if (!this->fmt[len]) {
			this->out.write(this->fmt, len);
			return;
		}

		if (this->fmt[len] == '%' && this->fmt[len + 1] == '%') {
			this->out.write(this->fmt, len);
			this->fmt += len + 1;
			len = 0;
			continue;
		}
	}
}

template class formatter<char>;
template class formatter<wchar_t>;

} }

