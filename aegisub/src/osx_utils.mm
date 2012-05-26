// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file osx_utils.mm
/// @see utils.h
/// @ingroup utils
///

#include "config.h"

#include <wx/window.h>

#import <AppKit/AppKit.h>

void AddFullScreenButton(wxWindow *window) {
	NSWindow *nsWindow = [window->GetHandle() window];
	if (![nsWindow respondsToSelector:@selector(toggleFullScreen:)])
		return;

	NSWindowCollectionBehavior collectionBehavior = [nsWindow collectionBehavior];
	collectionBehavior |= NSWindowCollectionBehaviorFullScreenPrimary;
	[nsWindow setCollectionBehavior:collectionBehavior];
}
