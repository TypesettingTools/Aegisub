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
#include "athenasub.h"
#include "section_entry.h"


namespace Athenasub {

	// Dialogue class
	class CDialogue : public IDialogue {
	private:
#define ThrowUnsupported() THROW_ATHENA_EXCEPTION(Exception::Unsupported_Format_Feature)
		static const String& EmptyString();

	public:
		// Destructor
		virtual ~CDialogue() {}

		// Type
		SectionEntryType GetType() const { return SECTION_ENTRY_DIALOGUE; }
		Dialogue GetAsDialogue() { return Dialogue(this); }

		// Capabilities
		virtual bool HasText() const { return false; }
		virtual bool HasImage() const { return false; }
		virtual bool HasTime() const { return false; }
		virtual bool HasFrame() const { return false; }
		virtual bool HasStyle() const { return false; }
		virtual bool HasActor() const { return false; }
		virtual bool HasMargins() const { return false; }

		// Read accessors
		virtual const String& GetText() const { ThrowUnsupported(); }
		//virtual Time& GetStartTime() const { ThrowUnsupported(); }
		//virtual Time& GetEndTime() const { ThrowUnsupported(); }
		virtual int GetStartFrame() const { ThrowUnsupported(); }
		virtual int GetEndFrame() const { ThrowUnsupported(); }
		virtual bool IsComment() const { ThrowUnsupported(); }
		virtual int GetLayer() const { ThrowUnsupported(); }
		virtual int GetMargin(int n) const { (void) n; ThrowUnsupported(); }
		virtual const String& GetStyle() const { ThrowUnsupported(); }
		virtual const String& GetActor() const { ThrowUnsupported(); }
		virtual const String& GetUserField() const { ThrowUnsupported(); }

		// Write accessors
		virtual void SetText(const String& text) { (void) text; ThrowUnsupported(); }
		virtual void SetStartTime(const Time& start) { (void) start; ThrowUnsupported(); }
		virtual void SetEndTime(const Time& end) { (void) end; ThrowUnsupported(); }
		virtual void SetStartFrame(int start) { (void) start; ThrowUnsupported(); }
		virtual void SetEndFrame(int end) { (void) end; ThrowUnsupported(); }
		virtual void SetComment(bool isComment) { (void) isComment; ThrowUnsupported(); }
		virtual void SetLayer(int layer) { (void) layer; ThrowUnsupported(); }
		virtual void SetMargin(int margin,int value) { (void) margin; (void) value; ThrowUnsupported(); }
		virtual void SetStyle(const String& style) { (void) style; ThrowUnsupported(); }
		virtual void SetActor(const String& actor) { (void) actor; ThrowUnsupported(); }
		virtual void SetUserField(const String& userField) { (void) userField; ThrowUnsupported(); }
	};

}
