// Copyright (c) 2008-2009, Karl Blomster
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

/// @file audio_provider_ffmpegsource.h
/// @see audio_provider_ffmpegsource.cpp
/// @ingroup audio_input ffms
///

#ifdef WITH_FFMPEGSOURCE
#include "include/aegisub/audio_provider.h"
#include "ffmpegsource_common.h"


/// @class FFmpegSourceAudioProvider
/// @brief Implements audio loading with the FFMS library.
class FFmpegSourceAudioProvider : public AudioProvider, FFmpegSourceProvider {
private:
	FFMS_AudioSource *AudioSource;	/// audio source object
	bool COMInited;					/// COM initialization state

	char FFMSErrMsg[1024];			/// FFMS error message
	FFMS_ErrorInfo ErrInfo;			/// FFMS error codes/messages

	void Close();
	void LoadAudio(wxString filename);

public:
	FFmpegSourceAudioProvider(wxString filename);
	virtual ~FFmpegSourceAudioProvider();

	/// @brief Checks sample endianness
	/// @return Returns true.
	/// FFMS always delivers native endian samples.
	bool AreSamplesNativeEndian() const { return true; }
	bool NeedsCache() const { return true; }

	virtual void GetAudio(void *buf, int64_t start, int64_t count);
};
#endif
