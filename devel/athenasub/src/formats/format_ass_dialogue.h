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
#include "section_entry_dialogue.h"
#include "format_ass_dialogue_delta.h"
#include "serialize.h"
#include "athenatime.h"

namespace Athenasub {

	// Dialogue
	class DialogueASS : public CDialogue, public SerializeText {
		friend class DialogueASSDeltaCoder;

	private:
		array<String,4> text;	// 0 = text, 1 = style, 2 = actor, 3 = effect
		array<Time,2> time;
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
		String GetDefaultGroup() const { return "Events"; }
		Entry Clone() const { return Entry(new DialogueASS(*this)); }
		//DeltaCoderPtr GetDeltaCoder() const { return DeltaCoderPtr(new DialogueASSDeltaCoder()); }

		// Capabilities
		bool HasText() const { return true; }
		bool HasTime() const { return true; }
		bool HasStyle() const { return true; }
		bool HasMargins() const { return true; }

		// Read accessors
		const Time& GetStartTime() const { return time[0]; }
		const Time& GetEndTime() const { return time[1]; }
		bool IsComment() const { return isComment; }
		int GetLayer() const { return layer; }
		int GetMargin(int n) const { return margin.at(n); }
		const String& GetText() const { return text[0]; }
		const String& GetStyle() const { return text[1]; }
		const String& GetActor() const { return text[2]; }
		const String& GetUserField() const { return text[3]; }

		// Write acessors
		void SetStartTime(const Time &setStart) { time[0].SetMS(setStart.GetMS()); }
		void SetEndTime(const Time &setEnd) { time[1].SetMS(setEnd.GetMS()); }
		void SetComment(bool _isComment) { isComment = _isComment; }
		void SetLayer(int _layer) { layer = _layer; }
		void SetMargin(int _margin,int value) { margin.at(_margin) = value; }
		void SetText(const String &setText) { text[0] = setText; }
		void SetStyle(const String &style) { text[1] = style; }
		void SetActor(const String &actor) { text[2] = actor; }
		void SetUserField(const String &userField) { text[3] = userField; }

		// Operator overloading
		bool operator==(const DialogueASS& param) const;
		bool operator!=(const DialogueASS& param) const;
	};

}
