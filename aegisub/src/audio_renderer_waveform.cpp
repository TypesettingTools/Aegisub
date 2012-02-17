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

/// @file audio_renderer_waveform.cpp
/// @ingroup audio_ui
///
/// Render a waveform display of PCM audio data

#include "config.h"

#include "audio_renderer_waveform.h"

#ifndef AGI_PRE
#include <algorithm>

#include <wx/dcmemory.h>
#endif

#include "audio_colorscheme.h"
#include "block_cache.h"
#include "colorspace.h"
#include "include/aegisub/audio_provider.h"
#include "main.h"

enum {
	/// Only render the peaks
	Waveform_MaxOnly = 0,
	/// Render the peaks and averages
	Waveform_MaxAvg,
	Waveform_Continuous
};

AudioWaveformRenderer::AudioWaveformRenderer(std::string const& color_scheme_name)
: colors_normal(new AudioColorScheme(6, color_scheme_name, AudioStyle_Normal))
, colors_primary(new AudioColorScheme(6, color_scheme_name, AudioStyle_Primary))
, colors_inactive(new AudioColorScheme(6, color_scheme_name, AudioStyle_Inactive))
, audio_buffer(0)
, render_averages(OPT_GET("Audio/Display/Waveform Style")->GetInt() == Waveform_MaxAvg)
{
}


AudioWaveformRenderer::~AudioWaveformRenderer()
{
	delete[] audio_buffer;
}


void AudioWaveformRenderer::Render(wxBitmap &bmp, int start, AudioRenderingStyle style)
{
	wxMemoryDC dc(bmp);
	wxRect rect(wxPoint(0, 0), bmp.GetSize());
	int midpoint = rect.height / 2;

	const AudioColorScheme *pal = GetColorScheme(style);

	int pixel_samples = (pixel_ms * provider->GetSampleRate() + 500) / 1000;

	// Fill the background
	dc.SetBrush(wxBrush(pal->get(0.0f)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(rect);

	// Make sure we've got a buffer to fill with audio data
	if (!audio_buffer)
	{
		// Buffer for one pixel strip of audio
		size_t buffer_needed = pixel_samples * provider->GetChannels() * provider->GetSampleRate() * provider->GetBytesPerSample();
		audio_buffer = new char[buffer_needed];
	}

	int64_t cur_sample = start * pixel_samples;

	assert(provider->GetBytesPerSample() == 2);
	assert(provider->GetChannels() == 1);

	wxPen pen_peaks(wxPen(pal->get(0.4f)));
	wxPen pen_avgs(wxPen(pal->get(0.7f)));

	for (int x = 0; x < rect.width; ++x)
	{
		provider->GetAudio(audio_buffer, cur_sample, pixel_samples);
		cur_sample += pixel_samples;
		
		int peak_min = 0, peak_max = 0;
		int64_t avg_min_accum = 0, avg_max_accum = 0;
		const int16_t *aud = (const int16_t *)audio_buffer;
		for (int si = pixel_samples; si > 0; --si, ++aud)
		{
			if (*aud > 0)
			{
				peak_max = std::max(peak_max, (int)*aud);
				avg_max_accum += *aud;
			}
			else
			{
				peak_min = std::min(peak_min, (int)*aud);
				avg_min_accum += *aud;
			}
		}

		// midpoint is half height
		peak_min = std::max((int)(peak_min * amplitude_scale * midpoint) / 0x8000, -midpoint);
		peak_max = std::min((int)(peak_max * amplitude_scale * midpoint) / 0x8000, midpoint);
		int avg_min = std::max((int)(avg_min_accum * amplitude_scale * midpoint / pixel_samples) / 0x8000, -midpoint);
		int avg_max = std::min((int)(avg_max_accum * amplitude_scale * midpoint / pixel_samples) / 0x8000, midpoint);

		dc.SetPen(pen_peaks);
		dc.DrawLine(x, midpoint - peak_max, x, midpoint - peak_min);
		if (render_averages) {
			dc.SetPen(pen_avgs);
			dc.DrawLine(x, midpoint - avg_max, x, midpoint - avg_min);
		}
	}

	// Horizontal zero-point line
	if (render_averages)
		dc.SetPen(wxPen(pal->get(1.0f)));
	else
		dc.SetPen(pen_peaks);

	dc.DrawLine(0, midpoint, rect.width, midpoint);
}


void AudioWaveformRenderer::RenderBlank(wxDC &dc, const wxRect &rect, AudioRenderingStyle style)
{
	const AudioColorScheme *pal = GetColorScheme(style);
	wxColor line(pal->get(1.0));
	wxColor bg(pal->get(0.0));

	// Draw the line as background above and below, and line in the middle, to avoid
	// overdraw flicker (the common theme in all of audio display direct drawing).
	int halfheight = rect.height / 2;

	dc.SetBrush(wxBrush(bg));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(rect.x, rect.y, rect.width, halfheight);
	dc.DrawRectangle(rect.x, rect.y + halfheight + 1, rect.width, rect.height - halfheight - 1);

	dc.SetPen(wxPen(line));
	dc.DrawLine(rect.x, rect.y+halfheight, rect.x+rect.width, rect.y+halfheight);
}

void AudioWaveformRenderer::OnSetProvider()
{
	delete[] audio_buffer;
	audio_buffer = 0;
}

void AudioWaveformRenderer::OnSetMillisecondsPerPixel()
{
	delete[] audio_buffer;
	audio_buffer = 0;
}

const AudioColorScheme *AudioWaveformRenderer::GetColorScheme(AudioRenderingStyle style) const
{
	switch (style)
	{
		case AudioStyle_Primary: return colors_primary.get();
		case AudioStyle_Inactive: return colors_inactive.get();
		default: return colors_normal.get();
	}
}

wxArrayString AudioWaveformRenderer::GetWaveformStyles() {
	wxArrayString ret;
	ret.push_back(_("Maximum"));
	ret.push_back(_("Maximum + Average"));
	return ret;
}
