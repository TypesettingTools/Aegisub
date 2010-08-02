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

#include "ass_file.h"
#include "subtitles_provider_csri.h"
#include "text_file_writer.h"
#include "video_context.h"

/// @brief Constructor 
/// @param type 
///
CSRISubtitlesProvider::CSRISubtitlesProvider(std::string type) : subType(type) {
}

/// @brief Destructor 
///
CSRISubtitlesProvider::~CSRISubtitlesProvider() {
	if (!tempfile.empty()) wxRemoveFile(tempfile);
}

/// @brief Load subtitles 
/// @param subs 
///
void CSRISubtitlesProvider::LoadSubtitles(AssFile *subs) {
	// CSRI variables
	csri_rend *cur,*renderer=NULL;

	// Select renderer
	bool canOpenMem = true;
	for (cur = csri_renderer_default();cur;cur=csri_renderer_next(cur)) {
		std::string name(csri_renderer_info(cur)->name);
		if (name == subType) {
			renderer = cur;
			if (name.find("vsfilter") != name.npos) canOpenMem = false;
			break;
		}
	}

	// Matching renderer not found, fallback to default
	if (!renderer) {
		renderer = csri_renderer_default();
		if (!renderer) {
			throw _T("No CSRI renderer available, cannot show subtitles. Try installing one or switch to another subtitle provider.");
		}
	}

	// Open from memory
	if (canOpenMem) {
		std::vector<char> data;
		subs->SaveMemory(data,wxSTRING_ENCODING);
		instance.reset(csri_open_mem(renderer,&data[0],data.size(),NULL), &csri_close);
	}

	// Open from disk
	else {
		if (tempfile.empty()) {
			tempfile = wxFileName::CreateTempFileName(_T("aegisub"));
			wxRemoveFile(tempfile);
			tempfile += L".ass";
		}
		subs->Save(tempfile,false,false,wxSTRING_ENCODING);
		instance.reset(csri_open_file(renderer,tempfile.utf8_str(),NULL), &csri_close);
	}
}

/// @brief Draw subtitles 
/// @param dst  
/// @param time 
/// @return 
///
void CSRISubtitlesProvider::DrawSubtitles(AegiVideoFrame &dst,double time) {
	// Check if CSRI loaded properly
	if (!instance.get()) return;

	// Load data into frame
	csri_frame frame;
	for (int i=0;i<4;i++) {
		if (dst.flipped) {
			frame.planes[i] = dst.data[i] + (dst.h-1) * dst.pitch[i];
			frame.strides[i] = -(signed)dst.pitch[i];
		}
		else {
			frame.planes[i] = dst.data[i];
			frame.strides[i] = dst.pitch[i];
		}
	}
	switch (dst.format) {
		case FORMAT_RGB32: frame.pixfmt = CSRI_F_BGR_; break;
		case FORMAT_RGB24: frame.pixfmt = CSRI_F_BGR; break;
		default: frame.pixfmt = CSRI_F_BGR_;
	}

	// Set format
	csri_fmt format;
	format.width = dst.w;
	format.height = dst.h;
	format.pixfmt = frame.pixfmt;
	int error = csri_request_fmt(instance.get(),&format);
	if (error) return;

	// Render
	csri_render(instance.get(),&frame,time);
}

/// @brief Get CSRI subtypes 
///
std::vector<std::string> CSRISubtitlesProvider::GetSubTypes() {
	std::vector<std::string> final;
	for (csri_rend *cur = csri_renderer_default();cur;cur = csri_renderer_next(cur)) {
		final.push_back(csri_renderer_info(cur)->name);
	}
	return final;
}

#endif // WITH_CSRI
