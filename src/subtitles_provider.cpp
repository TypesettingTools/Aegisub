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

/// @file subtitles_provider.cpp
/// @brief Base class for subtitle renderers
/// @ingroup subtitle_rendering
///

#include "config.h"

#include "options.h"
#include "subtitles_provider_csri.h"
#include "subtitles_provider_libass.h"
#include "include/aegisub/subtitles_provider.h"

std::unique_ptr<SubtitlesProvider> SubtitlesProviderFactory::GetProvider() {
	std::vector<std::string> list = GetClasses(OPT_GET("Subtitle/Provider")->GetString());
	if (list.empty()) throw std::string("No subtitle providers are available.");

	std::string error;
	for (auto const& factory : list) {
		try {
			size_t pos = factory.find('/');
			std::string subType = pos < factory.size() - 1 ? factory.substr(pos + 1) : "";
			auto provider = Create(factory, subType);
			if (provider) return provider;
		}
		catch (agi::UserCancelException const&) { throw; }
		catch (std::string const& err) { error += factory + " factory: " + err + "\n"; }
		catch (const char *err) { error += factory + " factory: " + std::string(err) + "\n"; }
		catch (...) { error += factory + " factory: Unknown error\n"; }
	}

	throw error;
}

void SubtitlesProviderFactory::RegisterProviders() {
#ifdef WITH_CSRI
	std::vector<std::string> csri_providers(CSRISubtitlesProvider::GetSubTypes());
	if (!csri_providers.empty())
		Register<CSRISubtitlesProvider>("CSRI", false, csri_providers);
#endif
	Register<LibassSubtitlesProvider>("libass");
	LibassSubtitlesProvider::CacheFonts();
}
