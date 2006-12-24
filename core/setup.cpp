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
#include <wx/wxprec.h>
#include "setup.h"


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

#if wxCHECK_VERSION(2, 8, 0)
#ifdef __WXDEBUG__
//#pragma comment(lib, "wxmsw28ud_richtext.lib")
//#pragma comment(lib, "wxmsw28ud_html.lib")
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase28ud.lib")
#pragma comment(lib, "wxmsw28ud_media.lib")
#pragma comment(lib, "wxmsw28ud_core.lib")
#pragma comment(lib, "wxmsw28ud_adv.lib")
#else
//#pragma comment(lib, "wxmsw28u_richtext.lib")
//#pragma comment(lib, "wxmsw28u_html.lib")
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase28u.lib")
#pragma comment(lib, "wxmsw28u_media.lib")
#pragma comment(lib, "wxmsw28u_core.lib")
#pragma comment(lib, "wxmsw28u_adv.lib")
#endif

#else 
#if wxCHECK_VERSION(2, 7, 0)
#ifdef __WXDEBUG__
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase27ud.lib")
#pragma comment(lib, "wxmsw27ud_media.lib")
#pragma comment(lib, "wxmsw27ud_core.lib")
#pragma comment(lib, "wxmsw27ud_adv.lib")
#else
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase27u.lib")
#pragma comment(lib, "wxmsw27u_media.lib")
#pragma comment(lib, "wxmsw27u_core.lib")
#pragma comment(lib, "wxmsw27u_adv.lib")
#endif

#else if wxCHECK_VERSION(2, 6, 0)
#ifdef __WXDEBUG__
#pragma comment(lib, "wxregexud.lib")
#pragma comment(lib, "wxbase26ud.lib")
#pragma comment(lib, "wxmsw26ud_media.lib")
#pragma comment(lib, "wxmsw26ud_core.lib")
#pragma comment(lib, "wxmsw26ud_adv.lib")
#else
#pragma comment(lib, "wxregexu.lib")
#pragma comment(lib, "wxbase26u.lib")
#pragma comment(lib, "wxmsw26u_media.lib")
#pragma comment(lib, "wxmsw26u_core.lib")
#pragma comment(lib, "wxmsw26u_adv.lib")
#endif

#endif

#endif // wxWidgets


/////////////
// Scintilla
#ifdef __WXDEBUG__
#pragma comment(lib, "wxscintillaud.lib")
#else
#pragma comment(lib, "wxscintillau.lib")
#endif


////////////////////////////
// Standard Win32 Libraries
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "wsock32.lib")


///////////////
// DirectSound
#if USE_DIRECTSOUND == 1
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#endif


/////////////
// PortAudio
#if USE_PORTAUDIO == 1
#pragma comment(lib,"portaudio.lib")
#endif


///////
// Lua
#ifdef __WXDEBUG__
#pragma comment(lib,"lua503d.lib")
#else
#pragma comment(lib,"lua503.lib")
#endif


//////////////
// FreeType 2
#ifdef __WXDEBUG__
#pragma comment(lib,"freetype2110MT_D.lib")
#else
#pragma comment(lib,"freetype2110MT.lib")
#endif


#endif // VisualC
