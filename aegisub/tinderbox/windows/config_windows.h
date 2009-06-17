// Copyright (c) 2007, Rodrigo Braz Monteiro
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
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#pragma once


// Build credit: Set this to a string with your name as you want it to appear on the program
#ifndef BUILD_CREDIT
#define BUILD_CREDIT "Buildbot-ANPAN"
#endif


#define HAVE_LITTLE_ENDIAN
#undef HAVE_BIG_ENDIAN


#define WITH_AUTOMATION
#define WITH_AUTO4_LUA
#define WITH_AUTO3
//#define WITH_PERL
//#define WITH_PERLCONSOLE
//#define WITH_RUBY


#define WITH_DIRECTSOUND
//#define WITH_PORTAUDIO
//#define WITH_ALSA
//#define WITH_OPENAL
//#define WITH_PULSEAUDIO


#if !defined(_M_X64) && !defined(_M_IA64)
# define WITH_AVISYNTH
# define WITH_FFMPEGSOURCE
#endif
//#define WITH_DIRECTSHOW
//#define WITH_FFMPEG
//#define WITH_STATIC_FFMPEG


#define WITH_FREETYPE2
#define FT2_LIB_RELEASE "freetype239.lib"
#define FT2_LIB_DEBUG "freetype239_D.lib"
//#define WITH_FONTCONFIG

#define WITH_UNIVCHARDET

#define WITH_CSRI
//#define WITH_LIBASS

#define WITH_HUNSPELL

//#define WITH_STARTUPLOG
//#define FINAL_RELEASE
