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

/// @file video_provider_cache.h
/// @see video_provider_cache.cpp
/// @ingroup video_input
///

#include <boost/container/list.hpp>

#include "include/aegisub/video_provider.h"

#include <libaegisub/scoped_ptr.h>

struct CachedFrame;

/// @class VideoProviderCache
/// @brief A wrapper around a video provider which provides LRU caching
class VideoProviderCache : public VideoProvider {
	/// The source provider to get frames from
	agi::scoped_ptr<VideoProvider> master;

	/// @brief Maximum size of the cache in bytes
	///
	/// Note that this is a soft limit. The cache stops allocating new frames
	/// once it has exceeded the limit, but it never tries to shrink
	const size_t max_cache_size;

	/// Cache of video frames with the most recently used ones at the front
	boost::container::list<CachedFrame> cache;

public:
	VideoProviderCache(VideoProvider *master);
	~VideoProviderCache();

	const AegiVideoFrame GetFrame(int n);

	int GetFrameCount() const             { return master->GetFrameCount(); }
	int GetWidth() const                  { return master->GetWidth(); }
	int GetHeight() const                 { return master->GetHeight(); }
	double GetDAR() const                 { return master->GetDAR(); }
	agi::vfr::Framerate GetFPS() const    { return master->GetFPS(); }
	std::vector<int> GetKeyFrames() const { return master->GetKeyFrames(); }
	wxString GetWarning() const           { return master->GetWarning(); }
	wxString GetDecoderName() const       { return master->GetDecoderName(); }
	wxString GetColorSpace() const        { return master->GetColorSpace(); }
};
