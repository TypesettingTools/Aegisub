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

// You seriously want these.
// Aegisub might be impossible to build without some of these,
// or just be nearly useless without them.


// Enable Automation
// All this does is allow Automation libraries to be built at all,
// and makes the GUI for Automation stuff show up.
// (It should be possible to build WITH_AUTOMATION but without any
// Automation engines enabled, albeit not interesting.)
// Requires: Nothing
#define WITH_AUTOMATION


// Enable Automation 4 Lua
// Requires: Lua 5.1 (in repository)
#define WITH_AUTO4_LUA


// Enable DirectSound audio player
// This enables both the new and old DSound player implementations.
// The new DSound player (DirectSoundPlayer2) is the recommended
// audio player on Windows.
// Requires: DirectX SDK (or Windows SDK 7.0)
#define WITH_DIRECTSOUND
#ifdef WITH_DIRECTSOUND
# pragma comment(lib, "dsound.lib")
# pragma comment(lib, "dxguid.lib")
#endif


// Enable Avisynth video and audio providers
// Requires: nothing (just the Avisynth DLLs at runtime)
#define WITH_AVISYNTH


// Enable CSRI (Common Subtitle Renderer Interface)
// Used to interface with VSFilter and potentially other subtitle renderers
// supporting the CSRI API, though none other do.
// You really want
// Requires: csri helper library (in repository) or another CSRI implementation
#define WITH_CSRI
// Easiest is to include the CSRI helper library in your solution and make
// Aegisub depend on it, so it gets linked in.
// Alternatively, link VSFilter directly and avoid the CSRI helper library, like so:
//#pragma comment(lib,"vsfilter-aegisub32.lib")


// Enable FreeType2 font lister for the fonts collector
// (You cannot build without having a font lister, and the FT2-based
// one is the only currently working on Windows.)
// Make sure to replace the library name by the actual one you use,
// or otherwise ensure the FT2 library gets linked to Aegisub.
// Requires: FreeType2
#define WITH_FREETYPE2
#ifdef WITH_FREETYPE2
# ifndef _DEBUG
#  pragma comment(lib,"freetype235.lib")
# else
#  pragma comment(lib,"freetype235_D.lib")
# endif
#endif



///////////// MEDIUM PRIORITY ////////////

// Nice to have things, easy to get working.


// Enable Automation 3
// Requires: auto3 dll (in repository), Lua 5.0 (in repository)
#define WITH_AUTO3


// Enable ffmpegsource video and audio providers
// Requires: ffmpegsource version 2 .lib file
//#define WITH_FFMPEGSOURCE
#ifdef WITH_FFMPEGSOURCE
# pragma comment(lib, "ffms2.lib")
#endif


// Enable universal charset detector, so Aegisub can automatically detect the encoding of non-unicode subtitles
// Requires: universalcharset (in repository)
#define WITH_UNIVCHARDET


// Enable Hunspell-based spellchecker
// Requires: hunspell (in repository for Win32)
// If you have an old version of Hunspell (that uses Hunspell::put_word() instead of Hunspell::add()),
// uncomment the second line as well.
#define WITH_HUNSPELL
//#define WITH_OLD_HUNSPELL


// Enable "final release" mode
// Displays different version numbers in About box and title bar, and omits detailed version information from
// the title bar. Only core developers should enable this, and only when making builds for mass consumption.
//#define FINAL_RELEASE



///////////// NOT RECOMMENDED /////////////

// The options in this section are generally deprecated or just not
// suited for Windows builds. They might not be compileable at all.


// Enable DirectShow video provider, unmaintained
// Requires: DirectShow "baseclasses", DirectX SDK, luck
//#define WITH_DIRECTSHOW
#ifdef WITH_DIRECTSHOW
# pragma comment(lib, "strmiids.lib")
# ifdef _DEBUG
#  pragma comment(lib, "strmbasdu.lib")
# else
#  pragma comment(lib, "strmbaseu.lib")
# endif
#endif


// Enable Perl scripting, unmaintainted
// Requires: perl library (ActivePerl comes with one for Visual C++ under lib\core\), luck
//#define WITH_PERL
#ifdef WITH_PERL
# pragma comment(lib,"perl510.lib")
#endif

// Enable PerlConsole (a debug tool for the perl engine)
// You don't want it
//#define WITH_PERLCONSOLE


// Enable FontConfig
// Alternate font lister for fonts collector, probably doesn't work on Windows.
// Requires: fontconfig
//#define WITH_FONTCONFIG
#ifdef WITH_FONTCONFIG
# pragma comment(lib,"libfontconfig.lib")
#endif


// Enable libass
// Requires: libass
//#define WITH_LIBASS


// Enable FFmpeg video and audio decoders
// Deprecated by the FFmpegSource library, might not compile
// Requires: libavcodec, libavformat, libswscale, libavutil
// If you compiled static libraries (yes, by default), uncomment the second line as well,
// and remember to add the correct .a files to the linker's additional dependencies.
//#define WITH_FFMPEG
//#define WITH_STATIC_FFMPEG
#ifdef WITH_FFMPEG
# ifndef WITH_STATIC_FFMPEG
#  pragma comment(lib, "avcodec-51.lib")
#  pragma comment(lib, "avformat-51.lib")
#  pragma comment(lib, "avutil-49.lib")
# endif
#endif


// Enable Ruby support for Automation, unmaintained
// Requires: Ruby 1.9
//#define WITH_RUBY
#ifdef WITH_RUBY
# pragma comment(lib,"ws2_32.lib")
# pragma comment(lib,"msvcrt-ruby18-static.lib")
#endif


// Enable PortAudio audio player
// Requires PortAudio release 18
//#define WITH_PORTAUDIO
#ifdef WITH_PORTAUDIO
# pragma comment(lib,"portaudio_x86.lib")
#endif


// Enable PortAudio audio player version 2
// Requires PortAudio release 19
//#define WITH_PORTAUDIO2


// Enable ALSA audio player
// Requires Linux and libasound
//#define WITH_ALSA


// Enable OpenAL audio player
// Works on Windows, but no real advantage of using it over DSound.
// Requires OpenAL development libraries and headers
//#define WITH_OPENAL


// Enable Pulse Audio audio player
// Requires libpulse (and a *NIX compatible system and a running sound server to actually use)
//#define WITH_PULSEAUDIO


// Display trace-level diagnostic messages during startup
// Only enable for making special builds for end users having trouble with starting Aegisub
//#define WITH_STARTUPLOG
