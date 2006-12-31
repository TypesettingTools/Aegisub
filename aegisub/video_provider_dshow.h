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


#pragma once


///////////
// Headers
#include "setup.h"
#if USE_DIRECTSHOW == 1
#include "video_provider.h"
#pragma warning(disable: 4995)
#include <wx/wxprec.h>
#include <dshow.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>
#include "videosink.h"


///////////////////////////////////
// DirectShow Video Provider class
class DirectShowVideoProvider: public VideoProvider {
	struct DF {
	public:
	    REFERENCE_TIME  timestamp;  // DS timestamp that we used for this frame
		wxBitmap frame;

		DF() : timestamp(-1) { }
		DF(wxBitmap f) : timestamp(-1), frame(f) { }
		DF(const DF& f) { operator=(f); }
		DF& operator=(const DF& f) { timestamp = f.timestamp; frame = f.frame; return *this; }
	};

private:
	wxString subfilename;

	unsigned int last_fnum;
	unsigned int width;
	unsigned int height;
	unsigned int num_frames;
	double fps;
	long long defd;

	int depth;
	double dar;
	double zoom;

	unsigned char* data;
	wxBitmap last_frame;

	void AttachOverlay(SubtitleProvider::Overlay *overlay) {}

	HRESULT OpenVideo(wxString _filename);
	void CloseVideo();

	static void ReadFrame(long long timestamp, unsigned format, unsigned bpp, const unsigned char *frame, unsigned width, unsigned height, unsigned stride, unsigned arx, unsigned ary,	void *arg);
	bool NextFrame(DF &df,int &fn);

	void RegROT();
	void UnregROT();

	REFERENCE_TIME duration;
	DF rdf;
	CComPtr<IVideoSink>     m_pR;
	CComPtr<IMediaControl>  m_pGC;
	CComPtr<IMediaSeeking>  m_pGS;
	HANDLE                  m_hFrameReady;
	bool                    m_registered;
	DWORD                   m_rot_cookie;

public:
	DirectShowVideoProvider(wxString _filename, wxString _subfilename);
	~DirectShowVideoProvider();

	void RefreshSubtitles();
	void SetDAR(double _dar);
	void SetZoom(double _zoom);

	wxBitmap GetFrame(int n);
	void GetFloatFrame(float* Buffer, int n);

	int GetPosition() { return last_fnum; };
	int GetFrameCount() { return num_frames; };
	double GetFPS() { return fps; };

	int GetWidth() { return width; };
	int GetHeight() { return height; };
	double GetZoom() { return zoom; };

	int GetSourceWidth() { return width; };
	int GetSourceHeight() { return height; };
};

#endif
