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
#include "../section_entry_style.h"
#include "../serialize.h"
#include "tr1.h"

namespace Athenasub {

	// Style
	class StyleASS : public CStyle, public SerializeText {
	private:
		String name;
		String font;
		float fontSize;
		int formatVersion;

		array<Colour,5> colour;	// 0 = Primary, 1 = Secondary, 2 = Tertiary, 3 = Outline, 4 = Shadow
		array<int,4> margin;

		bool bold;
		bool italic;
		bool underline;
		bool strikeout;

		int borderStyle;
		int alignment;
		int encoding;
		int relativeTo;

		float scalex;
		float scaley;
		float spacing;
		float angle;
		float outline_w;
		float shadow_w;

		bool Parse(String data,int version);
		int AlignSSAtoASS(int ssaAlignment) const;
		int AlignASStoSSA(int assAlignment) const;
		String ToText(int param) const;

	public:
		// Constructors
		StyleASS();
		StyleASS(String data,int version);

		// Basic features
		String GetDefaultGroup() const;
		Entry Clone() const { return Entry(new StyleASS(*this)); }

		// Indexing
		virtual bool IsIndexable() const { return true; }
		virtual String GetIndexName() const { return GetName(); }

		// Read accessors
		String GetName() const { return name; }
		String GetFontName() const { return font; }
		float GetFontSize() const { return fontSize; }
		const Colour& GetColour(int n) const { return colour.at(n); }
		int GetMargin(int n) const { return margin.at(n); }
	};

}
