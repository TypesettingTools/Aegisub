// Copyright (c) 2006, Rodrigo Braz Monteiro
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


///////////
// Headers
#include "png_wrap.h"
#include "prs_image.h"
#include "prs_video_frame.h"


///////////////
// Constructor
PRSImage::PRSImage() {
	id = -1;
	imageType = NULL_IMG;
	dataLen = 0;
	data = 0;
}


//////////////
// Destructor
PRSImage::~PRSImage() {
	if (data) {
		delete [] data;
		data = 0;
		dataLen = 0;
	}
}


//////////////
// Write data
void PRSImage::WriteData(std::vector<char> &vec) {
	// Set length
	unsigned __int32 size = 4 + 4 + 4 + dataLen;
	vec.resize(size+8);
	size_t pos = 0;

	// Write block identifier
	char str[] = "IMG";
	memcpy(&vec[pos],str,4);
	pos += 4;

	// Write block length
	memcpy(&vec[pos],&size,4);
	pos += 4;

	// Write image identifier
	memcpy(&vec[pos],&id,4);
	pos += 4;

	// Write image format
	memcpy(&vec[pos],&imageType,4);
	pos += 4;

	// Write data length
	memcpy(&vec[pos],&dataLen,4);
	pos += 4;

	// Write data
	memcpy(&vec[pos],data,dataLen);
	pos += dataLen;
}


/////////////
// Read data
void PRSImage::ReadData(std::vector<char> &vec) {
	// Set length
	size_t size = vec.size();
	size_t pos = 0;

	// Read image identifier
	memcpy(&id,&vec[pos],4);
	pos += 4;

	// Read image format
	memcpy(&imageType,&vec[pos],4);
	pos += 4;

	// Read data length
	memcpy(&dataLen,&vec[pos],4);
	pos += 4;

	// Read data
	data = new char[dataLen];
	memcpy(data,&vec[pos],dataLen);
	pos += dataLen;
}


////////////////
// Decode frame
PRSVideoFrame *PRSImage::GetDecodedFrame() {
	// Create frame
	PRSVideoFrame *frame = new PRSVideoFrame;

	try {
		PNGWrapper png;
		png.SetData(data);
		png.Read(frame);
	}
	
	// Handle errors
	catch (...) {
		delete frame;
		return NULL;
	}

	// Return frame
	return frame;
}
