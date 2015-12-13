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
struct FontMatch {
	NSURL *url = nil;
	NSCharacterSet *codepoints = nil;
	int weight = 400;
	int width = 5;
	bool italic = false;
	bool family_match = false;
};

FontMatch process_descriptor(NSFontDescriptor *desc, NSString *name) {
	FontMatch ret;

	// For whatever reason there is no NSFontURLAttribute
	ret.url = [desc objectForKey:(__bridge NSString *)kCTFontURLAttribute];
	if (!ret.url)
		return ret;

	NSFont *font = [NSFont fontWithDescriptor:desc size:10];

	// Ask CoreText if the font is italic, but if it says no double-check
	// by reading the macStyle field of the 'head' table as CT doesn't honor
	// that
	auto traits = CTFontGetSymbolicTraits((__bridge CTFontRef)font);
	ret.italic = !!(traits & kCTFontItalicTrait);
	if (!ret.italic) {
		auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)font, kCTFontTableHead, 0);
		if (data.length > 45) {
			ret.italic = static_cast<const char *>(data.bytes)[45] & 2;
		}
	}

	// Get the weight from the OS/2 table because the weights reported by CT
	// are often uncorrelated with the OS/2 weight, which is what GDI uses
	auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)font, kCTFontTableOS2, 0);
	if (data.length > 7) {
		auto bytes = static_cast<const uint8_t *>(data.bytes);
		// These values are big-endian in the file, so byteswap them
		ret.weight = (bytes[4] << 8) + bytes[5];
		ret.width = (bytes[6] << 8) + bytes[7];
	}

	ret.family_match = [font.familyName isEqualToString:name];
	ret.codepoints = [desc objectForKey:NSFontCharacterSetAttribute];
	return ret;
}
}

CollectionResult CoreTextFontFileLister::GetFontPaths(std::string const& facename,
                                                      int bold, bool italic,
                                                      std::vector<int> const& characters) {
	CollectionResult ret;

	if (bold == 0)
		bold = 400;
	else if (bold == 1)
		bold = 700;

	FontMatch best;
	bool have_match = false;

	auto match_is_better = [&](FontMatch const& m) {
		// Prioritize family name matches over postscript name matches
		if (m.family_match != best.family_match) {
			return m.family_match > best.family_match;
		}

		if (m.italic != best.italic) {
			return (m.italic != italic) < (best.italic != italic);
		}
		if (m.weight != best.weight) {
			// + 1 to bias in favor of lighter fonts when they're equal distances
			// from the correct weight
			return std::abs(m.weight - bold + 1) < std::abs(best.weight - bold + 1);
		}
		// Pick closest to "medium" weight
		return std::abs(m.width - 5) < std::abs(best.width - 5);

	};
	auto compare_match = [&](FontMatch match) {
		if (!match.url)
			return;
		if (!have_match) {
			best = match;
			have_match = true;
		}
		else if (match_is_better(match))
			best = match;
	};

	@autoreleasepool {
		NSString *name = @(facename.c_str() + (facename[0] == '@'));

		NSArray *attrs = @[
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFamilyAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFaceAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontNameAttribute: name}]
		];

		auto collection = [NSFontCollection fontCollectionWithDescriptors:attrs];
		if (NSArray *matching = collection.matchingDescriptors) {
			for (NSFontDescriptor *desc in matching) {
				compare_match(process_descriptor(desc, name));
			}
		}

		if (!have_match)
			return ret;

		ret.fake_italic = italic && !best.italic;
		ret.fake_bold = bold > 400 && best.weight < bold;

		ret.paths.push_back(best.url.fileSystemRepresentation);
		for (int chr : characters) {
			if (![best.codepoints longCharacterIsMember:chr]) {
				ret.missing += chr;
			}
		}
	}

	return ret;
}
