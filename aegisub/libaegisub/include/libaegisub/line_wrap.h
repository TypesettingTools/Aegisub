// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file line_wrap.h
/// @brief Generic paragraph formatting logic

#include <algorithm>
#include <climits>
#include <numeric>
#include <vector>

namespace agi {
	enum WrapMode {
		/// Semi-balanced, with the first line guaranteed to be longest if possible
		Wrap_Balanced_FirstLonger = 0,
		/// Simple greedy matching with breaks as late as possible
		Wrap_Greedy = 1,
		/// No line breaking at all
		Wrap_None = 2,
		/// Semi-balanced, with the last line guaranteed to be longest if possible
		Wrap_Balanced_LastLonger = 3,
		/// Balanced, with lines as close to equal in length as possible
		Wrap_Balanced = 4
	};

	namespace line_wrap_detail {
		template<class Width>
		Width waste(Width width, Width desired) {
			return (width - desired) * (width - desired);
		}

		template<class StartCont, class Iter, class WidthCont>
		inline void get_line_widths(StartCont const& line_start_points, Iter begin, Iter end, WidthCont &line_widths) {
			size_t line_start = 0;
			for (auto & line_start_point : line_start_points) {
				line_widths.push_back(std::accumulate(begin + line_start, begin + line_start_point, 0));
				line_start = line_start_point;
			}
			line_widths.push_back(std::accumulate(begin + line_start, end, 0));
		}

		// For first-longer and last-longer, bubble words forward/backwards when
		// possible and needed to make the first/last lines longer
		//
		// This is done rather than just using VSFilter's simpler greedy
		// algorithm due to that VSFilter's algorithm is incorrect; it can
		// produce incorrectly unbalanced lines and even excess line breaks
		template<class StartCont, class WidthCont, class Width>
		void unbalance(StartCont &ret, WidthCont const& widths, Width max_width, WrapMode wrap_mode) {
			WidthCont line_widths;
			get_line_widths(ret, widths.begin(), widths.end(), line_widths);

			int from_offset = 0;
			int to_offset = 1;
			if (wrap_mode == agi::Wrap_Balanced_LastLonger)
				std::swap(from_offset, to_offset);

			for (size_t i = 0; i < ret.size(); ++i) {
				// shift words until they're unbalanced in the correct direction
				// or shifting a word would exceed the length limit
				while (line_widths[i + from_offset] < line_widths[i + to_offset]) {
					int shift_word_width = widths[ret[i]];
					if (line_widths[i + from_offset] + shift_word_width > max_width)
						break;

					line_widths[i + from_offset] += shift_word_width;
					line_widths[i + to_offset] -= shift_word_width;
					ret[i] += to_offset + -from_offset;
				}
			}
		}

		template<class StartCont, class WidthCont, class Width>
		void break_greedy(StartCont &ret, WidthCont const& widths, Width max_width) {
			// Simple greedy matching that just starts a new line every time the
			// max length is exceeded
			Width cur_line_width = 0;
			for (size_t i = 0; i < widths.size(); ++i) {
				if (cur_line_width > 0 && widths[i] + cur_line_width > max_width) {
					ret.push_back(i);
					cur_line_width = 0;
				}

				cur_line_width += widths[i];
			}
		}
	}

	/// Get the indices at which the blocks should be wrapped
	/// @tparam WidthCont A random-access container of Widths
	/// @tparam Width A numeric type which represents a width
	/// @param widths The widths of the objects to fit within the space
	/// @param max_width The available space for the objects
	/// @param wrap_mode WrapMode to use to decide where to insert breaks
	/// @return Indices into widths which breaks should be inserted before
	template<class WidthCont, class Width>
	std::vector<size_t> get_wrap_points(WidthCont const& widths, Width max_width, WrapMode wrap_mode) {
		using namespace line_wrap_detail;

		std::vector<size_t> ret;

		if (wrap_mode == Wrap_None || widths.size() < 2)
			return ret;

		// Check if any wrapping is actually needed
		Width total_width = std::accumulate(widths.begin(), widths.end(), 0);
		if (total_width <= max_width)
			return ret;


		if (wrap_mode == Wrap_Greedy) {
			break_greedy(ret, widths, max_width);
			return ret;
		}

		size_t num_words = distance(widths.begin(), widths.end());

		// the cost of the optimal arrangement of words [0..i]
		std::vector<Width> optimal_costs(num_words, INT_MAX);

		// the optimal start word for a line ending at i
		std::vector<size_t> line_starts(num_words, INT_MAX);

		// O(num_words * min(num_words, max_width))
		for (size_t end_word = 0; end_word < num_words; ++end_word) {
			Width current_line_width = 0;
			for (int start_word = end_word; start_word >= 0; --start_word) {
				current_line_width += widths[start_word];

				// Only evaluate lines over the limit if they're one word
				if (current_line_width > max_width && (size_t)start_word != end_word)
					break;

				Width cost = waste(current_line_width, max_width);

				if (start_word > 0)
					cost += optimal_costs[start_word - 1];

				if (cost < optimal_costs[end_word]) {
					optimal_costs[end_word] = cost;
					line_starts[end_word] = start_word;
				}
			}
		}

		// Select the optimal start word for each line ending with last_word
		for (size_t last_word = num_words; last_word > 0 && line_starts[last_word - 1] > 0; last_word = line_starts[last_word]) {
			--last_word;
			ret.push_back(line_starts[last_word]);
		}
		std::reverse(ret.begin(), ret.end());

		if (wrap_mode != Wrap_Balanced)
			unbalance(ret, widths, max_width, wrap_mode);

		return ret;
	}
}
