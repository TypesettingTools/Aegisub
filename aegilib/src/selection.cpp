// Copyright (c) 2008, Rodrigo Braz Monteiro
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
// AEGISUB/GORGONSUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "selection.h"
using namespace Gorgonsub;


///////////////
// Constructor
Selection::Selection()
: count(0)
{
}


////////////////
// Adds a range
void Selection::AddRange(const Range &range)
{
	// TODO
	(void) range;
}


///////////////////
// Removes a range
void Selection::RemoveRange(const Range &range)
{
	// TODO
	(void) range;
}


//////////////////
// Get a specific
size_t Selection::GetLine(size_t n) const
{
	// Find the nth line
	size_t cur = 0;
	size_t len = ranges.size();
	for (size_t i=0;i<len;i++) {
		cur += ranges[i].GetSize();
		if (cur > n) return ranges[i].GetLine(n-ranges[i].GetStart());
	}
	return ~0UL;
}


////////////////////////////
// Append another selection
void Selection::AddSelection (const Selection &param)
{
	(void) param;
}


//////////////////////////////
// Subtract another selection
void Selection::RemoveSelection (const Selection &param)
{
	(void) param;
}

