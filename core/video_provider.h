// Copyright (c) 2006, Rodrigo Braz Monteiro, Fredrik Mellbin
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


//////////////
// Prototypes
class SubtitleRasterizer;


////////////////////////////
// Video Provider interface
class VideoProvider {
public:
	virtual ~VideoProvider() {}

	virtual void RefreshSubtitles()=0;		// Refresh subtitles display

	virtual wxBitmap GetFrame(int n)=0;						// Get frame as bitmap
	virtual void GetFloatFrame(float* Buffer, int n)=0;		// Get frame as float (for FexTracker)

	virtual int GetPosition()=0;			// Get the current frame being displayed
	virtual int GetFrameCount()=0;			// Get total number of frames
	virtual double GetFPS()=0;				// Get framerate in frames per second

	virtual void SetDAR(double dar)=0;		// Set display aspect ratio (width/height)
	virtual void SetZoom(double zoom)=0;	// Set zoom factor
	virtual int GetWidth()=0;				// Returns the display width in pixels
	virtual int GetHeight()=0;				// Returns the display height in pixels
	virtual double GetZoom()=0;				// Returns the zoom factor

	virtual int GetSourceWidth()=0;			// Returns the original source width in pixels
	virtual int GetSourceHeight()=0;		// Returns the original source height in pixels

	SubtitleRasterizer *subsRaster;

	static VideoProvider *GetProvider(wxString video,wxString subtitles);
};
