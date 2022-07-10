// Copyright (c) 2009-2010, Niels Martin Hansen
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

#include "audio_colorscheme.h"

#include "audio_rendering_style.h"
#include "colorspace.h"
#include "options.h"

#include <libaegisub/exception.h>

AudioColorScheme::AudioColorScheme(int prec, std::string const& scheme_name,
                                   int audio_rendering_style)
    : palette((3 << prec) + 3), factor((size_t)1 << prec) {
	std::string opt_base = "Colour/Schemes/" + scheme_name + "/";
	switch(static_cast<AudioRenderingStyle>(audio_rendering_style)) {
		case AudioStyle_Normal: opt_base += "Normal/"; break;
		case AudioStyle_Inactive: opt_base += "Inactive/"; break;
		case AudioStyle_Selected: opt_base += "Selection/"; break;
		case AudioStyle_Primary: opt_base += "Primary/"; break;
		default: throw agi::InternalError("Unknown audio rendering styling");
	}

	double h_base = OPT_GET(opt_base + "Hue Offset")->GetDouble();
	double h_scale = OPT_GET(opt_base + "Hue Scale")->GetDouble();
	double s_base = OPT_GET(opt_base + "Saturation Offset")->GetDouble();
	double s_scale = OPT_GET(opt_base + "Saturation Scale")->GetDouble();
	double l_base = OPT_GET(opt_base + "Lightness Offset")->GetDouble();
	double l_scale = OPT_GET(opt_base + "Lightness Scale")->GetDouble();

	for(size_t i = 0; i <= factor; ++i) {
		auto t = (double)i / factor;
		hsl_to_rgb(mid<int>(0, h_base + t * h_scale, 255), mid<int>(0, s_base + t * s_scale, 255),
		           mid<int>(0, l_base + t * l_scale, 255), &palette[i * 3 + 0], &palette[i * 3 + 1],
		           &palette[i * 3 + 2]);
	}
}
