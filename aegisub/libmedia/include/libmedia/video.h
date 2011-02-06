// Copyright (c) 2006-2008, Rodrigo Braz Monteiro, Fredrik Mellbin
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

/// @file video_provider.h
/// @brief Declaration of base-class for video providers
/// @ingroup main_headers video_input
///

#pragma once

#include "video_frame.h"
#include <libaegisub/exception.h>
#include <libaegisub/vfr.h>

/// @class VideoProvider
/// @brief DOCME
///
/// DOCME
class VideoProvider {
public:
	virtual ~VideoProvider() {}

	// Override this method to actually get frames
	virtual const AegiVideoFrame GetFrame(int n)=0;

	// Override the following methods to get video information:
	virtual int GetPosition() const=0;				///< Get the number of the last frame loaded
	virtual int GetFrameCount() const=0;			///< Get total number of frames
	virtual int GetWidth() const=0;					///< Returns the video width in pixels
	virtual int GetHeight() const=0;				///< Returns the video height in pixels
	virtual agi::vfr::Framerate GetFPS() const=0;	///< Get frame rate
	virtual std::vector<int> GetKeyFrames() const=0;///< Returns list of keyframes


	/// @brief Use this to set any post-loading warnings, such as "being loaded with unreliable seeking"
	virtual wxString GetWarning() const { return L""; }

	/// @brief Name of decoder, e.g. "Avisynth/FFMpegSource"
	virtual wxString GetDecoderName() const = 0;

	/// @brief Does this provider want Aegisub to cache video frames?
	/// @return Returns true if caching is desired, false otherwise.
	virtual bool WantsCaching() const { return false; }
};

DEFINE_BASE_EXCEPTION_NOINNER(VideoProviderError, agi::Exception);
/// File could be opened, but is not a supported format
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoNotSupported, VideoProviderError, "video/open/notsupported");
/// File appears to be a supported format, but could not be opened
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoOpenError, VideoProviderError, "video/open/failed");

/// Error of some sort occurred while decoding a frame
DEFINE_SIMPLE_EXCEPTION_NOINNER(VideoDecodeError, VideoProviderError, "video/error");
