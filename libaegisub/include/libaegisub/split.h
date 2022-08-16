// Copyright (c) 2015, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <locale>
#include <string_view>

namespace agi {
	template<typename Char>
	class split_iterator {
		std::basic_string_view<Char> str;
		size_t pos = 0;
		Char delim;

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = std::string_view;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = ptrdiff_t;

		split_iterator(std::basic_string_view<Char> str, Char c)
		: str(str), delim(c)
		{
			pos = str.find(delim);
		}

		split_iterator() = default;

		bool eof() const { return str.size() == 0; }

		std::basic_string_view<Char> operator*() const {
			return str.substr(0, pos);
		}

		bool operator==(split_iterator const& it) const {
			return str == it.str && (str.size() == 0 || delim == it.delim);
		}

		split_iterator& operator++() {
			if (pos == str.npos) {
				str = str.substr(str.size());
			} else {
				str = str.substr(pos + 1);
				pos = str.find(delim);
			}
			return *this;
		}

		split_iterator operator++(int) {
			split_iterator tmp = *this;
			++*this;
			return tmp;
		}
	};

	template<typename Char>
	split_iterator<Char> begin(split_iterator<Char> const& it) {
		return it;
	}

	template<typename Char>
	split_iterator<Char> end(split_iterator<Char> const&) {
		return split_iterator<Char>();
	}

	template<typename Char>
	split_iterator<Char> Split(std::basic_string_view<Char> str, Char delim) {
		return split_iterator<Char>(str, delim);
	}

	inline split_iterator<char> Split(std::basic_string_view<char> str, char delim) {
		return split_iterator<char>(str, delim);
	}

	template<typename Cont, typename Char>
	void Split(Cont& out, std::basic_string_view<Char> str, Char delim) {
		out.clear();
		for (auto const& tok : Split(str, delim))
			out.emplace_back(begin(tok), end(tok));
	}

	template<typename Cont>
	void Split(Cont& out, std::basic_string_view<char> str, char delim) {
		Split<Cont, char>(out, str, delim);
	}

	inline std::string_view Trim(std::string_view str) {
		std::locale loc;
		while (str.size() && std::isspace(str.front(), loc))
			str.remove_prefix(1);
		while (str.size() && std::isspace(str.back(), loc))
			str.remove_suffix(1);
		return str;
	}
}
