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
#include "subtitles_provider.h"
#ifdef WITH_CSRI
#include "subtitles_provider_csri.h"
#endif
#ifdef WITH_LIBASS
#include "subtitles_provider_libass.h"
#endif
#include "options.h"


//////////////
// Destructor
SubtitlesProvider::~SubtitlesProvider() {
}


////////////////////////////////////////////////////////////////
// Check if provider available (doesn't verify provider works!)
bool SubtitlesProviderFactory::ProviderAvailable() {
	// List of providers
	wxArrayString list = GetFactoryList(Options.AsText(_T("Subtitles provider")));

	// None available
	return (list.Count() > 0);
}


////////////////
// Get provider
SubtitlesProvider* SubtitlesProviderFactory::GetProvider() {
	// List of providers
	wxArrayString list = GetFactoryList(Options.AsText(_T("Subtitles provider")));

	// None available
	if (list.Count() == 0) throw _T("No subtitle providers are available.");

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.Count();i++) {
		try {
			size_t pos = list[i].Find(_T('/'));
			wxString name = list[i].Left(pos);
			wxString subType = list[i].Mid(pos+1);
			SubtitlesProvider *provider = GetFactory(list[i])->CreateProvider(subType);
			if (provider) return provider;
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	throw error;
}


//////////////////////
// Register providers
void SubtitlesProviderFactory::RegisterProviders() {
#ifdef WITH_CSRI
	new CSRISubtitlesProviderFactory();
#endif
#ifdef WITH_LIBASS
	new LibassSubtitlesProviderFactory();
#endif
}


//////////
// Static
template <class SubtitlesProviderFactory> std::map<wxString,SubtitlesProviderFactory*>* AegisubFactory<SubtitlesProviderFactory>::factories=NULL;
