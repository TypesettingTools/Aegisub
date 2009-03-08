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
#include "prs_display.h"


///////////////
// Constructor
PRSDisplay::PRSDisplay() {
	start = -1;
	end = -1;
	startFrame = -1;
	endFrame = -1;
	id = -1;
	layer = 0;
	x = 0;
	y = 0;
	alpha = 255;
	blend = BLEND_NORMAL;
}


//////////////
// Destructor
PRSDisplay::~PRSDisplay() {
}


//////////////
// Write data
void PRSDisplay::WriteData(std::vector<char> &vec) {
	// Set length
	unsigned __int32 size = 4 + 4 + 4 + 4 + 4 + 4 + 2 + 2 + 1 + 1;
	vec.resize(size+8);
	size_t pos = 0;

	// Write block identifier
	char str[] = "DSP";
	memcpy(&vec[pos],str,4);
	pos += 4;

	// Write block length
	memcpy(&vec[pos],&size,4);
	pos += 4;

	// Write start frame
	memcpy(&vec[pos],&startFrame,4);
	pos += 4;

	// Write end frame
	memcpy(&vec[pos],&endFrame,4);
	pos += 4;

	// Write start time
	memcpy(&vec[pos],&start,4);
	pos += 4;

	// Write end time
	memcpy(&vec[pos],&end,4);
	pos += 4;

	// Write image identifier
	memcpy(&vec[pos],&id,4);
	pos += 4;

	// Write layer
	memcpy(&vec[pos],&layer,4);
	pos += 4;

	// Write x
	memcpy(&vec[pos],&x,2);
	pos += 2;

	// Write y
	memcpy(&vec[pos],&y,2);
	pos += 2;

	// Write alpha multiplier
	memcpy(&vec[pos],&alpha,1);
	pos += 1;

	// Write blend mode
	memcpy(&vec[pos],&blend,1);
	pos += 1;
}


/////////////
// Read data
void PRSDisplay::ReadData(std::vector<char> &vec) {
	// Set length
	unsigned __int32 size = 4 + 4 + 4 + 4 + 4 + 4 + 2 + 2 + 1 + 1;
	if (size != vec.size()) return;
	size_t pos = 0;

	// Read start frame
	memcpy(&startFrame,&vec[pos],4);
	pos += 4;

	// Read end frame
	memcpy(&endFrame,&vec[pos],4);
	pos += 4;

	// Read start time
	memcpy(&start,&vec[pos],4);
	pos += 4;

	// Read end time
	memcpy(&end,&vec[pos],4);
	pos += 4;

	// Read image identifier
	memcpy(&id,&vec[pos],4);
	pos += 4;

	// Read layer
	memcpy(&layer,&vec[pos],4);
	pos += 4;

	// Read x
	memcpy(&x,&vec[pos],2);
	pos += 2;

	// Read y
	memcpy(&y,&vec[pos],2);
	pos += 2;

	// Read alpha multiplier
	memcpy(&alpha,&vec[pos],1);
	pos += 1;

	// Read blend mode
	memcpy(&blend,&vec[pos],1);
	pos += 1;
}
