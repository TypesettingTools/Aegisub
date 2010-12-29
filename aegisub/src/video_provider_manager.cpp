// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file video_provider_manager.cpp
/// @brief Keep track of installed video providers
/// @ingroup video_input
///

#include "config.h"

#include <libaegisub/log.h>

#include "compat.h"
#include "main.h"

#ifdef WITH_AVISYNTH
#include "video_provider_avs.h"
#endif
#include "video_provider_cache.h"
#include "video_provider_dummy.h"
#ifdef WITH_FFMPEGSOURCE
#include "video_provider_ffmpegsource.h"
#endif
#include "video_provider_manager.h"
#include "video_provider_yuv4mpeg.h"


/// @brief Get provider 
/// @param video 
/// @return 
///
VideoProvider *VideoProviderFactory::GetProvider(wxString video) {
	std::vector<std::string> list = GetClasses(OPT_GET("Video/Provider")->GetString());
	if (video.StartsWith("?dummy")) list.insert(list.begin(), "Dummy");
	list.insert(list.begin(), "YUV4MPEG");

	bool fileFound = false;
	bool fileSupported = false;
	std::string errors;
	errors.reserve(1024);
	for (int i = 0; i < (signed)list.size(); ++i) {
		std::string err;
		try {
			VideoProvider *provider = Create(list[i], video);
			LOG_I("manager/video/provider") << list[i] << ": opened " << STD_STR(video);
			if (provider->WantsCaching()) {
				return new VideoProviderCache(provider);
			}
			return provider;
		}
		catch (agi::FileNotFoundError const&) {
			err = list[i] + ": file not found.";
			// Keep trying other providers as this one may just not be able to
			// open a valid path
		}
		catch (VideoNotSupported const&) {
			fileFound = true;
			err = list[i] + ": video is not in a supported format.";
		}
		catch (VideoOpenError const& ex) {
			fileSupported = true;
			err = list[i] + ": " + ex.GetMessage();
		}
		catch (agi::vfr::Error const& ex) {
			fileSupported = true;
			err = list[i] + ": " + ex.GetMessage();
		}
		errors += err;
		errors += "\n";
		LOG_D("manager/video/provider") << err;
	}

	// No provider could open the file
	LOG_E("manager/video/provider") << "Could not open " << STD_STR(video);
	std::string msg = "Could not open " + STD_STR(video) + ":\n" + errors;

	if (!fileFound) throw agi::FileNotFoundError(STD_STR(video));
	if (!fileSupported) throw VideoNotSupported(msg);
	throw VideoOpenError(msg);
}

/// @brief Register all providers 
///
void VideoProviderFactory::RegisterProviders() {
#ifdef WITH_AVISYNTH
	Register<AvisynthVideoProvider>("Avisynth");
#endif
#ifdef WITH_FFMPEGSOURCE
	Register<FFmpegSourceVideoProvider>("FFmpegSource");
#endif
	Register<DummyVideoProvider>("Dummy", true);
	Register<YUV4MPEGVideoProvider>("YUV4MPEG", true);
}

template<> VideoProviderFactory::map *FactoryBase<VideoProvider *(*)(wxString)>::classes = NULL;
