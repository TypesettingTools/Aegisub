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

#pragma once

#include <memory>
#include <vector>

#include <wx/gdicmn.h>

#include "audio_rendering_style.h"
#include "block_cache.h"

class AudioRenderer;
class AudioRendererBitmapProvider;
class wxDC;
namespace agi { class AudioProvider; }

/// @class AudioRendererBitmapCacheBitmapFactory
/// @brief Produces wxBitmap objects for DataBlockCache storage for the audio renderer
struct AudioRendererBitmapCacheBitmapFactory {
	typedef std::unique_ptr<wxBitmap> BlockType;

	/// The audio renderer we're producing bitmaps for
	AudioRenderer *renderer;

	/// @brief Constructor
	/// @param renderer The audio renderer to produce bitmaps for
	AudioRendererBitmapCacheBitmapFactory(AudioRenderer *renderer);

	/// @brief Create a new bitmap
	/// @param i Unused
	/// @return A fresh wxBitmap
	///
	/// Produces a wxBitmap with dimensions pulled from our master AudioRenderer.
	std::unique_ptr<wxBitmap> ProduceBlock(int i);

	/// @brief Calculate the size of bitmaps
	/// @return The size of bitmaps created
	size_t GetBlockSize() const;
};

/// The type of a bitmap cache
typedef DataBlockCache<wxBitmap, 8, AudioRendererBitmapCacheBitmapFactory> AudioRendererBitmapCache;


/// @class AudioRenderer
/// @brief Renders audio to bitmap images for display on screen
///
/// Manages a bitmap cache and paints to device contexts.
///
/// To implement a new audio renderer, see AudioRendererBitmapProvider.
class AudioRenderer {
	friend struct AudioRendererBitmapCacheBitmapFactory;

	/// Horizontal zoom level, milliseconds per pixel
	double pixel_ms = 0.f;
	/// Rendering height in pixels
	int pixel_height = 0;
	/// Vertical zoom level/amplitude scale
	float amplitude_scale = 0.f;

	/// Width of bitmaps to store in cache
	const int cache_bitmap_width = 32; // Completely arbitrary value

	/// Cached bitmaps for audio ranges
	std::vector<AudioRendererBitmapCache> bitmaps;
	/// The maximum allowed size of each bitmap cache, in bytes
	size_t cache_bitmap_maxsize = 0;
	/// The maximum allowed size of the renderer's cache, in bytes
	size_t cache_renderer_maxsize = 0;
	/// Do the caches need to be aged?
	bool needs_age = false;

	/// Actual renderer for bitmaps
	AudioRendererBitmapProvider *renderer = nullptr;

	/// Audio provider to use as source
	agi::AudioProvider *provider = nullptr;

	/// @brief Make sure bitmap index i is in cache
	/// @param i     Index of bitmap to get into cache
	/// @param style Rendering style required for bitmap
	/// @return The requested bitmap
	///
	/// Will attempt retrieving the requested bitmap from the cache, creating it
	/// if the cache doesn't have it.
	wxBitmap const& GetCachedBitmap(int i, AudioRenderingStyle style);

	/// @brief Update the block count in the bitmap caches
	///
	/// Should be called when the width of the virtual bitmap has changed, i.e.
	/// when the samples-per-pixel resolution or the number of audio samples
	/// has changed.
	void ResetBlockCount();

	/// Calculate the number of cache blocks needed for a given number of samples
	size_t NumBlocks(int64_t samples) const;

public:
	/// @brief Constructor
	///
	/// Initialises audio rendering to a do-nothing state. An audio provider
	/// and bitmap provider must be set before the audio renderer is functional.
	AudioRenderer();

	/// @brief Set horizontal zoom
	/// @param pixel_ms Milliseconds per pixel to render audio at
	///
	/// Changing the zoom level invalidates all cached bitmaps.
	void SetMillisecondsPerPixel(double pixel_ms);

	/// @brief Set rendering height
	/// @param pixel_height Height in pixels to render at
	///
	/// Changing the rendering height invalidates all cached bitmaps.
	void SetHeight(int pixel_height);

	/// @brief Set vertical zoom
	/// @param amplitude_scale Scaling factor
	///
	/// Changing the scaling factor invalidates all cached bitmaps.
	///
	/// A scaling factor of 1.0 is no scaling, a factor of 0.5 causes the audio to be
	/// rendered as if it had half its actual amplitude, a factor of 2 causes the audio
	/// to be rendered as if it had double amplitude. (The exact meaning of the scaling
	/// depends on the bitmap provider used.)
	void SetAmplitudeScale(float amplitude_scale);

	/// @brief Set the maximum allowed cache size
	/// @param max_size Size in bytes that may be used for caching
	///
	/// The given max size is not a hard limit and does generally not include overhead
	/// added by the cache management. The allowed size might be distributed among
	/// several separate objects.
	///
	/// Changing the max cache size does not trigger cache aging.
	void SetCacheMaxSize(size_t max_size);

