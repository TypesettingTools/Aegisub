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

/// @file audio_renderer_spectrum.h
/// @see audio_renderer_spectrum.cpp
/// @ingroup audio_ui
///
/// Calculate and render a frequency-power spectrum for PCM audio data.


#ifndef AUDIO_SPECTRUM_H

/// Include guard for audio_spectrum.h
#define AUDIO_SPECTRUM_H

#ifndef AGI_PRE
#include <stdint.h>
#endif

#include "audio_provider_manager.h"


// Specified and implemented in cpp file, interface is private to spectrum code
class AudioSpectrumCacheManager;


/// @class AudioSpectrum
/// @brief Render frequency-power spectrum graphs for audio data.
///
/// Renders frequency-power spectrum graphs of PCM audio data using a fast fourier transform
/// to derive the data. The frequency-power data are cached to avoid re-computing them
/// frequently, and the cache size is limited by a configuration setting.
///
/// The spectrum image is rendered to a 32 bit RGB bitmap. Power data is scaled linearly
/// and not logarithmically, since the rendering is done with limited precision, but
/// an amplification factor can be specified to see different ranges.
class AudioSpectrum {
private:

	/// Internal cache management for the spectrum
	AudioSpectrumCacheManager *cache;

	/// Colour table used for regular rendering
	unsigned char colours_normal[256*3];

	/// Colour table used for rendering the audio selection
	unsigned char colours_selected[256*3];

	/// The audio provider to use as source
	AudioProvider *provider;

	unsigned long line_length; ///< Number of frequency components per line (half of number of samples)
	unsigned long num_lines;   ///< Number of lines needed for the audio
	unsigned int fft_overlaps; ///< Number of overlaps used in FFT
	float power_scale;         ///< Amplification of displayed power
	int minband;               ///< Smallest frequency band displayed
	int maxband;               ///< Largest frequency band displayed

public:
	/// @brief Constructor
	/// @param _provider Audio provider to render spectrum data for.
	///
	/// Reads configuration data for the spectrum display and initialises itself following that.
	AudioSpectrum(AudioProvider *_provider);
	/// @brief Destructor
	~AudioSpectrum();

	/// @brief Render a range of audio spectrum to a bitmap buffer.
	/// @param range_start First audio sample in the range to render.
	/// @param range_end   Last audio sample in the range to render.
	/// @param selected    Use the alternate colour palette?
	/// @param img         Pointer to 32 bit RGBX data
	/// @param imgleft     Offset from left edge of bitmap to render to, in pixels
	/// @param imgwidth    Width of bitmap to render, in pixels
	/// @param imgpitch    Offset from one scanline to the next in the bitmap, in bytes
	/// @param imgheight   Number of lines in the bitmap
	void RenderRange(int64_t range_start, int64_t range_end, bool selected, unsigned char *img, int imgleft, int imgwidth, int imgpitch, int imgheight);

	/// @brief Set the amplification to use when rendering.
	/// @param _power_scale Amplification factor to use.
	void SetScaling(float _power_scale);
};


#endif


