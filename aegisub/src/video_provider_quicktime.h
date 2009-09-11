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

/// @file video_provider_quicktime.h
/// @see video_provider_quicktime.cpp
/// @ingroup video_input quicktime
///


#pragma once

#include "quicktime_common.h"

#ifdef WITH_QUICKTIME
#ifndef AGI_PRE
#include <map>
#include <vector>

#include <wx/dynarray.h>
#include <wx/filename.h>
#endif


#include "include/aegisub/video_provider.h"
#include "vfr.h"


/// DOCME
/// @class QuickTimeVideoProvider
/// @brief DOCME
///
/// DOCME
class QuickTimeVideoProvider : public VideoProvider, QuickTimeProvider {
private:

	/// DOCME
	Movie movie;			// source object

	/// DOCME
	GWorldPtr gw;			// render buffer

	/// DOCME
	Handle in_dataref;		// input data handle


	/// DOCME

	/// DOCME
	int w, h;				// width/height

	/// DOCME
	int num_frames;			// length of file in frames

	/// DOCME
	int cur_fn;				// current frame number

	/// DOCME
	FrameRate vfr_fps;		// vfr framerate

	/// DOCME
	double assumed_fps;		// average framerate

	/// DOCME
	wxArrayInt keyframes;	// list of keyframes

	/// DOCME
	std::vector<int> qt_timestamps;	 // qt timestamps (used for seeking)


	/// DOCME
	OSErr qt_err;			// quicktime error code

	/// DOCME
	wxString errmsg;		// aegisub error message

	void LoadVideo(const wxString filename);
	std::vector<int> IndexFile();
	void Close();

public:
	QuickTimeVideoProvider(wxString filename);
	~QuickTimeVideoProvider();

	const AegiVideoFrame GetFrame(int n);
	int GetPosition();
	int GetFrameCount();

	int GetWidth();
	int GetHeight();
	double GetFPS();

	/// @brief DOCME
	/// @return 
	///
	bool IsVFR() { return true; };
	FrameRate GetTrueFrameRate();
	wxArrayInt GetKeyFrames();
	bool QuickTimeVideoProvider::AreKeyFramesLoaded();

	/// @brief DOCME
	/// @return 
	///
	wxString GetDecoderName() { return L"QuickTime"; };

	/// @brief DOCME
	/// @return 
	///
	int GetDesiredCacheSize() { return 8; };
};



/// DOCME
/// @class QuickTimeVideoProviderFactory
/// @brief DOCME
///
/// DOCME
class QuickTimeVideoProviderFactory : public VideoProviderFactory {
public:

	/// @brief DOCME
	/// @param video 
	///
	VideoProvider *CreateProvider(wxString video) { return new QuickTimeVideoProvider(video); }
};


#endif /* WITH_QUICKTIME */


