// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "config.h"

#include "video_provider_cache.h"

#include "options.h"
#include "video_frame.h"

#include <algorithm>
#include <boost/version.hpp>

#if BOOST_VERSION <= 105200
// Compilation fails without this with boost 1.52. I have no idea why.
static bool operator==(VideoFrame const& a, VideoFrame const& b) {
	return a.width == b.width
		&& a.height == b.height
		&& a.pitch == b.pitch
		&& a.flipped == b.flipped
		&& a.data == b.data;
}
#endif

/// A video frame and its frame number
struct CachedFrame : public VideoFrame {
	int frame_number;

	CachedFrame(int frame_number, VideoFrame const& frame)
	: VideoFrame(frame.data.data(), frame.width, frame.height, frame.pitch, frame.flipped)
	, frame_number(frame_number)
	{
	}
};

VideoProviderCache::VideoProviderCache(std::unique_ptr<VideoProvider> parent)
: master(std::move(parent))
, max_cache_size(OPT_GET("Provider/Video/Cache/Size")->GetInt() << 20) // convert MB to bytes
{
}

VideoProviderCache::~VideoProviderCache() {
}

std::shared_ptr<VideoFrame> VideoProviderCache::GetFrame(int n) {
	size_t total_size = 0;

	for (auto cur = cache.begin(); cur != cache.end(); ++cur) {
		if (cur->frame_number == n) {
			cache.splice(cache.begin(), cache, cur); // Move to front
			return std::make_shared<VideoFrame>(cache.front());
		}

		total_size += cur->data.size();
	}

	auto frame = master->GetFrame(n);

	if (total_size >= max_cache_size)
		cache.pop_back();
	cache.emplace_front(n, *frame);

	return frame;
}
