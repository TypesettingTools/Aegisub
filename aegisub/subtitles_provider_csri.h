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
#include "subtitles_provider.h"
#ifdef WIN32
#define CSRIAPI
#endif

ifdef __WINDOWS__
#include "../csri/include/csri/csri.h"
#else
#include "csri/csri.h"
#fi

/////////////////////////////////////////////////
// Common Subtitles Rendering Interface provider
class CSRISubtitlesProvider : public SubtitlesProvider {
private:
	wxString subType;
	csri_inst *instance;

public:
	CSRISubtitlesProvider(wxString subType);
	~CSRISubtitlesProvider();

	bool CanRaster() { return true; }

	void LoadSubtitles(AssFile *subs);
	void DrawSubtitles(AegiVideoFrame &dst,double time);
};


///////////
// Factory
class CSRISubtitlesProviderFactory : public SubtitlesProviderFactory {
public:
	SubtitlesProvider *CreateProvider(wxString subType=_T("")) { return new CSRISubtitlesProvider(subType); }
	wxArrayString GetSubTypes();
	CSRISubtitlesProviderFactory() : SubtitlesProviderFactory(_T("csri"),GetSubTypes()) {}
};

#endif
