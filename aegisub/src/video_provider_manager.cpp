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


///////////
// Headers
#include "config.h"

#include "options.h"
#include "vfr.h"
#ifdef WITH_AVISYNTH
#include "video_provider_avs.h"
#endif
#include "video_provider_cache.h"
#include "video_provider_dummy.h"
#ifdef WITH_FFMPEGSOURCE
#include "video_provider_ffmpegsource.h"
#endif
#include "video_provider_manager.h"
#ifdef WITH_QUICKTIME
#include "video_provider_quicktime.h"
#endif
#include "video_provider_yuv4mpeg.h"


/// @brief Get provider 
/// @param video 
/// @return 
///
VideoProvider *VideoProviderFactoryManager::GetProvider(wxString video) {
	// First check special case of dummy video
	if (video.StartsWith(_T("?dummy:"))) {
		return new DummyVideoProvider(video.wc_str());
	}

	try {
		VideoProvider *y4m_provider = new YUV4MPEGVideoProvider(video.wc_str());
		if (y4m_provider)
			y4m_provider = new VideoProviderCache(y4m_provider);
		return y4m_provider;
	}
	catch (wxString temp) {
		wxLogDebug(_T("YUV4MPEG provider creation failed with reason: %s; trying other providers"), temp.c_str());
	}
	catch (...) {
		wxLogDebug(_T("YUV4MPEG provider creation failed for unknown reasons, trying other providers"));
	}

	// List of providers
	wxArrayString list = GetFactoryList(Options.AsText(_T("Video provider")));

	// None available
	if (list.Count() == 0) throw _T("No video providers are available.");

	// Get provider
	wxString error;
	for (unsigned int i=0;i<list.Count();i++) {
		try {
			// Create provider
			VideoProvider *provider = GetFactory(list[i])->CreateProvider(video.wc_str());
			if (provider) {
				// Cache if necessary
				if (provider->GetDesiredCacheSize()) {
					provider = new VideoProviderCache(provider);
				}
				return provider;
			}
		}
		catch (wxString err) { error += list[i] + _T(" factory: ") + err + _T("\n"); }
		catch (const wxChar *err) { error += list[i] + _T(" factory: ") + wxString(err) + _T("\n"); }
		catch (...) { error += list[i] + _T(" factory: Unknown error\n"); }
	}

	// Failed
	throw error;
}



/// @brief Register all providers 
///
void VideoProviderFactoryManager::RegisterProviders() {
#ifdef WITH_AVISYNTH
	RegisterFactory(new AvisynthVideoProviderFactory(),_T("Avisynth"));
#endif
#ifdef WITH_FFMPEGSOURCE
	RegisterFactory(new FFmpegSourceVideoProviderFactory(),_T("FFmpegSource"));
#endif
#ifdef WITH_QUICKTIME
	RegisterFactory(new QuickTimeVideoProviderFactory(),_T("QuickTime"));
#endif
}



/// @brief Clear all providers 
///
void VideoProviderFactoryManager::ClearProviders() {
	ClearFactories();
}



/// DOCME
template <class VideoProviderFactory> std::map<wxString,VideoProviderFactory*>* FactoryManager<VideoProviderFactory>::factories=NULL;


