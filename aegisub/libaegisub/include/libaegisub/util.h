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

#include <algorithm>
#include <cstdint>
#include <string>

#include <libaegisub/types.h>

struct tm;

namespace agi {
	namespace util {
	/// Clamp `b` to the range [`a`,`c`]
	template<typename T> inline T mid(T a, T b, T c) { return std::max(a, std::min(b, c)); }

	/// Get time suitable for logging mechanisms.
	/// @param tv timeval
	void time_log(agi_timeval &tv);

	bool try_parse(std::string const& str, double *out);
	bool try_parse(std::string const& str, int *out);

	/// strftime, but on std::string rather than a fixed buffer
	/// @param fmt strftime format string
	/// @param tmptr Time to format, or nullptr for current time
	/// @return The strftime-formatted string
	std::string strftime(const char *fmt, const tm *tmptr = nullptr);

	struct delete_ptr {
		template<class T>
		void operator()(T* ptr) const {
			delete ptr;
		}
	};
	template<class T>
	void delete_clear(T& container) {
		if (!container.empty()) {
			std::for_each(container.begin(), container.end(), delete_ptr());
			container.clear();
		}
	}

	} // namespace util
} // namespace agi
