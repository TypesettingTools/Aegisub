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

#include "libaegisub/audio/provider.h"


#include <algorithm>
#include <array>
#include <boost/container/stable_vector.hpp>
#include <thread>

namespace {
using namespace agi;

constexpr int64_t CacheBlockSize = 4 * 1024 * 1024;
constexpr int64_t chunk_samples = 65536;

class RAMAudioProvider final : public AudioProviderWrapper {
	// Fit whole decode chunks into each 4 MiB block.
	const int64_t block_samples = CacheBlockSize / bytes_per_sample / chunk_samples * chunk_samples;
#ifdef _MSC_VER
	boost::container::stable_vector<char[CacheBlockSize]> blockcache;
#else
	boost::container::stable_vector<std::array<char, CacheBlockSize>> blockcache;
#endif
	std::atomic<bool> cancelled = {false};
	std::thread decoder;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override;

public:
	RAMAudioProvider(std::unique_ptr<AudioProvider> src)
	: AudioProviderWrapper(std::move(src))
	{
		decoded_samples = 0;

		if (block_samples == 0)
			throw AudioProviderError("Audio format does not fit in a RAM cache block");

		try {
			blockcache.resize((num_samples + block_samples - 1) / block_samples);
		}
		catch (std::bad_alloc const&) {
			throw AudioProviderError("Not enough memory available to cache in RAM");
		}

		decoder = std::thread([&] {
			for (int64_t i = 0; i < num_samples; i += chunk_samples) {
				if (cancelled) break;
				auto actual_read = std::min(chunk_samples, num_samples - i);
				source->GetAudio(&blockcache[i / block_samples][(i % block_samples) * bytes_per_sample], i, actual_read);
				decoded_samples += actual_read;
			}
		});
	}

	~RAMAudioProvider() override {
		cancelled = true;
		decoder.join();
	}
};

void RAMAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	auto charbuf = static_cast<char *>(buf);
	for (int64_t bytes_remaining = count * bytes_per_sample; bytes_remaining; ) {
		const int64_t decoded_bytes = (decoded_samples - start) * bytes_per_sample;
		if (decoded_bytes <= 0) {
			memset(charbuf, 0, bytes_remaining);
			break;
		}

		const int64_t i = start / block_samples;
		const int64_t start_offset = (start % block_samples) * bytes_per_sample;
		const int64_t read_size = std::min({bytes_remaining, decoded_bytes, block_samples * bytes_per_sample - start_offset});

		memcpy(charbuf, &blockcache[i][start_offset], read_size);
		charbuf += read_size;
		bytes_remaining -= read_size;
		start += read_size / bytes_per_sample;
	}
}
}

namespace agi {
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> src) {
	return std::make_unique<RAMAudioProvider>(std::move(src));
}
}
