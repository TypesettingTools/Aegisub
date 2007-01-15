// Copyright (c) 2006, Fredrik Mellbin
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
#include "avisynth_wrap.h"
#include "video_provider.h"

/*class GetFrameVPThread: public wxThread {
private:
	int getting_n;
	int current_n;

	PClip video;

	wxThread::ExitCode Entry();
public:
	void GetFrame(int n);
	GetFrameVPThread(PClip clip);
};*/

class AvisynthVideoProvider: public VideoProvider, AviSynthWrapper {
private:
	VideoInfo vi;

	wxString subfilename;
	wxString rendererCallString;

	int last_fnum;
	int num_frames;

	int depth;

	unsigned char* data;
	wxBitmap last_frame;

	double dar;
	double zoom;
	double fps;
	wxArrayInt frameTime;

	PClip RGB32Video;
	PClip SubtitledVideo;
	PClip ResizedVideo;

	PClip OpenVideo(wxString _filename, bool mpeg2dec3_priority = true);
	PClip ApplySubtitles(wxString _filename, PClip videosource);
	PClip ApplyDARZoom(double _zoom, double _dar, PClip videosource);
	wxBitmap GetFrame(int n, bool force);
	void LoadVSFilter();
	void LoadASA();
	void LoadRenderer();
	void AttachOverlay(SubtitleProvider::Overlay *_overlay) {}

public:
	AvisynthVideoProvider(wxString _filename, wxString _subfilename, double fps=0.0);
	~AvisynthVideoProvider();

	void RefreshSubtitles();
	void SetDAR(double _dar);
	void SetZoom(double _zoom);

	wxBitmap GetFrame(int n) { return GetFrame(n,false); };
	void GetFloatFrame(float* Buffer, int n);

	// properties
	int GetPosition() { return last_fnum; };
	int GetFrameCount() { return num_frames? num_frames: vi.num_frames; };
	double GetFPS() { return (double)vi.fps_numerator/(double)vi.fps_denominator; };

	int GetWidth() { return vi.width; };
	int GetHeight() { return vi.height; };
	double GetZoom() { return zoom; };

	int GetSourceWidth() { return RGB32Video->GetVideoInfo().width; };
	int GetSourceHeight() { return RGB32Video->GetVideoInfo().height; };

	void OverrideFrameTimeList(wxArrayInt list);
};

#endif
