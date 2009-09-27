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

/// @file platform_unix.cpp
/// @brief Unix Platform extension.
/// @ingroup unix

#ifndef R_PRECOMP
#include <wx/string.h>
#include <wx/app.h>
#include <wx/apptrait.h>
#endif

#include "include/platform.h"
#include "platform_unix.h"

extern "C" {
#include <sys/utsname.h>
#include "GL/glx.h"
#include "GL/gl.h"
}

PlatformUnix::PlatformUnix() {
	GetVideoInfo();
}

/**
 * @brief Gather video adapter information via OpenGL/GLX.
 *
 */
void PlatformUnix::GetVideoInfo() {
	Display *dpy;
	XVisualInfo *vi;
	GLXContext cx;
	Window root, win;
	XSetWindowAttributes attr;
	unsigned long mask;
	int attList[] = { GLX_RGBA, GLX_DOUBLEBUFFER, None };

	vendor = wxString();
	renderer = wxString();
	version = wxString();

	// Like in glxinfo.c, but somewhat simpler and shorter.
	dpy = XOpenDisplay(0);
	if (!dpy) return;
	vi = glXChooseVisual(dpy, DefaultScreen(dpy), attList);
	if (!vi) return;
	cx = glXCreateContext(dpy, vi, 0, GL_TRUE);
	if (!cx) return;

	root = RootWindow(dpy, DefaultScreen(dpy));
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root, 0, 0, 100, 100, 0, vi->depth,
						InputOutput, vi->visual, mask, &attr);

	if (!glXMakeCurrent(dpy, win, cx)) {
		glXDestroyContext(dpy, cx);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		return;
	}

	vendor = wxString(glGetString(GL_VENDOR));
	renderer = wxString(glGetString(GL_RENDERER));
	version = wxString(glGetString(GL_VERSION));

	glXDestroyContext(dpy, cx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

wxString PlatformUnix::OSVersion() {
	struct utsname name;
	if (uname(&name) != -1) {
		return name.release;
	}
	return "";
}

wxString PlatformUnix::DesktopEnvironment() {
	return wxTheApp->GetTraits()->GetDesktopEnvironment();
}

wxString PlatformUnix::CPUId() {
	return "";
};

wxString PlatformUnix::CPUSpeed() {
	return "";
};

wxString PlatformUnix::CPUCores() {
	return "";
};

wxString PlatformUnix::CPUCount() {
	return "";
};

wxString PlatformUnix::CPUFeatures() {
	return "";
};

wxString PlatformUnix::CPUFeatures2() {
	return "";
};

wxString PlatformUnix::Memory() {
	return "";
};

wxString PlatformUnix::Video() {
	return wxString::Format("%s %s (%s)", vendor, renderer, version);
}

wxString PlatformUnix::VideoVendor() {
	return vendor;
};

wxString PlatformUnix::VideoRenderer() {
	return renderer;
};

wxString PlatformUnix::VideoVersion() {
	return version;
};

wxString PlatformUnix::UnixLibraries() {
	return "";
};
