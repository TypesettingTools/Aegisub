// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#ifdef USE_LAVC
#define EMULATE_INTTYPES
#include <wx/wxprec.h>
#include <lavc/avcodec.h>
#include <lavc/avformat.h>
#include "video_provider_lavc.h"


///////////////
// Constructor
LAVCVideoProvider::LAVCVideoProvider(wxString filename, wxString subfilename, double zoom) {
	// Register types
	static bool avRegistered = false;
	if (!avRegistered) {
		av_register_all();
		avRegistered = true;
	}
}


//////////////
// Destructor
LAVCVideoProvider::~LAVCVideoProvider() {
}


//
//
void LAVCVideoProvider::RefreshSubtitles() {
}


//
//
wxBitmap LAVCVideoProvider::GetFrame(int n) {
	wxBitmap frame;
	return frame;
}


//
//
void LAVCVideoProvider::GetFloatFrame(float* Buffer, int n) {
}


//
//
int LAVCVideoProvider::GetPosition() {
	return 0;
}


//
//
int LAVCVideoProvider::GetFrameCount() {
	return 0;
}


//
//
double LAVCVideoProvider::GetFPS() {
	return 1;
}


//
//
void LAVCVideoProvider::SetDAR(double dar) {
}


//
//
void LAVCVideoProvider::SetZoom(double zoom) {
}


//
//
int LAVCVideoProvider::GetWidth() {
	return 640;
}


//
//
int LAVCVideoProvider::GetHeight() {
	return 480;
}


//
//
double LAVCVideoProvider::GetZoom() {
	return 1;
}


//
//
int LAVCVideoProvider::GetSourceWidth() {
	return 640;
}


//
//
int LAVCVideoProvider::GetSourceHeight() {
	return 480;
}


#endif
