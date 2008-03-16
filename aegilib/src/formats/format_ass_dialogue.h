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

#pragma once
#include "gorgonstring.h"
#include "section_entry_dialogue.h"
#include "serialize.h"

namespace Gorgonsub {

	// Dialogue
	class DialogueASS : public SectionEntryDialogue, public SerializeText {
	private:
		String text;
		String style;
		String effect;
		String actor;
		Time start,end;
		array<short,4> margin;
		int layer;
		bool isComment;

		bool Parse(String data,int version);
		String ToText(int param) const;

	public:
		// Constructors
		DialogueASS();
		DialogueASS(const String &data,int version);

		// Basic features
		String GetDefaultGroup() const { return L"Events"; }
		SectionEntryPtr Clone() const { return SectionEntryPtr(new DialogueASS(*this)); }

		// Capabilities
		bool HasText() const { return true; }
		bool HasTime() const { return true; }
		bool HasStyle() const { return true; }
		bool HasMargins() const { return true; }

		// Read accessors
		const String& GetText() const { return text; }
		Time GetStartTime() const { return start; }
		Time GetEndTime() const { return end; }
		bool IsComment() const { return isComment; }
		int GetLayer() const { return layer; }
		int GetMargin(int n) const { return margin.at(n); }
		const String& GetStyle() const { return style; }
		const String& GetActor() const { return actor; }
		const String& GetUserField() const { return effect; }

		// Write acessors
		void SetText(const String &setText) { text = setText; }
		void SetStartTime(Time setStart) { start = setStart; }
		void SetEndTime(Time setEnd) { end = setEnd; }
		void SetComment(bool _isComment) { isComment = _isComment; }
		void SetLayer(int _layer) { layer = _layer; }
		void SetMargin(int _margin,int value) { margin.at(_margin) = value; }
		void SetStyle(const String &_style) { style = _style; }
		void SetUserField(const String &userField) { effect = userField; }
	};

};
