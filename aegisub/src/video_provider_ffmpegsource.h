// Copyright (c) 2008-2009, Karl Blomster
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
	FFMS_VideoSource *VideoSource;			/// video source object
	const FFMS_VideoProperties *VideoInfo;	/// video properties

	int Width;					/// width in pixels
	int Height;					/// height in pixels
	int FrameNumber;			/// current framenumber
	wxArrayInt KeyFramesList;	/// list of keyframes
	bool KeyFramesLoaded;		/// keyframe loading state
	std::vector<int> TimecodesVector;	/// list of timestamps
	FrameRate Timecodes;		/// vfr object
	bool COMInited;				/// COM initialization state
	
	AegiVideoFrame CurFrame;	/// current video frame
	
	char FFMSErrMsg[1024];		/// FFMS error message
	FFMS_ErrorInfo ErrInfo;		/// FFMS error codes/messages
	wxString ErrorMsg;			/// wx-ified error message

	void LoadVideo(wxString filename);
	void Close();

public:
	FFmpegSourceVideoProvider(wxString filename);
	~FFmpegSourceVideoProvider();

	const AegiVideoFrame GetFrame(int n);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
	bool AreKeyFramesLoaded() { return KeyFramesLoaded; };
	wxArrayInt GetKeyFrames() { return KeyFramesList; };
	bool IsVFR() { return true; };
	FrameRate GetTrueFrameRate() { return Timecodes; };
	wxString GetDecoderName() { return L"FFmpegSource"; }
	int GetDesiredCacheSize() { return 8; }
};


///////////
// Factory
class FFmpegSourceVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video) { return new FFmpegSourceVideoProvider(video); }
};


#endif /* WITH_FFMPEGSOURCE */
