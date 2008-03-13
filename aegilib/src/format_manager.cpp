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

#include "format_manager.h"
#include "formats/format_ass.h"
#include <wx/string.h>
using namespace Gorgonsub;


////////
// List
std::vector<const FormatPtr> FormatManager::formats;


////////////////
// Add a format
void FormatManager::AddFormat(const FormatPtr format)
{
	formats.push_back(format);
}


///////////////////////////////////
// Initialzie all built-in formats
void FormatManager::InitializeFormats()
{
	formats.push_back(FormatPtr(new FormatASS));
	formats.push_back(FormatPtr(new FormatSSA));
	formats.push_back(FormatPtr(new FormatASS2));
}


///////////////////////
// Removes all formats
void FormatManager::ClearFormats()
{
	formats.clear();
}


/////////////////////
// Number of formats
int FormatManager::GetFormatCount()
{
	return (int) formats.size();
}


////////////
// By index
const FormatPtr FormatManager::GetFormatByIndex(const int index)
{
	try {
		return formats.at(index);
	}
	catch (...) {
		return FormatPtr();
	}
}


///////////////
// By filename
const FormatPtr FormatManager::GetFormatFromFilename(const String &filename,bool read)
{
	size_t len = formats.size();
	for (size_t i=0;i<len;i++) {
		StringArray exts;
		if (read) exts = formats[i]->GetReadExtensions();
		else exts = formats[i]->GetWriteExtensions();
		size_t extn = exts.size();
		for (size_t j=0;j<extn;j++) {
			if (filename.EndsWith(exts[j])) return formats[i];
		}
	}
	return FormatPtr();
}


//////////////////
// By format name
const FormatPtr FormatManager::GetFormatFromName(const String &name)
{
	size_t len = formats.size();
	for (size_t i=0;i<len;i++) {
		if (name == formats[i]->GetName()) return formats[i];
	}
	return FormatPtr();
}

