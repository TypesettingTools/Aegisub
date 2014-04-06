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

/// @file subtitles_provider_csri.cpp
/// @brief Wrapper for CSRI-based subtitle renderers
/// @ingroup subtitle_rendering
///

#include "config.h"

#ifdef WITH_CSRI
#include "subtitles_provider_csri.h"

#include "options.h"
#include "subtitle_format.h"
#include "video_frame.h"

#include <libaegisub/fs.h>
#include <libaegisub/path.h>

#include <boost/filesystem.hpp>
#include <mutex>

#ifdef WIN32
#define CSRIAPI
#include "../../contrib/csri/include/csri/csri.h"
#else
#include <csri/csri.h>
#endif

// CSRI renderers are not required to be thread safe (and VSFilter very much
// is not)
static std::mutex csri_mutex;

CSRISubtitlesProvider::CSRISubtitlesProvider(std::string type)
: instance(nullptr, csri_close)
{
	std::lock_guard<std::mutex> lock(csri_mutex);

	for (csri_rend *cur = csri_renderer_default(); cur; cur = csri_renderer_next(cur)) {
		if (type == csri_renderer_info(cur)->name) {
			renderer = cur;
			break;
		}
	}

	if (!renderer)
		throw agi::InternalError("CSRI renderer vanished between initial list and creation?", 0);
}

CSRISubtitlesProvider::~CSRISubtitlesProvider() {
	agi::fs::Remove(tempfile);
}

void CSRISubtitlesProvider::LoadSubtitles(AssFile *subs) {
	if (tempfile.empty())
		tempfile = unique_path(config::path->Decode("?temp/csri-%%%%-%%%%-%%%%-%%%%.ass"));
	SubtitleFormat::GetWriter(tempfile)->WriteFile(subs, tempfile, "utf-8");

	std::lock_guard<std::mutex> lock(csri_mutex);
	instance = csri_open_file(renderer, tempfile.string().c_str(), nullptr);
}

void CSRISubtitlesProvider::DrawSubtitles(VideoFrame &dst,double time) {
	if (!instance) return;

	csri_frame frame;
	if (dst.flipped) {
		frame.planes[0] = dst.data.data() + (dst.height-1) * dst.width * 4;
		frame.strides[0] = -(signed)dst.width * 4;
	}
	else {
		frame.planes[0] = dst.data.data();
		frame.strides[0] = dst.width * 4;
	}
	frame.pixfmt = CSRI_F_BGR_;

	csri_fmt format = { frame.pixfmt, dst.width, dst.height };

	std::lock_guard<std::mutex> lock(csri_mutex);
	if (!csri_request_fmt(instance, &format))
		csri_render(instance, &frame, time);
}

std::vector<std::string> CSRISubtitlesProvider::GetSubTypes() {
	std::vector<std::string> final;
	for (csri_rend *cur = csri_renderer_default(); cur; cur = csri_renderer_next(cur)) {
		std::string name(csri_renderer_info(cur)->name);
		if (name.find("aegisub") != name.npos)
			final.insert(final.begin(), name);
		else
			final.push_back(name);
	}
	return final;
}

#endif // WITH_CSRI
