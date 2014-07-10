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

#include <libaegisub/log.h>
#include <libaegisub/make_unique.h>

#include <limits>

using namespace agi;

/// Anything integral -> 16 bit signed machine-endian audio converter
namespace {
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
				for (int j = src_bytes_per_sample; j > 0; --j) {
					sample <<= 8;
					sample += src_buf[i * src_bytes_per_sample + j - 1];
				}
			}

			if (static_cast<size_t>(src_bytes_per_sample) > sizeof(Target))
				sample /= 1 << (src_bytes_per_sample - sizeof(Target)) * 8;
			else if (static_cast<size_t>(src_bytes_per_sample) < sizeof(Target))
				sample *=  1 << (sizeof(Target) - src_bytes_per_sample ) * 8;

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

		for (size_t i = 0; i < static_cast<size_t>(count * channels); ++i) {
			Source expanded;
			if (src_buf[i] < 0)
				expanded = static_cast<Target>(-src_buf[i] * std::numeric_limits<Target>::min());
			else
				expanded = static_cast<Target>(src_buf[i] * std::numeric_limits<Target>::max());

			if (expanded < std::numeric_limits<Target>::min())
				dest[i] = std::numeric_limits<Target>::min();
			else if (expanded > std::numeric_limits<Target>::max())
				dest[i] = std::numeric_limits<Target>::max();
			else
				dest[i] = static_cast<Target>(expanded);
		}
	}
};

/// Non-mono 16-bit signed machine-endian -> mono 16-bit signed machine endian converter
class DownmixAudioProvider final : public AudioProviderWrapper {
	int src_channels;
	mutable std::vector<int16_t> src_buf;

public:
	DownmixAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		src_channels = channels;
		channels = 1;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		src_buf.resize(count * src_channels);
		source->GetAudio(&src_buf[0], start, count);

		auto dst = static_cast<int16_t*>(buf);
		// Just average the channels together
		while (count-- > 0) {
			int sum = 0;
			for (int c = 0; c < src_channels; ++c)
				sum += src_buf[count * src_channels + c];
			dst[count] = static_cast<int16_t>(sum / src_channels);
		}
	}
};

/// Sample doubler with linear interpolation for the samples provider
/// Requires 16-bit mono input
class SampleDoublingAudioProvider final : public AudioProviderWrapper {
public:
	SampleDoublingAudioProvider(std::unique_ptr<AudioProvider> src) : AudioProviderWrapper(std::move(src)) {
		sample_rate *= 2;
		num_samples *= 2;
		decoded_samples = decoded_samples * 2;
	}

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		bool not_end = start + count < num_samples;
		int64_t src_count = count / 2;
		source->GetAudio(buf, start / 2, src_count + not_end);

		auto buf16 = reinterpret_cast<int16_t*>(buf);

		if (!not_end) {
			// We weren't able to request a sample past the end so just
			// duplicate the last sample
			buf16[src_count] = buf16[src_count + 1];
		}

		if (count % 2)
			buf16[count - 1] = buf16[src_count];

		// walking backwards so that the conversion can be done in place
		for (int64_t i = src_count - 1; i >= 0; --i) {
			buf16[i * 2] = buf16[i];
			buf16[i * 2 + 1] = (int16_t)(((int32_t)buf16[i] + buf16[i + 1]) / 2);
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
			provider = agi::make_unique<FloatConvertAudioProvider<float, int16_t>>(std::move(provider));
		else
			provider = agi::make_unique<FloatConvertAudioProvider<double, int16_t>>(std::move(provider));
	}
	if (provider->GetBytesPerSample() != 2) {
		LOG_D("audio_provider") << "Converting " << provider->GetBytesPerSample() << " bytes per sample or wrong endian to S16";
		provider = agi::make_unique<BitdepthConvertAudioProvider<int16_t>>(std::move(provider));
	}

	// We currently only support mono audio
	if (provider->GetChannels() != 1) {
		LOG_D("audio_provider") << "Downmixing to mono from " << provider->GetChannels() << " channels";
		provider = agi::make_unique<DownmixAudioProvider>(std::move(provider));
	}

	// Some players don't like low sample rate audio
	while (provider->GetSampleRate() < 32000) {
		LOG_D("audio_provider") << "Doubling sample rate";
		provider = agi::make_unique<SampleDoublingAudioProvider>(std::move(provider));
	}

	return provider;
}
}
