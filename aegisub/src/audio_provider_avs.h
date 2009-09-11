// Copyright (c) 2005-2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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

/// @file audio_provider_avs.h
/// @see audio_provider_avs.cpp
/// @ingroup audio_input
///


///////////
// Headers
#ifdef WITH_AVISYNTH
#include <Mmreg.h>
#include "include/aegisub/audio_provider.h"
#include "avisynth_wrap.h"


/// DOCME
/// @class AvisynthAudioProvider
/// @brief DOCME
///
/// DOCME
class AvisynthAudioProvider : public AudioProvider, public AviSynthWrapper {
private:

	/// DOCME
	wxString filename;

	/// DOCME
	PClip clip;

	void LoadFromClip(AVSValue clip);
	void OpenAVSAudio();
	void SetFile();
	void Unload();

public:
	AvisynthAudioProvider(wxString _filename);
	~AvisynthAudioProvider();

	wxString GetFilename();


	/// @brief // Only exists for x86 Windows, always delivers machine (little) endian
	/// @return 
	///
	bool AreSamplesNativeEndian() { return true; }

	void GetAudio(void *buf, int64_t start, int64_t count);
	void GetWaveForm(int *min,int *peak,int64_t start,int w,int h,int samples,float scale);
};



/// DOCME
/// @class AvisynthAudioProviderFactory
/// @brief DOCME
///
/// DOCME
class AvisynthAudioProviderFactory : public AudioProviderFactory {
public:

	/// @brief DOCME
	/// @param file 
	///
	AudioProvider *CreateProvider(wxString file) { return new AvisynthAudioProvider(file); }
};

#endif


