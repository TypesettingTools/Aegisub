// Copyright (c) 2005-2006, Rodrigo Braz Monteiro
// Copyright (c) 2006-2010, Niels Martin Hansen
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

/// @file audio_renderer_spectrum.cpp
/// @brief Caching frequency-power spectrum renderer for audio display
/// @ingroup audio_ui

#include "audio_renderer_spectrum.h"

#include "audio_colorscheme.h"
#ifndef WITH_FFTW3
#include "fft.h"
#endif

#include <libaegisub/audio/provider.h>

#include <algorithm>

#include <wx/image.h>
#include <wx/dcmemory.h>

/// Allocates blocks of derived data for the audio spectrum
struct AudioSpectrumCacheBlockFactory {
	typedef std::unique_ptr<float, std::default_delete<float[]>> BlockType;

	/// Pointer back to the owning spectrum renderer
	AudioSpectrumRenderer *spectrum;

	/// @brief Allocate and fill a data block
	/// @param i Index of the block to produce data for
	/// @return Newly allocated and filled block
	///
	/// The filling is delegated to the spectrum renderer
	BlockType ProduceBlock(size_t i)
	{
		auto res = new float[((size_t)1)<<spectrum->derivation_size];
		spectrum->FillBlock(i, res);
		return BlockType(res);
	}

	/// @brief Calculate the in-memory size of a spec
	/// @return The size in bytes of a spectrum cache block
	size_t GetBlockSize() const
	{
		return sizeof(float) << spectrum->derivation_size;
	}
};

/// @brief Cache for audio spectrum frequency-power data
class AudioSpectrumCache
: public DataBlockCache<float, 10, AudioSpectrumCacheBlockFactory> {
public:
	AudioSpectrumCache(size_t block_count, AudioSpectrumRenderer *renderer)
	: DataBlockCache(block_count, AudioSpectrumCacheBlockFactory{renderer})
	{
	}
};

AudioSpectrumRenderer::AudioSpectrumRenderer(std::string const& color_scheme_name)
{
	colors.reserve(AudioStyle_MAX);
	for (int i = 0; i < AudioStyle_MAX; ++i)
		colors.emplace_back(12, color_scheme_name, i);
}

AudioSpectrumRenderer::~AudioSpectrumRenderer()
{
	// This sequence will clean up
	provider = nullptr;
	RecreateCache();
}

void AudioSpectrumRenderer::RecreateCache()
{
	update_derivation_values ();

#ifdef WITH_FFTW3
	if (dft_plan)
	{
		fftw_destroy_plan(dft_plan);
		fftw_free(dft_input);
		fftw_free(dft_output);
		dft_plan = nullptr;
		dft_input = nullptr;
		dft_output = nullptr;
	}
#endif

	if (provider)
	{
		size_t block_count = (size_t)((provider->GetNumSamples() + ((size_t)1<<derivation_dist) - 1) >> derivation_dist);
		cache = std::make_unique<AudioSpectrumCache>(block_count, this);

#ifdef WITH_FFTW3
		dft_input = fftw_alloc_real(2<<derivation_size);
		dft_output = fftw_alloc_complex(2<<derivation_size);
		dft_plan = fftw_plan_dft_r2c_1d(
			2<<derivation_size,
			dft_input,
			dft_output,
			FFTW_MEASURE);
#else
		// Allocate scratch for 6x the derivation size:
		// 2x for the input sample data
		// 2x for the real part of the output
		// 2x for the imaginary part of the output
		fft_scratch.resize(6 << derivation_size);
#endif
		audio_scratch.resize(2 << derivation_size);
	}
}

void AudioSpectrumRenderer::OnSetProvider()
{
	RecreateCache();
}

void AudioSpectrumRenderer::SetResolution(size_t _derivation_size, size_t _derivation_dist)
{
	if (derivation_dist_user != _derivation_dist)
	{
		derivation_dist_user = _derivation_dist;
		update_derivation_values ();
		AgeCache (0);
	}

	if (derivation_size_user != _derivation_size)
	{
		derivation_size_user = _derivation_size;
		RecreateCache();
	}
}

