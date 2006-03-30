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
	unsigned __int32 utemp = 4 + 4 + 4 + 2 + 2 + 2 + 1 + 1;
	vec.resize(utemp+8);
	size_t pos = 0;

	// Write block identifier
	char str[] = "DSP";
	memcpy(&vec[pos],str,4);
	pos += 4;

	// Write block length
	memcpy(&vec[pos],&utemp,4);
	pos += 4;

	// Write start time
	utemp = start;
	memcpy(&vec[pos],&utemp,4);
	pos += 4;

	// Write end time
	utemp = end;
	memcpy(&vec[pos],&utemp,4);
	pos += 4;

	// Write image identifier
	utemp = id;
	memcpy(&vec[pos],&utemp,4);
	pos += 4;

	// Write layer
	__int16 shorttemp = layer;
	memcpy(&vec[pos],&shorttemp,2);
	pos += 2;

	// Write x
	shorttemp = x;
	memcpy(&vec[pos],&shorttemp,2);
	pos += 2;

	// Write y
	shorttemp = y;
	memcpy(&vec[pos],&shorttemp,2);
	pos += 2;

	// Write alpha multiplier
	unsigned __int8 chartemp = alpha;
	memcpy(&vec[pos],&chartemp,1);
	pos += 1;

	// Write blend mode
	chartemp = blend;
	memcpy(&vec[pos],&chartemp,1);
	pos += 1;
}


/////////////
// Read data
void PRSDisplay::ReadData(std::vector<char> &vec) {
}
