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

/// @file font_file_lister_fontconfig.cpp
/// @brief Font Config-based font collector
/// @ingroup font_collector
///

#include "config.h"

#ifdef WITH_FONTCONFIG
#include "font_file_lister_fontconfig.h"

#include <fontconfig/fontconfig.h>

#include "compat.h"

#include <libaegisub/log.h>
#include <libaegisub/util.h>

#ifdef __APPLE__
#include <libaegisub/util_osx.h>
#endif

#ifndef AGI_PRE
#include <wx/intl.h>
#endif

namespace {
	// In SSA/ASS fonts are sometimes referenced by their "full name",
	// which is usually a concatenation of family name and font
	// style (ex. Ottawa Bold). Full name is available from
	// FontConfig pattern element FC_FULLNAME, but it is never
	// used for font matching.
	// Therefore, I'm removing words from the end of the name one
	// by one, and adding shortened names to the pattern. It seems
	// that the first value (full name in this case) has
	// precedence in matching.
	// An alternative approach could be to reimplement FcFontSort
	// using FC_FULLNAME instead of FC_FAMILY.
	int add_families(FcPattern *pat, std::string family) {
		int family_cnt = 1;
		FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)family.c_str());

		for (std::string::reverse_iterator p = family.rbegin(); p != family.rend(); ++p) {
			if (*p == ' ' || *p == '-') {
				*p = '\0';
				FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)family.c_str());
				++family_cnt;
			}
		}
		return family_cnt;
	}

	int strcasecmp(std::string a, std::string b) {
		agi::util::str_lower(a);
		agi::util::str_lower(b);
		return a.compare(b);
	}

	FcConfig *init_fontconfig() {
#ifdef __APPLE__
		FcConfig *config = FcConfigCreate();
		std::string conf_path = agi::util::OSX_GetBundleResourcesDirectory() + "/etc/fonts/fonts.conf";
		if (FcConfigParseAndLoad(config, (unsigned char *)conf_path.c_str(), FcTrue))
			return config;

		LOG_E("font_collector/fontconfig") << "Loading fontconfig configuration file failed";
		FcConfigDestroy(config);
#endif
		return FcInitLoadConfig();
	}
}

FontConfigFontFileLister::FontConfigFontFileLister(FontCollectorStatusCallback cb)
: config(init_fontconfig(), FcConfigDestroy)
{
	cb(_("Updating font cache\n"), 0);
	FcConfigBuildFonts(config);
}

FcFontSet *FontConfigFontFileLister::MatchFullname(const char *family, int weight, int slant) {
	FcFontSet *sets[2];
	FcFontSet *result = FcFontSetCreate();
	int nsets = 0;

	if ((sets[nsets] = FcConfigGetFonts(config, FcSetSystem)))
		nsets++;
	if ((sets[nsets] = FcConfigGetFonts(config, FcSetApplication)))
		nsets++;

	// Run over font sets and patterns and try to match against full name
	for (int i = 0; i < nsets; i++) {
		FcFontSet *set = sets[i];
		for (int fi = 0; fi < set->nfont; fi++) {
			FcPattern *pat = set->fonts[fi];
			char *fullname;
			int pi = 0, at;
			FcBool ol;
			while (FcPatternGetString(pat, FC_FULLNAME, pi++, (FcChar8 **)&fullname) == FcResultMatch) {
				if (FcPatternGetBool(pat, FC_OUTLINE, 0, &ol) != FcResultMatch || ol != FcTrue)
					continue;
				if (FcPatternGetInteger(pat, FC_SLANT, 0, &at) != FcResultMatch || at < slant)
					continue;
				if (FcPatternGetInteger(pat, FC_WEIGHT, 0, &at) != FcResultMatch || at < weight)
					continue;
				if (strcasecmp(fullname, family) == 0) {
					FcFontSetAdd(result, FcPatternDuplicate(pat));
					break;
				}
			}
		}
	}

	return result;
}

FontFileLister::CollectionResult FontConfigFontFileLister::GetFontPaths(wxString const& facename, int bold, bool italic, std::set<wxUniChar> const& characters) {
	CollectionResult ret;

	std::string family = STD_STR(facename);
	if (family[0] == '@')
		family.erase(0, 1);

	int weight = bold == 0 ? 80 :
	             bold == 1 ? 200 :
	                         bold;
	int slant  = italic ? 110 : 0;

	agi::scoped_holder<FcPattern*> pat(FcPatternCreate(), FcPatternDestroy);
	if (!pat) return ret;

	int family_cnt = add_families(pat, family);

	FcPatternAddBool(pat, FC_OUTLINE, true);
	FcPatternAddInteger(pat, FC_SLANT, slant);
	FcPatternAddInteger(pat, FC_WEIGHT, weight);

	FcDefaultSubstitute(pat);

	if (!FcConfigSubstitute(config, pat, FcMatchPattern)) return ret;

	FcResult result;
	agi::scoped_holder<FcFontSet*> fsorted(FcFontSort(config, pat, true, nullptr, &result), FcFontSetDestroy);
	agi::scoped_holder<FcFontSet*> ffullname(MatchFullname(family.c_str(), weight, slant), FcFontSetDestroy);
	if (!fsorted || !ffullname) return ret;

	agi::scoped_holder<FcFontSet*> fset(FcFontSetCreate(), FcFontSetDestroy);
	for (int cur_font = 0; cur_font < ffullname->nfont; ++cur_font) {
		FcPattern *curp = ffullname->fonts[cur_font];
		FcPatternReference(curp);
		FcFontSetAdd(fset, curp);
	}
	for (int cur_font = 0; cur_font < fsorted->nfont; ++cur_font) {
		FcPattern *curp = fsorted->fonts[cur_font];
		FcPatternReference(curp);
		FcFontSetAdd(fset, curp);
	}

	int cur_font;
	for (cur_font = 0; cur_font < fset->nfont; ++cur_font) {
		FcBool outline;
		result = FcPatternGetBool(fset->fonts[cur_font], FC_OUTLINE, 0, &outline);
		if (result == FcResultMatch && outline) break;
	}

	if (cur_font >= fset->nfont) return ret;

	// Remove all extra family names from original pattern.
	// After this, FcFontRenderPrepare will select the most relevant family
	// name in case there are more than one of them.
	for (; family_cnt > 1; --family_cnt)
		FcPatternRemove(pat, FC_FAMILY, family_cnt - 1);

	agi::scoped_holder<FcPattern*> rpat(FcFontRenderPrepare(config, pat, fset->fonts[cur_font]), FcPatternDestroy);
	if (!rpat) return ret;

	FcChar8 *r_family;
	if (FcPatternGetString(rpat, FC_FAMILY, 0, &r_family) != FcResultMatch)
		return ret;

	FcChar8 *r_fullname;
	if (FcPatternGetString(rpat, FC_FULLNAME, 0, &r_fullname) != FcResultMatch)
		return ret;

	if (strcasecmp(family, (char*)r_family) && strcasecmp(family, (char*)r_fullname))
		return ret;

	FcChar8 *file;
	if(FcPatternGetString(rpat, FC_FILE, 0, &file) != FcResultMatch)
		return ret;

	FcCharSet *charset;
	if (FcPatternGetCharSet(rpat, FC_CHARSET, 0, &charset) == FcResultMatch) {
		for (wxUniChar chr : characters) {
			if (!FcCharSetHasChar(charset, chr))
				ret.missing += chr;
		}
	}

	ret.paths.push_back((const char *)file);
	return ret;
}
#endif
