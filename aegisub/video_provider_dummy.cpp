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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "video_provider_dummy.h"
#include "colorspace.h"
#include <wx/tokenzr.h>


///////////
// Factory
// Shouldn't be needed
/*class DummyVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video,double fps=0.0) { return new DummyVideoProvider(video,fps); }
	DummyVideoProviderFactory() : VideoProviderFactory(_T("dummy")) {}
} registerDummyVideo; */


///////////////
// Constructor
void DummyVideoProvider::Create(double _fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	lastFrame = -1;
	framecount = frames;
	fps = _fps;
	width = _width;
	height = _height;

	frame = AegiVideoFrame(width,height,FORMAT_RGB32);
	unsigned char *dst = frame.data[0];
	unsigned char r = colour.Red(), g = colour.Green(), b = colour.Blue();

	unsigned char h, s, l, lr, lg, lb; // light variants
	rgb_to_hsl(r, g, b, &h, &s, &l);
	l = 255 - (255 - l) / 2;
	hsl_to_rgb(h, s, l, &lr, &lg, &lb);

	if (pattern) {
		int ppitch = frame.pitch[0] / frame.GetBpp();
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
		for (int i=frame.pitch[0]*frame.h/frame.GetBpp();--i>=0;) {
			*dst++ = b;
			*dst++ = g;
			*dst++ = r;
			*dst++ = 0;
		}
	}
}


///////////////////////
// Parsing constructor
DummyVideoProvider::DummyVideoProvider(Aegisub::String _filename, double _fps)
{
	wxString filename = _filename;
	wxString params;
	if (!filename.StartsWith(_T("?dummy:"), &params)) {
		throw _T("Attempted creating dummy video provider with non-dummy filename");
	}

	wxStringTokenizer t(params, _T(":"));
	if (t.CountTokens() < 7) {
		throw _T("Too few fields in dummy video parameter list");
	}

	double parsedfps;
	long _frames, _width, _height, red, green, blue;
	bool pattern = false;

	wxString field = t.GetNextToken();
	if (!field.ToDouble(&parsedfps)) {
		throw _T("Unable to parse fps field in dummy video parameter list");
	}
	if (_fps == 0.0)
		_fps = parsedfps;

	field = t.GetNextToken();
	if (!field.ToLong(&_frames)) {
		throw _T("Unable to parse framecount field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&_width)) {
		throw _T("Unable to parse width field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&_height)) {
		throw _T("Unable to parse height field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&red)) {
		throw _T("Unable to parse red colour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&green)) {
		throw _T("Unable to parse green colour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (!field.ToLong(&blue)) {
		throw _T("Unable to parse bluecolour field in dummy video parameter list");
	}

	field = t.GetNextToken();
	if (field == _T("c")) {
		pattern = true;
	}

	Create(_fps, _frames, _width, _height, wxColour(red, green, blue), pattern);
}


//////////////////////
// Direct constructor
DummyVideoProvider::DummyVideoProvider(double _fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	Create(_fps, frames, _width, _height, colour, pattern);
}


//////////////
// Destructor
DummyVideoProvider::~DummyVideoProvider() {
}


//////////////////////////////////////////////////
// Construct a fake filename describing the video
wxString DummyVideoProvider::MakeFilename(double fps, int frames, int _width, int _height, const wxColour &colour, bool pattern) {
	return wxString::Format(_T("?dummy:%f:%d:%d:%d:%d:%d:%d:%s"), fps, frames, _width, _height, colour.Red(), colour.Green(), colour.Blue(), pattern?_T("c"):_T(""));
}


/////////////
// Get frame
const AegiVideoFrame DummyVideoProvider::GetFrame(int n,int formatMask) {
	lastFrame = n;
	return frame;
}


////////////////
// Get position
int DummyVideoProvider::GetPosition() {
	return lastFrame;
}


///////////////////
// Get frame count
int DummyVideoProvider::GetFrameCount() {
	return framecount;
}


/////////////
// Get width
int DummyVideoProvider::GetWidth() {
	return width;
}


//////////////
// Get height
int DummyVideoProvider::GetHeight() {
	return height;
}


///////////
// Get FPS
double DummyVideoProvider::GetFPS() {
	return fps;
}


////////////////////
// Get decoder name
Aegisub::String DummyVideoProvider::GetDecoderName() {
	return L"Dummy Video Provider";
}

