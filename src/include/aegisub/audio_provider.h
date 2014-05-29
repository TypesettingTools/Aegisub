// Copyright (c) 2006, Rodrigo Braz Monteiro
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

#pragma once

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>

#include <atomic>
#include <vector>

class AudioProvider {
protected:
	int channels;

	/// for one channel, ie. number of PCM frames
	int64_t num_samples;
	std::atomic<int64_t> decoded_samples;
	int sample_rate;
	int bytes_per_sample;
	bool float_samples;

	virtual void FillBuffer(void *buf, int64_t start, int64_t count) const = 0;

	void ZeroFill(void *buf, int64_t count) const;

public:
	virtual ~AudioProvider() = default;

	void GetAudio(void *buf, int64_t start, int64_t count) const;
	void GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const;

	int64_t       GetNumSamples()     const { return num_samples; }
	int64_t       GetDecodedSamples() const { return decoded_samples; }
	int           GetSampleRate()     const { return sample_rate; }
	int           GetBytesPerSample() const { return bytes_per_sample; }
	int           GetChannels()       const { return channels; }
	bool          AreSamplesFloat()   const { return float_samples; }

	/// @brief Does this provider benefit from external caching?
	virtual bool NeedsCache() const { return false; }
};

/// Helper base class for an audio provider which wraps another provider
class AudioProviderWrapper : public AudioProvider {
protected:
	std::unique_ptr<AudioProvider> source;
public:
	AudioProviderWrapper(std::unique_ptr<AudioProvider> src)
	: source(std::move(src))
	{
		channels = source->GetChannels();
		num_samples = source->GetNumSamples();
		decoded_samples = source->GetDecodedSamples();
		sample_rate = source->GetSampleRate();
		bytes_per_sample = source->GetBytesPerSample();
		float_samples = source->AreSamplesFloat();
	}
};

namespace agi { class BackgroundRunner; }

struct AudioProviderFactory {
	static std::vector<std::string> GetClasses();

	/// Get a provider for the file
	/// @param filename URI to open
	static std::unique_ptr<AudioProvider> GetProvider(agi::fs::path const& filename, agi::BackgroundRunner *br);
};

DEFINE_EXCEPTION(AudioProviderError, agi::Exception);
/// Error of some sort occurred while decoding a frame
DEFINE_EXCEPTION(AudioDecodeError, AudioProviderError);
