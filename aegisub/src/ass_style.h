// Copyright (c) 2005, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/

/// @file ass_style.h
/// @see ass_style.cpp
/// @ingroup subs_storage
///

#include "ass_entry.h"

#include <libaegisub/color.h>

#include <array>
#include <wx/arrstr.h>

class AssStyle : public AssEntry {
	std::string data;

public:
	std::string name;   ///< Name of the style; must be case-insensitively unique within a file despite being case-sensitive
	std::string font;   ///< Font face name
	double fontsize; ///< Font size

	agi::Color primary;   ///< Default text color
	agi::Color secondary; ///< Text color for not-yet-reached karaoke syllables
	agi::Color outline;   ///< Outline color
	agi::Color shadow;    ///< Shadow color

	bool bold;
	bool italic;
	bool underline;
	bool strikeout;

	double scalex;    ///< Font x scale with 100 = 100%
	double scaley;    ///< Font y scale with 100 = 100%
	double spacing;   ///< Additional spacing between characters in pixels
	double angle;     ///< Counterclockwise z rotation in degrees
	int borderstyle;  ///< 1: Normal; 3: Opaque box; others are unused in Aegisub
	double outline_w; ///< Outline width in pixels
	double shadow_w;  ///< Shadow distance in pixels
	int alignment;    ///< \an-style line alignment
	std::array<int, 3> Margin; ///< Left/Right/Vertical
	int encoding;     ///< ASS font encoding needed for some non-unicode fonts

	/// Update the raw line data after one or more of the public members have been changed
	void UpdateData();

	/// @brief Get a list of valid ASS font encodings
	static void GetEncodings(wxArrayString &encodingStrings);

	AssStyle();
	AssStyle(std::string const& data, int version=1);

	const std::string GetEntryData() const override { return data; }
	std::string GetSSAText() const override;
	AssEntryGroup Group() const override { return ENTRY_STYLE; }
	AssEntry *Clone() const override;

	/// Convert an ASS alignment to the equivalent SSA alignment
	static int AssToSsa(int ass_align);
	/// Convert a SSA  alignment to the equivalent ASS alignment
	static int SsaToAss(int ssa_align);
};
