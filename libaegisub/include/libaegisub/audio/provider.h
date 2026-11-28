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

#pragma once

#include <libaegisub/exception.h>
#include <libaegisub/fs.h>

#include <atomic>
#include <memory>
#include <new>
#include <vector>

namespace agi {
class AudioProvider {
protected:
	int channels = 0;
	/// Total number of samples per channel
	int64_t num_samples = 0;
	/// Total samples per channel available so far, including disjoint cached ranges
	std::atomic<int64_t> decoded_samples{0};
	int sample_rate = 0;
	int bytes_per_sample = 0;
	bool float_samples = false;

	virtual void FillBuffer(void *buf, int64_t start, int64_t count) const = 0;

	void ZeroFill(void *buf, int64_t count) const;

public:
	virtual ~AudioProvider() = default;

	void GetAudio(void *buf, int64_t start, int64_t count) const;
	/// Read audio, logging decoding failures and filling the buffer with silence.
	void GetAudio(void *buf, int64_t start, int64_t count, std::nothrow_t) const;
	void GetAudioWithVolume(void *buf, int64_t start, int64_t count, double volume) const;

	int64_t GetNumSamples()     const { return num_samples; }
	int64_t GetDecodedSamples() const { return decoded_samples; }
	int     GetSampleRate()     const { return sample_rate; }
	int     GetBytesPerSample() const { return bytes_per_sample; }
	int     GetChannels()       const { return channels; }
	bool    AreSamplesFloat()   const { return float_samples; }

	/// Are all in-bounds samples in [start, end) available?
	virtual bool IsAudioReady(int64_t, int64_t) const { return true; }
	/// Prioritize filling the first gap at or after the given sample.
	virtual void RequestAudio(int64_t) const { }
	/// Is a cache worker still filling samples?
	virtual bool IsAudioCacheFilling() const { return false; }
	/// Restore decoder state for sequential reads from sample zero.
	virtual void ResetAudioDecoder() { }

	/// Does this provider benefit from external caching?
	virtual bool NeedsCache() const { return false; }
	/// Can the cache fill disjoint ranges using random-access reads?
	virtual bool SupportsCacheSeeking() const { return true; }
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

	bool SupportsCacheSeeking() const override { return source->SupportsCacheSeeking(); }
	void ResetAudioDecoder() override { source->ResetAudioDecoder(); }
};

DEFINE_EXCEPTION(AudioProviderError, Exception);

/// Error of some sort occurred while decoding a frame
DEFINE_EXCEPTION(AudioDecodeError, AudioProviderError);

/// This provider could not find any audio data in the file
DEFINE_EXCEPTION(AudioDataNotFound, AudioProviderError);

class BackgroundRunner;

std::unique_ptr<AudioProvider> CreateDummyAudioProvider(agi::fs::path const& filename, BackgroundRunner *);
std::unique_ptr<AudioProvider> CreatePCMAudioProvider(agi::fs::path const& filename, BackgroundRunner *);

std::unique_ptr<AudioProvider> CreateConvertAudioProvider(std::unique_ptr<AudioProvider> source_provider);
std::unique_ptr<AudioProvider> CreateLockAudioProvider(std::unique_ptr<AudioProvider> source_provider);
std::unique_ptr<AudioProvider> CreateHDAudioProvider(std::unique_ptr<AudioProvider> source_provider,
                                                     agi::fs::path const& dir);
std::unique_ptr<AudioProvider> CreateRAMAudioProvider(std::unique_ptr<AudioProvider> source_provider);

void SaveAudioClip(AudioProvider const& provider, agi::fs::path const& path, int start_time, int end_time);
}
