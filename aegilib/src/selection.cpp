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
// AEGISUB/ATHENASUB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#include "selection.h"
using namespace Athenasub;


///////////////
// Constructor
CSelection::CSelection()
: count(0)
{
}


////////////////
// Adds a range
void CSelection::AddRange(const Range &range)
{
	ranges.push_back(range);
	UpdateCount();
}


///////////////////
// Removes a range
void CSelection::RemoveRange(const Range &range)
{
	// TODO
	(void) range;
	THROW_ATHENA_EXCEPTION(Exception::TODO);
}


////////////////////////////////////////////////////////////////////
// Normalizes all ranges, that is, gets rid of overlaps and whatnot
void CSelection::NormalizeRanges()
{
	// Has anything to do?
	if (ranges.size() == 0) return;

	// Find largest value
	size_t max = 0;
	size_t len = ranges.size();
	for (size_t i=0;i<len;i++) {
		if (ranges[i].GetEnd() > max) max = ranges[i].GetEnd();
	}

	// Allocate a vector of that size
	std::vector<bool> selected(max);
	for (size_t i=0;i<len;i++) {
		for (size_t j=ranges[i].GetStart();j<ranges[i].GetEnd();j++) {
			selected[j] = true;
		}
	}

	// Clear ranges and re-build them
	ranges.clear();
	size_t start = 0;
	bool inside = false;
	for (size_t i=0;i<max;i++) {
		// Enter
		if (!inside && selected[i]) {
			start = i;
			inside = true;
		}

		// Exit
		else if (inside && !selected[i]) {
			ranges.push_back(Range(start,i));
			inside = false;
		}
	}
	if (inside) ranges.push_back(Range(start,max));

	// Update count
	UpdateCount();
}


//////////////////
// Get a specific
size_t CSelection::GetLine(size_t n) const
{
	// Find the nth line
	size_t cur = 0;
	size_t len = ranges.size();
	for (size_t i=0;i<len;i++) {
		size_t curLen = ranges[i].GetSize();
		if (cur+curLen > n) return ranges[i].GetLine(n-cur);
		cur += curLen;
	}
	return ~0UL;
}


////////////////////////////
// Append another CSelection
void CSelection::AddSelection (const Selection &param)
{
	(void) param;
}


////////////////
// Update count
void CSelection::UpdateCount()
{
	count = 0;
	size_t len = ranges.size();
	for (size_t i=0;i<len;i++) {
		count += ranges[i].GetSize();
	}
}


//////////////////////////////
// Subtract another CSelection
void CSelection::RemoveSelection (const Selection &param)
{
	(void) param;
}
