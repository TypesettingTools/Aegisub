// Copyright (c) 2006-2007, Rodrigo Braz Monteiro, Evgeniy Stepanov
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

/// @file subtitles_provider_libass.cpp
/// @brief libass-based subtitle renderer
/// @ingroup subtitle_rendering
///

#include "config.h"

#ifdef WITH_LIBASS

#ifndef AGI_PRE
#include <wx/filefn.h>
#include <wx/utils.h>
#endif

#ifdef __APPLE__
#include <sys/param.h>
#include <libaegisub/util_osx.h>
#endif

#include <libaegisub/log.h>

#include "ass_file.h"
#include "dialog_progress.h"
#include "frame_main.h"
#include "main.h"
#include "standard_paths.h"
#include "subtitles_provider_libass.h"
#include "utils.h"
#include "video_context.h"
#include "video_frame.h"


/// @brief Handle libass messages
///
static void msg_callback(int level, const char *fmt, va_list args, void *data) {
	if (level >= 7) return;
	char buf[1024];
#ifdef _WIN32
	vsprintf_s(buf, sizeof(buf), fmt, args);
#else
	vsnprintf(buf, sizeof(buf), fmt, args);
#endif

	if (level < 2) // warning/error
		LOG_I("subtitle/provider/libass") << buf;
	else // verbose
		LOG_D("subtitle/provider/libass") << buf;
}

static void load_config(ASS_Renderer *ass_renderer) {
#ifdef __APPLE__
	char config_path[MAXPATHLEN];
	char *config_dir;

	config_dir = agi::util::OSX_GetBundleResourcesDirectory();
	snprintf(config_path, MAXPATHLEN, "%s/etc/fonts/fonts.conf", config_dir);
	free(config_dir);
#else
	const char *config_path = NULL;
#endif

	ass_set_fonts(ass_renderer, NULL, "Sans", 1, config_path, true);
}

/// @brief Constructor
///
LibassSubtitlesProvider::LibassSubtitlesProvider(std::string) {
	if (cache_worker) {
		bool canceled;
		DialogProgress *progress = new DialogProgress(AegisubApp::Get()->frame, L"", &canceled, L"Caching fonts", 0, 1);
		progress->Show();
		while (cache_worker) {
			if (canceled) throw agi::UserCancelException("Font caching cancelled");
			progress->Pulse();
			wxYield();
			wxMilliSleep(100);
		}
		progress->Destroy();
	}
	ass_set_message_cb(ass_library, msg_callback, this);

	// Initialize renderer
	ass_track = NULL;
	ass_renderer = ass_renderer_init(ass_library);
	if (!ass_renderer) throw _T("ass_renderer_init failed");
	ass_set_font_scale(ass_renderer, 1.);
	load_config(ass_renderer);
}

/// @brief Destructor
///
LibassSubtitlesProvider::~LibassSubtitlesProvider() {
}

/// @brief Load subtitles
/// @param subs
///
void LibassSubtitlesProvider::LoadSubtitles(AssFile *subs) {
	// Prepare subtitles
	std::vector<char> data;
	subs->SaveMemory(data,_T("UTF-8"));

	// Load file
	if (ass_track) ass_free_track(ass_track);
	ass_track = ass_read_memory(ass_library, &data[0], data.size(),(char *)"UTF-8");
	if (!ass_track) throw _T("libass failed to load subtitles.");
}

/// DOCME
#define _r(c)  ((c)>>24)

/// DOCME
#define _g(c)  (((c)>>16)&0xFF)

/// DOCME
#define _b(c)  (((c)>>8)&0xFF)

/// DOCME
#define _a(c)  ((c)&0xFF)


/// @brief Draw subtitles
/// @param frame
/// @param time
/// @return
///
void LibassSubtitlesProvider::DrawSubtitles(AegiVideoFrame &frame,double time) {
	// Set size
	ass_set_frame_size(ass_renderer, frame.w, frame.h);

	// Get frame
	ASS_Image* img = ass_render_frame(ass_renderer, ass_track, int(time * 1000), NULL);

	// libass actually returns several alpha-masked monochrome images.
	// Here, we loop through their linked list, get the colour of the current, and blend into the frame.
	// This is repeated for all of them.
	while (img) {
		// Get colours
		unsigned int opacity = 255 - ((unsigned int)_a(img->color));
		unsigned int r = (unsigned int)_r(img->color);
		unsigned int g = (unsigned int)_g(img->color);
		unsigned int b = (unsigned int) _b(img->color);

		// Prepare copy
		int src_stride = img->stride;
		int dst_stride = frame.pitch[0];
		int dst_delta = dst_stride - img->w*4;
		//int stride = MIN(src_stride,dst_stride);
		const unsigned char *src = img->bitmap;
		unsigned char *dst = frame.data[0] + (img->dst_y * dst_stride + img->dst_x * 4);
		unsigned int k,ck,t;

		// Copy image to destination frame
		for (int y=0;y<img->h;y++) {
			//memcpy(dst,src,stride);
			for (int x = 0; x < img->w; ++x) {
				k = ((unsigned)src[x]) * opacity / 255;
				ck = 255 - k;
				t = *dst;
				*dst++ = (k*b + ck*t) / 255;
				t = *dst;
				*dst++ = (k*g + ck*t) / 255;
				t = *dst;
				*dst++ = (k*r + ck*t) / 255;
				dst++;
			}

			dst += dst_delta;
			src += src_stride;
		}

		// Next image
		img = img->next;
	}
}

class FontConfigCacheThread : public wxThread {
	ASS_Library *ass_library;
	FontConfigCacheThread** thisPtr;
	ExitCode Entry() {
		ASS_Renderer *ass_renderer = ass_renderer_init(ass_library);
		load_config(ass_renderer);
		ass_renderer_done(ass_renderer);
		*thisPtr = NULL;
		return EXIT_SUCCESS;
	}
public:
	FontConfigCacheThread(ASS_Library *ass_library, FontConfigCacheThread **thisPtr)
		: ass_library(ass_library)
		, thisPtr(thisPtr)
	{
		*thisPtr = this;
		Create();
		Run();
	}
};

void LibassSubtitlesProvider::CacheFonts() {
	ass_library = ass_library_init();
	new FontConfigCacheThread(ass_library, &cache_worker);
}

/// DOCME
ASS_Library* LibassSubtitlesProvider::ass_library;
FontConfigCacheThread* LibassSubtitlesProvider::cache_worker = NULL;

#endif // WITH_LIBASS
