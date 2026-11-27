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


#include <array>
#include <boost/container/stable_vector.hpp>
#include <thread>

namespace {
using namespace agi;

#define CacheBits 22
#define CacheBlockSize (1 << CacheBits)

class RAMAudioProvider final : public AudioProviderWrapper<> {
#ifdef _MSC_VER
	boost::container::stable_vector<char[CacheBlockSize]> blockcache;
#else
	boost::container::stable_vector<std::array<char, CacheBlockSize>> blockcache;
#endif
	std::atomic<bool> cancelled = {false};
	std::thread decoder;

	// If the number of channels is not a power of two, sample frames may not cleanly fit into blocks
	int64_t sample_frames_per_block = 0;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override;

public:
	RAMAudioProvider(std::unique_ptr<AudioProvider> src)
	: AudioProviderWrapper(std::move(src))
	, sample_frames_per_block(CacheBlockSize / source->GetChannels() / source->GetBytesPerSample())
	{
		decoded_sample_frames = 0;

		try {
			blockcache.resize((source->GetNumSampleFrames() + sample_frames_per_block - 1) / sample_frames_per_block);
		}
		catch (std::bad_alloc const&) {
			throw AudioProviderError("Not enough memory available to cache in RAM");
		}

		decoder = std::thread([&] {
			for (size_t i = 0; i < blockcache.size(); i++) {
				if (cancelled) break;
				auto actual_read = std::min<int64_t>(sample_frames_per_block, num_sample_frames - i * sample_frames_per_block);
				source->GetAudio(&blockcache[i][0], i * sample_frames_per_block, actual_read);
				decoded_sample_frames += actual_read;
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
	for (int64_t sample_frames_remaining = count; sample_frames_remaining; ) {
		if (start >= decoded_sample_frames) {
			memset(charbuf, 0, sample_frames_remaining * channels * bytes_per_sample);
			break;
		}

		const int64_t i = start / sample_frames_per_block;
		const int64_t start_offset = start % sample_frames_per_block;
		const int64_t read_count = std::min(sample_frames_remaining, sample_frames_per_block - start_offset);
		const int64_t read_size = read_count * channels * bytes_per_sample;

		memcpy(charbuf, &blockcache[i][start_offset * channels * bytes_per_sample], read_size);
		charbuf += read_size;
		sample_frames_remaining -= read_count;
		start += read_count;
	}
}
}

namespace agi {
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> src) {
	return std::make_unique<RAMAudioProvider>(std::move(src));
}
}
