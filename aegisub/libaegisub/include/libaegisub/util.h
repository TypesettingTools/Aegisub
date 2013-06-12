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

/// @file util.h
/// @brief Public interface for general utilities.
/// @ingroup libaegisub

#include <libaegisub/time.h>

#include <algorithm>
#include <cstdint>
#include <string>

struct tm;

namespace agi {
	namespace util {
	/// Clamp `b` to the range [`a`,`c`]
	template<typename T> inline T mid(T a, T b, T c) { return std::max(a, std::min(b, c)); }

	/// Get time suitable for logging mechanisms.
	agi_timeval time_log();

	bool try_parse(std::string const& str, double *out);
	bool try_parse(std::string const& str, int *out);

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

	/// Set the name of the calling thread in the Visual Studio debugger
	/// @param name New name for the thread
	void SetThreadName(const char *name);

#ifdef _MSC_VER
#define MAKE_UNIQUE(TEMPLATE_LIST, PADDING_LIST, LIST, COMMA, X1, X2, X3, X4) \
	template<class T COMMA LIST(_CLASS_TYPE)> \
	inline std::unique_ptr<T> make_unique(LIST(_TYPE_REFREF_ARG)) { \
		return std::unique_ptr<T>(new T(LIST(_FORWARD_ARG))); \
	}
	_VARIADIC_EXPAND_0X(MAKE_UNIQUE, , , , )
#undef MAKE_UNIQUE
#else
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#endif

	} // namespace util
} // namespace agi
