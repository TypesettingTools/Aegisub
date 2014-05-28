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

/// @file ass_style.cpp
/// @brief Class for style definitions in subtitles
/// @ingroup subs_storage
///

#include "ass_style.h"

#include "subtitle_format.h"
#include "utils.h"

#include <libaegisub/format.h>
#include <libaegisub/split.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <wx/intl.h>

AssStyle::AssStyle() {
	std::fill(Margin.begin(), Margin.end(), 10);

	UpdateData();
}

namespace {
class parser {
	boost::split_iterator<agi::StringRange::const_iterator> pos;

	std::string next_tok() {
		if (pos.eof())
			throw SubtitleFormatParseError("Malformed style: not enough fields", nullptr);
		return agi::str(trim_copy(*pos++));
	}

public:
	parser(std::string const& str) {
		auto colon = find(str.begin(), str.end(), ':');
		if (colon != str.end())
			pos = agi::Split(agi::StringRange(colon + 1, str.end()), ',');
	}

	void check_done() const {
		if (!pos.eof())
			throw SubtitleFormatParseError("Malformed style: too many fields", nullptr);
	}

	std::string next_str() { return next_tok(); }
	agi::Color next_color() { return next_tok(); }

	int next_int() {
		try {
			return boost::lexical_cast<int>(next_tok());
		}
		catch (boost::bad_lexical_cast const&) {
			throw SubtitleFormatParseError("Malformed style: bad int field", nullptr);
		}
	}

	double next_double() {
		try {
			return boost::lexical_cast<double>(next_tok());
		}
		catch (boost::bad_lexical_cast const&) {
			throw SubtitleFormatParseError("Malformed style: bad double field", nullptr);
		}
	}

	void skip_token() {
		if (!pos.eof())
			++pos;
	}
};
}

AssStyle::AssStyle(std::string const& str, int version) {
	parser p(str);

	name = p.next_str();
	font = p.next_str();
	fontsize = p.next_double();

	if (version != 0) {
		primary = p.next_color();
		secondary = p.next_color();
		outline = p.next_color();
		shadow = p.next_color();
	}
	else {
		primary = p.next_color();
		secondary = p.next_color();

		// Skip tertiary color
		p.skip_token();

		// Read shadow/outline color
		outline = p.next_color();
		shadow = outline;
	}

	bold = !!p.next_int();
	italic = !!p.next_int();

	if (version != 0) {
		underline = !!p.next_int();
		strikeout = !!p.next_int();

		scalex = p.next_double();
		scaley = p.next_double();
		spacing = p.next_double();
		angle = p.next_double();
	}
	else {
		// SSA defaults
		underline = false;
		strikeout = false;

		scalex = 100;
		scaley = 100;
		spacing = 0;
		angle = 0.0;
	}

	borderstyle = p.next_int();
	outline_w = p.next_double();
	shadow_w = p.next_double();
	alignment = p.next_int();

	if (version == 0)
		alignment = SsaToAss(alignment);

	Margin[0] = mid(0, p.next_int(), 9999);
	Margin[1] = mid(0, p.next_int(), 9999);
	Margin[2] = mid(0, p.next_int(), 9999);

	// Skip alpha level
	if (version == 0)
		p.skip_token();

	encoding = p.next_int();

	p.check_done();

	UpdateData();
}

void AssStyle::UpdateData() {
	replace(name.begin(), name.end(), ',', ';');
	replace(font.begin(), font.end(), ',', ';');

	data = agi::format("Style: %s,%s,%g,%s,%s,%s,%s,%d,%d,%d,%d,%g,%g,%g,%g,%d,%g,%g,%i,%i,%i,%i,%i",
		name, font, fontsize,
		primary.GetAssStyleFormatted(),
		secondary.GetAssStyleFormatted(),
		outline.GetAssStyleFormatted(),
		shadow.GetAssStyleFormatted(),
		(bold? -1 : 0), (italic ? -1 : 0),
		(underline ? -1 : 0), (strikeout ? -1 : 0),
		scalex, scaley, spacing, angle,
		borderstyle, outline_w, shadow_w, alignment,
		Margin[0], Margin[1], Margin[2], encoding);
}

void AssStyle::GetEncodings(wxArrayString &encodingStrings) {
	encodingStrings.Clear();
	encodingStrings.Add(wxString("0 - ") + _("ANSI"));
	encodingStrings.Add(wxString("1 - ") + _("Default"));
	encodingStrings.Add(wxString("2 - ") + _("Symbol"));
	encodingStrings.Add(wxString("77 - ") + _("Mac"));
	encodingStrings.Add(wxString("128 - ") + _("Shift_JIS"));
	encodingStrings.Add(wxString("129 - ") + _("Hangeul"));
	encodingStrings.Add(wxString("130 - ") + _("Johab"));
	encodingStrings.Add(wxString("134 - ") + _("GB2312"));
	encodingStrings.Add(wxString("136 - ") + _("Chinese BIG5"));
	encodingStrings.Add(wxString("161 - ") + _("Greek"));
	encodingStrings.Add(wxString("162 - ") + _("Turkish"));
	encodingStrings.Add(wxString("163 - ") + _("Vietnamese"));
	encodingStrings.Add(wxString("177 - ") + _("Hebrew"));
	encodingStrings.Add(wxString("178 - ") + _("Arabic"));
	encodingStrings.Add(wxString("186 - ") + _("Baltic"));
	encodingStrings.Add(wxString("204 - ") + _("Russian"));
	encodingStrings.Add(wxString("222 - ") + _("Thai"));
	encodingStrings.Add(wxString("238 - ") + _("East European"));
	encodingStrings.Add(wxString("255 - ") + _("OEM"));
}

int AssStyle::AssToSsa(int ass_align) {
	switch (ass_align) {
		case 1:  return 1;
		case 2:  return 2;
		case 3:  return 3;
		case 4:  return 9;
		case 5:  return 10;
		case 6:  return 11;
		case 7:  return 5;
		case 8:  return 6;
		case 9:  return 7;
		default: return 2;
	}
}

int AssStyle::SsaToAss(int ssa_align) {
	switch(ssa_align) {
		case 1:  return 1;
		case 2:  return 2;
		case 3:  return 3;
		case 5:  return 7;
		case 6:  return 8;
		case 7:  return 9;
		case 9:  return 4;
		case 10: return 5;
		case 11: return 6;
		default: return 2;
	}
}
