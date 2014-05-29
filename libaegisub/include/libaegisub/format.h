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

#include <libaegisub/fs_fwd.h>

#include <boost/interprocess/streams/vectorstream.hpp>
#include <boost/io/ios_state.hpp>
#include <codecvt>
#include <type_traits>

class wxString;

namespace agi { namespace format_detail {
// A static cast which throws at runtime if the cast is invalid rather than
// failing to compile, as with format strings we don't know what type to cast
// to at compile time.
template<typename In, typename Out, bool = std::is_convertible<In, Out>::value>
struct runtime_cast_helper {
	static Out cast(In const&) { throw std::bad_cast(); }
};

template<typename In, typename Out>
struct runtime_cast_helper<In, Out, true> {
	static Out cast(In const& value) {
		return static_cast<Out>(value);
	}
};

template<typename Out, typename In>
Out runtime_cast(In const& value) {
	return runtime_cast_helper<In, Out>::cast(value);
}

// Check length for string types
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
inline void do_write_str(std::basic_ostream<Char>& out, const Char *str, int len) {
	out.write(str, len);
}

inline void do_write_str(std::ostream& out, const wchar_t *str, int len) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	auto u8 = utf8_conv.to_bytes(str, str + len);
	out.write(u8.data(), u8.size());
}

inline void do_write_str(std::wostream& out, const char *str, int len) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	auto wc = utf8_conv.from_bytes(str, str + len);
	out.write(wc.data(), wc.size());
}
}

template<typename Char, typename T>
struct writer {
	static void write(std::basic_ostream<Char>& out, int, T const& value) {
		out << value;
	}
};

// Ensure things with specializations don't get implicitly initialized
template<> struct writer<char, agi::fs::path>;
template<> struct writer<wchar_t, agi::fs::path>;
template<> struct writer<char, wxString>;
template<> struct writer<wchar_t, wxString>;

template<typename StreamChar, typename Char>
struct writer<StreamChar, const Char *> {
	static void write(std::basic_ostream<StreamChar>& out, int max_len, const Char *value) {
		format_detail::do_write_str(out, value, format_detail::actual_len(max_len, value));
	}
};

template<typename StreamChar, typename Char>
struct writer<StreamChar, std::basic_string<Char>> {
	static void write(std::basic_ostream<StreamChar>& out, int max_len, std::basic_string<Char> const& value) {
		format_detail::do_write_str(out, value.data(), format_detail::actual_len(max_len, value));
	}
};

namespace format_detail {
template<typename Char>
class formatter {
	formatter(const formatter&) = delete;
	formatter& operator=(const formatter&) = delete;

	std::basic_ostream<Char>& out;
	const Char *fmt;
	const Char *fmt_cur = nullptr;

	bool read_width = false;
	bool read_precision = false;
	bool pending = false;

	int width = 0;
	int precision = 0;

	boost::io::basic_ios_all_saver<Char> saver;

	void read_and_append_up_to_next_specifier() {
		for (std::streamsize len = 0; ; ++len) {
			// Ran out of format specifiers; not an error due to that
			// translated strings may not need them all
			if (!fmt[len]) {
				out.write(fmt, len);
				fmt += len;
				return;
			}

			if (fmt[len] == '%') {
				if (fmt[len + 1] == '%') {
					out.write(fmt, len);
					fmt += len + 1;
					len = 0;
					continue;
				}

				out.write(fmt, len);
				fmt += len;
				break;
			}
		}
	}

	int read_int() {
		int i = 0;
		for (; *fmt_cur >= '0' && *fmt_cur <= '9'; ++fmt_cur)
			i = 10 * i + (*fmt_cur - '0');
		return i;
	}

	void parse_flags() {
		for (; ; ++fmt_cur) {
			switch (*fmt_cur) {
			// Not supported: ' ' (add a space before positive numers to align with negative)
			case '#':
				out.setf(std::ios::showpoint | std::ios::showbase);
				continue;
			case '0':
				// overridden by left alignment ('-' flag)
				if (!(out.flags() & std::ios::left)) {
					// Use internal padding so that numeric values are
					// formatted correctly, eg -00010 rather than 000-10
					out.fill('0');
					out.setf(std::ios::internal, std::ios::adjustfield);
				}
				continue;
			case '-':
				out.fill(' ');
				out.setf(std::ios::left, std::ios::adjustfield);
				continue;
			case '+':
				out.setf(std::ios::showpos);
				continue;
			}
			break;
		}
	}

	void parse_width() {
		if (*fmt_cur >= '0' && *fmt_cur <= '9')
			width = read_int();
		else if (*fmt_cur == '*') {
			read_width = true;
			pending = true;
			++fmt_cur;
		}
	}

