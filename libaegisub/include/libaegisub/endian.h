// Copyright (c) 2026 Aegisub Contributors
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
// Aegisub Project https://aegisub.org/

#pragma once

#include <bit>
#include <cstddef>
#include <span>
#include <utility>

namespace agi::endian {
	constexpr bool IsBigEndian = std::endian::native == std::endian::big;
	constexpr bool IsLittleEndian = std::endian::native == std::endian::little;

	inline void SwapBytesInPlace(std::span<std::byte> data) {
		for (size_t i = 0; i < data.size() / 2; ++i) {
			std::swap(data[i], data[data.size() - 1 - i]);
		}
	}
}
