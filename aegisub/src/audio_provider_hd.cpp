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
//
// $Id$

/// @file audio_provider_hd.cpp
/// @brief Caching audio provider using a file for backing
/// @ingroup audio_input
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/filefn.h>
#include <wx/filename.h>
#endif

#include <libaegisub/background_runner.h>
#include <libaegisub/io.h>

#include "audio_provider_hd.h"

#include "audio_controller.h"
#include "audio_provider_pcm.h"
#include "compat.h"
#include "main.h"
#include "standard_paths.h"
#include "utils.h"

namespace {
wxString cache_dir() {
	wxString path = lagi_wxString(OPT_GET("Audio/Cache/HD/Location")->GetString());
	if (path == "default")
		path = "?temp/";

	return DecodeRelativePath(StandardPaths::DecodePath(path), StandardPaths::DecodePath("?user/"));
}

wxString cache_path() {
	wxString pattern = lagi_wxString(OPT_GET("Audio/Cache/HD/Name")->GetString());
	if (pattern.Find("%02i") == wxNOT_FOUND) pattern = "audio%02i.tmp";

	// Try from 00 to 99
	for (int i=0;i<100;i++) {
		// File exists?
		wxFileName curNameTry(cache_dir(), wxString::Format(pattern, i));
		if (!curNameTry.Exists())
			return curNameTry.GetFullPath();
	}
	return "";
}

/// A PCM audio provider for raw dumps with no header
class RawAudioProvider : public PCMAudioProvider {
public:
	RawAudioProvider(wxString const& cache_filename, AudioProvider *src)
	: PCMAudioProvider(cache_filename)
	{
		bytes_per_sample = src->GetBytesPerSample();
		num_samples      = src->GetNumSamples();
		channels         = src->GetChannels();
		sample_rate      = src->GetSampleRate();
		filename         = src->GetFilename();
		float_samples    = src->AreSamplesFloat();

		IndexPoint p = { 0, 0, num_samples };
		index_points.push_back(p);
	}

	bool AreSamplesNativeEndian() const { return true; }
};

}

HDAudioProvider::HDAudioProvider(AudioProvider *src, agi::BackgroundRunner *br) {
	agi::scoped_ptr<AudioProvider> source(src);
	assert(src->AreSamplesNativeEndian()); // Byteswapping should be done before caching

	bytes_per_sample = source->GetBytesPerSample();
	num_samples      = source->GetNumSamples();
	channels         = source->GetChannels();
	sample_rate      = source->GetSampleRate();
	filename         = source->GetFilename();
	float_samples    = source->AreSamplesFloat();

	// Check free space
	wxDiskspaceSize_t freespace;
	if (wxGetDiskSpace(cache_dir(), 0, &freespace)) {
		if (num_samples * channels * bytes_per_sample > freespace)
			throw agi::AudioCacheOpenError("Not enough free disk space in " + STD_STR(cache_dir()) + " to cache the audio", 0);
	}

	diskCacheFilename = cache_path();

	try {
		{
			agi::io::Save out(STD_STR(diskCacheFilename), true);
			br->Run(bind(&HDAudioProvider::FillCache, this, src, &out.Get(), std::tr1::placeholders::_1));
		}
		cache_provider.reset(new RawAudioProvider(diskCacheFilename, src));
	}
	catch (...) {
		wxRemoveFile(diskCacheFilename);
		throw;
	}
}

HDAudioProvider::~HDAudioProvider() {
	cache_provider.reset(); // explicitly close the file so we can delete it
	wxRemoveFile(diskCacheFilename);
}

void HDAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	cache_provider->GetAudio(buf, start, count);
}

void HDAudioProvider::FillCache(AudioProvider *src, std::ofstream *out, agi::ProgressSink *ps) {
	ps->SetMessage(STD_STR(_("Reading to Hard Disk cache")));

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
