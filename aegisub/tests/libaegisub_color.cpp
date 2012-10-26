	// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

#include <libaegisub/color.h>

#include "main.h"
#include "util.h"

class lagi_color : public libagi {
};

namespace agi {
	::std::ostream& operator<<(::std::ostream& os, Color const& c) {
		return os << "agi::Color(" << (int)c.r << "," << (int)c.g << "," << (int)c.b << "," << (int)c.a << ")";
	}
}


TEST(lagi_color, hex) {
	EXPECT_EQ(agi::Color(0, 0, 0), agi::Color("#000"));
	EXPECT_EQ(agi::Color(0, 0, 0), agi::Color("#000000"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("#FFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("#FFF"));
	EXPECT_EQ(agi::Color(255, 0, 127), agi::Color("#FF007F"));
	EXPECT_EQ(agi::Color(16, 32, 48), agi::Color("#102030"));

	EXPECT_EQ("#000000", agi::Color(0, 0, 0).GetHexFormatted());
	EXPECT_EQ("#FFFFFF", agi::Color(255, 255, 255).GetHexFormatted());
	EXPECT_EQ("#FF007F", agi::Color(255, 0, 127).GetHexFormatted());
	EXPECT_EQ("#102030", agi::Color(16, 32, 48).GetHexFormatted());
}

TEST(lagi_color, rgb) {
	EXPECT_EQ(agi::Color(0, 0, 0), "rgb(0, 0, 0)");
	EXPECT_EQ(agi::Color(255, 255, 255), "rgb(255, 255, 255)");
	EXPECT_EQ(agi::Color(255, 255, 255), "rgb(255,255,255)");
	EXPECT_EQ(agi::Color(255, 0, 127), "rgb(255, 0, 127)");
	EXPECT_EQ(agi::Color(16, 32, 48), "rgb(  16  , 32  , 48  )");

	EXPECT_EQ("rgb(0, 0, 0)", agi::Color(0, 0, 0).GetRgbFormatted());
	EXPECT_EQ("rgb(255, 255, 255)", agi::Color(255, 255, 255).GetRgbFormatted());
	EXPECT_EQ("rgb(255, 0, 127)", agi::Color(255, 0, 127).GetRgbFormatted());
	EXPECT_EQ("rgb(16, 32, 48)", agi::Color(16, 32, 48).GetRgbFormatted());
}

TEST(lagi_color, ass_ovr) {
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("&HFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("&HFFFFFF&"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("&hFFFFFF&"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("HFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("hFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("FFFFFF"));
	EXPECT_EQ(agi::Color(48, 32, 16), agi::Color("&H102030&"));

	EXPECT_EQ("&H000000&", agi::Color().GetAssOverrideFormatted());
	EXPECT_EQ("&HFFFFFF&", agi::Color(255, 255, 255).GetAssOverrideFormatted());
	EXPECT_EQ("&H030201&", agi::Color(1, 2, 3).GetAssOverrideFormatted());
	EXPECT_EQ("&H030201&", agi::Color(1, 2, 3, 4).GetAssOverrideFormatted());
}

TEST(lagi_color, ass_style) {
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("&HFFFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("&HFFFFFFFF&"));
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("&hFFFFFFFF&"));
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("HFFFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("hFFFFFFFF"));
	EXPECT_EQ(agi::Color(255, 255, 255, 255), agi::Color("FFFFFFFF"));
	EXPECT_EQ(agi::Color(4, 3, 2, 1), agi::Color("&H01020304&"));

	EXPECT_EQ("&H00000000", agi::Color().GetAssStyleFormatted());
	EXPECT_EQ("&H00FFFFFF", agi::Color(255, 255, 255).GetAssStyleFormatted());
	EXPECT_EQ("&H00030201", agi::Color(1, 2, 3).GetAssStyleFormatted());
	EXPECT_EQ("&H04030201", agi::Color(1, 2, 3, 4).GetAssStyleFormatted());
}

TEST(lagi_color, ssa) {
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("16777215"));
	EXPECT_EQ(agi::Color(255, 255, 255), agi::Color("-1"));
	EXPECT_EQ(agi::Color(127, 0, 255), agi::Color("16711807"));
	EXPECT_EQ(agi::Color(48, 32, 16), agi::Color("1056816"));

	EXPECT_EQ("0", agi::Color(0, 0, 0).GetSsaFormatted());
	EXPECT_EQ("16777215", agi::Color(255, 255, 255).GetSsaFormatted());
	EXPECT_EQ("16711807", agi::Color(127, 0, 255).GetSsaFormatted());
	EXPECT_EQ("1056816", agi::Color(48, 32, 16).GetSsaFormatted());
}
