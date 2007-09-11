// Copyright (c) 2006-2007, Rodrigo Braz Monteiro, Fredrik Mellbin
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
#include <list>
#include <wx/intl.h>
#include "video_frame.h"
#include "factory.h"


//////////////
// Prototypes
class SubtitlesProvider;


////////////////
// Cached frame
class CachedFrame {
public:
	AegiVideoFrame frame;
	int n;
};


////////////////////////////
// Video Provider interface
class VideoProvider {
private:
	unsigned int cacheMax;
	std::list<CachedFrame> cache;
	AegiVideoFrame tempRGBFrame;

	void Cache(int n,const AegiVideoFrame frame);
	AegiVideoFrame GetCachedFrame(int n);

protected:
	// Override this method to actually get frames
	virtual const AegiVideoFrame DoGetFrame(int n)=0;	// Get frame as AegiVideoFrame

	// Cache functions
	void SetCacheMax(int n_frames);
	void ClearCache();

public:
	// Base methods
	void GetFloatFrame(float* Buffer, int n);	// Get frame as float
	const AegiVideoFrame GetFrame(int n, int formatMask=FORMAT_RGB32);
	VideoProvider();
	virtual ~VideoProvider();

	// Subtitles
	virtual SubtitlesProvider *GetAsSubtitlesProvider() { return NULL; }	// Get subtitles provider

	// Override the following methods:
	virtual int GetPosition()=0;				// Get the number of the last frame loaded
	virtual int GetFrameCount()=0;				// Get total number of frames
	virtual int GetWidth()=0;					// Returns the video width in pixels
	virtual int GetHeight()=0;					// Returns the video height in pixels
	virtual double GetFPS()=0;					// Get framerate in frames per second
	virtual void OverrideFrameTimeList(wxArrayInt list) {}	// Override the list with the provided one, for VFR handling
	virtual bool IsNativelyByFrames() { return false; }
	virtual wxString GetWarning() { return _T(""); }
	virtual wxString GetDecoderName() { return _("Unknown"); }
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
};
