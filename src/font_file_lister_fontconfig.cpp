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

#include "font_file_lister.h"

#include <libaegisub/charset_conv_win.h>
#include <libaegisub/log.h>

#ifdef __APPLE__
#include <libaegisub/util_osx.h>
#endif

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/path.hpp>
#include <fontconfig/fontconfig.h>
#include <wx/intl.h>

namespace {
FcConfig *init_fontconfig() {
#ifdef __APPLE__
	FcConfig *config = FcConfigCreate();
	std::string conf_path = agi::util::GetBundleResourcesDirectory() + "/etc/fonts/fonts.conf";
	if (FcConfigParseAndLoad(config, (unsigned char *)conf_path.c_str(), FcTrue))
		return config;

	LOG_E("font_collector/fontconfig") << "Loading fontconfig configuration file failed";
	FcConfigDestroy(config);
#endif
	return FcInitLoadConfig();
}

bool pattern_matches(FcPattern *pat, const char *field, std::string const& name) {
	FcChar8 *str;
	for (int i = 0; FcPatternGetString(pat, field, i, &str) == FcResultMatch; ++i) {
		std::string sstr((char *)str);
		boost::to_lower(sstr);
		if (sstr == name)
			return true;
	}
	return false;
}

void find_font(FcFontSet *src, FcFontSet *dst, std::string const& family) {
	if (!src) return;

	for (FcPattern *pat : boost::make_iterator_range(&src->fonts[0], &src->fonts[src->nfont])) {
		int val;
		if (FcPatternGetBool(pat, FC_OUTLINE, 0, &val) != FcResultMatch || val != FcTrue) continue;

		if (pattern_matches(pat, FC_FULLNAME, family) || pattern_matches(pat, FC_FAMILY, family))
			FcFontSetAdd(dst, FcPatternDuplicate(pat));
	}
}

}

FontConfigFontFileLister::FontConfigFontFileLister(FontCollectorStatusCallback &cb)
: config(init_fontconfig(), FcConfigDestroy)
{
	cb(_("Updating font cache\n"), 0);
	FcConfigBuildFonts(config);
}

CollectionResult FontConfigFontFileLister::GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters) {
	CollectionResult ret;

	std::string family = facename[0] == '@' ? facename.substr(1) : facename;
	boost::to_lower(family);

	int weight = bold == 0 ? 80 :
	             bold == 1 ? 200 :
	                         bold;
	int slant  = italic ? 110 : 0;

	// Create a fontconfig pattern to match the desired weight/slant
	agi::scoped_holder<FcPattern*> pat(FcPatternCreate(), FcPatternDestroy);
	if (!pat) return ret;

	FcPatternAddBool(pat, FC_OUTLINE, true);
	FcPatternAddInteger(pat, FC_SLANT, slant);
	FcPatternAddInteger(pat, FC_WEIGHT, weight);

	FcDefaultSubstitute(pat);
	if (!FcConfigSubstitute(config, pat, FcMatchPattern)) return ret;

	// Create a font set with only correctly named fonts
	// This is needed because the patterns returned by font matching only
	// include the first family and fullname, so we can't always verify that
	// we got the actual font we were asking for after the fact
	agi::scoped_holder<FcFontSet*> fset(FcFontSetCreate(), FcFontSetDestroy);
	find_font(FcConfigGetFonts(config, FcSetApplication), fset, family);
	find_font(FcConfigGetFonts(config, FcSetSystem), fset, family);

	// Get the best match from fontconfig
	FcResult result;
	FcFontSet *sets[] = { (FcFontSet*)fset };

	agi::scoped_holder<FcFontSet*> matches(FcFontSetSort(config, sets, 1, pat, false, nullptr, &result), FcFontSetDestroy);
	if (matches->nfont == 0)
		return ret;

	auto match = matches->fonts[0];

#ifdef _WIN32
	wxMemoryDC dc;
	// Use EnumFontFamiliesEx to verify the match, as fontconfig sometimes gives
	// us some incorrect matches along with the correct one
	for (FcPattern *pat : boost::make_iterator_range(&matches->fonts[0], &matches->fonts[matches->nfont])) {
		FcChar8 *fullname;
		if (FcPatternGetString(pat, FC_FULLNAME, 0, &fullname) != FcResultMatch)
			continue;

		LOGFONT lf = {0};
		lf.lfCharSet = DEFAULT_CHARSET;
		wcsncpy(lf.lfFaceName, agi::charset::ConvertW((const char *)fullname).c_str(), 31);

		auto cb = [&](const LOGFONT *lf) {
			auto face = agi::charset::ConvertW(lf->lfFaceName);
			boost::to_lower(face);
			return face == family;
		};

		using type = decltype(cb);
		bool found = !EnumFontFamiliesEx(dc.GetHDC(), &lf,
			[](const LOGFONT *lf, const TEXTMETRIC *, DWORD, LPARAM lParam) -> int {
				return !(*reinterpret_cast<type*>(lParam))(lf);
			}, (LPARAM)&cb, 0);

		if (found) {
			match = pat;
			break;
		}
	}
#endif

	FcChar8 *file;
	if(FcPatternGetString(match, FC_FILE, 0, &file) != FcResultMatch)
		return ret;

	FcCharSet *charset;
	if (FcPatternGetCharSet(match, FC_CHARSET, 0, &charset) == FcResultMatch) {
		for (int chr : characters) {
			if (!FcCharSetHasChar(charset, chr))
				ret.missing += chr;
		}
	}

	if (weight > 80) {
		int actual_weight = weight;
		if (FcPatternGetInteger(match, FC_WEIGHT, 0, &actual_weight) == FcResultMatch)
			ret.fake_bold = actual_weight <= 80;
	}

	int actual_slant = slant;
	if (FcPatternGetInteger(match, FC_SLANT, 0, &actual_slant) == FcResultMatch)
		ret.fake_italic = italic && !actual_slant;

	ret.paths.emplace_back((const char *)file);
	return ret;
}
