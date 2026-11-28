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

#include <libaegisub/endian.h>
#include <libaegisub/log.h>

#include <algorithm>
#include <array>
#include <limits>
#include <numbers>
#include <numeric>
#include <ranges>
#include <span>
#include <string_view>

using namespace agi;

/// Anything integral -> 16 bit signed machine-endian audio converter
namespace {
int64_t AssembleSample(std::span<uint8_t> data) {
	const auto accumulate_bytes = [](auto view) {
		return std::accumulate(view.begin(), view.end(), int64_t{0},
			[](int64_t acc, uint8_t value) {
				return (acc << 8) + value;
			});
	};

	if (endian::IsBigEndian)
		return accumulate_bytes(data);

	return accumulate_bytes(data | std::views::reverse);
}

template<class Target>
class BitdepthConvertAudioProvider final : public AudioProviderWrapper {
	int src_bytes_per_sample;
	mutable std::vector<uint8_t> src_buf;

public:
	BitdepthConvertAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		if (bytes_per_sample > 8)
			throw AudioProviderError("Audio format converter: audio with bitdepths greater than 64 bits/sample is currently unsupported");

		src_bytes_per_sample = bytes_per_sample;
		bytes_per_sample = sizeof(Target);
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		src_buf.resize(count * src_bytes_per_sample * channels);
		source->GetAudio(src_buf.data(), start, count);

		auto dest = static_cast<int16_t*>(buf);

		for (int64_t i = 0; i < count * channels; ++i) {
			int64_t sample = 0;

			// 8 bits per sample is assumed to be unsigned with a bias of 127,
			// while everything else is assumed to be signed with zero bias
			if (src_bytes_per_sample == 1)
				sample = src_buf[i] - 128;
			else {
				sample = AssembleSample(std::span(src_buf.data() + i * src_bytes_per_sample, src_bytes_per_sample));
			}

			if (src_bytes_per_sample > static_cast<int>(sizeof(Target)))
				sample /= 1LL << (src_bytes_per_sample - sizeof(Target)) * 8;
			else if (src_bytes_per_sample < static_cast<int>(sizeof(Target)))
				sample *=  1LL << (sizeof(Target) - src_bytes_per_sample ) * 8;

			dest[i] = static_cast<Target>(sample);
		}
	}
};

/// Floating point -> 16 bit signed machine-endian audio converter
template<class Source, class Target>
class FloatConvertAudioProvider final : public AudioProviderWrapper {
	mutable std::vector<Source> src_buf;

public:
	FloatConvertAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		bytes_per_sample = sizeof(Target);
		float_samples = false;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		src_buf.resize(count * channels);
		source->GetAudio(&src_buf[0], start, count);

		auto dest = static_cast<Target*>(buf);

		for (int i = 0; i < count * channels; ++i) {
			Source expanded;
			if (src_buf[i] < 0)
				expanded = static_cast<Target>(-src_buf[i] * std::numeric_limits<Target>::min() - 0.5);
			else
				expanded = static_cast<Target>(src_buf[i] * std::numeric_limits<Target>::max() + 0.5);

			dest[i] = expanded < std::numeric_limits<Target>::min() ? std::numeric_limits<Target>::min() :
			          expanded > std::numeric_limits<Target>::max() ? std::numeric_limits<Target>::max() :
			                                                          static_cast<Target>(expanded);
		}
	}
};

