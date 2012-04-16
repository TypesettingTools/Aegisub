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
//
// $Id$

/// @file subtitles_provider_csri.cpp
/// @brief Wrapper for CSRI-based subtitle renderers
/// @ingroup subtitle_rendering
///

#include "config.h"

#ifdef WITH_CSRI

#ifndef AGI_PRE
#include <wx/thread.h>
#endif

#include "subtitles_provider_csri.h"

#include "ass_file.h"
#include "text_file_writer.h"
#include "video_context.h"
#include "video_frame.h"

#ifdef WIN32
#define CSRIAPI
#include "../../contrib/csri/include/csri/csri.h"
#else
#include <csri/csri.h>
#endif

static wxMutex csri_mutex;

CSRISubtitlesProvider::CSRISubtitlesProvider(std::string type)
: can_open_mem(true)
, instance(0, csri_close)
{
	wxMutexLocker lock(csri_mutex);

	for (csri_rend *cur = csri_renderer_default(); cur; cur = csri_renderer_next(cur)) {
		if (type == csri_renderer_info(cur)->name) {
			renderer = cur;
			break;
		}
	}

	if (!renderer)
		throw agi::InternalError("CSRI renderer vanished between initial list and creation?", 0);

	std::string name(csri_renderer_info(renderer)->name);
	can_open_mem = (name.find("vsfilter") == name.npos);
}

CSRISubtitlesProvider::~CSRISubtitlesProvider() {
	if (!tempfile.empty()) wxRemoveFile(tempfile);
}

void CSRISubtitlesProvider::LoadSubtitles(AssFile *subs) {
	if (can_open_mem) {
		std::vector<char> data;
		subs->SaveMemory(data);

		wxMutexLocker lock(csri_mutex);
		instance = csri_open_mem(renderer, &data[0], data.size(), NULL);
	}
	// Open from disk
	else {
		if (tempfile.empty()) {
			tempfile = wxFileName::CreateTempFileName("aegisub");
			wxRemoveFile(tempfile);
			tempfile += ".ass";
		}
		subs->Save(tempfile, false, false, wxSTRING_ENCODING);

		wxMutexLocker lock(csri_mutex);
		instance = csri_open_file(renderer, tempfile.utf8_str(), NULL);
	}
}

void CSRISubtitlesProvider::DrawSubtitles(AegiVideoFrame &dst,double time) {
	// Check if CSRI loaded properly
	if (!instance) return;

	// Load data into frame
	csri_frame frame;
	if (dst.flipped) {
		frame.planes[0] = dst.data + (dst.h-1) * dst.pitch;
		frame.strides[0] = -(signed)dst.pitch;
	}
	else {
		frame.planes[0] = dst.data;
		frame.strides[0] = dst.pitch;
	}
	frame.pixfmt = CSRI_F_BGR_;

	// Set format
	csri_fmt format;
	format.width = dst.w;
	format.height = dst.h;
	format.pixfmt = frame.pixfmt;

	wxMutexLocker lock(csri_mutex);
	int error = csri_request_fmt(instance, &format);
	if (error) return;

	// Render
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
