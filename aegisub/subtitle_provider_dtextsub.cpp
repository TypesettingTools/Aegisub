// Copyright (c) 2007, Niels Martin Hansen
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
// Contact: mailto:jiifurusu@gmail.com
//

// Use Gabest's RenderedTextSubtitles directly

#if USE_DTEXTSUB == 1

#include <wx/wxprec.h>
#include <wx/image.h>
#include "ass_file.h"
#include "subtitle_provider.h"
#include "video_provider.h"
#include <windows.h>
#include "vsfilter_editor_plugin.h"

#pragma comment(lib, "vsfilter.lib")

class SubtitleProviderDTextSub : public SubtitleProvider, public SubtitleProvider::Overlay {
private:
	// A little copy-paste from the libass code...
	class MyClass : public Class {
	public:
		MyClass() : Class(L"DirectTextSub")
		{
		};

		virtual SubtitleProvider *Get(AssFile *subs)
		{
			return new SubtitleProviderDTextSub(subs);
		};
	};
	static MyClass me;

	EditorPluginRenderer *renderer;
	RECT rect;

	AssFile *subs;
	VideoProvider *vpro;

public:
	SubtitleProviderDTextSub(AssFile *_subs);
	virtual ~SubtitleProviderDTextSub();
	virtual void Bind(VideoProvider *_vpro);

	virtual void SetParams(int width, int height);
	virtual void Render(wxImage &frame, int ms);
	virtual void Unbind();
};

SubtitleProviderDTextSub::MyClass SubtitleProviderDTextSub::me;


SubtitleProviderDTextSub::SubtitleProviderDTextSub(AssFile *_subs)
: vpro(0)
{
	subs = _subs;
	renderer = renderer_new();
	if (!renderer)
		throw _T("Failed to create VSFilter Editor Interface renderer");
}


SubtitleProviderDTextSub::~SubtitleProviderDTextSub()
{
	Unbind();
	renderer_free(renderer);
}


void SubtitleProviderDTextSub::Bind(VideoProvider *_vpro)
{
	vpro = _vpro;
	vpro->AttachOverlay(this);
}


void SubtitleProviderDTextSub::SetParams(int width, int height)
{
	SIZE screen;
	SIZE script;
	screen.cx = width;
	screen.cy = height;
	if (subs) {
		int w, h;
		subs->GetResolution(w, h);
		script.cx = w;
		script.cy = h;
	} else {
		script.cx = width;
		script.cy = height;
	}
	renderer_set_resolution(renderer, &script, &screen, 0);
}


void SubtitleProviderDTextSub::Render(wxImage &frame, int ms)
{
	// TODO: some way to discover whether it was just a seek and nothing needs to be reloaded
	renderer_clear(renderer);

	for (entryIter l = subs->Line.begin(); l != subs->Line.end(); ++l) {
	}

	BYTE *bits = frame.GetData();

	renderer_render_overlay(renderer, ms, bits);
}


void SubtitleProviderDTextSub::Unbind()
{
	if (vpro)
		vpro->AttachOverlay(NULL);
	vpro = 0;
}


#endif
