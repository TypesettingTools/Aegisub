// Copyright (c) 2026, arch1t3cht <arch1t3cht@gmail.com>
// Copyright (c) 2012 Thomas Goyne, <plorkyeran@aegisub.org>
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

#include <libaegisub/scoped_ptr.h>

#include <wx/dcmemory.h>
#include <wx/thread.h>

#import <ApplicationServices/ApplicationServices.h>
#import <CoreGraphics/CGDirectDisplay.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

namespace osx {

namespace {

void CopyToDC(CGImageRef img, wxMemoryDC &capdc, int resx, int resy, int magnification) {
	int width = CGImageGetWidth(img);
	int height = CGImageGetHeight(img);
	std::vector<uint8_t> imgdata(height * width * 4);

	agi::scoped_holder<CGColorSpaceRef> colorspace(CGColorSpaceCreateDeviceRGB(), CGColorSpaceRelease);
	agi::scoped_holder<CGContextRef> bmp_context(CGBitmapContextCreate(&imgdata[0], width, height, 8, 4 * width, colorspace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big), CGContextRelease);

	CGContextDrawImage(bmp_context, CGRectMake(0, 0, width, height), img);

	for (int x = 0; x < resx && x < width; x++) {
		for (int y = 0; y < resy && y < height; y++) {
			uint8_t *pixel = &imgdata[y * width * 4 + x * 4];
			capdc.SetBrush(wxBrush(wxColour(pixel[0], pixel[1], pixel[2])));
			capdc.DrawRectangle(x * magnification, y * magnification, magnification, magnification);
		}
	}
}

}

void DropFromScreen(int x, int y, int resx, int resy, int magnification, wxMemoryDC &capdc) {
	CGRect rect = CGRectMake(x - resx / 2., y - resy / 2., resx, resy);

#if MAC_OS_X_VERSION_MIN_REQUIRED < 150000
	// Doesn't bother handling the case where the rect overlaps two monitors
	CGDirectDisplayID display_id;
	uint32_t display_count;
	CGGetDisplaysWithPoint(CGPointMake(x, y), 1, &display_id, &display_count);

	agi::scoped_holder<CGImageRef> img(CGDisplayCreateImageForRect(display_id, rect), CGImageRelease);
	CopyToDC(img, capdc, resx, resy, magnification);
#else
	wxSemaphore screenshot_notify{0, 1};
	wxSemaphore &screenshot_notify_ref = screenshot_notify;

#if MAC_OS_X_VERSION_MIN_REQUIRED < 152000
	// captureImageInRect is only available from 15.2 onward so before that we need to do it the cumbersome (and slow) way.

	rect.origin.x = std::roundl(rect.origin.x);
	rect.origin.y = std::roundl(rect.origin.y);

	[SCShareableContent getShareableContentWithCompletionHandler:^(SCShareableContent * _Nullable shareableContent, NSError * _Nullable error) {
		if (error) {
			screenshot_notify_ref.Post();
			return;
		}

		CGDirectDisplayID display_id;
		uint32_t display_count;
		CGGetDisplaysWithPoint(CGPointMake(x, y), 1, &display_id, &display_count);

		CGDisplayModeRef displaymode = CGDisplayCopyDisplayMode(display_id);
		int scale_factor = CGDisplayModeGetPixelWidth(displaymode) / CGDisplayModeGetWidth(displaymode);

		SCDisplay* point_display = nullptr;

		for (SCDisplay *display in shareableContent.displays) {
			if (display.displayID == display_id) {
				point_display = display;
			}
		}

		if (!point_display) {
			screenshot_notify_ref.Post();
			return;
		}

		SCContentFilter *filter = [[SCContentFilter alloc] initWithDisplay:point_display excludingWindows:@[]];
		SCStreamConfiguration *configuration = [[SCStreamConfiguration alloc] init];
		configuration.capturesAudio = NO;
		configuration.showsCursor = NO;
		configuration.captureDynamicRange = SCCaptureDynamicRangeSDR;
		configuration.sourceRect = rect;
		configuration.width = resx * scale_factor;
		configuration.height = resy * scale_factor;

		[SCScreenshotManager captureImageWithFilter:filter configuration:configuration completionHandler:^(CGImageRef  _Nullable sampleBuffer, NSError * _Nullable error) {
			if (error) {
				screenshot_notify_ref.Post();
				return;
			}

			CopyToDC(sampleBuffer, capdc, resx, resy, magnification);
			screenshot_notify_ref.Post();
		}];
	}];

#else
	[SCScreenshotManager captureImageInRect:rect completionHandler:^(CGImageRef  _Nullable image, NSError * _Nullable error){
		if (!error)
			CopyToDC(image, capdc, resx, resy, magnification);

		screenshot_notify_ref.Post();
	}];
#endif

	screenshot_notify.Wait();
	return;
#endif

}

}