void AudioSpectrumRenderer::set_reference_frequency_position (float pos_fref_)
{
	assert (pos_fref_ > 0.f);
	assert (pos_fref_ < 1.f);

	pos_fref = pos_fref_;
}


template<class T>
void AudioSpectrumRenderer::ConvertToFloat(size_t count, T *dest) {
	for (size_t si = 0; si < count; ++si)
	{
		dest[si] = (T)(audio_scratch[si]) / 32768.0;
	}
}

void AudioSpectrumRenderer::update_derivation_values ()
{
	// Below this sampling rate (Hz), the derivation values are identical to
	// the user-provided ones. Otherwise, they are scaled according to the
	// ratio between the sampling rates.
	// The threshold is set at 50 kHz so with standard rates like 48 kHz,
	// the values are kept identical, and scaled with higher standard rates
	// like 88.2 or 96 kHz.
	constexpr float sample_rate_ref = 50000.f;

	derivation_dist = derivation_dist_user;
	derivation_size = derivation_size_user;

	if (provider != nullptr)
	{
		const int sample_rate = provider->GetSampleRate ();
		float mult = float (sample_rate) / sample_rate_ref;
		while (mult > 1)
		{
			++ derivation_dist;
			++ derivation_size;
			mult *= 0.5f;
		}
	}
}

void AudioSpectrumRenderer::FillBlock(size_t block_index, float *block)
{
	assert(cache);
	assert(block);

	int64_t first_sample = (((int64_t)block_index) << derivation_dist) - ((int64_t)1 << derivation_size);
	provider->GetAudio(&audio_scratch[0], first_sample, 2 << derivation_size);

	// Because the FFTs used here are unnormalized DFTs, we have to compensate
	// the possible length difference between derivation_size used in the
	// calculations and its user-provided counterpart. Thus, the display is
	// kept independent of the sampling rate.
	const float scale_fix =
		1.f / sqrtf (float (1 << (derivation_size - derivation_size_user)));

#ifdef WITH_FFTW3
	ConvertToFloat(2 << derivation_size, dft_input);

	fftw_execute(dft_plan);

	double scale_factor = scale_fix * 9 / sqrt(2 << (derivation_size + 1));

	fftw_complex *o = dft_output;
	for (size_t si = (size_t)1<<derivation_size; si > 0; --si)
	{
		*block++ = log10( sqrt(o[0][0] * o[0][0] + o[0][1] * o[0][1]) * scale_factor + 1 );
		o++;
	}
#else
	ConvertToFloat(2 << derivation_size, &fft_scratch[0]);

	float *fft_input = &fft_scratch[0];
	float *fft_real = &fft_scratch[0] + (2 << derivation_size);
	float *fft_imag = &fft_scratch[0] + (4 << derivation_size);

	FFT fft;
	fft.Transform(2<<derivation_size, fft_input, fft_real, fft_imag);

	float scale_factor = scale_fix * 9 / sqrt(2 * (float)(2<<derivation_size));

	for (size_t si = 1<<derivation_size; si > 0; --si)
	{
		// With x in range [0;1], log10(x*9+1) will also be in range [0;1],
		// although the FFT output can apparently get greater magnitudes than 1
		// despite the input being limited to [-1;+1).
		*block++ = log10( sqrt(*fft_real * *fft_real + *fft_imag * *fft_imag) * scale_factor + 1 );
		fft_real++; fft_imag++;
	}
#endif
}

