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
//
// $Id$

/// @file audio_renderer_spectrum.cpp
/// @brief Caching frequency-power spectrum renderer for audio display
/// @ingroup audio_ui

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/image.h>
#include <wx/rawbmp.h>
#include <wx/dcmemory.h>
#endif

#include <libaegisub/log.h>

#include "block_cache.h"
#include "include/aegisub/audio_provider.h"
#include "audio_colorscheme.h"
#include "audio_renderer.h"
#include "audio_renderer_spectrum.h"

#ifdef WITH_FFTW
#include <fftw3.h>
#else
#include "fft.h"
#endif
#include "main.h"
#include "utils.h"

/// Allocates blocks of derived data for the audio spectrum
struct AudioSpectrumCacheBlockFactory {
	/// Pointer back to the owning spectrum renderer
	AudioSpectrumRenderer *spectrum;

	/// @brief Constructor
	/// @param s The owning spectrum renderer
	AudioSpectrumCacheBlockFactory(AudioSpectrumRenderer *s) : spectrum(s) { }

	/// @brief Allocate and fill a data block
	/// @param i Index of the block to produce data for
	/// @return Newly allocated and filled block
	///
	/// The filling is delegated to the spectrum renderer
	float *ProduceBlock(size_t i)
	{
		float *res = new float[((size_t)1)<<spectrum->derivation_size];
		spectrum->FillBlock(i, res);
		return res;
	}

	/// @brief De-allocate a cache block
	/// @param block The block to dispose of
	void DisposeBlock(float *block)
	{
		delete[] block;
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
		: DataBlockCache<float, 10, AudioSpectrumCacheBlockFactory>(
			block_count, AudioSpectrumCacheBlockFactory(renderer))
	{
	}
};




AudioSpectrumRenderer::AudioSpectrumRenderer()
: AudioRendererBitmapProvider()
, cache(0)
, colors_normal(12)
, colors_selected(12)
, derivation_size(8)
, derivation_dist(8)
#ifdef WITH_FFTW
, dft_plan(0)
, dft_input(0)
, dft_output(0)
#else
, fft_scratch(0)
#endif
, audio_scratch(0)
{
	colors_normal.InitIcyBlue_Normal();
	colors_selected.InitIcyBlue_Selected();
}


AudioSpectrumRenderer::~AudioSpectrumRenderer()
{
	// This sequence will clean up
	provider = 0;
	RecreateCache();
}


void AudioSpectrumRenderer::RecreateCache()
{
	delete cache;
	delete[] audio_scratch;
	cache = 0;
	audio_scratch = 0;

#ifdef WITH_FFTW
	if (dft_plan)
	{
		fftw_destroy_plan(dft_plan);
		fftw_free(dft_input);
		fftw_free(dft_output);
		dft_plan = 0;
		dft_input = 0;
		dft_output = 0;
	}
#else
	delete[] fft_scratch;
	fft_scratch = 0;
#endif

	if (provider)
	{
		size_t block_count = (size_t)((provider->GetNumSamples() + (size_t)(1<<derivation_dist) - 1) >> derivation_dist);
		cache = new AudioSpectrumCache(block_count, this);

#ifdef WITH_FFTW
		dft_input = (double*)fftw_malloc(sizeof(double) * (2<<derivation_size));
		dft_output = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (2<<derivation_size));
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
		fft_scratch = new float[6<<derivation_size];
#endif
		audio_scratch = new int16_t[2<<derivation_size];
	}
}


void AudioSpectrumRenderer::OnSetProvider()
{
	RecreateCache();
}


void AudioSpectrumRenderer::SetResolution(size_t _derivation_size, size_t _derivation_dist)
{
	if (derivation_dist != _derivation_dist)
	{
		derivation_dist = _derivation_dist;
		if (cache)
			cache->Age(0);
	}

	if (derivation_size != _derivation_size)
	{
		derivation_size = _derivation_size;
		RecreateCache();
	}
}


