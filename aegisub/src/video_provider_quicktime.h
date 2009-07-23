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

#include "config.h"
#include <wx/wxprec.h>

#ifdef WITH_QUICKTIME

#ifdef _MSC_VER
// avoid conflicts between MSVC's stdint.h and QT's stdint.h
#define _STDINT_H
// get MSVC to shut up about a macro redefinition in QT's ConditionalMacros.h
#pragma warning(disable: 4004)
#endif

#ifndef WIN32
#define MacOffsetRect OffsetRect
#endif


#include "include/aegisub/video_provider.h"
extern "C" {
#ifdef WIN32
#include <QTML.h>
#include <Movies.h>
#include <Files.h>
#include <QDOffscreen.h>
#else
#include <QuickTime/QuickTime.h> // not sure about this path, someone on mac needs to test it
#endif
}
#include <wx/dynarray.h>
#include <wx/filename.h>
#include <vector>
#include <map>
#include "vfr.h"


class QuickTimeVideoProvider : public VideoProvider {
private:
	Movie movie;			// source object
	GWorldPtr gw, gw_tmp;	// render buffers
	Handle in_dataref;		// input data handle

	int w, h;				// width/height
	int num_frames;			// length of file in frames
	int cur_fn;				// current frame number
	FrameRate vfr_fps;		// vfr framerate
	double assumed_fps;		// average framerate
	wxArrayInt keyframes;	// list of keyframes
	std::vector<int> qt_timestamps;	 // qt timestamps (used for seeking)

	OSErr qt_err;			// quicktime error code
	wxString errmsg;		// aegisub error message

	bool CanOpen(const Handle& dataref, const OSType dataref_type);
	void LoadVideo(const wxString filename);
	std::vector<int> IndexFile();
	void Close();

	void QTCheckError(OSErr err, wxString errmsg);

public:
	QuickTimeVideoProvider(wxString filename);
	~QuickTimeVideoProvider();

	const AegiVideoFrame GetFrame(int n);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();
	bool IsVFR() { return true; };
	FrameRate GetTrueFrameRate();
	wxArrayInt GetKeyFrames();
	bool QuickTimeVideoProvider::AreKeyFramesLoaded();
	wxString GetDecoderName() { return L"QuickTime"; };
	bool IsNativelyByFrames() { return true; };
	int GetDesiredCacheSize() { return 8; };
};


class QuickTimeVideoProviderFactory : public VideoProviderFactory {
public:
	VideoProvider *CreateProvider(wxString video) { return new QuickTimeVideoProvider(video); }
};


#endif /* WITH_QUICKTIME */
