// Copyright (c) 2008-2009, Karl Blomster
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

/// @file ffmpegsource_common.h
/// @see ffmpegsource_common.cpp
/// @ingroup video_input audio_input ffms
///

#ifdef WITH_FFMS2
#include <map>

#include <ffms.h>

#include <libaegisub/fs.h>
#include <libaegisub/scoped_ptr.h>

namespace agi { class BackgroundRunner; }

/// @class FFmpegSourceProvider
/// @brief Base class for FFMS2 source providers; contains common functions etc
class FFmpegSourceProvider {
	friend class FFmpegSourceCacheCleaner;
	agi::BackgroundRunner *br;

public:
	FFmpegSourceProvider(agi::BackgroundRunner *br);

	// X11 is wonderful
#undef None

	enum class TrackSelection : int {
		None = -1,
		All = -2
	};

	void CleanCache();

	FFMS_Index *DoIndexing(FFMS_Indexer *Indexer, agi::fs::path const& Cachename,
		                   TrackSelection Track,
		                   FFMS_IndexErrorHandling IndexEH);
	std::map<int, std::string> GetTracksOfType(FFMS_Indexer *Indexer, FFMS_TrackType Type);
	TrackSelection AskForTrackSelection(const std::map<int, std::string>& TrackList, FFMS_TrackType Type);
	agi::fs::path GetCacheFilename(agi::fs::path const& filename);
	void SetLogLevel();
	FFMS_IndexErrorHandling GetErrorHandlingMode();
};

#endif /* WITH_FFMS2 */
