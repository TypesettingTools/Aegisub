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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_provider_ffmpegsource.h
/// @see video_provider_ffmpegsource.cpp
/// @ingroup video_input ffms
///

///////////
// Headers
#ifdef WITH_FFMPEGSOURCE
#ifndef AGI_PRE
#include <vector>
#endif

#include "ffmpegsource_common.h"
#include "include/aegisub/video_provider.h"
#include "vfr.h"


/// DOCME
/// @class FFmpegSourceVideoProvider
/// @brief DOCME
///
/// DOCME
class FFmpegSourceVideoProvider : public VideoProvider, FFmpegSourceProvider {
private:

	/// DOCME
	FFVideo *VideoSource;

	/// DOCME
	const FFVideoProperties *VideoInfo;


	/// DOCME
	int FrameNumber;

	/// DOCME
	wxArrayInt KeyFramesList;

	/// DOCME
	bool KeyFramesLoaded;

	/// DOCME
	std::vector<int> TimecodesVector;

	/// DOCME
	FrameRate Timecodes;


	/// DOCME
	AegiVideoFrame CurFrame;


	/// DOCME
	char FFMSErrMsg[1024];

	/// DOCME
	unsigned MsgSize;

	/// DOCME
	wxString ErrorMsg;


	/// DOCME
	bool COMInited;

	void LoadVideo(wxString filename);
	void Close();

protected:

public:
	FFmpegSourceVideoProvider(wxString filename);
	~FFmpegSourceVideoProvider();

	const AegiVideoFrame GetFrame(int n);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();

	/// @brief DOCME
	/// @return 
	///
	bool AreKeyFramesLoaded() { return KeyFramesLoaded; };

	/// @brief DOCME
	/// @return 
	///
	wxArrayInt GetKeyFrames() { return KeyFramesList; };

	/// @brief DOCME
	/// @return 
	///
	bool IsVFR() { return true; };

	/// @brief DOCME
	/// @return 
	///
	FrameRate GetTrueFrameRate() { return Timecodes; };

	/// @brief DOCME
	/// @return 
	///
	wxString GetDecoderName() { return L"FFmpegSource"; }

	/// @brief DOCME
	/// @return 
	///
	int GetDesiredCacheSize() { return 8; }
};



/// DOCME
/// @class FFmpegSourceVideoProviderFactory
/// @brief DOCME
///
/// DOCME
class FFmpegSourceVideoProviderFactory : public VideoProviderFactory {
public:

	/// @brief DOCME
	/// @param video 
	///
	VideoProvider *CreateProvider(wxString video) { return new FFmpegSourceVideoProvider(video); }
};


#endif /* WITH_FFMPEGSOURCE */


