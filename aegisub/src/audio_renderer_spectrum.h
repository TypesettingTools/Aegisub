// Copyright (c) 2009, Niels Martin Hansen
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

#ifndef AGI_PRE
#include <stdint.h>
#endif

#ifdef WITH_FFTW
#include <fftw3.h>
#endif



// Specified and implemented in cpp file, to avoid pulling in too much
// complex template code in this header.
class AudioSpectrumCache;
struct AudioSpectrumCacheBlockFactory;



/// @class AudioSpectrumRenderer
/// @brief Render frequency-power spectrum graphs for audio data.
///
/// Renders frequency-power spectrum graphs of PCM audio data using a derivation function
/// such as the fast fourier transform.
class AudioSpectrumRenderer : public AudioRendererBitmapProvider {
	friend struct AudioSpectrumCacheBlockFactory;

	/// Internal cache management for the spectrum
	AudioSpectrumCache *cache;

	/// Colour table used for regular rendering
	AudioColorScheme colors_normal;

	/// Colour table used for rendering the audio selection
	AudioColorScheme colors_selected;

	/// Binary logarithm of number of samples to use in deriving frequency-power data
	size_t derivation_size;

	/// Binary logarithm of number of samples between the start of derivations
	size_t derivation_dist;

	/// @brief Reset in response to changing audio provider
	///
	/// Overrides the OnSetProvider event handler in the base class, to reset things
	/// when the audio provider is changed.
	void OnSetProvider();

	/// @brief Recreates the cache
	///
	/// To be called when the number of blocks in cache might have changed,
	// eg. new audio provider or new resolution.
	void RecreateCache();

	/// @brief Fill a block with frequency-power data for a time range
	/// @param      block_index Index of the block to fill data for
	/// @param[out] block       Address to write the data to
	void FillBlock(size_t block_index, float *block);

#ifdef WITH_FFTW
	/// FFTW plan data
	fftw_plan dft_plan;
	/// Pre-allocated input array for FFTW
	double *dft_input;
	/// Pre-allocated output array for FFTW
	fftw_complex *dft_output;
#else
	/// Pre-allocated scratch area for doing FFT derivations
	float *fft_scratch;
#endif

	/// Pre-allocated scratch area for storing raw audio data
	int16_t *audio_scratch;

public:
	/// @brief Constructor
	AudioSpectrumRenderer();

	/// @brief Destructor
	virtual ~AudioSpectrumRenderer();

	/// @brief Render a range of audio spectrum
	/// @param bmp      [in,out] Bitmap to render into, also carries lenght information
	/// @param start    First column of pixel data in display to render
	/// @param selected Whether to use the alternate colour scheme
	void Render(wxBitmap &bmp, int start, bool selected);

	/// @brief Render blank area
	void RenderBlank(wxDC &dc, const wxRect &rect, bool selected);

	/// @brief Set the derivation resolution
	/// @param derivation_size Binary logarithm of number of samples to use in deriving frequency-power data
	/// @param derivation_dist Binary logarithm of number of samples between the start of derivations
	///
	/// The derivations done will each use 2^derivation_size audio samples and at a distance
	/// of 2^derivation_dist samples.
	///
	/// The derivation distance must be smaller than or equal to the size. If the distance
	/// is specified too large, it will be clamped to the size.
	void SetResolution(size_t derivation_size, size_t derivation_dist);

	/// @brief Cleans up the cache
	/// @param max_size Maximum size in bytes for the cache
	void AgeCache(size_t max_size);
};
