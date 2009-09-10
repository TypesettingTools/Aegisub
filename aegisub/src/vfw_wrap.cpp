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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file vfw_wrap.cpp
/// @brief Reading timecode and keyframe data from AVI files using Video for Windows
/// @ingroup video_input
///


#pragma once


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#ifdef __WINDOWS__
#include <vfw.h>
#endif
#endif

#include "vfw_wrap.h"


/// @brief Get keyframe list 
/// @param filename 
///
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
		switch (hr) {
			case AVIERR_BADFORMAT:
				throw _T("Unable to open AVI file for reading keyframes:\nThe file is corrupted, incomplete or has an otherwise bad format.");
			case AVIERR_MEMORY:
				throw _T("Unable to open AVI file for reading keyframes:\nThe file could not be opened because of insufficient memory.");
			case AVIERR_FILEREAD:
				throw _T("Unable to open AVI file for reading keyframes:\nAn error occurred reading the file. There might be a problem with the storage media.");
			case AVIERR_FILEOPEN:
				throw _T("Unable to open AVI file for reading keyframes:\nThe file could not be opened. It might be in use by another application, or you do not have permission to access it.");
			case REGDB_E_CLASSNOTREG:
				throw _T("Unable to open AVI file for reading keyframes:\nThere is no handler installed for the file extension. This might indicate a fundameltal problem in your Video for Windows installation, and can be caused by extremely stripped Windows installations.");
			default:
				throw _T("Unable to open AVI file for reading keyframes:\nUnknown error.");
		}
    }

	// Open stream
	PAVISTREAM ppavi;
	hr = AVIFileGetStream(pfile,&ppavi,streamtypeVIDEO,0);
	if (hr != 0) {
		AVIFileRelease(pfile);
		AVIFileExit();
		switch (hr) {
			case AVIERR_NODATA:
				throw _T("Unable to open AVI video stream for reading keyframes:\nThe file does not contain a usable video stream.");
			case AVIERR_MEMORY:
				throw _T("Unable to open AVI video stream for reading keyframes:\nNot enough memory.");
			default:
				throw _T("Unable to open AVI video stream for reading keyframes:\nUnknown error.");
		}
		
	}

	// Get stream data
	AVISTREAMINFO avis;
	hr = AVIStreamInfo(ppavi,&avis,sizeof(avis));
	if (hr != 0) {
		AVIStreamRelease(ppavi);
		AVIFileRelease(pfile);
		AVIFileExit();
		throw _T("Unable to read keyframes from AVI file:\nCould not get stream information.");
	}
	size_t frame_c = avis.dwLength;

	// Loop through stream
	for (size_t i=0;i<frame_c;i++) {
		if (AVIStreamIsKeyFrame(ppavi,(int)i)) {
			frames.Add(i);
		}
	}

	// Clean up
	AVIStreamRelease(ppavi);
	AVIFileRelease(pfile);
	AVIFileExit();
#endif

	return frames;
}


