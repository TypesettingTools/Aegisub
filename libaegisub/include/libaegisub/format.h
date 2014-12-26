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
}

template<typename Char, typename T>
struct writer {
	static void write(std::basic_ostream<Char>& out, int, T const& value) {
		out << value;
	}
};

template<typename StreamChar, typename Char>
struct writer<StreamChar, const Char *> {
	static void write(std::basic_ostream<StreamChar>& out, int max_len, const Char *value);
};

template<typename StreamChar, typename Char>
struct writer<StreamChar, std::basic_string<Char>> {
	static void write(std::basic_ostream<StreamChar>& out, int max_len, std::basic_string<Char> const& value);
};

// Ensure things with specializations don't get implicitly initialized
template<> struct writer<char, agi::fs::path>;
template<> struct writer<wchar_t, agi::fs::path>;
template<> struct writer<char, wxString>;
template<> struct writer<wchar_t, wxString>;

namespace format_detail {
template<typename Char>
struct formatter_state {
	std::basic_ostream<Char>& out;

	const Char *fmt;
	const Char *fmt_cur = nullptr;

	bool read_width = false;
	bool read_precision = false;
	bool pending = false;

	int width = 0;
	int precision = 0;

	formatter_state(std::basic_ostream<Char>&out , const Char *fmt)
	: out(out), fmt(fmt) { }
};

template<typename Char>
class formatter : formatter_state<Char> {
	formatter(const formatter&) = delete;
	formatter& operator=(const formatter&) = delete;

	boost::io::basic_ios_all_saver<Char> saver;

	bool parse_next();
	Char next_format();

public:
	formatter(std::basic_ostream<Char>& out, const Char *fmt)
	: formatter_state<Char>(out, fmt), saver(out) { }
	~formatter();

	template<typename T>
	void operator()(T&& value) {
		if (!this->pending && !parse_next()) return;

		if (this->read_width) {
			this->width = runtime_cast<int>(value);
			this->read_width = false;
			return;
		}

		if (this->read_precision) {
			this->precision = runtime_cast<int>(value);
			this->read_precision = false;
			return;
		}

		Char c = next_format();

		switch (c) {
		case 'c':
			this->out << runtime_cast<Char>(value);
			break;
		case 'd': case 'i':
			this->out << runtime_cast<intmax_t>(value);
			break;
		case 'o':
			this->out << runtime_cast<intmax_t>(value);
			break;
		case 'x':
			this->out << runtime_cast<intmax_t>(value);
			break;
		case 'u':
			this->out << runtime_cast<uintmax_t>(value);
			break;
		case 'e':
			this->out << runtime_cast<double>(value);
			break;
		case 'f':
			this->out << runtime_cast<double>(value);
			break;
		case 'g':
			this->out << runtime_cast<double>(value);
			break;
		case 'p':
			this->out << runtime_cast<const void *>(value);
			break;
		default: // s and other
			writer<Char, typename std::decay<T>::type>::write(this->out, this->precision, value);
			break;
		}
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
