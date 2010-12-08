// Copyright (c) 2009-2010, Niels Martin Hansen
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

/// @file audio_renderer.cpp
/// @brief Base classes for audio renderers (spectrum, waveform, ...)
/// @ingroup audio_ui


// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#endif

#include "block_cache.h"
#include "audio_renderer.h"
#include "include/aegisub/audio_provider.h"

#undef min
#undef max


AudioRendererBitmapCacheBitmapFactory::AudioRendererBitmapCacheBitmapFactory(AudioRenderer *_renderer)
{
	assert(_renderer != 0);
	renderer = _renderer;
}


wxBitmap *AudioRendererBitmapCacheBitmapFactory::ProduceBlock(int i)
{
	(void)i;
	return new wxBitmap(renderer->cache_bitmap_width, renderer->pixel_height, 24);
}


void AudioRendererBitmapCacheBitmapFactory::DisposeBlock(wxBitmap *bmp)
{
	delete bmp;
}


size_t AudioRendererBitmapCacheBitmapFactory::GetBlockSize() const
{
	return sizeof(wxBitmap) + renderer->cache_bitmap_width * renderer->pixel_height * 3;
}




AudioRenderer::AudioRenderer()
: cache_bitmap_width(32) // arbitrary value for now
, bitmaps_normal(256, AudioRendererBitmapCacheBitmapFactory(this))
, bitmaps_selected(256, AudioRendererBitmapCacheBitmapFactory(this))
, cache_bitmap_maxsize(0)
, cache_renderer_maxsize(0)
, renderer(0)
, provider(0)
{
	// Make sure there's *some* values for those fields, and in the caches
	SetSamplesPerPixel(1);
	SetHeight(1);
}


AudioRenderer::~AudioRenderer()
{
	// Nothing to do, everything is auto-allocated
}


void AudioRenderer::SetSamplesPerPixel(int _pixel_samples)
{
	if (pixel_samples == _pixel_samples) return;

	pixel_samples = _pixel_samples;

	if (renderer)
		renderer->SetSamplesPerPixel(pixel_samples);

	ResetBlockCount();
}


void AudioRenderer::SetHeight(int _pixel_height)
{
	if (pixel_height == _pixel_height) return;

	pixel_height = _pixel_height;
	Invalidate();
}


void AudioRenderer::SetAmplitudeScale(float _amplitude_scale)
{
	if (amplitude_scale == _amplitude_scale) return;

	// A scaling of 0 or a negative scaling makes no sense
	assert(_amplitude_scale > 0);

	amplitude_scale = _amplitude_scale;

	if (renderer)
		renderer->SetAmplitudeScale(amplitude_scale);
	Invalidate();
}


void AudioRenderer::SetRenderer(AudioRendererBitmapProvider *_renderer)
{
	if (renderer == _renderer) return;

	if (renderer)
		renderer->SetProvider(0);

	renderer = _renderer;
	Invalidate();

	if (renderer)
	{
		renderer->SetProvider(provider);
		renderer->SetAmplitudeScale(amplitude_scale);
		renderer->SetSamplesPerPixel(pixel_samples);
	}
}


void AudioRenderer::SetAudioProvider(AudioProvider *_provider)
{
	if (provider == _provider) return;

	provider = _provider;
	Invalidate();

	if (renderer)
		renderer->SetProvider(provider);

	ResetBlockCount();
}


void AudioRenderer::SetCacheMaxSize(size_t max_size)
{
	// Limit the bitmap cache sizes to 16 MB hard, to avoid the risk of exhausting
	// system bitmap object resources and similar. Experimenting shows that 16 MB
	// bitmap cache should be plenty even if working with a one hour audio clip.
	cache_bitmap_maxsize = std::min(max_size/8, (size_t)0x1000000);
	// The renderer gets whatever is left.
	cache_renderer_maxsize = max_size - 2*cache_bitmap_maxsize;
}


void AudioRenderer::ResetBlockCount()
{
	if (provider)
	{
		size_t rendered_width = (size_t)((provider->GetNumSamples() + pixel_samples - 1) / pixel_samples);
		cache_numblocks = rendered_width / cache_bitmap_width;
		bitmaps_normal.SetBlockCount(cache_numblocks);
		bitmaps_selected.SetBlockCount(cache_numblocks);
	}
}


