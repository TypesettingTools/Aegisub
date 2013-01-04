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

/// @file video_provider_avs.h
/// @see video_provider_avs.cpp
/// @ingroup video_input
///

#ifdef WITH_AVISYNTH
#include "include/aegisub/video_provider.h"

#include "avisynth.h"
#include "avisynth_wrap.h"
#include "video_frame.h"

class AvisynthVideoProvider: public VideoProvider {
	AviSynthWrapper avs;
	AegiVideoFrame iframe;
	std::string decoderName;
	agi::vfr::Framerate fps;
	std::vector<int> KeyFrames;
	std::string warning;
	std::string colorspace;

	PClip RGB32Video;
	VideoInfo vi;

	int last_fnum;

	AVSValue Open(agi::fs::path const& filename);

public:
	AvisynthVideoProvider(agi::fs::path const& filename);
	~AvisynthVideoProvider();

	const AegiVideoFrame GetFrame(int n);

	int GetFrameCount() const { return vi.num_frames; };
	agi::vfr::Framerate GetFPS() const { return fps; };
	int GetWidth() const { return vi.width; };
	int GetHeight() const { return vi.height; };
	double GetDAR() const { return 0; }
	std::vector<int> GetKeyFrames() const { return KeyFrames; };
	std::string GetWarning() const { return warning; }
	std::string GetDecoderName() const { return decoderName; }
	std::string GetColorSpace() const { return colorspace; }
};
#endif
