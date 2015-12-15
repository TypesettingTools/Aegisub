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

	auto get_16 = [](const uint8_t *bytes, ptrdiff_t offset) -> uint16_t {
		return (bytes[offset] << 8) + bytes[offset + 1];
	};

	// Get the weight from the OS/2 table because the weights reported by CT
	// are often uncorrelated with the OS/2 weight, which is what GDI uses
	auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)font, kCTFontTableOS2, 0);
	if (data.length > 7) {
		auto bytes = static_cast<const uint8_t *>(data.bytes);
		ret.weight = get_16(bytes, 4);
		ret.width = get_16(bytes, 6);
	}

	ret.family_match = [font.familyName isEqualToString:name];
	ret.codepoints = [desc objectForKey:NSFontCharacterSetAttribute];

	// Some fonts have different family names for OS X and Windows, with all of
	// the styles in a single family on OS X, but only bold/italic variants in
	// the main family with different families for the other variants on Windows.
	// For VSFilter compatiblity we want to match based on the Windows name.
	if (ret.family_match) {
		auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)font, kCTFontTableName, 0);
		auto bytes = static_cast<const uint8_t *>(data.bytes);
		uint16_t count = get_16(bytes, 2);
		auto strings = bytes + get_16(bytes, 4);

		for (uint16_t i = 0; i < count; ++i) {
			auto name_record = bytes + 6 + i * 12;
			auto platform_id = get_16(name_record, 0);
			auto encoding_id = get_16(name_record, 2);
			auto name_id = get_16(name_record, 6);
			auto length = get_16(name_record, 8);
			auto offset = get_16(name_record, 10);

			if (name_id != 1) // font family
				continue;
			if (platform_id != 3 || encoding_id != 1) // only look at MS Unicode
				continue;

			NSString *msFamily = [[NSString alloc] initWithBytesNoCopy:(void *)(strings + offset)
			                                                    length:length
			                                                  encoding:NSUTF16BigEndianStringEncoding
			                                              freeWhenDone:NO];
			auto range = [msFamily rangeOfString:font.familyName];
			// If it's not even a prefix then it's probably for a different language
			if (range.location != 0)
				continue;

			ret.family_match = range.length == msFamily.length;
			break;
		}
	}

	return ret;
}

int weight_penalty(int desired, int actual) {
	int d = desired - actual;
	// If the font is too light but can be emboldened, reduce the penalty
	if (d > 150 && actual <= 550)
		d = d * 2 / 5;
	return std::abs(d);
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

		// GDI prefers to match weight over matching italic when it has to choose
		if (m.weight != best.weight) {
			return weight_penalty(bold, m.weight) < weight_penalty(bold, best.weight);
		}
		if (m.italic != best.italic) {
			return (m.italic != italic) < (best.italic != italic);
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
