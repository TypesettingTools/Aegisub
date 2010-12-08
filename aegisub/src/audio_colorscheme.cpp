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
//
// $Id$

/// @file audio_colorscheme.cpp
/// @ingroup audio_ui
///
/// Manage colour schemes for the audio display

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#endif

#include "audio_colorscheme.h"
#include "colorspace.h"

// Something is defining "min" and "max" macros, and they interfere with using std::min and std::max
#undef min
#undef max



void AudioColorScheme::InitIcyBlue_Normal()
{
	unsigned char *palptr = palette;
	for (size_t i = 0; i <= factor; ++i)
	{
		float t = (float)i / factor;
		int H = (int)(255 * (1.5 - t) / 2);
		int S = (int)(255 * (0.5 + t/2));
		int L = std::min(255, (int)(128 * 2 * t));
		hsl_to_rgb(H, S, L, palptr + 0, palptr + 1, palptr + 2);
		palptr += 4;
	}
}


void AudioColorScheme::InitIcyBlue_Selected()
{
	unsigned char *palptr = palette;
	for (size_t i = 0; i <= factor; ++i)
	{
		float t = (float)i / factor;
		int H = (int)(255 * (1.5 - t) / 2);
		int S = (int)(255 * (0.5 + t/2));
		int L = std::min(255, (int)(128 * (3 * t/2 + 0.5)));
		hsl_to_rgb(H, S, L, palptr + 0, palptr + 1, palptr + 2);
		palptr += 4;
	}
}
