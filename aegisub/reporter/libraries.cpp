// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id: libraries.cpp 3601 2009-09-28 08:08:16Z verm $

/// @file libraries.cpp
/// @brief Pragmas for automatically linking in required libraries during Windows build
/// @ingroup base

#if __VISUALC__ >= 1200
// wxWidgets
#if wxCHECK_VERSION(2, 9, 0)
#ifdef __WXDEBUG__
#pragma comment(lib, "wxzlibd.lib")
#pragma comment(lib, "wxpngd.lib")
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
#pragma comment(lib, "wxzlib.lib")
#pragma comment(lib, "wxpng.lib")
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

#else 
#error "wxWidgets 2.9 is required"
#endif // wxWidgets

// Standard Win32 Libraries
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")

#pragma comment(lib, "libcurl.lib")

#endif // VisualC

