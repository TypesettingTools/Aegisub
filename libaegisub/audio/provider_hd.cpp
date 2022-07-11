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

#include <libaegisub/file_mapping.h>
#include <libaegisub/format.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/make_unique.h>

#include <boost/filesystem/path.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <ctime>
#include <thread>

namespace {
using namespace agi;

class HDAudioProvider final : public AudioProviderWrapper {
	mutable temp_file_mapping file;
	std::atomic<bool> cancelled = {false};
	std::thread decoder;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto missing = std::min(count, start + count - decoded_samples);
		if (missing > 0) {
			memset(static_cast<int16_t*>(buf) + count - missing, 0, missing * bytes_per_sample);
			count -= missing;
		}

		if (count > 0) {
			start *= bytes_per_sample;
			count *= bytes_per_sample;
			memcpy(buf, file.read(start, count), count);
		}
	}

	fs::path CacheFilename(fs::path const& dir) {
		// Check free space
		if ((uint64_t)num_samples * bytes_per_sample > fs::FreeSpace(dir))
			throw AudioProviderError("Not enough free disk space in " + dir.string() + " to cache the audio");

		return format("audio-%lld-%lld", time(nullptr),
		              boost::interprocess::ipcdetail::get_current_process_id());
	}

public:
	HDAudioProvider(std::unique_ptr<AudioProvider> src, agi::fs::path const& dir)
	: AudioProviderWrapper(std::move(src))
	, file(dir / CacheFilename(dir), num_samples * bytes_per_sample)
	{
		decoded_samples = 0;
		decoder = std::thread([&] {
			int64_t block = 65536;
			for (int64_t i = 0; i < num_samples; i += block) {
				if (cancelled) break;
				block = std::min(block, num_samples - i);
				source->GetAudio(file.write(i * bytes_per_sample, block * bytes_per_sample), i, block);
				decoded_samples += block;
			}
		});
	}

	~HDAudioProvider() {
		cancelled = true;
		decoder.join();
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreateHDAudioProvider(std::unique_ptr<AudioProvider> src, agi::fs::path const& dir) {
	return agi::make_unique<HDAudioProvider>(std::move(src), dir);
}
}
