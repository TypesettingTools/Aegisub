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

#include <boost/container/list.hpp>

#include "include/aegisub/video_provider.h"

struct CachedFrame;

/// @class VideoProviderCache
/// @brief A wrapper around a video provider which provides LRU caching
class VideoProviderCache : public VideoProvider {
	/// The source provider to get frames from
	std::unique_ptr<VideoProvider> master;

	/// @brief Maximum size of the cache in bytes
	///
	/// Note that this is a soft limit. The cache stops allocating new frames
	/// once it has exceeded the limit, but it never tries to shrink
	const size_t max_cache_size;

	/// Cache of video frames with the most recently used ones at the front
	boost::container::list<CachedFrame> cache;

public:
	VideoProviderCache(std::unique_ptr<VideoProvider> master);
	~VideoProviderCache();

	std::shared_ptr<VideoFrame> GetFrame(int n);

	int GetFrameCount() const             { return master->GetFrameCount(); }
	int GetWidth() const                  { return master->GetWidth(); }
	int GetHeight() const                 { return master->GetHeight(); }
	double GetDAR() const                 { return master->GetDAR(); }
	agi::vfr::Framerate GetFPS() const    { return master->GetFPS(); }
	std::vector<int> GetKeyFrames() const { return master->GetKeyFrames(); }
	std::string GetWarning() const        { return master->GetWarning(); }
	std::string GetDecoderName() const    { return master->GetDecoderName(); }
	std::string GetColorSpace() const     { return master->GetColorSpace(); }
};
