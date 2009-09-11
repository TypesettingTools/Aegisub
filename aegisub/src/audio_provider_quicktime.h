// Copyright (c) 2009, Karl Blomster
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

/// @file audio_provider_quicktime.h
/// @see audio_provider_quicktime.cpp
/// @ingroup audio_input quicktime
///


#ifdef WITH_QUICKTIME
#include "quicktime_common.h"

#ifndef AGI_PRE
#include <wx/log.h>
#endif

#include "include/aegisub/audio_provider.h"



/// DOCME
/// @class QuickTimeAudioProvider
/// @brief DOCME
///
/// DOCME
class QuickTimeAudioProvider : public AudioProvider, QuickTimeProvider {
private:

	/// DOCME
	Movie movie;			// input file

	/// DOCME
	Handle in_dataref;		// input file handle

	/// DOCME
	MovieAudioExtractionRef extract_ref; // extraction session object


	/// DOCME
	bool inited;


	/// DOCME
	OSErr qt_err;			// quicktime error code

	/// DOCME
	OSStatus qt_status;		// another quicktime error code

	/// DOCME
	wxString errmsg;		// aegisub error messages

	void Close();
	void LoadAudio(wxString filename);

public:
	QuickTimeAudioProvider(wxString filename);
	virtual ~QuickTimeAudioProvider();


	/// @brief DOCME
	/// @return 
	///
	bool AreSamplesNativeEndian() { return true; }

	virtual void GetAudio(void *buf, int64_t start, int64_t count);
};



/// DOCME
/// @class QuickTimeAudioProviderFactory
/// @brief DOCME
///
/// DOCME
class QuickTimeAudioProviderFactory : public AudioProviderFactory {
public:

	/// @brief DOCME
	/// @param file 
	///
	AudioProvider *CreateProvider(wxString file) { return new QuickTimeAudioProvider(file); }
};


#endif /* WITH_QUICKTIME */


