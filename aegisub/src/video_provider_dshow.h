// Copyright (c) 2006, Rodrigo Braz Monteiro, Mike Matsnev
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

#ifdef WITH_DIRECTSHOW

#pragma warning(disable: 4995)
#include <wx/wxprec.h>
#ifdef __WINDOWS__
#include <dshow.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <windows.h>
#include <tchar.h>
#include <initguid.h>
#include "include/aegisub/video_provider.h"
#include "videosink.h"
#include "vfr.h"
#include "vfw_wrap.h"
#include "mkv_wrap.h"


///////////////////////////////////
// DirectShow Video Provider class
class DirectShowVideoProvider: public VideoProvider {
	struct DF {
	public:
	    REFERENCE_TIME  timestamp;  // DS timestamp that we used for this frame
		AegiVideoFrame frame;

		DF() : timestamp(-1) { }
		DF(AegiVideoFrame f) : timestamp(-1), frame(f) { }
		DF(const DF& f) { operator=(f); }
		DF& operator=(const DF& f) { timestamp = f.timestamp; frame = f.frame; return *this; }
	};

private:
	Aegisub::IntArray frameTime;

	unsigned int last_fnum;
	unsigned int width;
	unsigned int height;
	unsigned int num_frames;
	double fps;
	__int64 defd;

	wxArrayInt KeyFrames;
	bool keyFramesLoaded;
	bool isVfr;
	FrameRate trueFrameRate;

	HRESULT OpenVideo(wxString _filename);
	void CloseVideo();

	static void ReadFrame(__int64 timestamp, unsigned format, unsigned bpp, const unsigned char *frame, unsigned width, unsigned height, int stride, unsigned arx, unsigned ary,	void *arg);
	int NextFrame(DF &df,int &fn);

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
	DirectShowVideoProvider(Aegisub::String _filename);
	~DirectShowVideoProvider();

	void RefreshSubtitles();

	const AegiVideoFrame GetFrame(int n, int formatMask);
	void GetFloatFrame(float* Buffer, int n);

	int GetPosition() { return last_fnum; };
	int GetFrameCount() { return num_frames; };
	double GetFPS() { return fps; };
	int GetWidth() { return width; };
	int GetHeight() { return height; };
	bool AreKeyFramesLoaded() { return keyFramesLoaded; };
	wxArrayInt GetKeyFrames() { return KeyFrames; };
	bool IsVFR() { return isVfr; };
	FrameRate GetTrueFrameRate() { return isVfr ? trueFrameRate : FrameRate(); };

	Aegisub::String GetDecoderName() { return L"DirectShow"; }
	bool IsNativelyByFrames() { return false; }

	void OverrideFrameTimeList(Aegisub::IntArray list);
	int GetDesiredCacheSize() { return 8; }
};



///////////
// Factory
class DirectShowVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(Aegisub::String video) { return new DirectShowVideoProvider(video); }
};

#endif
#endif
