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
// $Id$

/// @file wx_pre.h
/// @brief Precompiled header.
/// @ingroup base

#ifdef __cplusplus

#define R_PRECOMP

#define _CRT_SECURE_NO_WARNINGS

// C + System.
#include <locale.h>
#include <stdint.h>

// C++ std
#include <sstream>

// 3rd party packages.
#include <curl/curl.h>


// WX headers
#include <wx/wxprec.h>
#include <wx/app.h>
#include <wx/apptrait.h>
#include <wx/button.h>
#include <wx/clipbrd.h>
#include <wx/cmdline.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/font.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/glcanvas.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/version.h>
#include <wx/wfstream.h>
#include <wx/window.h>
#include <wx/wx.h>
#include <wx/wxchar.h>
#include <wx/wxprec.h>

#endif
