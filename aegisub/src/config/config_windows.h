// Build configuration for Aegisub 2.1.8 release for Windows

// If making a custom 2.1.8 build please ensure you change the BUILD_CREDIT
// define and check that the FT2_LIB_* defines are correct.


#pragma once


#ifndef BUILD_CREDIT
#define BUILD_CREDIT "nielsm"
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
#define FT2_LIB_RELEASE "freetype237.lib"
#define FT2_LIB_DEBUG "freetype237_D.lib"
//#define WITH_FONTCONFIG

#define WITH_UNIVCHARDET

#define WITH_CSRI
//#define WITH_LIBASS

#define WITH_HUNSPELL

//#define WITH_STARTUPLOG
#define FINAL_RELEASE

// Define this if building an EXE for portable versions only. It makes the application
// manifest link the latest runtimes instead of the RTM version, but it might not work
// with plain vcredist_x86.exe installations.
//#define _BIND_TO_CURRENT_VCLIBS_VERSION 1
