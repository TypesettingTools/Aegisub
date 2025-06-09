// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

#include <algorithm>
#include <boost/range/irange.hpp>
#include <string>
#include <string_view>
#include <vector>

struct tm;

namespace agi::util {
	/// Clamp `b` to the range [`a`,`c`]
	template<typename T>
	static inline T mid(T a, T b, T c) {
		return std::max(a, std::min(b, c));
	}

	bool try_parse(std::string_view str, double *out);
	bool try_parse(std::string_view str, int *out);

	/// strftime, but on std::string rather than a fixed buffer
	/// @param fmt strftime format string
	/// @param tmptr Time to format, or nullptr for current time
	/// @return The strftime-formatted string
	std::string strftime(const char *fmt, const tm *tmptr = nullptr);

	/// Case-insensitive find with proper case folding
	/// @param haystack String to search
	/// @param needle String to look for
	/// @return make_pair(-1,-1) if `needle` could not be found, or a range equivalent to `needle` in `haystack` if it could
	///
	/// `needle` and `haystack` must both be in Normalization Form D. The size
	/// of the match might be different from the size of `needle`, since it's
	/// based on the unfolded length.
	std::pair<size_t, size_t> ifind(std::string const& haystack, std::string const& needle);

	class tagless_find_helper {
		std::vector<std::pair<size_t, size_t>> blocks;
		size_t start = 0;

	public:
		/// Strip ASS override tags at or after `start` in `str`, and initialize
		/// state for mapping ranges back to the input string
		std::string strip_tags(std::string const& str, size_t start);

		/// Convert a range in the string returned by `strip_tags()` to a range
		/// int the string last passed to `strip_tags()`
		void map_range(size_t& start, size_t& end);
	};

	/// Set the name of the calling thread in the Visual Studio debugger
	/// @param name New name for the thread
	void SetThreadName(const char *name);

	/// A thin wrapper around this_thread::sleep_for that uses std::thread on
	/// Windows (to avoid having to compile boost.thread) and boost::thread
	/// elsewhere (because libstdc++ 4.7 is missing it).
	void sleep_for(int ms);

	// boost.range doesn't have wrappers for the C++11 stuff
	template<typename Range, typename Predicate>
	bool any_of(Range&& r, Predicate&& p) {
		return std::any_of(std::begin(r), std::end(r), std::forward<Predicate>(p));
	}

	std::string ErrorString(int error);
	void InitLocale();

	template<typename Integer>
	auto range(Integer end) -> decltype(boost::irange<Integer>(0, end)) {
		return boost::irange<Integer>(0, end);
	}
} // namespace agi::util
