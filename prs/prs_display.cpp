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
void PRSDisplay::WriteData(FILE *fp) {
	// Write block identifier
	fwrite("DSP",1,4,fp);

	// Write block length
	unsigned __int32 utemp = 4 + 4 + 4 + 2 + 2 + 2 + 1 + 1;
	fwrite(&utemp,4,1,fp);

	// Write start time
	utemp = start;
	fwrite(&utemp,4,1,fp);

	// Write end time
	utemp = end;
	fwrite(&utemp,4,1,fp);

	// Write image identifier
	utemp = id;
	fwrite(&utemp,4,1,fp);

	// Write layer
	__int16 shorttemp = layer;
	fwrite(&shorttemp,2,1,fp);

	// Write x
	shorttemp = x;
	fwrite(&shorttemp,2,1,fp);

	// Write y
	shorttemp = y;
	fwrite(&shorttemp,2,1,fp);

	// Write alpha multiplier
	unsigned __int8 chartemp = alpha;
	fwrite(&chartemp,1,1,fp);

	// Write blend mode
	chartemp = blend;
	fwrite(&chartemp,1,1,fp);
}
