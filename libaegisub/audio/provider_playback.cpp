// Copyright (c) 2026, Matrew File
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

#include "libaegisub/audio/playback.h"
#include "libaegisub/audio/provider.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace {
using namespace agi;

class PlaybackAudioProvider final : public AudioProvider {
	AudioProvider *source;
	double rate;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		if (bytes_per_sample != sizeof(int16_t) || float_samples)
			throw InternalError("PlaybackAudioProvider requires 16-bit PCM input");

		auto out = static_cast<int16_t *>(buf);
		if (count <= 0) return;

		int64_t source_start = audio::SourceSamplesFromPlaybackSamplesFloor(start, rate);
		int64_t source_end = audio::SourceSamplesFromPlaybackSamplesCeil(start + count, rate) + 1;
		int64_t source_count = std::max<int64_t>(source_end - source_start, 2);

		std::vector<int16_t> source_buf(source_count * channels);
		source->GetAudio(source_buf.data(), source_start, source_count);

		for (int64_t frame = 0; frame < count; ++frame) {
			double source_pos = audio::SourceSamplesFromPlaybackSamplesExact(start + frame, rate) - source_start;
			auto source_index = static_cast<int64_t>(std::floor(source_pos));
			double frac = source_pos - source_index;
			auto next_index = std::min(source_index + 1, source_count - 1);

			for (int channel = 0; channel < channels; ++channel) {
				double left = source_buf[source_index * channels + channel];
				double right = source_buf[next_index * channels + channel];
				out[frame * channels + channel] = static_cast<int16_t>(std::lround(left + (right - left) * frac));
			}
		}
	}

public:
	PlaybackAudioProvider(AudioProvider *source_provider, double playback_rate)
	: source(source_provider)
	, rate(audio::ClampPlaybackRate(playback_rate)) {
		if (!source)
			throw InternalError("PlaybackAudioProvider requires a source provider");
		if (source->GetBytesPerSample() != sizeof(int16_t) || source->AreSamplesFloat())
			throw AudioProviderError("PlaybackAudioProvider requires 16-bit PCM input");

		channels = source->GetChannels();
		num_samples = audio::PlaybackSamplesFromSourceSamplesCeil(source->GetNumSamples(), rate);
		decoded_samples = audio::PlaybackSamplesFromSourceSamplesCeil(source->GetDecodedSamples(), rate);
		sample_rate = source->GetSampleRate();
		bytes_per_sample = source->GetBytesPerSample();
		float_samples = false;
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreatePlaybackAudioProvider(AudioProvider *source_provider, double rate) {
	return std::make_unique<PlaybackAudioProvider>(source_provider, rate);
}
}
