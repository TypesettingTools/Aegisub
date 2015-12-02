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
	double weight;
	double width;
	bool italic;
};

void process_descriptor(NSFontDescriptor *font, std::vector<FontMatch>& out) {
	// For whatever reason there is no NSFontURLAttribute
	NSURL *url = [font objectForKey:(__bridge NSString *)kCTFontURLAttribute];
	if (!url)
		return;

	NSDictionary *attributes = [font objectForKey:NSFontTraitsAttribute];
	double weight = [attributes[NSFontWeightTrait] doubleValue];
	double width = [attributes[NSFontWidthTrait] doubleValue];
	bool italic = [attributes[NSFontSlantTrait] doubleValue] > 0.03;

	// CoreText sometimes reports a slant of zero (or a very small slant)
	// despite the macStyle field indicating that the font is italic, so
	// double check by reading that field from the ttf
	if (!italic) @autoreleasepool {
		NSFont *nsfont = [NSFont fontWithDescriptor:font size:10];
		auto data = (__bridge_transfer NSData *)CTFontCopyTable((__bridge CTFontRef)nsfont, kCTFontTableHead, 0);
		if (data.length > 45) {
			italic = static_cast<const char *>(data.bytes)[45] & 2;
		}
	}

	out.push_back({url, [font objectForKey:NSFontCharacterSetAttribute], weight, width, italic});
}

void select_best(CollectionResult& ret, std::vector<FontMatch>& options, double bold,
	             bool italic, std::vector<int> const& characters) {
	if (options.empty())
		return;

	// Prioritize fonts with the width closest to normal
	sort(begin(options), end(options), [](FontMatch const& lft, FontMatch const& rgt) {
		return std::abs(lft.width) < std::abs(rgt.width);
	});

	const FontMatch *match = nullptr;

	auto check_weight = [&](FontMatch const& m) {
		if (m.weight < bold)
			return false;
		if (match && m.weight >= match->weight)
			return false;
		return true;
	};
	auto check_slant = [&](FontMatch const& m) {
		return (italic && m.italic) || (!italic && !m.italic);
	};

	// First look for the best valid match
	for (auto const& m : options) {
		if (check_slant(m) && check_weight(m))
			match = &m;
	}

	if (!match) {
		// Either none are italic, or none are heavy enough
		// Just pick the heaviest one (italic if applicable)
		for (auto const& m : options) {
			if (!check_slant(m) || (match && m.weight >= match->weight))
				continue;
			match = &m;
			ret.fake_bold = bold > 0;
		}
	}

	if (!match) {
		// We want italic but there is none, or we want regular and there is
		// only italic. Don't complain about fake italic in the latter case.
		ret.fake_italic = italic;
		for (auto const& m : options) {
			if (check_weight(m))
				match = &m;
		}
	}

	if (!match) {
		// No correct weight match even in non-italic, so just pick the heaviest
		ret.fake_bold = bold > 0;
		for (auto const& m : options) {
			if (match && m.weight >= match->weight)
				continue;
			match = &m;
		}
	}

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

	double desired_weight;
	if (bold == 0)
		desired_weight = 0;
	else if (bold == 1)
		desired_weight = 0.4;
	else if (bold < 400)
		desired_weight = -0.4;
	else if (bold < 500)
		desired_weight = 0;
	else if (bold < 600)
		desired_weight = 0.23;
	else if (bold < 700)
		desired_weight = 0.3;
	else if (bold < 800)
		desired_weight = 0.4;
	else
		desired_weight = 0.62;

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
		select_best(ret, fonts, desired_weight, italic, characters);
	}

	return ret;
}
