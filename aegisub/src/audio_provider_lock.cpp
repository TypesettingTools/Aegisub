// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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
// $Id$

/// @file audio_provider_lock.cpp
/// @brief An audio provider adapter for un-threadsafe audio providers
/// @ingroup audio_input

#include "config.h"

#include "audio_provider_lock.h"

LockAudioProvider::LockAudioProvider(AudioProvider *source) : source(source) {
	channels = source->GetChannels();
	num_samples = source->GetNumSamples();
	sample_rate = source->GetSampleRate();
	bytes_per_sample = source->GetBytesPerSample();
	float_samples = source->AreSamplesFloat();
}

void LockAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	wxMutexLocker lock(mutex);
	source->GetAudio(buf, start, count);
}
