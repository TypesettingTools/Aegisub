// Copyright (c) 2026, Aegisub contributors
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

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace agi::audio {
inline constexpr double MinPlaybackRate = 0.5;
inline constexpr double MaxPlaybackRate = 2.0;

inline double ClampPlaybackRate(double rate) {
	if (!std::isfinite(rate)) return 1.0;
	return std::clamp(rate, MinPlaybackRate, MaxPlaybackRate);
}

inline int64_t SourceSamplesFromMilliseconds(int64_t ms, int sample_rate) {
	if (sample_rate <= 0 || ms <= 0) return 0;
	return (ms * sample_rate + 999) / 1000;
}

inline int64_t MillisecondsFromSourceSamples(int64_t samples, int sample_rate) {
	if (sample_rate <= 0 || samples <= 0) return 0;
	return samples * 1000 / sample_rate;
}

inline double PlaybackSamplesFromSourceSamplesExact(int64_t source_samples, double rate) {
	return source_samples / ClampPlaybackRate(rate);
}

inline double SourceSamplesFromPlaybackSamplesExact(int64_t playback_samples, double rate) {
	return playback_samples * ClampPlaybackRate(rate);
}

inline int64_t PlaybackSamplesFromSourceSamplesFloor(int64_t source_samples, double rate) {
	return static_cast<int64_t>(std::floor(PlaybackSamplesFromSourceSamplesExact(source_samples, rate)));
}

inline int64_t PlaybackSamplesFromSourceSamplesCeil(int64_t source_samples, double rate) {
	return static_cast<int64_t>(std::ceil(PlaybackSamplesFromSourceSamplesExact(source_samples, rate)));
}

inline int64_t SourceSamplesFromPlaybackSamplesFloor(int64_t playback_samples, double rate) {
	return static_cast<int64_t>(std::floor(SourceSamplesFromPlaybackSamplesExact(playback_samples, rate)));
}

inline int64_t SourceSamplesFromPlaybackSamplesCeil(int64_t playback_samples, double rate) {
	return static_cast<int64_t>(std::ceil(SourceSamplesFromPlaybackSamplesExact(playback_samples, rate)));
}
}
