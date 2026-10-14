// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file subtitles_provider.h
/// @brief Declaration of base-class for subtitle renderers
/// @ingroup main_headers subtitle_rendering
///

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class AssFile;
class FrameStorage;
struct VideoFrame;

class SubtitlesProvider {
	std::vector<char> buffer;
	virtual void LoadSubtitles(const char *data, size_t len)=0;

public:
	virtual ~SubtitlesProvider() = default;
	void LoadSubtitles(AssFile *subs, int time = -1);
	virtual void DrawSubtitles(VideoFrame &dst, double time)=0;
	virtual void Reinitialize() { }
	/// Called when a different file is opened (or the file is closed), so
	/// that errors which are only reported once per file are reported again
	virtual void ResetReportedErrors() { }
	/// Wait until the provider is ready to draw, showing progress if needed.
	/// Must be called on the main thread. Drawing on other threads throws
	/// agi::UserCancelException if the provider isn't ready, as they can't
	/// show progress. Throws agi::UserCancelException if the user cancels.
	virtual void WaitUntilReady() { }
	/// Allocator (see FrameBuffer) for the memory of frames which subtitles
	/// will be drawn on, if the provider can draw on frames more efficiently
	/// when their memory comes from it. Anything else written to frames which
	/// subtitles have been drawn on should be treated as untrusted pixels.
	virtual std::function<std::unique_ptr<FrameStorage>(size_t)> GetFrameAllocator() { return nullptr; }
};

namespace agi { class BackgroundRunner; }

struct SubtitlesProviderFactory {
	static std::unique_ptr<SubtitlesProvider> GetProvider(agi::BackgroundRunner *br);
	static std::vector<std::string> GetClasses();
};
