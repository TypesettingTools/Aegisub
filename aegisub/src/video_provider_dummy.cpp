// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file video_provider_dummy.cpp
/// @brief Video provider returning a constant frame
/// @ingroup video_input
///

#include "config.h"

#include "video_provider_dummy.h"

#ifndef AGI_PRE
#include <wx/colour.h>
#include <wx/tokenzr.h>
#endif

#include "colorspace.h"

void DummyVideoProvider::Create(double fps, int frames, int width, int height, unsigned char red, unsigned char green, unsigned char blue, bool pattern) {
	this->framecount = frames;
	this->fps = fps;
	this->width = width;
	this->height = height;
	this->frame = AegiVideoFrame(width, height);

	unsigned char *dst = frame.data;
	unsigned char colors[2][4] = {
		{ blue, green, red, 0 },
		{ 0, 0, 0, 0 }
	};

	if (pattern) {
		// Generate light version
		unsigned char h, s, l;
		rgb_to_hsl(red, blue, green, &h, &s, &l);
		l += 24;
		if (l < 24) l -= 48;
		hsl_to_rgb(h, s, l, &colors[1][2], &colors[1][1], &colors[1][0]);

		// Divide into a 8x8 grid and use light colours when row % 2 != col % 2
		int ppitch = frame.pitch / frame.GetBpp();
		for (unsigned int y = 0; y < frame.h; ++y) {
			for (int x = 0; x < ppitch; ++x) {
				memcpy(dst, colors[((y / 8) & 1) != ((x / 8) & 1)], 4);
				dst += 4;
			}
		}
	}
	else {
		for (int i = frame.pitch * frame.h / frame.GetBpp() - 1; i >= 0; --i)
			memcpy(dst + i * 4, colors[0], 4);
	}
}

static long get_long(wxStringTokenizer &t, const char *err) {
	long ret;
	if (!t.GetNextToken().ToLong(&ret))
		throw VideoOpenError(err);
	return ret;
}

DummyVideoProvider::DummyVideoProvider(wxString const& filename) {
	wxString params;
	if (!filename.StartsWith("?dummy:", &params))
		throw agi::FileNotFoundError("Attempted creating dummy video provider with non-dummy filename");

	wxStringTokenizer t(params, ":");
	if (t.CountTokens() < 7)
		throw VideoOpenError("Too few fields in dummy video parameter list");

	double fps;
	if (!t.GetNextToken().ToDouble(&fps))
		throw VideoOpenError("Unable to parse fps field in dummy video parameter list");

	long frames = get_long(t, "Unable to parse framecount field in dummy video parameter list");
	long width  = get_long(t, "Unable to parse width field in dummy video parameter list");
	long height = get_long(t, "Unable to parse height field in dummy video parameter list");
	long red    = get_long(t, "Unable to parse red colour field in dummy video parameter list");
	long green  = get_long(t, "Unable to parse green colour field in dummy video parameter list");
	long blue   = get_long(t, "Unable to parse blue colour field in dummy video parameter list");

	bool pattern = t.GetNextToken() == "c";

	Create(fps, frames, width, height, red, green, blue, pattern);
}

DummyVideoProvider::DummyVideoProvider(double fps, int frames, int width, int height, const wxColour &colour, bool pattern) {
	Create(fps, frames, width, height, colour.Red(), colour.Green(), colour.Blue(), pattern);
}

DummyVideoProvider::~DummyVideoProvider() {
	frame.Clear();
}

wxString DummyVideoProvider::MakeFilename(double fps, int frames, int width, int height, const wxColour &colour, bool pattern) {
	return wxString::Format("?dummy:%f:%d:%d:%d:%d:%d:%d:%s", fps, frames, width, height, colour.Red(), colour.Green(), colour.Blue(), pattern ? "c" : "");
}
