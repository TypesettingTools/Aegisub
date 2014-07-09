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

#include "libaegisub/fs.h"
#include "libaegisub/make_unique.h"

#include <boost/algorithm/string/predicate.hpp>
#include <random>

/*
 * scheme            ::= "dummy-audio" ":" signal-specifier "?" signal-parameters
 * signal-specifier  ::= "silence" | "noise" | "sine" "/" frequency
 * frequency         ::= integer
 * signal-parameters ::= signal-parameter [ "&" signal-parameters ]
 * signal-parameter  ::= signal-parameter-name "=" integer
 * signal-parameter-name ::= "sr" | "bd" | "ch" | "ln"
 *
 * Signal types:
 * "silence", a silent signal is generated.
 * "noise", a white noise signal is generated.
 * "sine", a sine wave is generated at the specified frequency.
 *
 * Signal parameters:
 * "sr", sample rate to generate signal at.
 * "bd", bit depth to generate signal at (usually 16).
 * "ch", number of channels to generate, usually 1 or 2. The same signal is generated
 *       in every channel even if one would be LFE.
 * "ln", length of signal in samples. ln/sr gives signal length in seconds.
 */

namespace {
using namespace agi;
class DummyAudioProvider final : public AudioProvider {
	bool noise;

	void FillBuffer(void *buf, int64_t start, int64_t count) const override {
		if (noise) {
			std::default_random_engine e;
			std::uniform_int_distribution<int16_t> uniform_dist(-5000, 5000);
			for (size_t i = 0; i < count; ++i)
				static_cast<short *>(buf)[i] = uniform_dist(e);
		}
		else
			memset(buf, 0, count * bytes_per_sample);
	}

public:
	DummyAudioProvider(agi::fs::path const& uri) {
		noise = boost::contains(uri.string(), ":noise?");
		channels = 1;
		sample_rate = 44100;
		bytes_per_sample = 2;
		float_samples = false;
		decoded_samples = num_samples = (int64_t)5*30*60*1000 * sample_rate / 1000;
	}
};
}

namespace agi {
std::unique_ptr<AudioProvider> CreateDummyAudioProvider(agi::fs::path const& file, agi::BackgroundRunner *) {
	if (!boost::starts_with(file.string(), "dummy-audio:"))
		return {};
	return agi::make_unique<DummyAudioProvider>(file);
}
}
