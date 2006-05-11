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
#ifdef HAVE_ASA

#include <wx/wxprec.h>
#include <wx/image.h>
#include "subtitle_provider.h"
#include "video_provider.h"
#include "ass_file.h"

#ifdef HAVE_ASA_ASA_H
# include <asa/asa.h>
#else
# ifdef __WINDOWS__
#  include <asa.h>
# endif
#endif


class SubtitleProviderASA : public SubtitleProvider, SubtitleProvider::Overlay {
private:
	class MyClass : public Class {
	public:
		MyClass() : Class(L"asa") { asa_init(ASA_VERSION); };
		virtual SubtitleProvider *Get(AssFile *subs) { return new SubtitleProviderASA(subs); };
	};
	static MyClass me;

	AssFile *subs;
	asa_inst *inst;
	VideoProvider *vpro;
public:
	SubtitleProviderASA(AssFile *_subs);
	virtual ~SubtitleProviderASA();
	virtual void Bind(VideoProvider *_vpro);

	virtual void SetParams(int width, int height);
	virtual void Render(wxImage &frame, int ms);
	virtual void Unbind();
};

SubtitleProviderASA::MyClass SubtitleProviderASA::me;

SubtitleProviderASA::SubtitleProviderASA(AssFile *_subs)
{
	subs = _subs;
	wxString text = subs->GetString();

	inst = asa_open_mem((const char *)text.GetData(), sizeof(wchar_t) * text.Length(), (enum asa_oflags)0);
	if (!inst)
		throw L"failed to load script with asa.";
};

SubtitleProviderASA::~SubtitleProviderASA()
{
	Unbind();
	asa_close(inst);
};

void SubtitleProviderASA::Bind(VideoProvider *_vpro)
{
	vpro = _vpro;
	vpro->AttachOverlay(this);
}

void SubtitleProviderASA::SetParams(int width, int height)
{
	asa_setsize(inst, width, height);
}

void SubtitleProviderASA::Render(wxImage &frame, int ms)
{
	struct asa_frame aframe;
	aframe.csp = ASACSP_RGB;
	aframe.bmp.rgb.fmt = ASACSPR_RGB;
	aframe.bmp.rgb.d.d = frame.GetData();
	aframe.bmp.rgb.d.stride = 3 * frame.GetWidth();
	asa_render(inst, ms * 0.001, &aframe);
}

void SubtitleProviderASA::Unbind()
{
	if (vpro)
		vpro->AttachOverlay(NULL);
	vpro = NULL;
}

#endif /* HAVE_ASA */

