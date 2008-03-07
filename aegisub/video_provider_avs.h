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


///////////
// Headers
#include <wx/wxprec.h>
#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#include "include/aegisub/video_provider.h"
#include "include/aegisub/subtitles_provider.h"


////////////
// Provider
class AvisynthVideoProvider: public VideoProvider, SubtitlesProvider, AviSynthWrapper {
private:
	VideoInfo vi;
	AegiVideoFrame iframe;

	bool usedDirectShow;
	wxString rendererCallString;
	wxString decoderName;

	int num_frames;
	int last_fnum;

	double fps;
	wxArrayInt frameTime;
	bool byFrame;

	PClip RGB32Video;
	PClip SubtitledVideo;

	PClip OpenVideo(wxString _filename, bool mpeg2dec3_priority = true);
	PClip ApplySubtitles(wxString _filename, PClip videosource);

	void LoadVSFilter();
	void LoadASA();
	void LoadRenderer();

public:
	AvisynthVideoProvider(wxString _filename, double fps=0.0);
	~AvisynthVideoProvider();

	SubtitlesProvider *GetAsSubtitlesProvider();
	void LoadSubtitles(AssFile *subs);
	bool LockedToVideo() { return true; }

	const AegiVideoFrame GetFrame(int n,int formatMask);
	void GetFloatFrame(float* Buffer, int n);

	// properties
	int GetPosition() { return last_fnum; };
	int GetFrameCount() { return num_frames? num_frames: vi.num_frames; };
	double GetFPS() { return (double)vi.fps_numerator/(double)vi.fps_denominator; };
	int GetWidth() { return vi.width; };
	int GetHeight() { return vi.height; };

	void OverrideFrameTimeList(wxArrayInt list);
	bool IsNativelyByFrames() { return byFrame; }
	wxString GetWarning();
	wxString GetDecoderName() { return _T("Avisynth/") + decoderName; }
};


///////////
// Factory
class AvisynthVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video,double fps=0.0) { return new AvisynthVideoProvider(video,fps); }
};


#endif
