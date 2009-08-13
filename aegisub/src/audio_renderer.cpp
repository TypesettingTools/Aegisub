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

/// @file audio_renderer.cpp
/// @brief Base classes for audio renderers (spectrum, waveform, ...)
/// @ingroup audio_ui


#include "audio_renderer.h"
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <map>


/// @class AudioRendererBitmapCache
/// @brief Caches bitmaps of rendered audio at a specific resolution
///
/// Used internally by the AudioRenderer class.
class AudioRendererBitmapCache {
	/// Width of cached bitmaps
	int width;
	/// Height of cached bitmaps
	int height;

	/// Memory size of each cached image, for cache management
	size_t bytes_per_image;

	// Cached bitmaps
	// Intentionally not doxygened, too internal
	struct CachedBitmap {
		// The bitmap itself
		wxBitmap bmp;
		// "Timestamp" of last access (not necessarily related to any clocks)
		int last_access;

		CachedBitmap(wxBitmap bmp = wxBitmap()) : bmp(bmp), last_access(0) { }
		~CachedBitmap() { }

		bool operator < (const CachedBitmap &other) const { return last_access < other.last_access; }
	};

	typedef std::map<int, CachedBitmap> CacheType;
	CacheType cache;

public:
	/// @brief Constructor
	///
	/// Does nothing.
	AudioRendererBitmapCache();

	/// @brief Destructor
	///
	/// Does nothing.
	~AudioRendererBitmapCache();

	/// @brief Age the cache, purging old items
	/// @param max_size Maximum allowed cache size, in bytes
	///
	/// Purges the least recently used bitmaps to bring the cache size below max_size.
	/// Aging is a somewhat expensive operation and should only be done once in a while.
	///
	/// If max_size is specified to 0, the cache will be entirely cleared in a fast manner.
	void Age(size_t max_size);

	/// @brief Change the size of cached images
	/// @param new_width  New width of images to cache
	/// @param new_height New height of images to cache
	///
	/// Clears and re-initialises the cache.
	void Resize(int new_width, int new_height);

	/// @brief Retrieve an image from cache
	/// @param bmp       [out] Bitmap to return the image in
	/// @param key       Key to request the image for
	/// @param timestamp Timestamp to use for cache age management
	/// @return Returns false if the image had to be created in cache; if the return is
	///         false, the consumer must draw into the returned bitmap.
	///
	/// Example:
	/// @code
	///     wxBitmap bmp;
	///     if (!cache->Get(bmp, key, timestamp))
	///         RenderBitmap(bmp);
	///     // Use bmp
	/// @endcode
	///
	/// The timestamp passed should never decrease between calls to the same cache.
	///
	/// The key has no inherent meaning to the cache.
	bool Get(wxBitmap &bmp, int key, int timestamp);
};




AudioRendererBitmapCache::AudioRendererBitmapCache()
{
	Resize(0, 0);
}


AudioRendererBitmapCache::~AudioRendererBitmapCache()
{
	// Nothing to do
}


void AudioRendererBitmapCache::Age(size_t max_size)
{
	if (max_size == 0)
	{
		cache.clear();
		return;
	}

	/// @todo Make this faster than O(n^2)

	size_t max_items = max_size / bytes_per_image;
	
	while (cache.size() > max_items)
	{
		// Find the oldest item and remove it
		CacheType::iterator next = cache.begin();
		CacheType::iterator oldest = next;
		while (++next != cache.end())
		{
			if (next->second.last_access < oldest->second.last_access)
				oldest = next;
		}
		cache.erase(oldest);
	}
}


void AudioRendererBitmapCache::Resize(int new_width, int new_height)
{
	Age(0);

	width = new_width;
	height = new_height;

	// Assuming 32 bpp
	// This probably isn't completely accurate, but just a reasonable  approximation
	bytes_per_image = sizeof(CachedBitmap) + width*height*4;
}


bool AudioRendererBitmapCache::Get(wxBitmap &bmp, int key, int timestamp)
{
	CacheType::iterator item = cache.find(key);
	bool found = true;

	if (item == cache.end())
	{
		cache[key] = CachedBitmap(wxBitmap(width, height, 32));
		item = cache.find(key);
		assert(item != cache.end());
		found = false;
	}

	item->second.last_access = timestamp;
	bmp = item->second.bmp;

	return found;
}




