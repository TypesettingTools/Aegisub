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
#include <wx/wxprec.h>
#ifdef __WINDOWS__
#include "setup.h"
#if USE_DIRECTSHOW == 1
#include "video_provider.h"
#pragma warning(disable: 4995)
#include <dshow.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>
#include "videosink.h"


///////////////////////////////////
// DirectShow Video Provider class
class DirectShowVideoProvider: public VideoProvider {
private:
	wxString subfilename;

	unsigned int last_fnum;
	unsigned int width;
	unsigned int height;
	unsigned int num_frames;
	double fps;

	int depth;
	double dar;
	double zoom;

	unsigned char* data;
	wxBitmap last_frame;

	wxBitmap GetFrame(int n, bool force);
	void AttachOverlay(SubtitleProvider::Overlay *_overlay) {}
	HRESULT OpenVideo(wxString _filename);

	void RegROT();
	void UnregROT();

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

	wxBitmap GetFrame(int n) { return wxBitmap(64,64); };
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
#endif
