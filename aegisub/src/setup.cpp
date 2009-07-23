// Copyright (c) 2006, Rodrigo Braz Monteiro
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


///////////
// Headers
#include "config.h"

#include <wx/wxprec.h>



//////////////////////////////////
///////// MSVC Libraries /////////
//////////////////////////////////
#if __VISUALC__ >= 1200

/////////////
// wxWidgets
#ifdef __WXDEBUG__
#pragma comment(lib, "wxzlibd.lib")
#pragma comment(lib, "wxpngd.lib")
#else
#pragma comment(lib, "wxzlib.lib")
#pragma comment(lib, "wxpng.lib")
#endif

#if wxCHECK_VERSION(2, 9, 0)
#ifdef __WXDEBUG__
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase29ud.lib")
#pragma comment(lib, "wxbase29ud_net.lib")
#pragma comment(lib, "wxmsw29ud_media.lib")
#pragma comment(lib, "wxmsw29ud_core.lib")
#pragma comment(lib, "wxmsw29ud_adv.lib")
#pragma comment(lib, "wxmsw29ud_gl.lib")
#pragma comment(lib, "wxmsw29ud_stc.lib")
#pragma comment(lib, "wxscintillad.lib")
#pragma comment(lib, "wxbase29ud_xml.lib")
#pragma comment(lib, "wxexpatd.lib")
#else
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase29u.lib")
#pragma comment(lib, "wxbase29u_net.lib")
#pragma comment(lib, "wxmsw29u_media.lib")
#pragma comment(lib, "wxmsw29u_core.lib")
#pragma comment(lib, "wxmsw29u_adv.lib")
#pragma comment(lib, "wxmsw29u_gl.lib")
#pragma comment(lib, "wxmsw29u_stc.lib")
#pragma comment(lib, "wxscintilla.lib")
#pragma comment(lib, "wxbase29u_xml.lib")
#pragma comment(lib, "wxexpat.lib")
#endif

#elif wxCHECK_VERSION(2, 8, 0)
#ifdef __WXDEBUG__
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase28ud.lib")
#pragma comment(lib, "wxbase28ud_net.lib")
#pragma comment(lib, "wxmsw28ud_media.lib")
#pragma comment(lib, "wxmsw28ud_core.lib")
#pragma comment(lib, "wxmsw28ud_adv.lib")
#pragma comment(lib, "wxmsw28ud_gl.lib")
#pragma comment(lib, "wxmsw28ud_stc.lib")
#pragma comment(lib, "wxbase28ud_xml.lib")
#pragma comment(lib, "wxexpatd.lib")
#else
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase28u.lib")
#pragma comment(lib, "wxbase28u_net.lib")
#pragma comment(lib, "wxmsw28u_media.lib")
#pragma comment(lib, "wxmsw28u_core.lib")
#pragma comment(lib, "wxmsw28u_adv.lib")
#pragma comment(lib, "wxmsw28u_gl.lib")
#pragma comment(lib, "wxmsw28u_stc.lib")
#pragma comment(lib, "wxbase28u_xml.lib")
#pragma comment(lib, "wxexpat.lib")
#endif

#else 

#error "Aegisub requires wxWidgets 2.8.0 or superior"

#endif // wxWidgets


////////////////////////////
// Standard Win32 Libraries
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "opengl32.lib")


////////////////
// FFMpegSource
#ifdef WITH_FFMPEGSOURCE
#pragma comment(lib, "ffms2.lib")
#endif

#ifdef WITH_QUICKTIME
#pragma comment(lib, "QTMLClient.lib")
#endif

#ifdef WITH_PORTAUDIO
#pragma comment(lib,"portaudio_x86.lib")
#endif


////////////////
// Direct Sound
#ifdef WITH_DIRECTSOUND
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#endif


////////
// Ruby
#ifdef WITH_RUBY
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"msvcrt-ruby18-static.lib")
#endif


/////////////
// FreeType2
#ifdef WITH_FREETYPE2
#ifdef __WXDEBUG__
#ifdef FT2_LIB_DEBUG
#pragma comment(lib,FT2_LIB_DEBUG)
#endif
#else
#ifdef FT2_LIB_RELEASE
#pragma comment(lib,FT2_LIB_RELEASE)
#endif
#endif
#endif


///////////////
// Font Config
#ifdef WITH_FONTCONFIG
#pragma comment(lib,"libfontconfig.lib")
#endif


////////
// Perl
#ifdef WITH_PERL
#pragma comment(lib,"perl510.lib")
#endif


#endif // VisualC
