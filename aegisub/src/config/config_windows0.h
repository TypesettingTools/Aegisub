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
#define BUILD_CREDIT "Anonymous"
#endif


// Endianness: We don't support any Windows version that runs on big endian
#define HAVE_LITTLE_ENDIAN
#undef HAVE_BIG_ENDIAN


////////////// HIGH PRIORITY /////////////

// Enable Automation
// Requires: Nothing
#define WITH_AUTOMATION


// Enable Automation 4 Lua
// Requires: Lua 5.1 (in repository)
#define WITH_AUTO4_LUA


// Enable Automation 3
// Requires: auto3 dll (in repository), Lua (in repository)
#define WITH_AUTO3


// Enable DirectSound audio player
// Requires: DirectX SDK
#define WITH_DIRECTSOUND


// Enable Avisynth
// Requires: nothing (just the avisynth dlls)
#define WITH_AVISYNTH



///////////// MEDIUM PRIORITY ////////////

// Enable FreeType2 font lister for the fonts collector
// If you're on Visual Studio, also uncomment the library names and make sure they match the files that you have
// Requires: FreeType2
#define WITH_FREETYPE2
#define FT2_LIB_RELEASE "freetype235.lib"
#define FT2_LIB_DEBUG "freetype235_D.lib"


// Enable CSRI, required for styles previews in the style editor and some video providers
// Requires: csri (in repository)
#define WITH_CSRI


// Enable universal charset detector, so Aegisub can automatically detect the encoding of non-unicode subtitles
// Requires: universalcharset (in repository)
#define WITH_UNIVCHARDET


// Enable Hunspell-based spellchecker
// Requires: hunspell (in repository for Win32)
// If you have an old version of Hunspell (that uses Hunspell::put_word() instead of Hunspell::add()),
// uncomment the second line as well.
#define WITH_HUNSPELL
//#define WITH_OLD_HUNSPELL



/////////////// LOW PRIORITY ////////////

// Enable DirectShow video provider
// Requires: DirectShow "baseclasses", DirectX SDK
//#define WITH_DIRECTSHOW


// Enable Perl scripting
// Requires: perl library (ActivePerl comes with one for Visual C++ under lib\core\)
//#define WITH_PERL

// Enable PerlConsole (a debug tool for the perl engine)
// You don't want it
//#define WITH_PERLCONSOLE



///////////// NOT RECOMMENDED /////////////

// Enable FontConfig
// Requires: fontconfig
//#define WITH_FONTCONFIG


// Enable libass
// Requires: libass
//#define WITH_LIBASS


// Enable FFmpeg video and audio decoders
// Requires: libavcodec, libavformat, libswscale, libavutil
// If you compiled static libraries (yes, by default), uncomment the second line as well,
// and remember to add the correct .a files to the linker's additional dependencies.
// #define WITH_FFMPEG
// #define WITH_STATIC_FFMPEG


// Enable ffmpegsource video and audio providers
// Requires: ffmpegsource version 2
//#define WITH_FFMPEGSOURCE


// Enable Ruby support for Automation
// Requires: Ruby 1.9
//#define WITH_RUBY


// Enable PortAudio audio player
// Requires PortAudio release 18
//#define WITH_PORTAUDIO


// Enable PortAudio audio player version 2
// Requires PortAudio release 19
//#define WITH_PORTAUDIO2


// Enable ALSA audio player
// Requires Linux and libasound
//#define WITH_ALSA


// Enable OpenAL audio player
// Requires OpenAL development libraries and headers
//#define WITH_OPENAL


// Enable Pulse Audio audio player
// Requires libpulse (and a *NIX compatible system and a running sound server to actually use)
//#define WITH_PULSEAUDIO