	/// @brief Change renderer
	/// @param renderer New renderer to use
	///
	/// The consumer of audio rendering is responsible for creating, managing and destroying
	/// audio bitmap providers (renderers). If a renderer was previously set with this function
	/// and a new one is set, the consumer of audio rendering is still responsible for the
	/// life of the old renderer.
	///
	/// A bitmap provider must be assigned to a newly created audio renderer before it
	/// can be functional.
	///
	/// Changing renderer invalidates all cached bitmaps.
	void SetRenderer(AudioRendererBitmapProvider *renderer);

	/// @brief Change audio provider
	/// @param provider New audio provider to use
	///
	/// The consumer of audio rendering is responsible for managing audio providers.
	/// If an audio provider was previously assigned to the audio renderer and a
	/// new one is assigned, the consumer of audio rendering is still responsible for
	/// the life of the old audio provider.
	///
	/// An audio provider must be assigned to a newly created audio renderer before it
	/// can be functional.
	///
	/// Changing audio provider invalidates all cached bitmaps.
	///
	/// If a renderer is set, this will also set the audio provider for the renderer.
	void SetAudioProvider(agi::AudioProvider *provider);

	/// @brief Render audio to a device context
	/// @param dc       The device context to draw to
	/// @param origin   Top left corner to render at, in the DC's coordinates
	/// @param start    First pixel from beginning of the audio stream to render
	/// @param length   Number of pixels of audio to render
	/// @param style    Style to render audio in
	///
	/// The first audio sample rendered is start*pixel_samples, and the number
	/// of audio samples rendered is length*pixel_samples.
	void Render(wxDC &dc, wxPoint origin, int start, int length, AudioRenderingStyle style);

	/// @brief Invalidate all cached data
	///
	/// Invalidates all cached bitmaps for another reason, usually as a signal that
	/// implementation-defined data in the bitmap provider have been changed.
	///
	/// If the consumer of audio rendering changes properties of the bitmap renderer
	/// that will affect the rendered images, it should call this function to ensure
	/// the cache is kept consistent.
	void Invalidate();
};


/// @class AudioRendererBitmapProvider
/// @brief Base class for audio renderer implementations
///
/// Derive from this class to implement a way to render audio to images.
class AudioRendererBitmapProvider {
protected:
	/// Audio provider to use for rendering
	agi::AudioProvider *provider;
	/// Horizontal zoom in milliseconds per pixel
	double pixel_ms;
	/// Vertical zoom/amplitude scale factor
	float amplitude_scale;

	/// @brief Called when the audio provider changes
	///
	/// Implementations can override this method to do something when the audio provider is changed
	virtual void OnSetProvider() { }

	/// @brief Called when horizontal zoom changes
	///
	/// Implementations can override this method to do something when the horizontal zoom is changed
	virtual void OnSetMillisecondsPerPixel() { }

	/// @brief Called when vertical zoom changes
	///
	/// Implementations can override this method to do something when the vertical zoom is changed
	virtual void OnSetAmplitudeScale() { }

public:
	/// @brief Constructor
	AudioRendererBitmapProvider() : provider(nullptr), pixel_ms(0), amplitude_scale(0) { };

	/// @brief Destructor
	virtual ~AudioRendererBitmapProvider() = default;

	/// @brief Rendering function
	/// @param bmp   Bitmap to render to
	/// @param start First pixel from beginning of the audio stream to render
	/// @param style Style to render audio in
	///
	/// Deriving classes must implement this method. The bitmap in bmp holds
	/// the width and height to render.
	virtual void Render(wxBitmap &bmp, int start, AudioRenderingStyle style) = 0;

	/// @brief Blank audio rendering function
	/// @param dc    The device context to render to
	/// @param rect  The rectangle to fill with the image of blank audio
	/// @param style Style to render audio in
	///
	/// Deriving classes must implement this method. The rectangle has the height
	/// of the entire canvas the audio is being rendered in.
	virtual void RenderBlank(wxDC &dc, const wxRect &rect, AudioRenderingStyle style) = 0;

	/// @brief Change audio provider
	/// @param provider Audio provider to change to
	void SetProvider(agi::AudioProvider *provider);

	/// @brief Change horizontal zoom
	/// @param new_pixel_ms Milliseconds per pixel to zoom to
	void SetMillisecondsPerPixel(double new_pixel_ms);

	/// @brief Change vertical zoom
	/// @param amplitude_scale Scaling factor to zoom to
	void SetAmplitudeScale(float amplitude_scale);

	/// @brief Age any caches the renderer might keep
	/// @param max_size Maximum size in bytes the caches should be
	///
	/// Deriving classes should override this method if they implement any
	/// kind of caching.
	virtual void AgeCache(size_t max_size) { }
};
