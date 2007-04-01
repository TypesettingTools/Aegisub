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
#include "video_provider.h"


////////////////////////
// Dummy video provider
class DummyVideoProvider : public VideoProvider {
private:
	int lastFrame;

protected:
	const AegiVideoFrame DoGetFrame(int n);

public:
	DummyVideoProvider(wxString filename, double fps);
	~DummyVideoProvider();

	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
};


///////////
// Factory
class DummyVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video,double fps=0.0) { return new DummyVideoProvider(video,fps); }
	DummyVideoProviderFactory() : VideoProviderFactory(_T("dummy")) {}
} registerDummyVideo;


///////////////
// Constructor
DummyVideoProvider::DummyVideoProvider(wxString filename, double fps) {
	lastFrame = -1;
}


//////////////
// Destructor
DummyVideoProvider::~DummyVideoProvider() {
}


/////////////
// Get frame
const AegiVideoFrame DummyVideoProvider::DoGetFrame(int n) {
	lastFrame = n;
	return AegiVideoFrame(640,480);
}


////////////////
// Get position
int DummyVideoProvider::GetPosition() {
	return lastFrame;
}


///////////////////
// Get frame count
int DummyVideoProvider::GetFrameCount() {
	return 40000;
}


/////////////
// Get width
int DummyVideoProvider::GetWidth() {
	return 640;
}


//////////////
// Get height
int DummyVideoProvider::GetHeight() {
	return 480;
}


///////////
// Get FPS
double DummyVideoProvider::GetFPS() {
	return 24.0/1.001;
}
