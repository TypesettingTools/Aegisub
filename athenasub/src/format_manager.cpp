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

#include "format_manager.h"
#include "formats/format_ass.h"
#include "reader.h"
#include "text_reader.h"
#include <wx/string.h>
#include <algorithm>
using namespace Athenasub;


///////////
// Statics
std::vector<Format> FormatManager::formats;
bool FormatManager::formatsInitialized = false;


////////////////
// Add a format
void FormatManager::AddFormat(Format format)
{
	// Abort if there is already a format with this name
	String name = format->GetName();
	for (size_t i=0;i<formats.size();i++) {
		if (formats[i]->GetName() == name) return;
	}

	// Add
	formats.push_back(format);
}


///////////////////////////////////
// Initialize all built-in formats
void FormatManager::InitializeFormats()
{
	if (!formatsInitialized) {
		AddFormat(Format(new FormatASS()));
		AddFormat(Format(new FormatSSA()));
		AddFormat(Format(new FormatASS2()));

		formatsInitialized = true;
	}
}


///////////////////////
// Removes all formats
void FormatManager::ClearFormats()
{
	formats.clear();
	formatsInitialized = false;
}


/////////////////////
// Number of formats
int FormatManager::GetFormatCount()
{
	return (int) formats.size();
}


////////////
// By index
Format FormatManager::GetFormatByIndex(const int index)
{
	try {
		return formats.at(index);
	} catch (std::out_of_range &e) {
		(void) e;
		return Format();
	}
}


///////////////
// By filename
Format FormatManager::GetFormatFromFilename(const String &filename,bool read)
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
	return Format();
}


//////////////////
// By format name
Format FormatManager::GetFormatFromName(const String &name)
{
	size_t len = formats.size();
	for (size_t i=0;i<len;i++) {
		if (name == formats[i]->GetName()) return formats[i];
	}
	return Format();
}


///////////////////////////////////////////////////////
// Get a list of all formats compatible with this file
std::vector<Format> FormatManager::GetCompatibleFormatList(Reader reader)
{
	// Find all compatible formats and store them with their certainty
	std::vector<std::pair<float,Format> > results;
	size_t len = formats.size();
	for (size_t i=0;i<len;i++) {
		// Reset reader
		reader->Rewind();

		// Check how certain it is that it can read the format
		float certainty = formats[i]->CanReadFile(reader);

		// Compare to extension
		StringArray exts = formats[i]->GetReadExtensions();
		for (size_t j=0;j<exts.size();j++) {
			if (reader->GetFileName().EndsWith(exts[j],false)) {
				certainty *= 2.0f;
				break;
			}
		}
		
		// If it thinks that it can read the format, add to list.
		if (certainty > 0.0f) {
			results.push_back(std::pair<float,Format>(certainty,formats[i]));
		}
	}

	// Functor to sort them
	struct Comp {
		bool operator() (const std::pair<float,Format> &p1,const std::pair<float,Format> &p2) {
			return p1.first > p2.first;
		}
	};

	// Sort results and store them
	sort(results.begin(),results.end(),Comp());
	len = results.size();
	std::vector<Format> finalResults;
	for (size_t i=0;i<len;i++) {
		finalResults.push_back(results[i].second);
	}

	// Reset reader again and return results
	reader->Rewind();
	return finalResults;
}


//////////////////////////////////////
// Same as above, but from a filename
std::vector<Format> FormatManager::GetCompatibleFormatList(const String &filename,const String &encoding)
{
	return GetCompatibleFormatList(Reader(new CReader(filename,encoding)));
}
