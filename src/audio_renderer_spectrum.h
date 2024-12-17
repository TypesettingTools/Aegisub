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

/// @file audio_renderer_spectrum.h
/// @see audio_renderer_spectrum.cpp
/// @ingroup audio_ui
///
/// Calculate and render a frequency-power spectrum for PCM audio data.

#include <cstdint>
#include <memory>
#include <vector>

#include "audio_renderer.h"

#ifdef WITH_FFTW3
#include <fftw3.h>
#endif

class AudioColorScheme;
class AudioSpectrumCache;
struct AudioSpectrumCacheBlockFactory;

/// @class AudioSpectrumRenderer
/// @brief Render frequency-power spectrum graphs for audio data.
///
/// Renders frequency-power spectrum graphs of PCM audio data using a derivation function
/// such as the fast fourier transform.
class AudioSpectrumRenderer final : public AudioRendererBitmapProvider {
	friend struct AudioSpectrumCacheBlockFactory;

	/// Internal cache management for the spectrum
	std::unique_ptr<AudioSpectrumCache> cache;

	/// Colour tables used for rendering
	std::vector<AudioColorScheme> colors;

	/// User-provided value for derivation_size
	size_t derivation_size_user = 0;

	/// User-provided value for derivation_dist
	size_t derivation_dist_user = 0;

	/// Maximum audible, displayed frequency. Avoids wasting the display space
	/// with ultrasonic content at sampling rates > 40 kHz.
	float max_freq = 20000.f;

	/// Relative vertical position of the 1 kHz frequency, in (0 ; 1) open range
	/// 0 = bottom of the display zone, 1 = top
	/// The actual position, as displayed, is limited by the available mapping
	/// curves (linear and log).
	/// Values close to 0 will give a linear curve, and close to 1 a log curve.
	float pos_fref = 1.0f / 3;

	/// Reference frequency which vertical position is constant, Hz.
	const float freq_ref = 1000.0f;

	/// Binary logarithm of number of samples to use in deriving frequency-power data
	/// This could differ from the user-provided value because the actual value
	/// used in computations may be scaled, depending on the sampling rate.
	size_t derivation_size = 0;

	/// Binary logarithm of number of samples between the start of derivations
	/// This could differ from the user-provided value because the actual value
	/// used in computations may be scaled, depending on the sampling rate.
	size_t derivation_dist = 0;

	/// @brief Reset in response to changing audio provider
	///
	/// Overrides the OnSetProvider event handler in the base class, to reset things
	/// when the audio provider is changed.
	void OnSetProvider() override;

	/// @brief Recreates the cache
	///
	/// To be called when the number of blocks in cache might have changed,
	/// e.g. new audio provider or new resolution.
	void RecreateCache();

	/// @brief Fill a block with frequency-power data for a time range
	/// @param      block_index Index of the block to fill data for
	/// @param[out] block       Address to write the data to
	void FillBlock(size_t block_index, float *block);

	/// @brief Convert audio data to float range [-1;+1)
	/// @param count Samples to convert
	/// @param dest Buffer to fill
	template<class T>
	void ConvertToFloat(size_t count, T *dest);

	/// @brief Updates the derivation_* after a derivation_*_user change.
	void update_derivation_values ();

#ifdef WITH_FFTW3
	/// FFTW plan data
	fftw_plan dft_plan = nullptr;
	/// Pre-allocated input array for FFTW
	double *dft_input = nullptr;
	/// Pre-allocated output array for FFTW
	fftw_complex *dft_output = nullptr;
#else
	/// Pre-allocated scratch area for doing FFT derivations
	std::vector<float> fft_scratch;
#endif

	/// Pre-allocated scratch area for storing raw audio data
	std::vector<int16_t> audio_scratch;

public:
	/// @brief Constructor
	/// @param color_scheme_name Name of the color scheme to use
	AudioSpectrumRenderer(std::string const& color_scheme_name);

	/// @brief Destructor
	~AudioSpectrumRenderer();

	/// @brief Render a range of audio spectrum
	/// @param bmp   [in,out] Bitmap to render into, also carries length information
	/// @param start First column of pixel data in display to render
	/// @param style Style to render audio in
	void Render(wxBitmap &bmp, int start, AudioRenderingStyle style) override;

	/// @brief Render blank area
	void RenderBlank(wxDC &dc, const wxRect &rect, AudioRenderingStyle style) override;

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

	/// @brief Set the vertical relative position of the reference frequency (1 kHz)
	/// @param fref_pos_ Vertical position of the 1 kHz frequency. Between 0 and 1, boundaries excluded.
	///
	/// A value close to 0 gives a linear display, and close to 1 a logarithmic display.
	void set_reference_frequency_position (float pos_fref_);

	/// @brief Cleans up the cache
	/// @param max_size Maximum size in bytes for the cache
	void AgeCache(size_t max_size) override;
};
