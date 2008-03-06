// Copyright (c) 2006-2008, Rodrigo Braz Monteiro, Fredrik Mellbin
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


//////////
// Headers
#include <wx/intl.h>
#include "video_frame.h"
#include "factory.h"


//////////////
// Prototypes
class SubtitlesProvider;


////////////////////////////
// Video Provider interface
class VideoProvider {
public:
	// Virtual destructor
	virtual ~VideoProvider() {}

	// Override this method to actually get frames
	virtual const AegiVideoFrame GetFrame(int n, int formatMask)=0;

	// Override the following methods to get video information:
	virtual int GetPosition()=0;				// Get the number of the last frame loaded
	virtual int GetFrameCount()=0;				// Get total number of frames
	virtual int GetWidth()=0;					// Returns the video width in pixels
	virtual int GetHeight()=0;					// Returns the video height in pixels
	virtual double GetFPS()=0;					// Get framerate in frames per second

	// Use this to set any post-loading warnings, such as "being loaded with unreliable seeking"
	virtual wxString GetWarning() { return _T(""); }

	// Name of decoder, e.g. "Avisynth/FFMPegSource"
	virtual wxString GetDecoderName() { return _("Unknown"); }

	// How many frames does this provider wants that Aegisub caches? Set to 0 if it doesn't require caching.
	virtual int GetDesiredCacheSize() { return 0; }

	// For providers that are natively time-based (e.g. DirectShow)
	virtual bool IsNativelyByFrames() { return true; }
	virtual void OverrideFrameTimeList(wxArrayInt list) {}	// Override the list with the provided one, for VFR handling

	// If this video provider has a built-in subtitles provider, return that
	virtual SubtitlesProvider *GetAsSubtitlesProvider() { return NULL; }
};


///////////
// Factory
class VideoProviderFactory : public AegisubFactory<VideoProviderFactory> {
protected:
	virtual VideoProvider *CreateProvider(wxString video,double fps=0.0)=0;
	VideoProviderFactory(wxString name) { RegisterFactory(name); }

public:
	virtual ~VideoProviderFactory() {}
	static VideoProvider *GetProvider(wxString video,double fps=0.0);

	static void RegisterProviders();
};
