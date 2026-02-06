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

/// @file xdg_desktop_portal_utils.h
/// @see xdg_desktop_portal_utils.cpp
/// @ingroup utility linux
///

#pragma once

#ifdef WITH_LIBPORTAL

#include <libportal/portal.h>

#include <wx/window.h>

#include <memory>

namespace agi::xdp_utils {
	extern XdpPortal *portal;
	std::unique_ptr<XdpParent, decltype(&xdp_parent_free)> GetXdpParent(wxWindow *window);
	void Initialize();
	void Cleanup();
}

#else // WITH_LIBPORTAL

namespace agi::xdp_utils {
	inline void Initialize() {}
	inline void Cleanup() {}
}

#endif // WITH_LIBPORTAL
