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


///////////
// Headers
#include "config.h"
#include "options.h"

#include "video_provider_cache.h"


/// @brief Constructor 
/// @param parent 
///
VideoProviderCache::VideoProviderCache(VideoProvider *parent) {
	master = parent;
	cacheMax = 0;
	SetCacheMax(parent->GetDesiredCacheSize());
}



/// @brief Destructor 
///
VideoProviderCache::~VideoProviderCache() {
	delete master;
	ClearCache();
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
	pos = n;
	Cache(n,*srcFrame);
	return *srcFrame;
}

/// @brief Set maximum cache size 
/// @param n 
///
void VideoProviderCache::SetCacheMax(int n) {
	if (n <= 0)
		cacheMax = 0;
	else
		cacheMax = Options.AsInt(_T("Video cache size")) << 20; // convert MB to bytes
}



/// @brief Add to cache 
/// @param n     
/// @param frame 
/// @return 
///
void VideoProviderCache::Cache(int n,const AegiVideoFrame frame) {
	// Cache enabled?
	if (cacheMax == 0) return;

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



/// @brief Clear cache 
///
void VideoProviderCache::ClearCache() {
	while (cache.size()) {
		cache.front().frame.Clear();
		cache.pop_front();
	}
}

/// @brief Get the current size of the cache
/// @return Returns the size in bytes
unsigned VideoProviderCache::GetCurCacheSize() {
	int sz = 0;
	for (std::list<CachedFrame>::iterator i = cache.begin(); i != cache.end(); i++)
		sz += i->frame.memSize;
	return sz;
}


/// @brief Wrapper methods 
/// @return 
///
int VideoProviderCache::GetPosition() {
	return pos;
}

/// @brief DOCME
/// @return 
///
int VideoProviderCache::GetFrameCount() {
	return master->GetFrameCount();
}

/// @brief DOCME
/// @return 
///
int VideoProviderCache::GetWidth() {
	return master->GetWidth();
}

/// @brief DOCME
/// @return 
///
int VideoProviderCache::GetHeight() {
	return master->GetHeight();
}

/// @brief DOCME
/// @return 
///
double VideoProviderCache::GetFPS() {
	return master->GetFPS();
}

/// @brief DOCME
/// @return 
///
bool VideoProviderCache::IsVFR() {
	return master->IsVFR();
}

/// @brief DOCME
/// @return 
///
bool VideoProviderCache::AreKeyFramesLoaded() {
	return master->AreKeyFramesLoaded();
}

/// @brief DOCME
/// @return 
///
wxArrayInt VideoProviderCache::GetKeyFrames() {
	return master->GetKeyFrames();
}

/// @brief DOCME
/// @return 
///
FrameRate VideoProviderCache::GetTrueFrameRate() {
	return master->GetTrueFrameRate();
}

/// @brief DOCME
/// @param list 
///
void VideoProviderCache::OverrideFrameTimeList(std::vector<int> list) {
	master->OverrideFrameTimeList(list);
}

/// @brief DOCME
/// @return 
///
bool VideoProviderCache::IsNativelyByFrames() {
	return master->IsNativelyByFrames();
}

/// @brief DOCME
/// @return 
///
bool VideoProviderCache::NeedsVFRHack() {
	return master->NeedsVFRHack();
}

/// @brief DOCME
/// @return 
///
wxString VideoProviderCache::GetWarning() {
	return master->GetWarning();
}

/// @brief DOCME
///
wxString VideoProviderCache::GetDecoderName() {
	return master->GetDecoderName();
}


