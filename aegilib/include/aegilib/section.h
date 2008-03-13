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
// AEGISUB/AEGILIB
//
// Website: http://www.aegisub.net
// Contact: mailto:amz@aegisub.net
//

#pragma once
#include "aegistring.h"
#include "section_entry.h"
#include "tr1.h"
#include <list>
#include <map>

namespace Aegilib {

	// Section class
	class Section {
	private:
		std::vector<SectionEntryPtr> entries;
		std::map<String,String> properties;
		String name;

	public:
		Section(String name);
		~Section() {}

		// Section name
		String SetName(const String& newName) { name = newName; }
		const String& GetName() const { return name; }
		
		// Script properties
		void SetProperty(const String &key,const String &value);
		void UnsetProperty(const String &key);
		String GetProperty(const String &key) const;
		bool HasProperty(const String &key) const;
		size_t GetPropertyCount() const;
		String GetPropertyName(size_t index) const;

		// Entries
		void AddEntry(SectionEntryPtr entry);
		void RemoveEntryByIndex(size_t index);
		void RemoveEntry(SectionEntryPtr entry);
		SectionEntryConstPtr GetEntry(size_t index) const;
		size_t GetEntryCount() const;
	};
	typedef shared_ptr<Section> SectionPtr;

};
