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

#ifdef WITH_AVISYNTH
#include "avisynth_wrap.h"
#include "include/aegisub/video_provider.h"

/// DOCME
/// @class AvisynthVideoProvider
/// @brief DOCME
///
/// DOCME
class AvisynthVideoProvider: public VideoProvider, AviSynthWrapper {
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
	agi::vfr::Framerate real_fps;
	agi::vfr::Framerate vfr_fps;

	/// DOCME
	std::vector<int> KeyFrames;

	/// DOCME
	PClip RGB32Video;

	PClip OpenVideo(wxString _filename, bool mpeg2dec3_priority = true);

public:
	AvisynthVideoProvider(wxString _filename);
	~AvisynthVideoProvider();

	const AegiVideoFrame GetFrame(int n);

	int GetPosition() const { return last_fnum; };
	int GetFrameCount() const { return num_frames? num_frames: vi.num_frames; };
	agi::vfr::Framerate GetFPS() const { return vfr_fps.IsLoaded() ? vfr_fps : real_fps; };
	int GetWidth() const { return vi.width; };
	int GetHeight() const { return vi.height; };
	std::vector<int> GetKeyFrames() const { return KeyFrames; };
	wxString GetWarning() const;
	wxString GetDecoderName() const { return wxString(L"Avisynth/") + decoderName; }
};
#endif
