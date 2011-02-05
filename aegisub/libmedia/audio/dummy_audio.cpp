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

/// @file audio_provider_dummy.cpp
/// @brief Dummy (silence or noise) audio provider
/// @ingroup audio_input
///

#include "config.h"

#include "audio_provider_dummy.h"
#include "utils.h"


/// @brief Constructor 
/// @param dur_ms 
/// @param _noise 
///
DummyAudioProvider::DummyAudioProvider(unsigned long dur_ms, bool _noise) {
	noise = _noise;
	channels = 1;
	sample_rate = 44100;
	bytes_per_sample = 2;
	num_samples = (int64_t)dur_ms * sample_rate / 1000;
}

/// @brief Destructor 
///
DummyAudioProvider::~DummyAudioProvider() {
}

/// @brief Get audio 
/// @param buf   
/// @param start 
/// @param count 
///
void DummyAudioProvider::GetAudio(void *buf, int64_t start, int64_t count) const {
	short *workbuf = (short*)buf;

	if (noise) {
		while (--count > 0)
			*workbuf++ = (rand() - RAND_MAX/2) * 10000 / RAND_MAX;
	}
	else {
		while (--count > 0)
			*workbuf++ = 0;
	}
}
