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



#include "quicktime_common.h"

#ifdef WITH_QUICKTIME
#ifndef AGI_PRE
#include <map>
#include <vector>

#include <wx/dynarray.h>
#include <wx/filename.h>
#endif


#include "include/aegisub/video_provider.h"

/// DOCME
/// @class QuickTimeVideoProvider
/// @brief DOCME
///
/// DOCME
class QuickTimeVideoProvider : public VideoProvider, QuickTimeProvider {
	/// source object
	Movie movie;

	/// render buffer
	GWorldPtr gw;

	/// input data handle
	Handle in_dataref;


	/// DOCME

	/// width/height
	int w, h;

	/// length of file in frames
	int num_frames;

	/// current frame number
	int cur_fn;

	/// vfr framerate
	Framerate vfr_fps;

	/// list of keyframes
	std::vector<int> keyframes;

	/// qt timestamps (used for seeking)
	std::vector<int> qt_timestamps;


	/// quicktime error code
	OSErr qt_err;

	/// aegisub error message
	wxString errmsg;

	void LoadVideo(const wxString filename);
	std::vector<int> IndexFile();
	void Close();

public:
	QuickTimeVideoProvider(wxString filename);
	~QuickTimeVideoProvider();

	const AegiVideoFrame GetFrame(int n);

	int GetPosition() const               { return cur_fn; }
	int GetFrameCount() const             { return num_frames; }
	int GetWidth() const                  { return w; }
	int GetHeight() const                 { return h; }
	agi::vfr::Framerate GetFPS() const    { return vfr_fps; }
	std::vector<int> GetKeyFrames() const { return keyframes; };
	wxString GetDecoderName() const       { return L"QuickTime"; };
	bool WantsCaching() const             { return true; };
	wxString GetWarning() const           { return errmsg; }
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
