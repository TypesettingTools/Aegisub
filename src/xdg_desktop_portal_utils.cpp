// Copyright (c) 2026, Aegisub contributors
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
// Aegisub Project https://aegisub.org/

/// @file xdg_desktop_portal_utils.cpp
/// @brief Utilities related to XDG Desktop Portals
/// @ingroup utility linux
///


#ifdef WITH_LIBPORTAL
#include "xdg_desktop_portal_utils.h"

#ifdef __WXGTK3__
#include <libportal-gtk3/portal-gtk3.h>
#endif

namespace agi::xdp_utils {
	XdpPortal *portal = nullptr;

	XdpParent *xdp_parent_new_wx(wxWindow *window) {
#ifdef __WXGTK3__
		GtkWidget *gtk_widget = GTK_WIDGET(window->GetHandle());
		GtkWindow *gtk_window = GTK_WINDOW(gtk_widget_get_toplevel(gtk_widget));
		return xdp_parent_new_gtk(gtk_window);
#else
		return nullptr;
#endif
	}

	void Initialize() {
		portal = xdp_portal_new();
	}

	void Cleanup() {
		g_object_unref(portal);
		portal = nullptr;
	}
}

#endif // WITH_LIBPORTAL
