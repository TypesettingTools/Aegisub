// -*- c-basic-offset: 8; indent-tabs-mode: t -*-
// Copyright (c) 2006, David Lamparter
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
#ifdef HAVE_LIBASS

#include <wx/wxprec.h>
#include <wx/image.h>
#include "subtitle_provider.h"
#include "video_provider.h"
#include "ass_file.h"

extern "C" {
#include <ass/ass.h>
}

class SubtitleProviderLibASS : public SubtitleProvider, SubtitleProvider::Overlay {
private:
	class MyClass : public Class {
	public:
		MyClass() : Class(L"libass") {
			ass_library = ass_library_init();
			if (!ass_library) throw _T("ass_library_init failed");

			ass_set_fonts_dir(ass_library, "");
			ass_set_extract_fonts(ass_library, 0);
			ass_set_style_overrides(ass_library, NULL);
		};
		virtual SubtitleProvider *Get(AssFile *subs) { return new SubtitleProviderLibASS(subs); };
	};
	static MyClass me;

	static ass_library_t* ass_library;
	ass_renderer_t* ass_renderer;
	ass_track_t* ass_track;
	wxString last_sub;
	
	AssFile *subs;
	VideoProvider *vpro;
public:
	SubtitleProviderLibASS(AssFile *_subs);
	virtual ~SubtitleProviderLibASS();
	virtual void Bind(VideoProvider *_vpro);

	virtual void SetParams(int width, int height);
	virtual void Render(wxImage &frame, int ms);
	virtual void Unbind();
};

SubtitleProviderLibASS::MyClass SubtitleProviderLibASS::me;
ass_library_t* SubtitleProviderLibASS::ass_library;

SubtitleProviderLibASS::SubtitleProviderLibASS(AssFile *_subs) : ass_track(NULL)
{
	subs = _subs;
	wxString text = subs->GetString();

	ass_renderer = ass_renderer_init(ass_library);
	if (!ass_renderer) throw _T("ass_renderer_init failed");
  
	//ass_set_margins(ass_renderer, 0, 0, 0, 0);
	//ass_set_use_margins(ass_renderer, 0);
	ass_set_font_scale(ass_renderer, 1.);
	ass_set_fonts(ass_renderer, NULL, "Sans");
};

SubtitleProviderLibASS::~SubtitleProviderLibASS()
{
	Unbind();
	//asa_close(inst);
};

void SubtitleProviderLibASS::Bind(VideoProvider *_vpro)
{
	vpro = _vpro;
	vpro->AttachOverlay(this);
}

void SubtitleProviderLibASS::SetParams(int width, int height)
{
	ass_set_frame_size(ass_renderer, width, height);
}

#define _r(c)  ((c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>>8)&0xFF)
#define _a(c)  ((c)&0xFF)

static void blend_single(wxImage &frame, ass_image_t* img) {
	unsigned char opacity = 255 - _a(img->color);
	unsigned char r = _r(img->color);
	unsigned char g = _g(img->color);
	unsigned char b = _b(img->color);
	int dst_stride = frame.GetWidth() * 3;
  
	unsigned char* src;
	unsigned char* dst;

	src = img->bitmap;
	dst = frame.GetData() + img->dst_y * dst_stride + img->dst_x * 3;
	for (int y = 0; y < img->h; ++y) {
		for (int x = 0; x < img->w; ++x) {
			unsigned k = ((unsigned)src[x]) * opacity / 255;
			// possible endianness problems
			dst[x*3]   = (k*r + (255-k)*dst[x*3])   / 255;
			dst[x*3+1] = (k*g + (255-k)*dst[x*3+1]) / 255;
			dst[x*3+2] = (k*b + (255-k)*dst[x*3+2]) / 255;
		}
		src += img->stride;
		dst += dst_stride;
	}
}

static void blend(wxImage &frame, ass_image_t* img) {
	int cnt = 0;
	while (img) {
		blend_single(frame, img);
		++cnt;
		img = img->next;
	}
}

void SubtitleProviderLibASS::Render(wxImage &frame, int ms)
{
	wxString text = subs->GetString();
	if (text != last_sub) { // almost always true because of "Video Position" header
		if (ass_track)
			ass_free_track(ass_track);
		ass_track = ass_read_memory(ass_library, ((char*)text.GetData()) + 4, sizeof(wchar_t) * text.Length() - 4, "UCS-4LE");
	}
	
	if (!ass_track)
		throw _T("libass parse error");
	
	ass_image_t* img = ass_render_frame(ass_renderer, ass_track, ms);
	blend(frame, img);
}

void SubtitleProviderLibASS::Unbind()
{
	if (vpro)
		vpro->AttachOverlay(NULL);
	vpro = NULL;
}

#endif /* HAVE_LIBASS */
