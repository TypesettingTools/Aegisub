// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file audio_provider_hd.cpp
/// @brief Caching audio provider using a file for backing
/// @ingroup audio_input
///

#include "config.h"

#include "audio_provider_hd.h"

#include "audio_controller.h"
#include "compat.h"
#include "options.h"
#include "utils.h"

#include <libaegisub/access.h>
#include <libaegisub/background_runner.h>
#include <libaegisub/file_mapping.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/path.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace {
agi::fs::path cache_dir() {
	std::string path = OPT_GET("Audio/Cache/HD/Location")->GetString();
	if (path == "default")
		path = "?temp";

	return config::path->MakeAbsolute(config::path->Decode(path), "?temp");
}

agi::fs::path cache_path() {
	std::string pattern = OPT_GET("Audio/Cache/HD/Name")->GetString();
	if (!boost::contains(pattern, "%02i")) pattern = "audio%02i.tmp";
	boost::replace_all(pattern, "%02i", "%%%%-%%%%-%%%%-%%%%");
	return unique_path(cache_dir()/pattern);
}
}

HDAudioProvider::HDAudioProvider(std::unique_ptr<AudioProvider> src, agi::BackgroundRunner *br)
: AudioProviderWrapper(std::move(src))
, cache_filename(cache_path())
{
	// Check free space
	if ((uint64_t)num_samples * channels * bytes_per_sample > agi::fs::FreeSpace(cache_dir()))
		throw agi::AudioCacheOpenError("Not enough free disk space in " + cache_dir().string() + " to cache the audio", nullptr);

	try {
		{
			agi::io::Save out(cache_filename, true);
			br->Run(bind(&HDAudioProvider::FillCache, this, source.get(), &out.Get(), std::placeholders::_1));
		}
		file = agi::util::make_unique<agi::read_file_mapping>(cache_filename);
	}
	catch (...) {
		agi::fs::Remove(cache_filename);
		throw;
	}
}

HDAudioProvider::~HDAudioProvider() {
	file.reset(); // explicitly close the file so we can delete it
	agi::fs::Remove(cache_filename);
}

void HDAudioProvider::FillBuffer(void *buf, int64_t start, int64_t count) const {
	start *= channels * bytes_per_sample;
	count *= channels * bytes_per_sample;
	memcpy(buf, file->read(start, count), count);
}

void HDAudioProvider::FillCache(AudioProvider *src, std::ofstream *out, agi::ProgressSink *ps) {
	ps->SetMessage(from_wx(_("Reading to Hard Disk cache")));

	int64_t block = 65536;
	std::vector<char> read_buf;
	read_buf.resize(block * channels * bytes_per_sample);

	for (int64_t i = 0; i < num_samples; i += block) {
		block = std::min(block, num_samples - i);
		src->GetAudio(&read_buf[0], i, block);
		out->write(&read_buf[0], block * channels * bytes_per_sample);
		ps->SetProgress(i, num_samples);

		if (ps->IsCancelled()) return;
	}
}
