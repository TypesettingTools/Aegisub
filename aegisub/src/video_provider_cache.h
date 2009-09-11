// Copyright (c) 2008, Rodrigo Braz Monteiro, Fredrik Mellbin
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

/// @file video_provider_cache.h
/// @see video_provider_cache.cpp
/// @ingroup video_input
///




//////////
// Headers
#ifndef AGI_PRE
#include <list>
#endif

#include "include/aegisub/video_provider.h"
#include "vfr.h"


/// DOCME
/// @class CachedFrame
/// @brief DOCME
///
/// DOCME
class CachedFrame {
public:

	/// DOCME
	AegiVideoFrame frame;

	/// DOCME
	int n;
};



/// DOCME
/// @class VideoProviderCache
/// @brief DOCME
///
/// DOCME
class VideoProviderCache : public VideoProvider {
private:

	/// DOCME
	VideoProvider *master;

	/// DOCME
	unsigned int cacheMax;

	/// DOCME
	std::list<CachedFrame> cache;

	/// DOCME
	int pos;

	void Cache(int n,const AegiVideoFrame frame);
	AegiVideoFrame GetCachedFrame(int n);

protected:
	// Cache functions
	void SetCacheMax(int n_frames);
	void ClearCache();

public:
	// Base methods
	void GetFloatFrame(float* Buffer, int n);	// Get frame as float
	const AegiVideoFrame GetFrame(int n);
	VideoProviderCache(VideoProvider *master);
	virtual ~VideoProviderCache();

	// Override the following methods:
	virtual int GetPosition();				// Get the number of the last frame loaded
	virtual int GetFrameCount();			// Get total number of frames
	virtual int GetWidth();					// Returns the video width in pixels
	virtual int GetHeight();				// Returns the video height in pixels
	virtual double GetFPS();				// Get framerate in frames per second
	virtual bool AreKeyFramesLoaded();
	virtual bool IsVFR();
	virtual wxArrayInt GetKeyFrames();
	virtual FrameRate GetTrueFrameRate();
	virtual void OverrideFrameTimeList(std::vector<int> list);	// Override the list with the provided one, for VFR handling
	virtual bool IsNativelyByFrames();
	virtual bool NeedsVFRHack();
	virtual wxString GetWarning();
	virtual wxString GetDecoderName();
};


