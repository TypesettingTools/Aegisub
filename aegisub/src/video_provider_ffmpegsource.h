// Copyright (c) 2008, Karl Blomster
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
#ifdef WITH_FFMPEGSOURCE
#include "include/aegisub/video_provider.h"
#include "ffmpegsource_common.h"
#include "vfr.h"
#include <vector>


///////////////////////
// FFmpegSource video provider
class FFmpegSourceVideoProvider : public VideoProvider, FFmpegSourceProvider {
private:
	VideoBase *VideoSource;
	const VideoProperties *VideoInfo;
	FrameIndex *Index;

	int FrameNumber;
	wxArrayInt KeyFramesList;
	bool KeyFramesLoaded;
	std::vector<int> TimecodesVector;
	FrameRate Timecodes;

	int DstFormat;
	int LastDstFormat;
	AegiVideoFrame CurFrame;

	char FFMSErrorMessage[1024];
	unsigned MessageSize;
	wxString ErrorMsg;

	void LoadVideo(Aegisub::String filename, double fps);
	void Close();

protected:

public:
	FFmpegSourceVideoProvider(Aegisub::String filename, double fps);
	~FFmpegSourceVideoProvider();

	const AegiVideoFrame GetFrame(int n, int formatType);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
	bool AreKeyFramesLoaded() { return KeyFramesLoaded; };
	wxArrayInt GetKeyFrames() { return KeyFramesList; };
	bool IsVFR() { return true; };
	FrameRate GetTrueFrameRate() { return Timecodes; };
	Aegisub::String GetDecoderName() { return L"FFmpegSource"; }
	bool IsNativelyByFrames() { return true; }
	int GetDesiredCacheSize() { return 8; }
};


///////////
// Factory
class FFmpegSourceVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(Aegisub::String video,double fps=0.0) { return new FFmpegSourceVideoProvider(video,fps); }
};


#endif /* WITH_FFMPEGSOURCE */
