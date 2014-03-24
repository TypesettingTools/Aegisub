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

#include <string>
#include <vector>

namespace {
template<typename Container>
std::vector<std::string> GetClasses(Container const& c) {
	std::vector<std::string> list;
	for (auto const& provider : c) {
		if (!provider.hidden)
			list.push_back(provider.name);
	}
	return list;
}

template<typename Container>
auto GetSorted(Container const& c, std::string const& preferred) -> std::vector<decltype(&*c.begin())> {
	std::vector<decltype(&*c.begin())> sorted;
	sorted.reserve(std::distance(c.begin(), c.end()));
	size_t end_of_hidden = 0;
	bool any_hidden = false;
	for (auto const& provider : c) {
		if (provider.hidden) {
			sorted.push_back(&provider);
			any_hidden = true;
		}
		else if (any_hidden && end_of_hidden == 0) {
			end_of_hidden = sorted.size();
			sorted.push_back(&provider);
		}
		else if (preferred == provider.name)
			sorted.insert(sorted.begin() + end_of_hidden, &provider);
		else
			sorted.push_back(&provider);
	}
	return sorted;
}
}
