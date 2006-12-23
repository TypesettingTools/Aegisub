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
#include "video_provider_avs.h"
#include "video_provider_lavc.h"
#include "video_provider_dshow.h"
#include "options.h"
#include "setup.h"


////////////////
// Get provider
VideoProvider *VideoProvider::GetProvider(wxString video,wxString subtitles) {
	// Check if avisynth is available
	bool avisynthAvailable = false;
	bool dshowAvailable = false;
	#ifdef __WINDOWS__
	dshowAvailable = true;
	try {
		// If avisynth.dll cannot be loaded, an exception will be thrown and avisynthAvailable will never be set to true
		AviSynthWrapper avs;
		avisynthAvailable = true;
	}
	catch (...) {}
	#endif

	// Initialize to null
	VideoProvider *provider = NULL;

	// Preffered provider
	wxString preffered = Options.AsText(_T("Video provider")).Lower();

	// See if it's OK to use LAVC
	#if USE_LAVC == 1
	if (preffered == _T("ffmpeg") || (!avisynthAvailable && !dshowAvailable)) {
		// Load
		bool success = false;
		wxString error;
		try {
			provider = new LAVCVideoProvider(video,subtitles);
			success = true;
		}

		// Catch error
		catch (const wchar_t *err) {
			error = err;
		}
		catch (...) {
			error = _T("Unhandled exception.");
		}

		if (!success) {
			// Delete old provider
			delete provider;
			provider = NULL;

			// Try to fallback to avisynth
			if (avisynthAvailable) {
				wxMessageBox(_T("Failed loading FFmpeg decoder for video, falling back to Avisynth.\nError message: ") + error,_T("FFmpeg error."));
				provider = NULL;
			}

			// Out of options, rethrow
			else throw error.c_str();
		}
	}
	#endif

	#ifdef __WINDOWS__
	#if USE_DIRECTSHOW == 1
	// Use DirectShow provider
	if (!provider && (preffered == _T("dshow") || !avisynthAvailable)) {
		try {
			provider = new DirectShowVideoProvider(video,subtitles);
		}
		catch (...) {
			delete provider;
			provider = NULL;
			throw;
		}
	}
	#endif

	// Use Avisynth provider
	if (!provider) {
		try {
			provider = new AvisynthVideoProvider(video,subtitles);
		}
		catch (...) {
			delete provider;
			provider = NULL;
			throw;
		}
	}
	#endif

	// Return provider
	return provider;
}
