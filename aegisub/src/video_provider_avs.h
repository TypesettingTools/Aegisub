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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file video_provider_avs.h
/// @see video_provider_avs.cpp
/// @ingroup video_input
///


///////////
// Headers
#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#include "include/aegisub/video_provider.h"


/// DOCME
/// @class AvisynthVideoProvider
/// @brief DOCME
///
/// DOCME
class AvisynthVideoProvider: public VideoProvider, AviSynthWrapper {
private:

	/// DOCME
	VideoInfo vi;

	/// DOCME
	AegiVideoFrame iframe;


	/// DOCME
	bool usedDirectShow;

	/// DOCME
	wxString rendererCallString;

	/// DOCME
	wxString decoderName;


	/// DOCME
	int num_frames;

	/// DOCME
	int last_fnum;


	/// DOCME
	double fps;

	/// DOCME
	wxArrayInt frameTime;

	/// DOCME
	bool byFrame;


	/// DOCME
	wxArrayInt KeyFrames;

	/// DOCME
	bool keyFramesLoaded;

	/// DOCME
	bool isVfr;

	/// DOCME
	FrameRate trueFrameRate;


	/// DOCME
	PClip RGB32Video;

	PClip OpenVideo(wxString _filename, bool mpeg2dec3_priority = true);

public:
	AvisynthVideoProvider(wxString _filename);
	~AvisynthVideoProvider();

	const AegiVideoFrame GetFrame(int n);

	/// @brief // properties
	/// @return 
	///
	int GetPosition() { return last_fnum; };

	/// @brief DOCME
	/// @return 
	///
	int GetFrameCount() { return num_frames? num_frames: vi.num_frames; };

	/// @brief DOCME
	/// @return 
	///
	double GetFPS() { return (double)vi.fps_numerator/(double)vi.fps_denominator; };

	/// @brief DOCME
	/// @return 
	///
	int GetWidth() { return vi.width; };

	/// @brief DOCME
	/// @return 
	///
	int GetHeight() { return vi.height; };

	/// @brief DOCME
	/// @return 
	///
	bool AreKeyFramesLoaded() { return keyFramesLoaded; };

	/// @brief DOCME
	/// @return 
	///
	wxArrayInt GetKeyFrames() { return KeyFrames; };

	/// @brief DOCME
	/// @return 
	///
	bool IsVFR() { return isVfr; };

	/// @brief DOCME
	/// @return 
	///
	FrameRate GetTrueFrameRate() { return isVfr? trueFrameRate: FrameRate(); };

	void OverrideFrameTimeList(wxArrayInt list);

	/// @brief DOCME
	/// @return 
	///
	bool IsNativelyByFrames() { return byFrame; }

	/// @brief DOCME
	/// @return 
	///
	bool NeedsVFRHack() { return true; }
	wxString GetWarning();

	/// @brief DOCME
	/// @return 
	///
	wxString GetDecoderName() { return wxString(L"Avisynth/") + decoderName; }
};



/// DOCME
/// @class AvisynthVideoProviderFactory
/// @brief DOCME
///
/// DOCME
class AvisynthVideoProviderFactory : public VideoProviderFactory {
public:

	/// @brief DOCME
	/// @param video 
	///
	VideoProvider *CreateProvider(wxString video) { return new AvisynthVideoProvider(video); }
};


#endif


