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


////////////
// Includes
#include "ass_attachment.h"


///////////////
// Constructor
AssAttachment::AssAttachment() {
	data = boost::shared_ptr<AttachData> (new AttachData);
}


//////////////
// Destructor
AssAttachment::~AssAttachment() {
}


/////////
// Clone
AssEntry *AssAttachment::Clone() {
	// New object
	AssAttachment *clone = new AssAttachment;

	// Copy fields
	clone->filename = filename;
	clone->data = data;

	// Return
	return clone;
}


////////////
// Get data
const void *AssAttachment::GetData() {
	return data->GetData();
}


/////////////////
// Add more data
void AssAttachment::AddData(wxString _data) {
	data->AddData(_data);
}


//////////////////////
// Finish adding data
void AssAttachment::Finish() {
	data->Finish();
}



/////////////////// Attachment //////////////////
///////////////
// Constructor
AttachData::AttachData() {
	data = NULL;
}


//////////////
// Destructor
AttachData::~AttachData() {
	delete data;
}


////////////
// Get data
const void *AttachData::GetData() {
	return (void*) data;
}


////////////
// Add data
void AttachData::AddData(wxString data) {
}


//////////
// Finish
void AttachData::Finish() {
}