wxBitmap AudioRenderer::GetCachedBitmap(int i, bool selected)
{
	assert(provider);
	assert(renderer);

	// Pick the cache to use
	AudioRendererBitmapCache *cache = selected ? &bitmaps_selected : &bitmaps_normal;

	bool created = false;
	wxBitmap *bmp = cache->Get(i, &created);
	assert(bmp);
	if (created)
	{
		renderer->Render(*bmp, i*cache_bitmap_width, selected);
	}

	assert(bmp->IsOk());
	return *bmp;
}


void AudioRenderer::Render(wxDC &dc, wxPoint origin, int start, int length, bool selected)
{
	assert(start >= 0);

	if (!provider) return;
	if (!renderer) return;
	if (length <= 0) return;

	// One past last absolute pixel strip to render
	int end = start + length;
	// One past last X coordinate to render on
	int lastx = origin.x + length;
	// Figure out which range of bitmaps are required
	int firstbitmap = start / cache_bitmap_width;
	// And the offset in it to start its use at
	int firstbitmapoffset = start % cache_bitmap_width;
	// The last bitmap required
	int lastbitmap = end / cache_bitmap_width;
	// How many columns of the last bitmap to use
	int lastbitmapoffset = end % cache_bitmap_width;

	// Check if we need to render any blank audio past the last bitmap from cache,
	// this happens if we're asked to render more audio than the provider has.
	if (lastbitmap >= (int)cache_numblocks)
	{
		lastbitmap = cache_numblocks - 1;
		lastbitmapoffset = cache_bitmap_width;

		if (firstbitmap > lastbitmap)
			firstbitmap = lastbitmap;
	}

	// Three basic cases now:
	//  * Either we're just rendering blank audio,
	//  * Or there is exactly one bitmap to render,
	//  * Or there is more than one bitmap to render.

	// origin is passed by value because we'll be using it as a local var to keep track
	// of rendering progress!

	if (start / cache_bitmap_width >= (int)cache_numblocks)
	{
		// Do nothing, the blank audio rendering will happen later
	}
	else if (firstbitmap == lastbitmap)
	{
		const int renderwidth = lastbitmapoffset - firstbitmapoffset;
		wxBitmap bmp = GetCachedBitmap(firstbitmap, selected);
		wxMemoryDC bmpdc(bmp);
		dc.Blit(origin, wxSize(renderwidth, pixel_height), &bmpdc, wxPoint(firstbitmapoffset, 0));
		origin.x += renderwidth;
	}
	else
	{
		wxBitmap bmp;

		{
			bmp = GetCachedBitmap(firstbitmap, selected);
			// Can't use dc.DrawBitmap here because we need to clip the bitmap
			wxMemoryDC bmpdc(bmp);
			dc.Blit(origin, wxSize(cache_bitmap_width-firstbitmapoffset, pixel_height),
				&bmpdc, wxPoint(firstbitmapoffset, 0));
			origin.x += cache_bitmap_width-firstbitmapoffset;
		}

		for (int i = firstbitmap+1; i < lastbitmap; ++i)
		{
			bmp = GetCachedBitmap(i, selected);
			dc.DrawBitmap(bmp, origin);
			origin.x += cache_bitmap_width;
		}

		{
			bmp = GetCachedBitmap(lastbitmap, selected);
			// We also need clipping here
			wxMemoryDC bmpdc(bmp);
			dc.Blit(origin, wxSize(lastbitmapoffset+1, pixel_height), &bmpdc, wxPoint(0, 0));
			origin.x += lastbitmapoffset+1;
		}
	}

	// Now render blank audio from origin to end
	if (origin.x < lastx)
	{
		renderer->RenderBlank(dc, wxRect(origin.x-1, origin.y, lastx-origin.x+1, pixel_height), selected);
	}

	if (selected)
		bitmaps_selected.Age(cache_bitmap_maxsize);
	else
		bitmaps_normal.Age(cache_bitmap_maxsize);
	renderer->AgeCache(cache_renderer_maxsize);
}


void AudioRenderer::Invalidate()
{
	bitmaps_normal.Age(0);
	bitmaps_selected.Age(0);
}




void AudioRendererBitmapProvider::SetProvider(AudioProvider *_provider)
{
	if (provider == _provider) return;

	provider = _provider;

	OnSetProvider();
}


void AudioRendererBitmapProvider::SetSamplesPerPixel(int _pixel_samples)
{
	if (pixel_samples == _pixel_samples) return;

	pixel_samples = _pixel_samples;
	
	OnSetSamplesPerPixel();
}


void AudioRendererBitmapProvider::SetAmplitudeScale(float _amplitude_scale)
{
	if (amplitude_scale == _amplitude_scale) return;

	amplitude_scale = _amplitude_scale;

	OnSetAmplitudeScale();
}

