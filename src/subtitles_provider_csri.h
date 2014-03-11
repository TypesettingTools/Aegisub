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

/// @file subtitles_provider_csri.h
/// @see subtitles_provider_csri.cpp
/// @ingroup subtitle_rendering
///

#ifdef WITH_CSRI
#include "include/aegisub/subtitles_provider.h"

#include <vector>

#include <libaegisub/fs_fwd.h>
#include <libaegisub/scoped_ptr.h>

#include <boost/filesystem/path.hpp>

typedef void csri_rend;
typedef void csri_inst;

class CSRISubtitlesProvider : public SubtitlesProvider {
	agi::scoped_holder<csri_inst*> instance;
	csri_rend *renderer;

	/// Name of the file passed to renderers with can_open_mem false
	agi::fs::path tempfile;
public:
	CSRISubtitlesProvider(std::string subType);
	~CSRISubtitlesProvider();

	void LoadSubtitles(AssFile *subs);
	void DrawSubtitles(VideoFrame &dst, double time);

	static std::vector<std::string> GetSubTypes();
};
#endif
