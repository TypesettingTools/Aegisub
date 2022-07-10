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

#include "libaegisub/make_unique.h"

#include <array>
#include <boost/container/stable_vector.hpp>
#include <thread>

namespace {
using namespace agi;

#define CacheBits 22
#define CacheBlockSize (1 << CacheBits)

class RAMAudioProvider final : public AudioProviderWrapper {
#ifdef _MSC_VER
	boost::container::stable_vector<char[CacheBlockSize]> blockcache;
#else
	boost::container::stable_vector<std::array<char, CacheBlockSize>> blockcache;
#endif
	std::atomic<bool> cancelled = { false };
	std::thread decoder;

	void FillBuffer(void* buf, int64_t start, int64_t count) const override;

  public:
	RAMAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		decoded_samples = 0;

		try {
			blockcache.resize(
			    (source->GetNumSamples() * source->GetBytesPerSample() + CacheBlockSize - 1) >>
			    CacheBits);
		} catch(std::bad_alloc const&) {
			throw AudioProviderError("Not enough memory available to cache in RAM");
		}

		decoder = std::thread([&] {
			int64_t readsize = CacheBlockSize / source->GetBytesPerSample();
			for(size_t i = 0; i < blockcache.size(); i++) {
				if(cancelled) break;
				auto actual_read = std::min<int64_t>(readsize, num_samples - i * readsize);
				source->GetAudio(&blockcache[i][0], i * readsize, actual_read);
				decoded_samples += actual_read;
			}
		});
	}

	~RAMAudioProvider() {
		cancelled = true;
		decoder.join();
	}
};

void RAMAudioProvider::FillBuffer(void* buf, int64_t start, int64_t count) const {
	auto charbuf = static_cast<char*>(buf);
	for(int64_t bytes_remaining = count * bytes_per_sample; bytes_remaining;) {
		if(start >= decoded_samples) {
			memset(charbuf, 0, bytes_remaining);
			break;
		}

		const int i = (start * bytes_per_sample) >> CacheBits;
		const int start_offset = (start * bytes_per_sample) & (CacheBlockSize - 1);
		const int read_size = std::min<int>(bytes_remaining, CacheBlockSize - start_offset);

		memcpy(charbuf, &blockcache[i][start_offset], read_size);
		charbuf += read_size;
		bytes_remaining -= read_size;
		start += read_size / bytes_per_sample;
	}
}
} // namespace

namespace agi {
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> src) {
	return agi::make_unique<RAMAudioProvider>(std::move(src));
}
} // namespace agi
