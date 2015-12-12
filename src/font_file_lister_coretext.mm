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
	NSURL *url;
	NSCharacterSet *codepoints;
	int weight;
	int width;
	bool italic;
};

void process_descriptor(NSFontDescriptor *font, std::vector<FontMatch>& out) {
	// For whatever reason there is no NSFontURLAttribute
	NSURL *url = [font objectForKey:(__bridge NSString *)kCTFontURLAttribute];
	if (!url)
		return;

	NSFont *nsfont = [NSFont fontWithDescriptor:font size:10];
	int weight = 400;
	int width = 5;

	auto traits = CTFontGetSymbolicTraits((__bridge CTFontRef)nsfont);
	bool italic = !!(traits & kCTFontItalicTrait);

	// CoreText sometimes reports a slant of zero (or a very small slant)
	// despite the macStyle field indicating that the font is italic, so
	// double check by reading that field from the ttf
	if (!italic) {
		auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)nsfont, kCTFontTableHead, 0);
		if (data.length > 45) {
			italic = static_cast<const char *>(data.bytes)[45] & 2;
		}
	}

	auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)nsfont, kCTFontTableOS2, 0);
	if (data.length > 7) {
		auto bytes = static_cast<const uint8_t *>(data.bytes);
		weight = (bytes[4] << 8) + bytes[5];
		width = (bytes[6] << 8) + bytes[7];
	}

	out.push_back({url, [font objectForKey:NSFontCharacterSetAttribute], weight, width, italic});
}

void select_best(CollectionResult& ret, std::vector<FontMatch>& options, int bold,
	             bool italic, std::vector<int> const& characters) {
	if (options.empty())
		return;

	// Prioritize fonts with the width closest to normal
	sort(begin(options), end(options), [](FontMatch const& lft, FontMatch const& rgt) {
		return std::abs(lft.width - 100) < std::abs(rgt.width - 100);
	});

	const FontMatch *match = nullptr;

	for (auto const& m : options) {
		if (italic == m.italic && (!match || std::abs(match->weight - bold + 1) > std::abs(m.weight - bold + 1)))
			match = &m;
	}

	if (!match) {
		ret.fake_italic = italic;
		for (auto const& m : options) {
			if (!match || std::abs(match->weight - bold + 1) > std::abs(m.weight - bold + 1))
				match = &m;
		}
	}

	ret.fake_bold = bold > 400 && match->weight < bold;

	auto& best = *match;
	ret.paths.push_back(best.url.fileSystemRepresentation);
	for (int chr : characters) {
		if (![best.codepoints longCharacterIsMember:chr]) {
			ret.missing += chr;
		}
	}
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

	std::vector<FontMatch> fonts;
	@autoreleasepool {
		NSString *name = @(facename.c_str() + (facename[0] == '@'));

		NSArray *attrs = @[
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFamilyAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontFaceAttribute: name}],
			[NSFontDescriptor fontDescriptorWithFontAttributes:@{NSFontNameAttribute: name}]
		];

		auto font_collection = [NSFontCollection fontCollectionWithDescriptors:attrs];
		for (NSFontDescriptor *desc in font_collection.matchingDescriptors) {
			process_descriptor(desc, fonts);
		}
		select_best(ret, fonts, bold, italic, characters);
	}

	return ret;
}
