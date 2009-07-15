// This is a cleaned up version of the configuration used to build the Windows
// release version of Aegisub 2.1.7.
// The BUILD_CREDIT was changed from "nielsm" to "Anonymous".
//
// Copyright 2009 Niels Martin Hansen
// (Note that this copyright is dubious, this file might classify as
// a trivial work and not be coverable.)
// If applicable, this file is covered by the same terms and conditions as
// the rest of the Aegisub source code.
//
// Please see config_windows0.h from trunk for more details on these options.

#pragma once


// If you will be building a custom version of Aegisub 2.1.7, please change
// this to your name to mark the build.
#ifndef BUILD_CREDIT
#define BUILD_CREDIT "Anonymous"
#endif

// These defines should always apply on Windows
#define HAVE_LITTLE_ENDIAN
#undef HAVE_BIG_ENDIAN


// Automation 4 engines
#define WITH_AUTOMATION
#define WITH_AUTO4_LUA
#define WITH_AUTO3
//#define WITH_PERL
//#define WITH_PERLCONSOLE
//#define WITH_RUBY


// Audio players
#define WITH_DIRECTSOUND
//#define WITH_PORTAUDIO
//#define WITH_ALSA
//#define WITH_OPENAL
//#define WITH_PULSEAUDIO


// Avisynth and FFMS2 don't work satisfactory on Win64 yet, disable them on
// 64 bit builds.
#if !defined(_M_X64) && !defined(_M_IA64)
# define WITH_AVISYNTH
# define WITH_FFMPEGSOURCE
#endif
//#define WITH_DIRECTSHOW
//#define WITH_FFMPEG
//#define WITH_STATIC_FFMPEG


// The 2.1.7 release was built with Freetype 2.3.7
#define WITH_FREETYPE2
#define FT2_LIB_RELEASE "freetype237.lib"
#define FT2_LIB_DEBUG "freetype237_D.lib"
//#define WITH_FONTCONFIG

// Automatic text encoding detection
#define WITH_UNIVCHARDET

// Subtitle renderers (CSRI is used for VSFilter)
#define WITH_CSRI
//#define WITH_LIBASS

// Spell checker enabled
#define WITH_HUNSPELL

// Misc defines, you probably don't want startuplog
//#define WITH_STARTUPLOG
#define FINAL_RELEASE
