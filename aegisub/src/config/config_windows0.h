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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file config/config_windows0.h
/// @brief Base configuration for Windows builds, to be copied to config_windows.h and edited by the developer
/// @ingroup build
///

#pragma once

// Build credit: Set this to a string with your name as you want it to appear on the program
#ifndef BUILD_CREDIT
#define BUILD_CREDIT "Anonymous"
#endif

// Endianness: We don't support any Windows version that runs on big endian
#define HAVE_LITTLE_ENDIAN
#undef HAVE_BIG_ENDIAN

////////////// HIGH PRIORITY /////////////

// Enable Automation 4 Lua
// Requires: Lua 5.1 (in repository)
#define WITH_AUTO4_LUA

// Enable DirectSound audio player
// Requires: DirectX SDK
#define WITH_DIRECTSOUND
#ifdef WITH_DIRECTSOUND
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#endif

// Enable Avisynth
// Requires: nothing (just the avisynth dlls at runtime)
#define WITH_AVISYNTH


// Enable FFMS2 video and audio providers
// Requires: FFMS2 SDK
//#define WITH_FFMS2
#ifdef WITH_FFMS2
#pragma comment(lib, "ffms2.lib")
#endif

///////////// MEDIUM PRIORITY ////////////

// Enable FreeType2 font lister for the fonts collector
// Make sure the version numbers are correct, as the library names change
// Requires: FreeType2
#define WITH_FREETYPE2
#ifdef WITH_FREETYPE2
#ifdef _DEBUG
#pragma comment(lib, "freetype235_D.lib")
#else
#pragma comment(lib, "freetype235.lib")
#endif


// Enable FreeType2 font lister for the fonts collector
// If you're on Visual Studio, also uncomment the library names and make sure they match the files that you have
// Requires: FreeType2
#define WITH_FREETYPE2
#define FT2_LIB_RELEASE "freetype235.lib"
#define FT2_LIB_DEBUG "freetype235_D.lib"


// Enable CSRI, required for styles previews in the style editor and some video providers
// Requires: csri (in repository)
#define WITH_CSRI


// Enable Hunspell-based spellchecker
// Requires: hunspell (in repository for Win32)
#define WITH_HUNSPELL


// Use FFTW instead of shipped FFT code
// FFTW <http://fftw.org/> is a very fast library for computing the discrete fourier transform, but is a bit
// tricky to get working on Windows, and has the additional problem of being GPL licensed.
// Enable this option to use FFTW to get faster rendering of the audio spectrogram
//#define WITH_FFTW3
#ifdef WITH_FFTW3
#pragma comment(lib,libfftw.lib)
#endif
// Specify tags the update checker accepts
// See <http://devel.aegisub.org/wiki/Technical/UpdateChecker> for details on tags.
// Depending on who will be using your build, you may or may not want to have the
// "source" tag in here. If the string is empty, the update checker will reject any
// update offered.
#if defined(_M_IX86)
# define UPDATE_CHECKER_ACCEPT_TAGS "windows source"
#elif defined(_M_X64)
# define UPDATE_CHECKER_ACCEPT_TAGS "win64 source"
#endif

// Where the update checker should look for updates
#define UPDATE_CHECKER_SERVER "updates.aegisub.org"
#define UPDATE_CHECKER_BASE_URL "/3.0.3"

///////////// NOT RECOMMENDED /////////////

// Enable FontConfig
// Requires: fontconfig
//#define WITH_FONTCONFIG
#ifdef WITH_FONTCONFIG
#pragma comment(lib,"libfontconfig.lib")
#endif

// Enable libass
// Requires: libass
//#define WITH_LIBASS
#ifdef WITH_LIBASS
#pragma comment(lib, "libass.lib")
#endif

// Enable PortAudio audio player
// Requires PortAudio release 19
//#define WITH_PORTAUDIO
#ifdef WITH_PORTAUDIO
#pragma comment(lib,"portaudio_x86.lib")
#endif

// Enable ALSA audio player
// Requires Linux and libasound
//#define WITH_ALSA

// Enable OpenAL audio player
// Requires OpenAL development libraries and headers
//#define WITH_OPENAL

// Enable Pulse Audio audio player
// Requires libpulse (and a *NIX compatible system and a running sound server to actually use)
//#define WITH_LIBPULSE

// Display trace-level diagnostic messages during startup
// Only enable for making special builds for end users having trouble with starting Aegisub
//#define WITH_STARTUPLOG
