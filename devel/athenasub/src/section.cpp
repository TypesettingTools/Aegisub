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


#include "section.h"
using namespace Athenasub;


///////////////
// Constructor
CSection::CSection(String _name)
{
	name = _name;
}


///////////////////
// Append an entry
void CSection::AddEntry(Entry entry,int pos)
{
	// Insert to entry list
	if (pos == -1) entries.push_back(entry);
	else entries.insert(entries.begin()+pos,entry);

	// Add to index too if it's indexable
	if (entry->IsIndexable()) {
		index[entry->GetIndexName()] = entry;
	}
}


/////////////////////////
// Removes the nth entry
void CSection::RemoveEntryByIndex(size_t i)
{
	// Get entry iterator and erase
	std::vector<Entry>::iterator entry = entries.begin()+i;
	entries.erase(entry);

	// If it's indexable, remove it from index as well
	if ((*entry)->IsIndexable()) index.erase((*entry)->GetIndexName());
}


/////////////////////////////////
// Removes an entry by its value
void CSection::RemoveEntry(Entry entry)
{
	size_t len = entries.size();
	for (size_t i=0;i<len;i++) {
		if (entries[i] == entry) {
			entries.erase(entries.begin()+i);
			if (entry->IsIndexable()) index.erase(entry->GetIndexName());
			return;
		}
	}
}


////////////////////////////
// Retrieves entry by index
Entry CSection::GetEntry(size_t i) const
{
	return entries[i];
}


//////////////////////////////////////
// Retrieves entry reference by index
Entry& CSection::GetEntryRef(size_t i)
{
	return entries[i];
}


/////////////////////////
// Get number of entries
size_t CSection::GetEntryCount() const
{
	return entries.size();
}


//////////////////
// Set a property
void CSection::SetProperty(const String &key,const String &value)
{
	properties[key] = value;
}


//////////////////////
// Removes a property
void CSection::UnsetProperty(const String &key)
{
	std::map<String,String>::iterator iter = properties.find(key);
	if (iter != properties.end()) properties.erase(iter);
}


//////////////////////////
// Get a property's value
String CSection::GetProperty(const String &key) const
{
	std::map<String,String>::const_iterator iter = properties.find(key);
	if (iter != properties.end()) return iter->second;
	else return "";
}


///////////////////////////////
// Checks if it has a property
bool CSection::HasProperty(const String &key) const
{
	return properties.find(key) != properties.end();
}


//////////////////////
// Get property count
size_t CSection::GetPropertyCount() const
{
	return properties.size();
}


///////////////////////////////////
// Get name of a property by index
String CSection::GetPropertyName(size_t n) const
{
	std::map<String,String>::const_iterator iter=properties.begin();
	for (size_t i=0 ; iter!=properties.end() ; iter++,i++) {
		if (i == n) return iter->first;
	}
	return "";
}


///////////////////////
// Retrieve from index
Entry CSection::GetFromIndex(String key) const
{
	std::map<String,Entry>::const_iterator result = index.find(key);
	if (result != index.end()) return result->second;
	return Entry();
}