AudioRenderer::AudioRenderer()
: cache_bitmap_width(32) // arbitrary value for now
, bitmaps_normal(new AudioRendererBitmapCache)
, bitmaps_selected(new AudioRendererBitmapCache)
, cache_clock(0)
, cache_maxsize(0)
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
	pixel_samples = _pixel_samples;
}


void AudioRenderer::SetHeight(int _pixel_height)
{
	if (pixel_height == _pixel_height) return;

	pixel_height = _pixel_height;
	bitmaps_normal->Resize(cache_bitmap_width, pixel_height);
	bitmaps_selected->Resize(cache_bitmap_width, pixel_height);
}


void AudioRenderer::SetRenderer(AudioRendererBitmapProvider *_renderer)
{
	if (renderer == _renderer) return;

	if (renderer)
		renderer->SetProvider(0);

	renderer = _renderer;
	bitmaps_normal->Age(0);
	bitmaps_selected->Age(0);

	if (renderer)
		renderer->SetProvider(provider);
}


void AudioRenderer::SetAudioProvider(AudioProvider *_provider)
{
	if (provider == _provider) return;

	provider = _provider;
	bitmaps_normal->Age(0);
	bitmaps_selected->Age(0);

	if (renderer)
		renderer->SetProvider(provider);
}


wxBitmap AudioRenderer::GetCachedBitmap(int i, bool selected)
{
	assert(provider);
	assert(renderer);

	// Pick the cache to use
	AudioRendererBitmapCache *cache = (selected ? bitmaps_selected : bitmaps_normal).get();

	wxBitmap bmp;
	if (!cache->Get(bmp, i, cache_clock))
	{
		renderer->Render(bmp, i*cache_bitmap_width, selected);
	}

	assert(bmp.IsOk());
	return bmp;
}


void AudioRenderer::Render(wxDC &dc, wxPoint origin, int start, int length, bool selected)
{
	assert(start >= 0);
	assert(length >= 0);

	assert(start >= 0);

	if (!provider) return;
	if (!renderer) return;

	// Last absolute pixel strip to render
	int end = start + length - 1;
	// Figure out which range of bitmaps are required
	int firstbitmap = start / cache_bitmap_width;
	// And the offset in it to start its use at
	int firstbitmapoffset = start % cache_bitmap_width;
	// The last bitmap required
	int lastbitmap = end / cache_bitmap_width;
	// How many columns of the last bitmap to use
	int lastbitmapoffset = end % cache_bitmap_width;

	// Two basic cases now: Either firstbitmap is the same as lastbitmap, or they're different.

	// origin is passed by value because we'll be using it as a local var to keep track
	// of rendering progress!

	if (firstbitmap == lastbitmap)
	{
		// These better be the same: The first to the last column of the single bitmap
		// to use should equal the length of the area to render.
		assert(lastbitmapoffset - firstbitmapoffset == length);

		wxBitmap bmp = GetCachedBitmap(firstbitmap, selected);
		wxMemoryDC bmpdc(bmp);
		dc.Blit(origin, wxSize(length, pixel_height), &bmpdc, wxPoint(firstbitmapoffset, 0));
	}
	else
	{
		wxBitmap bmp;

		{
			bmp = GetCachedBitmap(firstbitmap, selected);
			wxMemoryDC bmpdc(bmp);
			dc.Blit(origin, wxSize(cache_bitmap_width-firstbitmapoffset, pixel_height),
				&bmpdc, wxPoint(firstbitmapoffset, 0));
			origin.x += cache_bitmap_width-firstbitmapoffset;
		}

		for (int i = 1; i < lastbitmap; ++i)
		{
			bmp = GetCachedBitmap(i, selected);
			wxMemoryDC bmpdc(bmp);
			dc.Blit(origin, wxSize(cache_bitmap_width, pixel_height), &bmpdc, wxPoint(0, 0));
			origin.x += cache_bitmap_width;
		}

		{
			bmp = GetCachedBitmap(lastbitmap, selected);
			wxMemoryDC bmpdc(bmp);
			dc.Blit(origin, wxSize(lastbitmapoffset, pixel_height), &bmpdc, wxPoint(0, 0));
		}
	}

	if (selected)
		bitmaps_selected->Age(cache_maxsize);
	else
		bitmaps_normal->Age(cache_maxsize);
}

