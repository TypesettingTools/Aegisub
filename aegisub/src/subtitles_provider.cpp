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

/// @file subtitles_provider.cpp
/// @brief Base class for subtitle renderers
/// @ingroup subtitle_rendering
///

#include "config.h"

#include "compat.h"
#include "main.h"
#ifdef WITH_CSRI
#include "subtitles_provider_csri.h"
#endif
#ifdef WITH_LIBASS
#include "subtitles_provider_libass.h"
#endif
#if !defined(WITH_CSRI) && !defined(WITH_LIBASS)
#include "include/aegisub/subtitles_provider.h"
#endif

/// @brief Get provider 
/// @return 
///
SubtitlesProvider* SubtitlesProviderFactory::GetProvider() {
	std::vector<std::string> list = GetClasses(OPT_GET("Subtitle/Provider")->GetString());
	if (list.empty()) throw _T("No subtitle providers are available.");

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.size();i++) {
		try {
			size_t pos = list[i].find('/');
			std::string name = list[i].substr(0, pos);
			std::string subType = pos < list[i].size() - 1 ? list[i].substr(pos + 1) : "";
			SubtitlesProvider *provider = Create(list[i], subType);
			if (provider) return provider;
		}
		catch (agi::UserCancelException const&) { throw; }
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	throw error;
}

/// @brief Register providers 
///
void SubtitlesProviderFactory::RegisterProviders() {
#ifdef WITH_CSRI
	Register<CSRISubtitlesProvider>("CSRI", false, CSRISubtitlesProvider::GetSubTypes());
#endif
#ifdef WITH_LIBASS
	Register<LibassSubtitlesProvider>("libass");
	LibassSubtitlesProvider::CacheFonts();
#endif
}

template<> SubtitlesProviderFactory::map *FactoryBase<SubtitlesProvider *(*)(std::string)>::classes = NULL;
