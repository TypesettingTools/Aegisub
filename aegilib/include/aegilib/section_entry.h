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

#pragma once
#include "athenastring.h"
#include "tr1.h"
#include "deltacoder.h"
#include "api.h"

namespace Athenasub {

	// Types
	enum SectionEntryType {
		SECTION_ENTRY_PLAIN,
		SECTION_ENTRY_DIALOGUE,
		SECTION_ENTRY_STYLE,
		SECTION_ENTRY_FILE,
		SECTION_ENTRY_RAW
	};

	// Prototypes
	class Entry;
	class PlainText;
	class Dialogue;
	class Style;
	class Attachment;
	class RawEntry;
	typedef shared_ptr<Entry> EntryPtr;
	typedef shared_ptr<PlainText> PlainTextPtr;
	typedef shared_ptr<Dialogue> DialoguePtr;
	typedef shared_ptr<Style> StylePtr;
	typedef shared_ptr<Attachment> AttachmentPtr;
	typedef shared_ptr<RawEntry> RawEntryPtr;
	typedef shared_ptr<const Entry> EntryConstPtr;
	typedef shared_ptr<const PlainText> PlainTextConstPtr;
	typedef shared_ptr<const Dialogue> DialogueConstPtr;
	typedef shared_ptr<const Style> StyleConstPtr;
	typedef shared_ptr<const Attachment> AttachmentConstPtr;
	typedef shared_ptr<const RawEntry> RawEntryConstPtr;

	// Section entry class
	class Entry {
	protected:
		virtual ~Entry() {}
		const String& EmptyString() const;

	public:
		virtual SectionEntryType GetType() const =0;
		virtual String GetDefaultGroup() const =0;
		virtual EntryPtr Clone() const =0;

		virtual DeltaCoderPtr GetDeltaCoder() const { return DeltaCoderPtr(); }
		virtual bool IsIndexable() const { return false; }
		virtual String GetIndexName() const { return L""; }

		static const PlainTextPtr GetAsPlain(const EntryPtr &ptr);
		static const DialoguePtr GetAsDialogue(const EntryPtr &ptr);
		static const DialogueConstPtr GetAsDialogue(const EntryConstPtr &ptr);
		static const StylePtr GetAsStyle(const EntryPtr &ptr);
		static const AttachmentPtr GetAsFile(const EntryPtr &ptr);
		static const RawEntryPtr GetAsRaw(const EntryPtr &ptr);
	};

	// Section plain-text entry
	class PlainText : public Entry {
	public:
		SectionEntryType GetType() const { return SECTION_ENTRY_PLAIN; }
		virtual ~PlainText() {}
		virtual String GetText() const =0;
		virtual void SetText(const String &_data) =0;
	};

}
