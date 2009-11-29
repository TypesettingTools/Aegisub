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


//////////
// Headers
#include "aegisub.h"
#include "vfr.h"
#include "video_frame.h"


/// @class VideoProvider
/// @brief DOCME
///
/// DOCME
class VideoProvider {
public:

	/// @brief // Virtual destructor
	/// @return 
	///
	virtual ~VideoProvider() {}

	// Override this method to actually get frames
	virtual const AegiVideoFrame GetFrame(int n)=0;

	// Override the following methods to get video information:
	virtual int GetPosition()=0;				// Get the number of the last frame loaded
	virtual int GetFrameCount()=0;				// Get total number of frames
	virtual int GetWidth()=0;					// Returns the video width in pixels
	virtual int GetHeight()=0;					// Returns the video height in pixels
	virtual double GetFPS()=0;					// Get framerate in frames per second
	virtual bool AreKeyFramesLoaded()=0;		// Returns true if keyframe info is loaded, false otherwise
	virtual bool IsVFR()=0;						// Returns true if video is VFR
	virtual wxArrayInt GetKeyFrames()=0;		// Returns list of keyframes
	virtual FrameRate GetTrueFrameRate()=0;		// Returns magic VFR stuff


	/// @brief // Use this to set any post-loading warnings, such as "being loaded with unreliable seeking"
	/// @return 
	///
	virtual wxString GetWarning() { return L""; }


	/// @brief // Name of decoder, e.g. "Avisynth/FFMpegSource"
	/// @return 
	///
	virtual wxString GetDecoderName() { return L"Unknown"; }


	/// @brief Does this provider want Aegisub to cache video frames?
	/// @return Returns true if caching is desired, false otherwise.
	virtual bool WantsCaching() { return false; }


	/// @brief // For "special" providers that don't deal well with VFR (i.e. Avisynth)
	/// @return 
	///
	virtual bool NeedsVFRHack() { return false; };					// Returns true if provider needs special VFR treatment

	/// @brief DOCME
	/// @return 
	///
	virtual bool IsNativelyByFrames() { return true; };

	/// @brief DOCME
	/// @param list 
	///
	virtual void OverrideFrameTimeList(std::vector<int> list) {}	// Override the list with the provided one, for VFR handling
};



/// @class VideoProviderFactory
/// @brief DOCME
///
/// DOCME
class VideoProviderFactory {
public:

	/// @brief DOCME
	///
	virtual ~VideoProviderFactory() {}
	virtual VideoProvider *CreateProvider(wxString video)=0;
};


