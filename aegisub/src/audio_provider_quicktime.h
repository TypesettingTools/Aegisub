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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once

#include "quicktime_common.h"

#ifdef WITH_QUICKTIME
#include <wx/wxprec.h>
#include <wx/log.h>
#include "include/aegisub/audio_provider.h"


class QuickTimeAudioProvider : public AudioProvider, QuickTimeProvider {
private:
	Movie movie;			// input file
	Handle in_dataref;		// input file handle
	MovieAudioExtractionRef extract_ref; // extraction session object

	bool inited;

	OSErr qt_err;			// quicktime error code
	OSStatus qt_status;		// another quicktime error code
	wxString errmsg;		// aegisub error messages

	void Close();
	void LoadAudio(wxString filename);

public:
	QuickTimeAudioProvider(wxString filename);
	virtual ~QuickTimeAudioProvider();

	bool AreSamplesNativeEndian() { return true; }

	virtual void GetAudio(void *buf, int64_t start, int64_t count);
};


class QuickTimeAudioProviderFactory : public AudioProviderFactory {
public:
	AudioProvider *CreateProvider(wxString file) { return new QuickTimeAudioProvider(file); }
};


#endif /* WITH_QUICKTIME */
