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

#include "format_ass_dialogue_delta.h"
#include "format_ass_dialogue.h"
#ifdef _MSC_VER
#include "../stdint.h"
#else
#include <stdint.h>
#endif

using namespace Athenasub;

////////////////////////////////////
// Encode delta between two entries
VoidPtr DialogueASSDeltaCoder::EncodeDelta(VoidPtr _from,VoidPtr _to,bool withTextFields) const
{
	// Cast pointers
	shared_ptr<DialogueASS> from = static_pointer_cast<DialogueASS> (_from);
	shared_ptr<DialogueASS> to = static_pointer_cast<DialogueASS> (_to);

	// Determine changes
	unsigned short mask = 0;
	if (from->isComment != to->isComment) mask |= 0x0001;
	if (from->layer != to->layer) mask |= 0x0002;
	if (from->time[0].GetMS() != to->time[0].GetMS()) mask |= 0x0004;
	if (from->time[1].GetMS() != to->time[1].GetMS()) mask |= 0x0008;
	for (size_t i=0;i<4;i++) {
		if (from->margin[i] != to->margin[i]) mask |= 0x0010 << i;
		if (withTextFields && from->text[i] != to->text[i]) mask |= 0x0100 << i;
	}

	// Calculate final size and allocate
	size_t size = 2 + (mask & 0x0002)*4 + (mask & 0x0004)*4 + (mask & 0x0008)*4;
	for (size_t i=0;i<4;i++) {
		if (mask & (0x0010 << i)) size += 2;
		if (mask & (0x0100 << i)) size += (to->text[i].Length()+1)*2;
	}
	shared_ptr<char> delta (new char[size],ArrayDeleter());

	// Write data
	char *final = delta.get();
	GetDelta(mask,final,to);

	// Return delta
	return delta;
}


////////////////////////////
// Calculates reverse delta
VoidPtr DialogueASSDeltaCoder::EncodeReverseDelta(VoidPtr _delta,VoidPtr object) const
{
	// Get mask
	char *data = (static_pointer_cast<char> (_delta)).get();
	int mask = *((short*) data);
	shared_ptr<DialogueASS> to = static_pointer_cast<DialogueASS> (object);

	// Calculate final size and allocate
	size_t size = 2 + (mask & 0x0002)*4 + (mask & 0x0004)*4 + (mask & 0x0008)*4;
	for (size_t i=0;i<4;i++) {
		size += (mask & (0x0010 << i)) * 2 + (mask & (0x0100 << i)) * (to->text[i].Length()+1)*2;
	}
	shared_ptr<char> delta (new char[size],ArrayDeleter());

	// Write data
	char *final = delta.get();
	GetDelta(mask,final,to);
	return delta;
}



///////////////////////////
// Applies delta to a line
void DialogueASSDeltaCoder::ApplyDelta(VoidPtr _delta,VoidPtr object) const
{
	// Process parameters
	char *data = (static_pointer_cast<char> (_delta)).get();
	shared_ptr<DialogueASS> to = static_pointer_cast<DialogueASS> (object);

	// Read mask
	int mask = *((int16_t*) data);
	data += 2;

	// Toggle comment
	if (mask & 0x0001) to->isComment = !to->isComment;

	// Read layer
	if (mask & 0x0002) {
		to->layer = *((int32_t*) data);
		data += 4;
	}

	// Read times
	for (size_t i=0;i<2;i++) {
		if (mask & (0x0004 << i)) {
			to->time[i].SetMS(*((int32_t*) data));
			data += 4;
		}
	}

	// Read margins
	for (size_t i=0;i<4;i++) {
		if (mask & (0x0010 << i)) {
			to->margin[i] = *((int16_t*) data);
			data += 2;
		}
	}

	// Read text fields
	for (size_t i=0;i<4;i++) {
		if (mask & (0x0100 << i)) {
			to->text[i] = String((wchar_t*) data);
			data += (to->text[i].Length() + 1)*2;
		}
	}
}


///////////////////////////
// Actually get delta data
void DialogueASSDeltaCoder::GetDelta(int mask,char *final,shared_ptr<DialogueASS> to) const
{
	// Write mask
	*((uint16_t*) final) = mask;
	final += 2;

	// Write layer
	if (mask & 0x0002) {
		*((int32_t*) final) = to->layer;
		final += 4;
	}

	// Write times
	for (size_t i=0;i<2;i++) {
		if (mask & (0x0004 << i)) {
			*((int32_t*) final) = to->time[i].GetMS();
			final += 4;
		}
	}

	// Write margins
	for (size_t i=0;i<4;i++) {
		if (mask & (0x0010 << i)) {
			*((int16_t*) final) = to->margin[i];
			final += 2;
		}
	}

	// Write text fields
	for (size_t i=0;i<4;i++) {
		if (mask & (0x0100 << i)) {
			size_t len = (to->text[i].Length() + 1)*2;
			memcpy(final,to->text[i].c_str(),len);
			final += len-2;
			*(final++) = 0;
			*(final++) = 0;
			final += len;
		}
	}
}
