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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers

#ifdef WITH_CSRI

#include <wx/wxprec.h>
#include "subtitles_provider_csri.h"
#include "ass_file.h"
#include "video_context.h"


///////////////
// Constructor
CSRISubtitlesProvider::CSRISubtitlesProvider(wxString type) {
	subType = type;
	instance = NULL;
}


//////////////
// Destructor
CSRISubtitlesProvider::~CSRISubtitlesProvider() {
	if (instance) csri_close(instance);
	instance = NULL;
}


//////////////////
// Load subtitles
void CSRISubtitlesProvider::LoadSubtitles(AssFile *subs) {
	// Close
	if (instance) csri_close(instance);
	instance = NULL;

	// CSRI variables
	csri_rend *cur,*renderer=NULL;
	csri_info *info;

	// Select renderer
	bool canOpenMem = true;
	for (cur = csri_renderer_default();cur;cur=csri_renderer_next(cur)) {
		info = csri_renderer_info(cur);
		wxString name(info->name,wxConvUTF8);
		if (name == subType) {
			renderer = cur;
			if (name.StartsWith(_T("vsfilter"))) canOpenMem = false;
			break;
		}
	}

	// Matching renderer not found, fallback to default
	if (!renderer) {
		renderer = csri_renderer_default();
		if (!renderer) {
			delete subs;
			throw _T("No CSRI renderer available, cannot show subtitles. Try installing one or switch to another subtitle provider.");
		}
	}

	// Open from memory
	if (canOpenMem) {
		std::vector<char> data;
		subs->SaveMemory(data,_T("UTF-8"));
		delete subs;
		instance = csri_open_mem(renderer,&data[0],data.size(),NULL);
	}

	// Open from disk
	else {
		wxString subsFileName = VideoContext::Get()->GetTempWorkFile();
		subs->Save(subsFileName,false,false,_T("UTF-8"));
		instance = csri_open_file(renderer,subsFileName.mb_str(wxConvUTF8),NULL);
	}
}


//////////////////
// Draw subtitles
void CSRISubtitlesProvider::DrawSubtitles(AegiVideoFrame &dst,double time) {
	// Check if CSRI loaded properly
	if (!instance) return;

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
	int error = csri_request_fmt(instance,&format);
	if (error) return;

	// Render
	csri_render(instance,&frame,time);
}


/////////////////////
// Get CSRI subtypes
wxArrayString CSRISubtitlesProviderFactory::GetSubTypes() {
	csri_info *info;
	wxArrayString final;
	for (csri_rend *cur = csri_renderer_default();cur;cur = csri_renderer_next(cur)) {
		// Get renderer name
		info = csri_renderer_info(cur);
		const char* buffer = info->name;

		// wxWidgets isn't initialized, so h4x into a wxString
		int len = strlen(buffer);
		wxString str;
		str.Alloc(len+1);
		for (int i=0;i<len;i++) {
			str.Append((wxChar)buffer[i]);
		}
		final.Add(str);
	}
	return final;
}


#endif // WITH_CSRI
