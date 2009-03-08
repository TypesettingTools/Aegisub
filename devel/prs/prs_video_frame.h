// Copyright (c) 2005, Rodrigo Braz Monteiro
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


///////////////
// Color space
enum ColorSpaceType {
	ColorSpace_RGB32,
	ColorSpace_RGB24,
	ColorSpace_YUY2,
	ColorSpace_YV12
};


/////////////////////
// Video frame class
class PRSVideoFrame {
public:
	bool ownData;				// If set to true, data will be deleted on destructor (defaults to false)
	char *data[4];				// Data for each of the planes (interleaved formats only use data[0])
	int w;						// Width
	int h;						// Height
	int pitch;					// Pitch (that is, width plus invisible area for optimization)
	ColorSpaceType colorSpace;	// Color space

	bool flipVertical;			// Frame is flipped vertically
	bool flipColors;			// Colors are flipped

	PRSVideoFrame();
	~PRSVideoFrame();

	void Overlay(PRSVideoFrame *dst,int x,int y,unsigned char alpha=255,unsigned char blend=0);
	int GetSize();
};


///////////////////////////
// Video frame cache class
class PRSCachedFrame {
public:
	PRSCachedFrame();
	~PRSCachedFrame();

	PRSVideoFrame *frame;
	int id;
};
