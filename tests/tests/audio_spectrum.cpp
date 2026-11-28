// Copyright (c) 2026, Aegisub contributors
// SPDX-License-Identifier: BSD-3-Clause

#include <main.h>
#include <util.h>
#include "../../src/options.h"

// The cache factory is a friend of the renderer; exercise the actual spectral
// calculation without allocating GUI bitmaps or adding a public testing API.
#include "../../src/audio_renderer_spectrum.cpp"

#include <cmath>
#include <fstream>
#include <iterator>

namespace config { agi::Options *opt = nullptr; }

namespace {
class ToneProvider : public agi::AudioProvider {
	bool opposite;
public:
	ToneProvider(int channel_count, bool opposite = false) : opposite(opposite) {
		channels = channel_count;
		sample_rate = 48000;
		bytes_per_sample = 2;
		num_samples = decoded_samples = 48000;
	}
	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		auto out = static_cast<int16_t *>(buf);
		for (int64_t i = start; i < start + count; ++i) {
			int16_t sample = std::round(12000 * std::sin(i * 2 * std::acos(-1.) / 32));
			for (int channel = 0; channel < channels; ++channel)
				*out++ = opposite && channel == 1 ? -sample : sample;
		}
	}
};
}

TEST(AudioSpectrum, OppositePhaseStereoPreservesMonoSpectralPower) {
	std::ifstream defaults(util::test_data_dir().parent_path() / "src/libresrc/default_config.json");
	ASSERT_TRUE(defaults.good());
	std::string json{std::istreambuf_iterator<char>(defaults), std::istreambuf_iterator<char>()};
	agi::Options options("", json, agi::Options::FLUSH_SKIP);
	struct RestoreOptions {
		agi::Options *previous = config::opt;
		~RestoreOptions() { config::opt = previous; }
	} restore;
	config::opt = &options;
	AudioSpectrumRenderer renderer(options.Get("Colour/Audio Display/Spectrum")->GetString());
	renderer.SetResolution(8, 8);
	ToneProvider mono(1), stereo(2), opposite(2, true);
	renderer.SetProvider(&mono);
	AudioSpectrumCacheBlockFactory factory{&renderer};
	auto reference = factory.ProduceBlock(8);
	renderer.SetProvider(&stereo);
	auto same_phase = factory.ProduceBlock(8);
	renderer.SetProvider(&opposite);
	auto opposite_phase = factory.ProduceBlock(8);
	// A derivation size of eight produces 256 frequency bins.
	for (int i = 0; i < 256; ++i) {
		EXPECT_NEAR(reference.get()[i], same_phase.get()[i], 1e-5);
		EXPECT_NEAR(reference.get()[i], opposite_phase.get()[i], 1e-5);
	}
	EXPECT_GT(reference.get()[16], 0.1f);
}
