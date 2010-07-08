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

#ifndef AGI_PRE
#include <list>
#include <memory>
#endif

#include "include/aegisub/video_provider.h"

struct CachedFrame;

/// DOCME
/// @class VideoProviderCache
/// @brief DOCME
///
/// DOCME
class VideoProviderCache : public VideoProvider {
	/// DOCME
	std::auto_ptr<VideoProvider> master;

	/// DOCME
	unsigned int cacheMax;

	/// DOCME
	std::list<CachedFrame> cache;

	void Cache(int n,const AegiVideoFrame frame);
	AegiVideoFrame GetCachedFrame(int n);

	// Cache functions
	unsigned GetCurCacheSize();

public:
	// Base methods
	const AegiVideoFrame GetFrame(int n);
	VideoProviderCache(VideoProvider *master);
	virtual ~VideoProviderCache();

	// Override the following methods:
	virtual int GetPosition() const               { return master->GetPosition(); }
	virtual int GetFrameCount() const             { return master->GetFrameCount(); }
	virtual int GetWidth() const                  { return master->GetWidth(); }
	virtual int GetHeight() const                 { return master->GetHeight(); }
	virtual agi::vfr::Framerate GetFPS() const    { return master->GetFPS(); }
	virtual std::vector<int> GetKeyFrames() const { return master->GetKeyFrames(); }
	virtual wxString GetWarning() const           { return master->GetWarning(); }
	virtual wxString GetDecoderName() const       { return master->GetDecoderName(); }


};
