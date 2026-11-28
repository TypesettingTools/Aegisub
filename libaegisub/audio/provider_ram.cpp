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

#include "provider_cache.h"


#include <array>
#include <boost/container/stable_vector.hpp>
#include <cstring>

namespace {
using namespace agi;

constexpr int64_t CacheBlockSize = 4 * 1024 * 1024;

class RAMAudioProvider final : public CacheAudioProvider {
	// Fit whole decode chunks into each 4 MiB block.
	const int64_t block_samples = CacheBlockSize / sample_bytes / chunk_samples * chunk_samples;
#ifdef _MSC_VER
	boost::container::stable_vector<char[CacheBlockSize]> blockcache;
#else
	boost::container::stable_vector<std::array<char, CacheBlockSize>> blockcache;
#endif

	void *WriteCache(int64_t start, int64_t) override {
		return &blockcache[start / block_samples][(start % block_samples) * sample_bytes];
	}

	void ReadCache(void *buf, int64_t start, int64_t count) const override {
		auto const& block = blockcache[start / block_samples];
		std::memcpy(buf, &block[(start % block_samples) * sample_bytes], count * sample_bytes);
	}

public:
	RAMAudioProvider(std::unique_ptr<AudioProvider> src)
	: CacheAudioProvider(std::move(src))
	{
		if (block_samples == 0)
			throw AudioProviderError("Audio format does not fit in a RAM cache block");

		try {
			blockcache.resize((num_samples + block_samples - 1) / block_samples);
		}
		catch (std::bad_alloc const&) {
			throw AudioProviderError("Not enough memory available to cache in RAM");
		}
		StartDecoder();
	}

	~RAMAudioProvider() override {
		StopDecoder();
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> src) {
	return std::make_unique<RAMAudioProvider>(std::move(src));
}
}
