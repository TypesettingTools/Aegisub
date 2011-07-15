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

/// @file audio_renderer.h
/// @see audio_renderer.cpp
/// @ingroup audio_ui
///
/// Base classes for audio renderers (spectrum, waveform, ...)


#ifndef AGI_PRE
#include <memory>

#include <wx/dc.h>
#include <wx/gdicmn.h>
#endif

#include "block_cache.h"

// Some forward declarations for outside stuff
class AudioProvider;

// Forwards declarations for internal stuff
class AudioRendererBitmapProvider;
class AudioRenderer;

/// @class AudioRendererBitmapCacheBitmapFactory
/// @brief Produces wxBitmap objects for DataBlockCache storage for the audio renderer
struct AudioRendererBitmapCacheBitmapFactory {
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
	wxBitmap *ProduceBlock(int i);

	/// @brief Delete a bitmap
	/// @param bmp The bitmap to delete
	///
	/// Deletes said bitmap.
	void DisposeBlock(wxBitmap *bmp);

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

	/// Horizontal zoom level, samples per pixel
	int pixel_samples;
	/// Rendering height in pixels
	int pixel_height;
	/// Vertical zoom level/amplitude scale
	float amplitude_scale;

	/// Width of bitmaps to store in cache
	const int cache_bitmap_width;

	/// Cached bitmaps for normal audio ranges
	AudioRendererBitmapCache bitmaps_normal;
	/// Cached bitmaps for marked (selected) audio ranges
	AudioRendererBitmapCache bitmaps_selected;
	/// Number of blocks in the bitmap caches
	size_t cache_numblocks;
	/// The maximum allowed size of each bitmap cache, in bytes
	size_t cache_bitmap_maxsize;
	/// The maximum allowed size of the renderer's cache, in bytes
	size_t cache_renderer_maxsize;

	/// Actual renderer for bitmaps
	AudioRendererBitmapProvider *renderer;

	/// Audio provider to use as source
	AudioProvider *provider;

	/// @brief Make sure bitmap index i is in cache
	/// @param i        Index of bitmap to get into cache
	/// @param selected Whether to get a "selected" state bitmap or not
	/// @return The requested bitmap
	///
	/// Will attempt retrieving the requested bitmap from the cache, creating it
	/// if the cache doesn't have it.
	wxBitmap GetCachedBitmap(int i, bool selected);

	/// @brief Update the block count in the bitmap caches
	///
	/// Should be called when the width of the virtual bitmap has changed, i.e.
	/// when the samples-per-pixel resolution or the number of audio samples
	/// has changed.
	void ResetBlockCount();

public:
	/// @brief Constructor
	///
	/// Initialises audio rendering to a do-nothing state. An audio provider and bitmap
	/// provider must be set before the audio renderer is functional.
	AudioRenderer();

	/// @brief Destructor
	///
	/// Only destroys internal data, audio provider and bitmap providers are
	/// owned by the consumer of audio rendering.
	~AudioRenderer();

	/// @brief Set horizontal zoom
	/// @param pixel_samples Audio samples per pixel to render at
	///
	/// Changing the zoom level invalidates all cached bitmaps.
	void SetSamplesPerPixel(int pixel_samples);

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

	/// @brief Get horizontal zoom
	/// @return Audio samples per pixel rendering at
	int GetSamplesPerPixel() const { return pixel_samples; }

	/// @brief Get rendering height
	/// @return Height in pixels rendering at
	int GetHeight() const { return pixel_height; }

	/// @brief Get vertical zoom
	/// @return The amplitude scaling factor
	float GetAmplitudeScale() const { return amplitude_scale; }

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
	///
	/// The old renderer will have its audio provider reset to 0 and the new renderer will
	/// have its audio provider set to the current. This is done in part to ensure that
	/// the renderers have any internal caches cleared.
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
	void SetAudioProvider(AudioProvider *provider);

	/// @brief Render audio to a device context
	/// @param dc       The device context to draw to
	/// @param origin   Top left corner to render at, in the DC's coordinates
	/// @param start    First pixel from beginning of the audio stream to render
	/// @param length   Number of pixels of audio to render
	/// @param selected Whether to render the audio as being selected or not
	///
	/// The first audio sample rendered is start*pixel_samples, and the number
	/// of audio samples rendered is length*pixel_samples.
	void Render(wxDC &dc, wxPoint origin, int start, int length, bool selected);

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
	AudioProvider *provider;
	/// Horizontal zoom in samples per pixel
	int pixel_samples;
	/// Vertical zoom/amplitude scale factor
	float amplitude_scale;

	/// @brief Called when the audio provider changes
	///
	/// Implementations can override this method to do something when the audio provider is changed
	virtual void OnSetProvider() { }

	/// @brief Called when horizontal zoom changes
	///
	/// Implementations can override this method to do something when the horizontal zoom is changed
	virtual void OnSetSamplesPerPixel() { }

	/// @brief Called when vertical zoom changes
	///
	/// Implementations can override this method to do something when the vertical zoom is changed
	virtual void OnSetAmplitudeScale() { }

public:
	/// @brief Constructor
	AudioRendererBitmapProvider() : provider(0), pixel_samples(0) { };

	/// @brief Destructor
	virtual ~AudioRendererBitmapProvider() { }

	/// @brief Rendering function
	/// @param bmp      Bitmap to render to
	/// @param start    First pixel from beginning of the audio stream to render
	/// @param selected Whether to render the audio as being selected or not
	///
	/// Deriving classes must implement this method. The bitmap in bmp holds
	/// the width and height to render.
	virtual void Render(wxBitmap &bmp, int start, bool selected) = 0;

	/// @brief Blank audio rendering function
	/// @param dc       The device context to render to
	/// @param rect     The rectangle to fill with the image of blank audio
	/// @param selected Whether to render as being selected or not
	///
	/// Deriving classes must implement this method. The rectangle has the height
	/// of the entire canvas the audio is being rendered in.
	virtual void RenderBlank(wxDC &dc, const wxRect &rect, bool selected) = 0;

	/// @brief Change audio provider
	/// @param provider Audio provider to change to
	void SetProvider(AudioProvider *provider);

	/// @brief Change horizontal zoom
	/// @param pixel_samples Samples per pixel to zoom to
	void SetSamplesPerPixel(int pixel_samples);

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
