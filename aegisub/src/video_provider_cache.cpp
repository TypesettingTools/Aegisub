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

/// @file video_provider_cache.cpp
/// @brief Aggregate video provider caching previously requested frames
/// @ingroup video_input
///

#include "config.h"

#include "video_provider_cache.h"

#include "options.h"
#include "video_frame.h"

#include <algorithm>
#include <functional>

/// A video frame and its frame number
struct CachedFrame : public AegiVideoFrame {
	int frame_number;
};

VideoProviderCache::VideoProviderCache(VideoProvider *parent)
: master(parent)
, max_cache_size(OPT_GET("Provider/Video/Cache/Size")->GetInt() << 20) // convert MB to bytes
{
}

VideoProviderCache::~VideoProviderCache() {
	for_each(cache.begin(), cache.end(), std::mem_fn(&AegiVideoFrame::Clear));
}

const AegiVideoFrame VideoProviderCache::GetFrame(int n) {
	size_t total_size = 0;

	// See if frame is cached
	for (auto cur = cache.begin(); cur != cache.end(); ++cur) {
		if (cur->frame_number == n) {
			cache.push_front(*cur);
			cache.erase(cur);
			return cache.front();
		}

		total_size += cur->memSize;
	}

	// Not cached, retrieve it
	const AegiVideoFrame frame = master->GetFrame(n);

	// Cache full, use oldest frame
	if (total_size >= max_cache_size) {
		cache.push_front(cache.back());
		cache.pop_back();
	}
	// Cache not full, insert new one
	else
		cache.push_front(CachedFrame());

	// Cache
	cache.front().frame_number = n;
	cache.front().CopyFrom(frame);
	return cache.front();
}
