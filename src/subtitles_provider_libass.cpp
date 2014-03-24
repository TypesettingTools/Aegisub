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

/// @file subtitles_provider_libass.cpp
/// @brief libass-based subtitle renderer
/// @ingroup subtitle_rendering
///

#include "config.h"

#include "subtitles_provider_libass.h"

#include "ass_info.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "dialog_progress.h"
#include "frame_main.h"
#include "include/aegisub/subtitles_provider.h"
#include "main.h"
#include "utils.h"
#include "video_frame.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/log.h>
#include <libaegisub/util.h>

#include <boost/gil/gil_all.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <memory>
#include <mutex>

#ifdef __APPLE__
#include <sys/param.h>
#include <libaegisub/util_osx.h>
#endif

extern "C" {
#include <ass/ass.h>
}

namespace {
std::unique_ptr<agi::dispatch::Queue> cache_queue;
ASS_Library *library;

void msg_callback(int level, const char *fmt, va_list args, void *) {
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

#ifdef __APPLE__
#define CONFIG_PATH (agi::util::GetBundleResourcesDirectory() + "/etc/fonts/fonts.conf").c_str()
#else
#define CONFIG_PATH nullptr
#endif

class LibassSubtitlesProvider final : public SubtitlesProvider {
	ASS_Renderer* ass_renderer = nullptr;
	ASS_Track* ass_track = nullptr;

public:
	LibassSubtitlesProvider();
	~LibassSubtitlesProvider();

	void LoadSubtitles(AssFile *subs) override;
	void DrawSubtitles(VideoFrame &dst, double time) override;
};

LibassSubtitlesProvider::LibassSubtitlesProvider() {
	auto done = std::make_shared<bool>(false);
	auto renderer = std::make_shared<ASS_Renderer*>(nullptr);
	cache_queue->Async([=]{
		auto ass_renderer = ass_renderer_init(library);
		if (ass_renderer) {
			ass_set_font_scale(ass_renderer, 1.);
			ass_set_fonts(ass_renderer, nullptr, "Sans", 1, CONFIG_PATH, true);
		}
		*done = true;
		*renderer = ass_renderer;
	});

	DialogProgress progress(wxGetApp().frame, _("Updating font index"), _("This may take several minutes"));
	progress.Run([=](agi::ProgressSink *ps) {
		ps->SetIndeterminate();
		while (!*done && !ps->IsCancelled())
			agi::util::sleep_for(250);
	});

	ass_renderer = *renderer;
	if (!ass_renderer) throw "ass_renderer_init failed";
}

LibassSubtitlesProvider::~LibassSubtitlesProvider() {
	if (ass_track) ass_free_track(ass_track);
	if (ass_renderer) ass_renderer_done(ass_renderer);
}

struct Writer {
	std::vector<char> data;
	AssEntryGroup group = AssEntryGroup::GROUP_MAX;

	template<typename T>
	void Write(T const& list) {
		for (auto const& line : list) {
			if (group != line.Group()) {
				group = line.Group();
				boost::push_back(data, line.GroupHeader() + "\r\n");
			}
			boost::push_back(data, line.GetEntryData() + "\r\n");
		}
	}
};

void LibassSubtitlesProvider::LoadSubtitles(AssFile *subs) {
	Writer writer;

	writer.data.reserve(0x4000);

	writer.Write(subs->Info);
	writer.Write(subs->Styles);
	writer.Write(subs->Events);

	if (ass_track) ass_free_track(ass_track);
	ass_track = ass_read_memory(library, &writer.data[0], writer.data.size(), nullptr);
	if (!ass_track) throw "libass failed to load subtitles.";
}

#define _r(c) ((c)>>24)
#define _g(c) (((c)>>16)&0xFF)
#define _b(c) (((c)>>8)&0xFF)
#define _a(c) ((c)&0xFF)

void LibassSubtitlesProvider::DrawSubtitles(VideoFrame &frame,double time) {
	ass_set_frame_size(ass_renderer, frame.width, frame.height);

	ASS_Image* img = ass_render_frame(ass_renderer, ass_track, int(time * 1000), nullptr);

	// libass actually returns several alpha-masked monochrome images.
	// Here, we loop through their linked list, get the colour of the current, and blend into the frame.
	// This is repeated for all of them.

	using namespace boost::gil;
	auto dst = interleaved_view(frame.width, frame.height, (bgra8_pixel_t*)frame.data.data(), frame.width * 4);
	if (frame.flipped)
		dst = flipped_up_down_view(dst);

	for (; img; img = img->next) {
		unsigned int opacity = 255 - ((unsigned int)_a(img->color));
		unsigned int r = (unsigned int)_r(img->color);
		unsigned int g = (unsigned int)_g(img->color);
		unsigned int b = (unsigned int)_b(img->color);

		auto srcview = interleaved_view(img->w, img->h, (gray8_pixel_t*)img->bitmap, img->stride);
		auto dstview = subimage_view(dst, img->dst_x, img->dst_y, img->w, img->h);

		transform_pixels(dstview, srcview, dstview, [=](const bgra8_pixel_t frame, const gray8_pixel_t src) -> bgra8_pixel_t {
			unsigned int k = ((unsigned)src) * opacity / 255;
			unsigned int ck = 255 - k;

			bgra8_pixel_t ret;
			ret[0] = (k * b + ck * frame[0]) / 255;
			ret[1] = (k * g + ck * frame[1]) / 255;
			ret[2] = (k * r + ck * frame[2]) / 255;
			ret[3] = 0;
			return ret;
		});
	}
}
}

namespace libass {
std::unique_ptr<SubtitlesProvider> Create(std::string const&) {
	return agi::util::make_unique<LibassSubtitlesProvider>();
}

void CacheFonts() {
	// Initialize the cache worker thread
	cache_queue = agi::dispatch::Create();

	// Initialize libass
	library = ass_library_init();
	ass_set_message_cb(library, msg_callback, nullptr);

	// Initialize a renderer to force fontconfig to update its cache
	cache_queue->Async([]{
		auto ass_renderer = ass_renderer_init(library);
		ass_set_fonts(ass_renderer, nullptr, "Sans", 1, CONFIG_PATH, true);
		ass_renderer_done(ass_renderer);
	});
}
}
