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

#include "ass_entry.h"

#include <libaegisub/color.h>

#include <array>

class wxArrayString;

class AssStyle final : public AssEntry, public AssEntryListHook {
	std::string data;

public:
	std::string name = "Default"; ///< Name of the style; must be case-insensitively unique within a file despite being case-sensitive
	std::string font = "Arial";   ///< Font face name
	double fontsize = 20.;        ///< Font size

	agi::Color primary{ 255, 255, 255 }; ///< Default text color
	agi::Color secondary{ 255, 0, 0 };   ///< Text color for not-yet-reached karaoke syllables
	agi::Color outline{ 0, 0, 0 };       ///< Outline color
	agi::Color shadow{ 0, 0, 0 };        ///< Shadow color

	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool strikeout = false;

	double scalex = 100.;      ///< Font x scale with 100 = 100%
	double scaley = 100.;      ///< Font y scale with 100 = 100%
	double spacing = 0.;       ///< Additional spacing between characters in pixels
	double angle = 0.;         ///< Counterclockwise z rotation in degrees
	int borderstyle = 1;       ///< 1: Normal; 3: Opaque box; others are unused in Aegisub
	double outline_w = 2.;     ///< Outline width in pixels
	double shadow_w = 2.;      ///< Shadow distance in pixels
	int alignment = 2;         ///< \an-style line alignment
	std::array<int, 3> Margin; ///< Left / Right / Vertical
	int encoding = 1;          ///< ASS font encoding needed for some non-unicode fonts

	/// Update the raw line data after one or more of the public members have been changed
	void UpdateData();

	/// @brief Get a list of valid ASS font encodings
	static void GetEncodings(wxArrayString &encodingStrings);

	AssStyle();
	AssStyle(std::string const& data, int version=1);

	std::string const& GetEntryData() const { return data; }
	AssEntryGroup Group() const override;

	/// Convert an ASS alignment to the equivalent SSA alignment
	static int AssToSsa(int ass_align);
	/// Convert a SSA  alignment to the equivalent ASS alignment
	static int SsaToAss(int ssa_align);
};
