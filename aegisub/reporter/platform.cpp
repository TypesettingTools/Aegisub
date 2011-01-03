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

/// @file platform.cpp
/// @brief Base functions for the Platform class.
/// @ingroup base

#ifndef R_PRECOMP
#include <wx/string.h>
#include <wx/app.h>
#include <wx/gdicmn.h>	// Display* functions.
#include <wx/version.h>	// Version info.
#include <wx/intl.h>	// Locale info.
#include <wx/glcanvas.h>
#endif

#include "include/platform.h"
//#include "platform_windows.h"
#include "platform_unix.h"
#include "platform_unix_bsd.h"
//#include "platform_unix_linux.h"
//#include "platform_unix_osx.h"

extern "C" {
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif
}

/// @brief Constructor.
Platform* Platform::GetPlatform() {
#if defined(__WINDOWS__)
	Platform *p = new PlatformWindows;
#elif defined(__UNIX__)
#   if defined(__FREEBSD__)
		Platform *p = new PlatformUnixBSD;
#   elif defined(__LINUX__)
		Platform *p = new PlatformUnixLinux;
#   elif defined(__APPLE__)
		Platform *p = new PlatformUnixOSX;
#   else
		Platform *p = new PlatformUnix;
#   endif
#else
	Platform *p = NULL;
#endif // __UNIX__
	p->Init();
	return p;
}

/// @brief Init variables to avoid duplicate instantiations.
void Platform::Init() {
	locale = new wxLocale();
	locale->Init();
}

/**
 * @brief Gather video adapter information via OpenGL
 *
 */
std::string Platform::GetVideoInfo(enum Platform::VideoInfo which) {
	int attList[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };
	wxGLCanvas *glc = new wxGLCanvas(wxTheApp->GetTopWindow(), wxID_ANY, attList, wxDefaultPosition, wxDefaultSize);
	wxGLContext *ctx = new wxGLContext(glc, 0);
	wxGLCanvas &cr = *glc;
	ctx->SetCurrent(cr);

	wxString value;

	switch (which) {
		case VIDEO_EXT:
			return reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
		break;
		case VIDEO_RENDERER:
			return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		break;
		case VIDEO_VENDOR:
			return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
		break;
		case VIDEO_VERSION:
			return reinterpret_cast<const char*>(glGetString(GL_VERSION));
		default:
			return "";
	}

	delete ctx;
	delete glc;
}

const char* Platform::ArchName() {
	return plat.GetArchName().c_str();
};

const char* Platform::OSFamily() {
	return plat.GetOperatingSystemFamilyName().c_str();
};

const char* Platform::OSName() {
	return plat.GetOperatingSystemIdName().c_str();
};

const char* Platform::Endian() {
	return plat.GetEndiannessName().c_str();
};


int Platform::DisplayDepth() {
	return wxDisplayDepth();
}

const char* Platform::DisplaySize() {
	int x, y;
	wxDisplaySize(&x, &y);
	return wxString::Format(L"%d %d", x, y).c_str();
}

const char* Platform::DisplayPPI() {
	return wxString::Format(L"%d %d", wxGetDisplayPPI().GetWidth(), wxGetDisplayPPI().GetHeight()).c_str();
}

const char* Platform::wxVersion() {
	return wxString::Format(L"%d.%d.%d.%d", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER).c_str();
}

const char* Platform::Locale() {
	return wxLocale().GetSysName().c_str();
}

const char* Platform::Language() {
	const wxLanguageInfo *info = locale->GetLanguageInfo(locale->GetLanguage());
	return info->CanonicalName.c_str();
}

const char* Platform::SystemLanguage() {
	const wxLanguageInfo *info = locale->GetLanguageInfo(locale->GetSystemLanguage());
	return info->CanonicalName.c_str();
}

std::string Platform::Date() {
	return "";
}

std::string Platform::Signature() {
	return "";
}

#ifdef __UNIX__
const char* Platform::DesktopEnvironment() {
	return "";
}
#endif

std::string Platform::OpenGLVendor() {
	return GetVideoInfo(VIDEO_VENDOR);
}

std::string Platform::OpenGLRenderer() {
	return GetVideoInfo(VIDEO_RENDERER);
}

std::string Platform::OpenGLVersion() {
	return GetVideoInfo(VIDEO_VERSION);
}

std::string Platform::OpenGLExt() {
	return GetVideoInfo(VIDEO_EXT);
}

#ifdef __APPLE__

wxString Platform::PatchLevel() {
	return "";
}

wxString Platform::QuickTimeExt() {
	return "";
}

wxString Platform::HardwareModel() {
	return "";
}

#endif