void AudioSpectrumRenderer::Render(wxBitmap &bmp, int start, AudioRenderingStyle style)
{
	// Misc. utility functions
	auto floor_int = [] (float val) { return int (floorf (val       )); };
	auto round_int = [] (float val) { return int (floorf (val + 0.5f)); };

	if (!cache)
		return;

	assert(bmp.IsOk());
	assert(bmp.GetDepth() == 24 || bmp.GetDepth() == 32);

	int end = start + bmp.GetWidth();

	assert(start >= 0);
	assert(end >= 0);
	assert(end >= start);

	// Prepare an image buffer to write
	wxImage img(bmp.GetSize());
	unsigned char *imgdata = img.GetData();
	ptrdiff_t stride = img.GetWidth()*3;
	int imgheight = img.GetHeight();

	const AudioColorScheme *pal = &colors[style];

	// Sampling rate, in Hz.
	const float sample_rate = float (provider->GetSampleRate ());

	// Number of FFT bins, excluding the "Nyquist" one
	const int nbr_bins = 1 << derivation_size;

	// minband and maxband define an half-open range.
	int minband = 1; // Starts at 1, we don't care about showing the DC.
	int maxband = std::min (
		round_int (nbr_bins * max_freq / (sample_rate * 0.5f)),
		nbr_bins
	);
	assert (minband < maxband);

	// Precomputes this once, this will be useful for the log curve.
	const float scale_log = logf (maxband / minband);

	// Turns the user-specified 1 kHz position into a ratio between the linear
	// and logarithmic curves that we can directly use in the following
	// calculations.
	assert (pos_fref > 0);
	assert (pos_fref < 1);
	float b_fref         = nbr_bins * freq_ref / (sample_rate * 0.5f);
	b_fref               = mid (1.f, b_fref, float (maxband - 1));
	const float clin     = minband + (maxband - minband) * pos_fref;
	const float clog     = minband * expf (pos_fref * scale_log);
	float log_ratio_calc = (b_fref - clin) / (clog - clin);
	log_ratio_calc       = mid (0.f, log_ratio_calc, 1.f);

	// ax = absolute x, absolute to the virtual spectrum bitmap
	for (int ax = start; ax < end; ++ax)
	{
		// Derived audio data
		size_t block_index = (size_t)(ax * pixel_ms * provider->GetSampleRate() / 1000) >> derivation_dist;
		float *power = &cache->Get(block_index);

		// Prepare bitmap writing
		unsigned char *px = imgdata + (imgheight-1) * stride + (ax - start) * 3;

		float bin_prv = minband;
		float bin_cur = minband;
		for (int y = 0; y < imgheight; ++y)
		{
			assert (bin_cur < float (maxband));

			float       bin_nxt = maxband;
			if (y + 1 < imgheight)
			{
				// Bin index is an interpolation between the linear and log curves.
				const float pos_rel = float (y + 1) / float (imgheight);
				const float b_lin   = minband + pos_rel * (maxband - minband);
				const float b_log   = minband * expf (pos_rel * scale_log);
				bin_nxt = b_lin + log_ratio_calc * (b_log - b_lin);
			}

			float val = 0;

			// Interpolate between consecutive bins
			if (bin_nxt - bin_prv < 2)
			{
				const int   bin_0 = floor_int (bin_cur);
				const int   bin_1 = std::min (bin_0 + 1, nbr_bins - 1);
				const float frac  = bin_cur - float (bin_0);
				const float v0    = power [bin_0];
				const float v1    = power [bin_1];
				val = v0 + frac * (v1 - v0);
			}

			// Pick the greatest bin on the interval
			else
			{
				int bin_inf = floor_int ((bin_prv + bin_cur) * 0.5f);
				int bin_sup = floor_int ((bin_cur + bin_nxt) * 0.5f);
				bin_inf = std::min (bin_inf, nbr_bins - 2);
				bin_sup = std::min (bin_sup, nbr_bins - 1);
				assert (bin_inf < bin_sup);
				val = *std::max_element (&power [bin_inf], &power [bin_sup]);
			}

			pal->map (val * amplitude_scale, px);

			px     -= stride;
			bin_prv = bin_cur;
			bin_cur = bin_nxt;
		}
	}

	wxBitmap tmpbmp(img);
	wxMemoryDC targetdc(bmp);
	targetdc.DrawBitmap(tmpbmp, 0, 0);
}

void AudioSpectrumRenderer::RenderBlank(wxDC &dc, const wxRect &rect, AudioRenderingStyle style)
{
	// Get the colour of silence
	wxColour col = colors[style].get(0.0f);
	dc.SetBrush(wxBrush(col));
	dc.SetPen(wxPen(col));
	dc.DrawRectangle(rect);
}

void AudioSpectrumRenderer::AgeCache(size_t max_size)
{
	if (cache)
		cache->Age(max_size);
}
