// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "config.h"

#include "retina_helper.h"

#include <Cocoa/Cocoa.h>
#include <wx/window.h>


@interface RetinaObserver : NSObject
@property (nonatomic, assign) NSWindow *window;
@property (nonatomic, copy) void (^block)();
@end

@implementation RetinaObserver
- (void)backingPropertiesDidChange:(NSNotification *)notification {
	self.block();
}

- (void)dealloc {
	[_block release];
	[super dealloc];
}
@end

RetinaHelper::RetinaHelper(wxWindow *window)
: window(window)
, observer([RetinaObserver new])
{
	NSView *view = window->GetHandle();
	RetinaObserver *obs = (id)observer;
	obs.window = view.window;
	obs.block = ^{ ScaleFactorChanged(GetScaleFactor()); };

	NSNotificationCenter *nc = NSNotificationCenter.defaultCenter;
	[nc addObserver:(id)observer
	       selector:@selector(backingPropertiesDidChange:)
	           name:NSWindowDidChangeBackingPropertiesNotification
	         object:view.window];

	if ([view respondsToSelector:@selector(setWantsBestResolutionOpenGLSurface:)])
		view.wantsBestResolutionOpenGLSurface = YES;
}

RetinaHelper::~RetinaHelper() {
	[NSNotificationCenter.defaultCenter removeObserver:(id)observer];
	[(id)observer release];
}

int RetinaHelper::GetScaleFactor() const {
	return static_cast<int>(window->GetHandle().window.backingScaleFactor);
}

