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
//
// $Id$

/// @file video_provider_dummy.cpp
/// @brief Video provider returning a constant frame
/// @ingroup video_input
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/tokenzr.h>
#endif

#include "colorspace.h"
#include "video_provider_dummy.h"

namespace media {

/// @brief Constructor 
/// @param _fps    
/// @param frames  
/// @param _width  
/// @param _height 
/// @param colour  
/// @param pattern 
///
void DummyVideoProvider::Create(double _fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	lastFrame = -1;
	framecount = frames;
	fps = _fps;
	width = _width;
	height = _height;

	frame = AegiVideoFrame(width,height);
	unsigned char *dst = frame.data;
	unsigned char r = colour.Red(), g = colour.Green(), b = colour.Blue();

	unsigned char h, s, l, lr, lg, lb; // light variants
	rgb_to_hsl(r, g, b, &h, &s, &l);
	l += 24;
	if (l < 24) l -= 48;
	hsl_to_rgb(h, s, l, &lr, &lg, &lb);

	if (pattern) {
		int ppitch = frame.pitch / frame.GetBpp();
		for (unsigned int y = 0; y < frame.h; ++y) {
			if ((y / 8) & 1) {
				for (int x = 0; x < ppitch; ++x) {
					if ((x / 8) & 1) {
						*dst++ = b;
						*dst++ = g;
						*dst++ = r;
						*dst++ = 0;
					}
					else {
						*dst++ = lb;
						*dst++ = lg;
						*dst++ = lr;
						*dst++ = 0;
					}
				}
			}
			else {
				for (int x = 0; x < ppitch; ++x) {
					if ((x / 8) & 1) {
						*dst++ = lb;
						*dst++ = lg;
						*dst++ = lr;
						*dst++ = 0;
					}
					else {
						*dst++ = b;
						*dst++ = g;
						*dst++ = r;
						*dst++ = 0;
					}
				}
			}
		}
	}
	else {
		for (int i=frame.pitch*frame.h/frame.GetBpp();--i>=0;) {
			*dst++ = b;
			*dst++ = g;
			*dst++ = r;
			*dst++ = 0;
		}
	}
}

/// @brief Parsing constructor 
/// @param filename 
///
DummyVideoProvider::DummyVideoProvider(wxString filename)
{
	wxString params;
	if (!filename.StartsWith(_T("?dummy:"), &params)) {
		throw agi::FileNotFoundError("Attempted creating dummy video provider with non-dummy filename");
	}

	wxStringTokenizer t(params, _T(":"));
	if (t.CountTokens() < 7) {
		throw VideoOpenError("Too few fields in dummy video parameter list");
	}

	double fps;
	long _frames, _width, _height, red, green, blue;
	bool pattern = false;

	wxString field = t.GetNextToken();
	if (!field.ToDouble(&fps)) {
		throw VideoOpenError("Unable to parse fps field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&_frames)) {
		throw VideoOpenError("Unable to parse framecount field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&_width)) {
		throw VideoOpenError("Unable to parse width field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&_height)) {
		throw VideoOpenError("Unable to parse height field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&red)) {
		throw VideoOpenError("Unable to parse red colour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&green)) {
		throw VideoOpenError("Unable to parse green colour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&blue)) {
		throw VideoOpenError("Unable to parse blue colour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (field == _T("c")) {
		pattern = true;
	}

	Create(fps, _frames, _width, _height, wxColour(red, green, blue), pattern);
}

/// @brief Direct constructor 
/// @param _fps    
/// @param frames  
/// @param _width  
/// @param _height 
/// @param colour  
/// @param pattern 
///
DummyVideoProvider::DummyVideoProvider(double _fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	Create(_fps, frames, _width, _height, colour, pattern);
}

/// @brief Destructor 
///
DummyVideoProvider::~DummyVideoProvider() {
	frame.Clear();
}

/// @brief Construct a fake filename describing the video 
/// @param fps     
/// @param frames  
/// @param _width  
/// @param _height 
/// @param colour  
/// @param pattern 
/// @return 
///
wxString DummyVideoProvider::MakeFilename(double fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	return wxString::Format(_T("?dummy:%f:%d:%d:%d:%d:%d:%d:%s"), fps, frames, _width, _height, colour.Red(), colour.Green(), colour.Blue(), pattern?_T("c"):_T(""));
}

/// @brief Get frame 
/// @param n 
/// @return 
///
const AegiVideoFrame DummyVideoProvider::GetFrame(int n) {
	lastFrame = n;
	return frame;
}

} // namespace media