void AudioSpectrumRenderer::FillBlock(size_t block_index, float *block)
{
	assert(cache);
	assert(block);

	int64_t first_sample = ((int64_t)block_index) << derivation_dist;
	provider->GetAudio(audio_scratch, first_sample, 2 << derivation_size);

#ifdef WITH_FFTW
	// Convert audio data to float range [-1;+1)
	for (size_t si = 0; si < (size_t)(2<<derivation_size); ++si)
	{
		dft_input[si] = (float)(audio_scratch[si]) / 32768.f;
	}

	fftw_execute(dft_plan);

	float scale_factor = 9 / sqrt(2 * (float)(2<<derivation_size));

	fftw_complex *o = dft_output;
	for (size_t si = 1<<derivation_size; si > 0; --si)
	{
		*block++ = log10( sqrt(o[0][0] * o[0][0] + o[0][1] * o[0][1]) * scale_factor + 1 );
		o++;
	}
#else
	float *fft_input = fft_scratch;
	float *fft_real = fft_scratch + (2 << derivation_size);
	float *fft_imag = fft_scratch + (4 << derivation_size);

	// Convert audio data to float range [-1;+1)
	for (size_t si = 0; si < (size_t)(2<<derivation_size); ++si)
	{
		fft_input[si] = (float)(audio_scratch[si]) / 32768.f;
	}
	fft_input = fft_scratch;

	FFT fft;
	fft.Transform(2<<derivation_size, fft_input, fft_real, fft_imag);

	float scale_factor = 9 / sqrt(2 * (float)(2<<derivation_size));

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


void AudioSpectrumRenderer::Render(wxBitmap &bmp, int start, bool selected)
{
	if (!cache)
		return;

	assert(bmp.IsOk());
	assert(bmp.GetDepth() == 24);

	int end = start + bmp.GetWidth();

	assert(start >= 0);
	assert(end >= 0);
	assert(end >= start);

	// Prepare an image buffer to write
	wxImage img(bmp.GetSize());
	unsigned char *imgdata = img.GetData();
	ptrdiff_t stride = img.GetWidth()*3;
	int imgheight = img.GetHeight();

	AudioColorScheme *pal = selected ? &colors_selected : &colors_normal;

	/// @todo Make minband and maxband configurable
	int minband = 0;
	int maxband = 1 << derivation_size;

	// ax = absolute x, absolute to the virtual spectrum bitmap
	for (int ax = start; ax < end; ++ax)
	{
		// Derived audio data
		size_t block_index = (size_t)(ax * pixel_samples) >> derivation_dist;
		float *power = cache->Get(block_index);

		// Prepare bitmap writing
		unsigned char *px = imgdata + (imgheight-1) * stride + (ax - start) * 3;

		// Scale up or down vertically?
		if (imgheight > 1<<derivation_size)
		{
			// Interpolate
			for (int y = 0; y < imgheight; ++y)
			{
				assert(px >= imgdata);
				assert(px < imgdata + imgheight*stride);
				float ideal = (float)(y+1.)/imgheight * (maxband-minband) + minband;
				float sample1 = power[(int)floor(ideal)+minband];
				float sample2 = power[(int)ceil(ideal)+minband];
				float frac = ideal - floor(ideal);
				float val = (1-frac)*sample1 + frac*sample2;
				pal->map(val*amplitude_scale, px);
				px -= stride;
			}
		}
		else
		{
			// Pick greatest
			for (int y = 0; y < imgheight; ++y)
			{
				assert(px >= imgdata);
				assert(px < imgdata + imgheight*stride);
				int sample1 = std::max(0, maxband * y/imgheight + minband);
				int sample2 = std::min((1<<derivation_size)-1, maxband * (y+1)/imgheight + minband);
				float maxval = 0;
				for (int samp = sample1; samp <= sample2; samp++)
					if (power[samp] > maxval) maxval = power[samp];
				pal->map(maxval*amplitude_scale, px);
				px -= stride;
			}
		}
	}

	wxBitmap tmpbmp(img);
	wxMemoryDC targetdc(bmp);
	targetdc.DrawBitmap(tmpbmp, 0, 0);
}


void AudioSpectrumRenderer::RenderBlank(wxDC &dc, const wxRect &rect, bool selected)
{
	// Get the colour of silence
	AudioColorScheme *pal = selected ? &colors_selected : &colors_normal;
	unsigned char color_raw[4];
	pal->map(0.0, color_raw);
	wxColour col(color_raw[0], color_raw[1], color_raw[2]);

	dc.SetBrush(wxBrush(col));
	dc.SetPen(wxPen(col));
	dc.DrawRectangle(rect);
}


void AudioSpectrumRenderer::AgeCache(size_t max_size)
{
	if (cache)
		cache->Age(max_size);
}