/// Downmix interleaved 16-bit surround audio when the decoder cannot do it.
class DownmixAudioProvider final : public AudioProviderWrapper {
	std::vector<std::array<double, 2>> gains;
	mutable std::vector<int16_t> src_buf;

public:
	DownmixAudioProvider(std::unique_ptr<AudioProvider> src)
	: AudioProviderWrapper(std::move(src))
	, gains(channels)
	{
		// The generic provider has no speaker metadata. Assume front L/R and
		// common surround orders, and center extra channels for unknown counts.
		// The defaults below are 3.0, quad, 5.0, 5.1, 6.1, and 7.1.
		static constexpr std::string_view extra_channels[] = {
			"C", "LR", "CLR", "C-LR", "C-CLR", "C-LRLR"
		};
		auto layout = channels <= 8 ? extra_channels[channels - 3] : std::string_view{};
		constexpr double surround_gain = std::numbers::sqrt2 / 2;
		gains[0] = {1, 0};
		gains[1] = {0, 1};

		std::array<double, 2> totals{1, 1};
		for (size_t i = 0; i < gains.size() - 2; ++i) {
			char speaker = i < layout.size() ? layout[i] : 'C';
			auto &gain = gains[i + 2];
			// A dash marks LFE. Leave it out of the stereo mix.
			gain = {speaker == 'R' || speaker == '-' ? 0. : surround_gain,
			        speaker == 'L' || speaker == '-' ? 0. : surround_gain};
			totals[0] += gain[0];
			totals[1] += gain[1];
		}

		// Leave enough headroom for all speakers to peak at the same time.
		double max_gain = std::max(totals[0], totals[1]);
		for (auto &gain : gains) {
			gain[0] /= max_gain;
			gain[1] /= max_gain;
		}

		channels = 2;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto src_channels = gains.size();
		src_buf.resize(count * src_channels);
		source->GetAudio(src_buf.data(), start, count);

		auto dst = static_cast<int16_t *>(buf);
		auto src = src_buf.data();
		for (int64_t frame = 0; frame < count; ++frame, src += src_channels) {
			double left = 0;
			double right = 0;
			for (size_t channel = 0; channel < src_channels; ++channel) {
				left += src[channel] * gains[channel][0];
				right += src[channel] * gains[channel][1];
			}

			*dst++ = static_cast<int16_t>(left);
			*dst++ = static_cast<int16_t>(right);
		}
	}
};

/// Sample doubler with linear interpolation for the samples provider
/// Requires interleaved 16-bit input
class SampleDoublingAudioProvider final : public AudioProviderWrapper {
	mutable std::vector<int16_t> src_buf;

public:
	SampleDoublingAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		sample_rate *= 2;
		num_samples *= 2;
		decoded_samples = decoded_samples * 2;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto first = start / 2;
		auto frames = (start + count - 1) / 2 - first + 2;
		src_buf.resize(frames * channels);
		source->GetAudio(src_buf.data(), first, frames);

		auto dst = static_cast<int16_t *>(buf);
		for (int64_t i = 0; i < count; ++i) {
			auto index = ((start + i) / 2 - first) * channels;

			for (int channel = 0; channel < channels; ++channel) {
				int sample = src_buf[index + channel];
				if ((start + i) & 1)
					sample = (sample + src_buf[index + channels + channel]) / 2;

				dst[i * channels + channel] = sample;
			}
		}
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreateConvertAudioProvider(std::unique_ptr<AudioProvider> provider) {
	// Ensure 16-bit audio with proper endianness
	if (provider->AreSamplesFloat()) {
		LOG_D("audio_provider") << "Converting float to S16";
		if (provider->GetBytesPerSample() == sizeof(float))
			provider = std::make_unique<FloatConvertAudioProvider<float, int16_t>>(std::move(provider));
		else
			provider = std::make_unique<FloatConvertAudioProvider<double, int16_t>>(std::move(provider));
	}
	if (provider->GetBytesPerSample() != 2) {
		LOG_D("audio_provider") << "Converting " << provider->GetBytesPerSample() << " bytes per sample or wrong endian to S16";
		provider = std::make_unique<BitdepthConvertAudioProvider<int16_t>>(std::move(provider));
	}

	// FFMS2 already downmixes with layout information when it can.
	if (provider->GetChannels() > 2) {
		LOG_D("audio_provider") << "Downmixing to stereo from " << provider->GetChannels() << " channels";
		provider = std::make_unique<DownmixAudioProvider>(std::move(provider));
	}

	// Some players don't like low sample rate audio
	while (provider->GetSampleRate() < 32000) {
		LOG_D("audio_provider") << "Doubling sample rate";
		provider = std::make_unique<SampleDoublingAudioProvider>(std::move(provider));
	}

	return provider;
}
}
