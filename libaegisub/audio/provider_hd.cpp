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

#include <libaegisub/file_mapping.h>
#include <libaegisub/format.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>

#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <ctime>
#include <mutex>

namespace {
using namespace agi;

class HDAudioProvider final : public CacheAudioProvider {
	mutable temp_file_mapping file;
	mutable std::mutex read_mutex;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto missing = std::min(count, start + count - decoded_samples);
		if (missing > 0) {
			count -= missing;
			ZeroFill(static_cast<char *>(buf) + count * sample_bytes, missing);
		}

		if (count > 0) {
			start *= sample_bytes;
			count *= sample_bytes;
			// The read mapping can be remapped on 32-bit systems.
			std::lock_guard lock(read_mutex);
			memcpy(buf, file.read(start, count).data(), count);
		}
	}

	void *WriteCache(int64_t start, int64_t count) override {
		return file.write(start * sample_bytes, count * sample_bytes);
	}

  fs::path CacheFilename(fs::path const& dir) {
		// Check free space
		if ((uint64_t)num_samples * sample_bytes > fs::FreeSpace(dir))
			throw AudioProviderError("Not enough free disk space in " + dir.string() + " to cache the audio");

		return format("audio-%lld-%lld", time(nullptr),
		              boost::interprocess::ipcdetail::get_current_process_id());
	}

public:
	HDAudioProvider(std::unique_ptr<AudioProvider> src, fs::path const& dir)
	: CacheAudioProvider(std::move(src))
	, file(dir / CacheFilename(dir), num_samples * sample_bytes)
	{
		StartDecoder();
	}

	~HDAudioProvider() override {
		StopDecoder();
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreateHDAudioProvider(std::unique_ptr<AudioProvider> src, agi::fs::path const& dir) {
	return std::make_unique<HDAudioProvider>(std::move(src), dir);
}
}
