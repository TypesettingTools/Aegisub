// Copyright (c) 2026, Aegisub contributors
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

	std::unique_ptr<XdpParent, decltype(&xdp_parent_free)> GetXdpParent(wxWindow *window) {
#ifdef __WXGTK3__
		GtkWidget *gtk_widget = GTK_WIDGET(window->GetHandle());
		GtkWindow *gtk_window = GTK_WINDOW(gtk_widget_get_toplevel(gtk_widget));
		return {xdp_parent_new_gtk(gtk_window), xdp_parent_free};
#else
		return {nullptr, xdp_parent_free};
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
