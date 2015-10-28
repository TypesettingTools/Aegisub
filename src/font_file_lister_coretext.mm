// Copyright (c) 2016, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "font_file_lister.h"

#include <AppKit/AppKit.h>
#include <CoreText/CoreText.h>

namespace {
void process_font(CollectionResult& ret, NSFontDescriptor *font, int bold, bool italic,
                  std::vector<int> const& characters) {
	// For whatever reason there is no NSFontURLAttribute
	NSURL *url = [font objectForKey:(__bridge NSString *)kCTFontURLAttribute];
	if (!url)
		return;

	NSDictionary *attributes = [font objectForKey:NSFontTraitsAttribute];
	double weight = [attributes[NSFontWeightTrait] doubleValue];
	double slant = [attributes[NSFontSlantTrait] doubleValue];

	if (italic != (slant > 0.03))
		return;
	if (bold == 0 && weight > 0)
		return;
	if (bold == 1 && weight < 0.4)
		return;

	NSCharacterSet *codepoints = [font objectForKey:NSFontCharacterSetAttribute];
	for (int chr : characters) {
		if (![codepoints longCharacterIsMember:chr]) {
			ret.missing += chr;
		}
	}

	ret.paths.push_back(url.absoluteString.UTF8String);
}
}

CollectionResult CoreTextFontFileLister::GetFontPaths(std::string const& facename,
                                                      int bold, bool italic,
                                                      std::vector<int> const& characters) {
	CollectionResult ret;

	@autoreleasepool {
		NSString *name = @(facename.c_str() + (facename[0] == '@'));
		NSArray *attrs = @[
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFamilyAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFaceAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontNameAttribute: name}]
		];

		auto font_collection = [NSFontCollection fontCollectionWithDescriptors:attrs];
		for (NSFontDescriptor *desc in font_collection.matchingDescriptors) {
			process_font(ret, desc, bold, italic, characters);

			// If we didn't get any results, check for non-bold/italic variants
			// and collect them with a warning
			if (ret.paths.empty() && bold) {
				process_font(ret, desc, 0, italic, characters);
				if (!ret.paths.empty())
					ret.fake_bold = true;
			}

			if (ret.paths.empty() && italic) {
				process_font(ret, desc, bold, false, characters);
				if (!ret.paths.empty())
					ret.fake_italic = true;
			}

			if (ret.paths.empty() && bold && italic) {
				process_font(ret, desc, 0, false, characters);
				if (!ret.paths.empty()) {
					ret.fake_bold = true;
					ret.fake_italic = true;
				}
			}
		}
	}

	return ret;
}
