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

#include "config.h"

#include "include/aegisub/audio_provider.h"

#include "audio_controller.h"
#include "compat.h"
#include "options.h"

#include <libaegisub/background_runner.h>
#include <libaegisub/file_mapping.h>
#include <libaegisub/fs.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <wx/intl.h>

namespace {
class HDAudioProvider final : public AudioProviderWrapper {
	std::unique_ptr<agi::temp_file_mapping> file;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		start *= channels * bytes_per_sample;
		count *= channels * bytes_per_sample;
		memcpy(buf, file->read(start, count), count);
	}

public:
	HDAudioProvider(std::unique_ptr<AudioProvider> src, agi::BackgroundRunner *br)
	: AudioProviderWrapper(std::move(src))
	{
		auto path = OPT_GET("Audio/Cache/HD/Location")->GetString();
		if (path == "default")
			path = "?temp";
		auto cache_dir = config::path->MakeAbsolute(config::path->Decode(path), "?temp");

		auto bps = bytes_per_sample * channels;

		// Check free space
		if ((uint64_t)num_samples * bps > agi::fs::FreeSpace(cache_dir))
			throw agi::AudioCacheOpenError("Not enough free disk space in " + cache_dir.string() + " to cache the audio", nullptr);

		auto filename = str(boost::format("audio-%lld-%lld")
			% (long long)time(nullptr)
			% (long long)boost::interprocess::ipcdetail::get_current_process_id());

		file = agi::util::make_unique<agi::temp_file_mapping>(cache_dir / filename, num_samples * bps);
		br->Run([&] (agi::ProgressSink *ps) {
			ps->SetTitle(from_wx(_("Load audio")));
			ps->SetMessage(from_wx(_("Reading to Hard Disk cache")));

			int64_t block = 65536;
			for (int64_t i = 0; i < num_samples; i += block) {
				block = std::min(block, num_samples - i);
				source->GetAudio(file->write(i * bps, block * bps), i, block);
				ps->SetProgress(i, num_samples);
				if (ps->IsCancelled()) return;
			}
		});
	}
};
}

std::unique_ptr<AudioProvider> CreateHDAudioProvider(std::unique_ptr<AudioProvider> src, agi::BackgroundRunner *br) {
	return agi::util::make_unique<HDAudioProvider>(std::move(src), br);
}
