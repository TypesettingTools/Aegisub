// Copyright (c) 2022, Thomas Goyne <plorkyeran@aegisub.org>
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
namespace detail {
inline void AppendStr(std::string& target, std::initializer_list<std::string_view> strs) {
	size_t size = 0;
	for (auto str : strs)
		size += str.size();
	target.reserve(target.size() + size);
	for (auto str : strs)
		target.append(str);
}
}

template<typename Range>
void AppendJoin(std::string& str, std::string_view sep, Range const& range)
{
	auto begin = std::begin(range);
	auto end = std::end(range);
	if (begin == end) return;

	str += *begin++;
	while (begin != end) {
		str += sep;
		str += *begin++;
	}
}

template<typename Range>
std::string Join(std::string_view sep, Range const& range) {
	std::string str;
	AppendJoin(str, sep, range);
	return str;
}

template<typename... Args>
std::string Str(Args&&... args) {
	std::string str;
	detail::AppendStr(str, {args...});
	return str;
}

template<typename... Args>
void AppendStr(std::string& str, Args&&... args) {
	detail::AppendStr(str, {args...});
}
} // namespace agi
