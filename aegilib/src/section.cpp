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


#include "section.h"
using namespace Gorgonsub;


///////////////
// Constructor
Section::Section(String _name)
{
	name = _name;
}


///////////////////
// Append an entry
void Section::AddEntry(SectionEntryPtr entry)
{
	entries.push_back(entry);
}

void Section::RemoveEntryByIndex(size_t index)
{
	entries.erase(entries.begin()+index);
}

void Section::RemoveEntry(SectionEntryPtr entry)
{
	size_t len = entries.size();
	for (size_t i=0;i<len;i++) {
		if (entries[i] == entry) {
			entries.erase(entries.begin()+i);
			return;
		}
	}
}

SectionEntryConstPtr Section::GetEntry(size_t index) const
{
	return entries[index];
}

size_t Section::GetEntryCount() const
{
	return entries.size();
}


//////////////////
// Set a property
void Section::SetProperty(const String &key,const String &value)
{
	properties[key] = value;
}


//////////////////////
// Removes a property
void Section::UnsetProperty(const String &key)
{
	std::map<String,String>::iterator iter = properties.find(key);
	if (iter != properties.end()) properties.erase(iter);
}


//////////////////////////
// Get a property's value
String Section::GetProperty(const String &key) const
{
	std::map<String,String>::const_iterator iter = properties.find(key);
	if (iter != properties.end()) return iter->second;
	else return L"";
}


///////////////////////////////
// Checks if it has a property
bool Section::HasProperty(const String &key) const
{
	return properties.find(key) != properties.end();
}


//////////////////////
// Get property count
size_t Section::GetPropertyCount() const
{
	return properties.size();
}


///////////////////////////////////
// Get name of a property by index
String Section::GetPropertyName(size_t index) const
{
	std::map<String,String>::const_iterator iter=properties.begin();
	for (size_t i=0 ; iter!=properties.end() ; iter++,i++) {
		if (i == index) return iter->first;
	}
	return L"";
}