	void parse_precision() {
		if (*fmt_cur != '.') return;
		++fmt_cur;

		// Ignoring negative precision because it's dumb and pointless
		if (*fmt_cur >= '0' && *fmt_cur <= '9')
			precision = read_int();
		else if (*fmt_cur == '*') {
			read_precision = true;
			pending = true;
			++fmt_cur;
		}
		else
			precision = 0;
	}

	void parse_length_modifiers() {
		// Where "parse" means "skip" since we don't need them
		for (Char c = *fmt_cur;
			c == 'l' || c == 'h' || c == 'L' || c == 'j' || c == 'z' || c == 't';
			c = *++fmt_cur);
	}

	void parse_format_specifier() {
		width = 0;
		precision = -1;
		out.fill(' ');
		out.unsetf(
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
		fmt_cur = fmt + 1;

		parse_flags();
		parse_width();
		parse_precision();
		parse_length_modifiers();
	}

public:
	formatter(std::basic_ostream<Char>& out, const Char *fmt) : out(out), fmt(fmt), saver(out) { }
	~formatter() {
		// Write remaining formatting string
		for (std::streamsize len = 0; ; ++len) {
			if (!fmt[len]) {
				out.write(fmt, len);
				return;
			}

			if (fmt[len] == '%' && fmt[len + 1] == '%') {
				out.write(fmt, len);
				fmt += len + 1;
				len = 0;
				continue;
			}
		}
	}

	template<typename T>
	void operator()(T&& value) {
		if (!pending) {
			read_and_append_up_to_next_specifier();
			if (!*fmt) return;
			parse_format_specifier();
		}

		if (read_width) {
			width = runtime_cast<int>(value);
			read_width = false;
			return;
		}

		if (read_precision) {
			precision = runtime_cast<int>(value);
			read_precision = false;
			return;
		}
		pending = false;

		if (width < 0) {
			out.fill(' ');
			out.setf(std::ios::left, std::ios::adjustfield);
			width = -width;
		}
		out.width(width);
		out.precision(precision < 0 ? 6 : precision);

		Char c = *fmt_cur ? fmt_cur[0] : 's';
		if (c >= 'A' && c <= 'Z') {
			out.setf(std::ios::uppercase);
			c += 'a' - 'A';
		}

		switch (c) {
		case 'c':
			out.setf(std::ios::dec, std::ios::basefield);
			out << runtime_cast<Char>(value);
			break;
		case 'd': case 'i':
			out.setf(std::ios::dec, std::ios::basefield);
			out << runtime_cast<intmax_t>(value);
			break;
		case 'o':
			out.setf(std::ios::oct, std::ios::basefield);
			out << runtime_cast<intmax_t>(value);
			break;
		case 'x':
			out.setf(std::ios::hex, std::ios::basefield);
			out << runtime_cast<intmax_t>(value);
			break;
		case 'u':
			out.setf(std::ios::dec, std::ios::basefield);
			out << runtime_cast<uintmax_t>(value);
			break;
		case 'e':
			out.setf(std::ios::scientific, std::ios::floatfield);
			out.setf(std::ios::dec, std::ios::basefield);
			out << runtime_cast<double>(value);
			break;
		case 'f':
			out.setf(std::ios::fixed, std::ios::floatfield);
			out << runtime_cast<double>(value);
			break;
		case 'g':
			out.setf(std::ios::dec, std::ios::basefield);
			out.flags(out.flags() & ~std::ios::floatfield);
			out << runtime_cast<double>(value);
			break;
		case 'p':
			out.setf(std::ios::hex, std::ios::basefield);
			out << runtime_cast<const void *>(value);
			break;
		default: // s and other
			out.setf(std::ios::boolalpha);
			writer<Char, typename std::decay<T>::type>::write(out, precision, value);
			break;
		}

		fmt = *fmt_cur ? fmt_cur + 1 : fmt_cur;
	}
};

// Base case for variadic template recursion
template<typename Char>
inline void format(formatter<Char>&&) { }

template<typename Char, typename T, typename... Args>
void format(formatter<Char>&& fmt, T&& first, Args&&... rest) {
	fmt(first);
	format(std::move(fmt), std::forward<Args>(rest)...);
}
} // namespace format_detail

template<typename Char, typename... Args>
void format(std::basic_ostream<Char>& out, const Char *fmt, Args&&... args) {
	format(format_detail::formatter<Char>(out, fmt), std::forward<Args>(args)...);
}

template<typename Char, typename... Args>
std::basic_string<Char> format(const Char *fmt, Args&&... args) {
	boost::interprocess::basic_vectorstream<std::basic_string<Char>> out;
	format(out, fmt, std::forward<Args>(args)...);
	return out.vector();
}
}
