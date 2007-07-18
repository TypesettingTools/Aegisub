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


///////////
// Headers
#include "vfw_wrap.h"
#ifdef __WINDOWS__
#include <vfw.h>
#endif


/////////////////////
// Get keyframe list
wxArrayInt VFWWrapper::GetKeyFrames(wxString filename) {
	wxArrayInt frames;

#ifdef __WINDOWS__
	// Init vfw
	AVIFileInit();

	// Open file
	PAVIFILE pfile; 
	long hr = AVIFileOpen(&pfile, filename.wc_str(), OF_SHARE_DENY_WRITE, 0); 
	if (hr != 0) {
		AVIFileExit();
		throw _T("Unable to open AVI file for reading keyframes.");
    }

	// Open stream
	PAVISTREAM ppavi;
	hr = AVIFileGetStream(pfile,&ppavi,streamtypeVIDEO,0);
	if (hr != 0) {
		AVIFileRelease(pfile);
		AVIFileExit();
		throw _T("Unable to open AVI stream for reading keyframes.");
	}

	// Get stream data
	AVISTREAMINFO avis;
	AVIStreamInfo(ppavi,&avis,sizeof(avis));
	size_t frame_c = avis.dwLength;

	// Loop through stream
	for (size_t i=0;i<frame_c;i++) {
		if (AVIStreamIsKeyFrame(ppavi,i)) {
			frames.Add(i);
		}
	}

	// Clean up
	AVIFileRelease(pfile);
	AVIFileExit();
#endif

	return frames;
}
