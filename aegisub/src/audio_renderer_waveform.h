// Copyright (c) 2010, Niels Martin Hansen
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

/// @file audio_renderer_waveform.h
/// @see audio_renderer_waveform.cpp
/// @ingroup audio_ui
///
/// Render a waveform display of PCM audio data

#include <stdint.h>



class AudioWaveformRenderer : public AudioRendererBitmapProvider {
	/// Colour table used for regular rendering
	AudioColorScheme colors_normal;

	/// Colour table used for rendering the audio selection
	AudioColorScheme colors_selected;

	/// Pre-allocated buffer for audio fetched from provider
	char *audio_buffer;

protected:
	virtual void OnSetProvider();
	virtual void OnSetSamplesPerPixel();

public:
	/// @brief Constructor
	AudioWaveformRenderer();

	/// @brief Destructor
	virtual ~AudioWaveformRenderer();

	/// @brief Render a range of audio waveform
	/// @param bmp      [in,out] Bitmap to render into, also carries lenght information
	/// @param start    First column of pixel data in display to render
	/// @param selected Whether to use the alternate colour scheme
	void Render(wxBitmap &bmp, int start, bool selected);

	/// @brief Render blank area
	void RenderBlank(wxDC &dc, const wxRect &rect, bool selected);

	/// @brief Cleans up the cache
	/// @param max_size Maximum size in bytes for the cache
	///
	/// Does nothing for waveform renderer, since it does not have a backend cache
	void AgeCache(size_t max_size) { }
};
