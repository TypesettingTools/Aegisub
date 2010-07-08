// Copyright (c) 2008, Rodrigo Braz Monteiro
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

/// @file video_provider_cache.cpp
/// @brief Aggregate video provider caching previously requested frames
/// @ingroup video_input
///

#include "config.h"
#include "main.h"
#include "options.h"

#include "video_provider_cache.h"

/// DOCME
/// @class CachedFrame
/// @brief DOCME
///
/// DOCME
struct CachedFrame {
	/// DOCME
	AegiVideoFrame frame;

	/// DOCME
	int n;
};

/// @brief Constructor 
/// @param parent 
///
VideoProviderCache::VideoProviderCache(VideoProvider *parent)
: master(parent)
, cacheMax(OPT_GET("Provider/Video/Cache/Size")->GetInt() << 20) // convert MB to bytes
{
}

/// @brief Destructor 
///
VideoProviderCache::~VideoProviderCache() {
	while (cache.size()) {
		cache.front().frame.Clear();
		cache.pop_front();
	}
}

/// @brief Get frame 
/// @param n 
/// @return 
///
const AegiVideoFrame VideoProviderCache::GetFrame(int n) {
	// See if frame is cached
	CachedFrame cached;
	for (std::list<CachedFrame>::iterator cur=cache.begin();cur!=cache.end();cur++) {
		cached = *cur;
		if (cached.n == n) {
			cache.erase(cur);
			cache.push_back(cached);
			return cached.frame;
		}
	}

	// Not cached, retrieve it
	const AegiVideoFrame frame = master->GetFrame(n);
	const AegiVideoFrame *srcFrame = &frame;

	// Cache frame
	Cache(n,*srcFrame);
	return *srcFrame;
}

/// @brief Add to cache 
/// @param n     
/// @param frame 
void VideoProviderCache::Cache(int n,const AegiVideoFrame frame) {
	// Cache full, use frame at front
	if (GetCurCacheSize() >= cacheMax) {
		cache.push_back(cache.front());
		cache.pop_front();
	}

	// Cache not full, insert new one
	else {
		cache.push_back(CachedFrame());
	}

	// Cache
	cache.back().n = n;
	cache.back().frame.CopyFrom(frame);
}

/// @brief Get the current size of the cache
/// @return Returns the size in bytes
unsigned VideoProviderCache::GetCurCacheSize() {
	int sz = 0;
	for (std::list<CachedFrame>::iterator i = cache.begin(); i != cache.end(); i++)
		sz += i->frame.memSize;
	return sz;
}
