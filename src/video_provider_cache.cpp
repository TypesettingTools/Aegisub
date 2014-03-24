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

#include "include/aegisub/video_provider.h"

#include "options.h"
#include "video_frame.h"

#include <libaegisub/util.h>

#include <list>

namespace {
/// A video frame and its frame number
struct CachedFrame final : public VideoFrame {
	int frame_number;

	CachedFrame(int frame_number, VideoFrame const& frame)
	: VideoFrame(frame.data.data(), frame.width, frame.height, frame.pitch, frame.flipped)
	, frame_number(frame_number)
	{
	}
};

/// @class VideoProviderCache
/// @brief A wrapper around a video provider which provides LRU caching
class VideoProviderCache final : public VideoProvider {
	/// The source provider to get frames from
	std::unique_ptr<VideoProvider> master;

	/// @brief Maximum size of the cache in bytes
	///
	/// Note that this is a soft limit. The cache stops allocating new frames
	/// once it has exceeded the limit, but it never tries to shrink
	const size_t max_cache_size;

	/// Cache of video frames with the most recently used ones at the front
	std::list<CachedFrame> cache;

public:
	VideoProviderCache(std::unique_ptr<VideoProvider> master)
	: master(std::move(master))
	, max_cache_size(OPT_GET("Provider/Video/Cache/Size")->GetInt() << 20) // convert MB to bytes
	{
	}

	std::shared_ptr<VideoFrame> GetFrame(int n) override;

	int GetFrameCount() const override             { return master->GetFrameCount(); }
	int GetWidth() const override                  { return master->GetWidth(); }
	int GetHeight() const override                 { return master->GetHeight(); }
	double GetDAR() const override                 { return master->GetDAR(); }
	agi::vfr::Framerate GetFPS() const override    { return master->GetFPS(); }
	std::vector<int> GetKeyFrames() const override { return master->GetKeyFrames(); }
	std::string GetWarning() const override        { return master->GetWarning(); }
	std::string GetDecoderName() const override    { return master->GetDecoderName(); }
	std::string GetColorSpace() const override     { return master->GetColorSpace(); }
};

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
}

std::unique_ptr<VideoProvider> CreateCacheVideoProvider(std::unique_ptr<VideoProvider> parent) {
	return agi::util::make_unique<VideoProviderCache>(std::move(parent));
}
