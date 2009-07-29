// Copyright (c) 2005, 2006, Rodrigo Braz Monteiro
// Copyright (c) 2006, 2007, Niels Martin Hansen
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

/// @file audio_spectrum.h
/// @see audio_spectrum.cpp
/// @ingroup audio_ui
///

#ifndef AUDIO_SPECTRUM_H

/// DOCME
#define AUDIO_SPECTRUM_H

#include <wx/wxprec.h>
#include <stdint.h>
#include "audio_provider_manager.h"


// Specified and implemented in cpp file, interface is private to spectrum code
class AudioSpectrumCacheManager;



/// DOCME
/// @class AudioSpectrum
/// @brief DOCME
///
/// DOCME
class AudioSpectrum {
private:

	/// DOCME
	AudioSpectrumCacheManager *cache;


	/// DOCME
	unsigned char colours_normal[256*3];

	/// DOCME
	unsigned char colours_selected[256*3];


	/// DOCME
	AudioProvider *provider;


	/// DOCME
	unsigned long line_length; // number of frequency components per line (half of number of samples)

	/// DOCME
	unsigned long num_lines; // number of lines needed for the audio

	/// DOCME
	unsigned int fft_overlaps; // number of overlaps used in FFT

	/// DOCME
	float power_scale; // amplification of displayed power

	/// DOCME
	int minband; // smallest frequency band displayed

	/// DOCME
	int maxband; // largest frequency band displayed

public:
	AudioSpectrum(AudioProvider *_provider);
	~AudioSpectrum();

	void RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight);

	void SetScaling(float _power_scale);
};


#endif


