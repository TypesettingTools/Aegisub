/*
This is the configuration used to build the official Windows release binaries
of Aegisub 2.1.9, provided for completeness. Also see config_windows0.h for
more detailed documentation.

Only Microsoft Visual C++ 2008 SP1 is supported for building, any other version
is at your own risk.
Non-Microsoft toolchains will likely not work and should not be attempted.

Remember to provide appropriate link libraries for all external dependencies.

Ensure the #error directive is removed before attempting to use these settings.
*/

#pragma once

#error Please make sure to edit configure_windows.h to your needs before attempting to build Aegisub.


// BUILD_CREDIT should be set to the name you want to identify your builds by
#ifndef BUILD_CREDIT
//#define BUILD_CREDIT "anonymous"
#endif

// Only little endian is supported on Windows
#define HAVE_LITTLE_ENDIAN
#undef HAVE_BIG_ENDIAN


// Perl and Ruby automation engines are deprecated
#define WITH_AUTOMATION
#define WITH_AUTO4_LUA
#define WITH_AUTO3
//#define WITH_PERL
//#define WITH_PERLCONSOLE
//#define WITH_RUBY


// While PortAudio and OpenAL do work on Windows, they are not recommended.
#define WITH_DIRECTSOUND
//#define WITH_PORTAUDIO
//#define WITH_ALSA
//#define WITH_OPENAL
//#define WITH_PULSEAUDIO


// Avisynth is not officially supported for 64 bit builds, though it may work
#if !defined(_M_X64) && !defined(_M_IA64)
# define WITH_AVISYNTH
#endif
// FFmpegSource is strongly recommended
#define WITH_FFMPEGSOURCE
// DirectShow video provider is deprecated and probably doesn't compile
//#define WITH_DIRECTSHOW


// Freetype2 is required, FontConfig is not supported on Windows.
#define WITH_FREETYPE2
//#define WITH_FONTCONFIG

// Just use it.
#define WITH_UNIVCHARDET

// Remember to link in a CSRI library, either the included dynamic discovery
// helper, or straight to a CSRI VSFilter build.
#define WITH_CSRI
//#define WITH_LIBASS

// Recommended.
#define WITH_HUNSPELL

// WITH_STARTUPLOG causes lots of annoying messages to be shown during startup
//#define WITH_STARTUPLOG

// This is what causes the program to identify itself as 2.1.9 and not as some
// development build
#define FINAL_RELEASE

// These tags define what kinds of updates will be offered
#if defined(_M_IX86)
# define UPDATE_CHECKER_ACCEPT_TAGS "windows"
#elif defined(_M_X64)
# define UPDATE_CHECKER_ACCEPT_TAGS "win64"
#endif

// This block is required if you want to link to VSFilter directly
/*
#if defined(_M_IX86)
# if defined(_DEBUG)
#  pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.VC90.DebugMFC' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
# else
#  pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.VC90.MFC' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
# endif
#elif defined(_M_X64)
# if defined(_DEBUG)
#  pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.VC90.DebugMFC' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
# else
#  pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.VC90.MFC' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
# endif
#endif
*/
