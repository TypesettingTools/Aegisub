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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "video_provider.h"
#include "video_provider_dummy.h"
#include "options.h"
#include "vfr.h"
#ifdef WITH_AVISYNTH
#include "video_provider_avs.h"
#endif
#ifdef WITH_DIRECTSHOW
#include "video_provider_dshow.h"
#endif
#ifdef WITH_FFMPEG
#include "video_provider_lavc.h"
#endif
#include "video_provider_dummy.h"
#include "video_provider_cache.h"


////////////////
// Get provider
VideoProvider *VideoProviderFactoryManager::GetProvider(wxString video,double fps) {
	// First check special case of dummy video
	if (video.StartsWith(_T("?dummy:"))) {
		return new DummyVideoProvider(video, fps);
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
			VideoProvider *provider = GetFactory(list[i])->CreateProvider(video,fps);
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


//////////////////////////
// Register all providers
void VideoProviderFactoryManager::RegisterProviders() {
#ifdef WITH_AVISYNTH
	RegisterFactory(new AvisynthVideoProviderFactory(),_T("Avisynth"));
#endif
#ifdef WITH_DIRECTSHOW
	RegisterFactory(new DirectShowVideoProviderFactory(),_T("DirectShow"));
#endif
#ifdef WITH_FFMPEG
	RegisterFactory(new LAVCVideoProviderFactory(),_T("FFMPEG"));
#endif
}


//////////
// Static
template <class VideoProviderFactory> std::map<wxString,VideoProviderFactory*>* FactoryManager<VideoProviderFactory>::factories=NULL;
