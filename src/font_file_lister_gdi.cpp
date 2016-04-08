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

#include "compat.h"

#include <libaegisub/charset_conv_win.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>

#include <ShlObj.h>
#include <boost/scope_exit.hpp>
#include <unicode/utf16.h>
#include <Usp10.h>

namespace {
uint32_t murmur3(const char *data, uint32_t len) {
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	static const uint32_t r1 = 15;
	static const uint32_t r2 = 13;
	static const uint32_t m = 5;
	static const uint32_t n = 0xe6546b64;

	uint32_t hash = 0;

	const int nblocks = len / 4;
	auto blocks = reinterpret_cast<const uint32_t *>(data);
	for (uint32_t i = 0; i * 4 < len; ++i) {
		uint32_t k = blocks[i];
		k *= c1;
		k = _rotl(k, r1);
		k *= c2;

		hash ^= k;
		hash = _rotl(hash, r2) * m + n;
	}

	hash ^= len;
	hash ^= hash >> 16;
	hash *= 0x85ebca6b;
	hash ^= hash >> 13;
	hash *= 0xc2b2ae35;
	hash ^= hash >> 16;

	return hash;
}

std::vector<agi::fs::path> get_installed_fonts() {
	static const auto fonts_key_name = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";

	std::vector<agi::fs::path> files;

	HKEY key;
	auto ret = RegOpenKeyExW(HKEY_LOCAL_MACHINE, fonts_key_name, 0, KEY_QUERY_VALUE, &key);
	if (ret != ERROR_SUCCESS) return files;
	BOOST_SCOPE_EXIT_ALL(=) { RegCloseKey(key); };

	wchar_t fdir[MAX_PATH];
	SHGetFolderPathW(NULL, CSIDL_FONTS, NULL, 0, fdir);
	agi::fs::path font_dir(fdir);

	for (DWORD i = 0;; ++i) {
		wchar_t font_name[SHRT_MAX], font_filename[MAX_PATH];
		DWORD name_len = sizeof(font_name);
		DWORD data_len = sizeof(font_filename);

		ret = RegEnumValueW(key, i, font_name, &name_len, NULL, NULL, reinterpret_cast<BYTE *>(font_filename), &data_len);
		if (ret == ERROR_NO_MORE_ITEMS) break;
		if (ret != ERROR_SUCCESS) continue;

		agi::fs::path font_path(font_filename);
		if (!agi::fs::FileExists(font_path))
			font_path = font_dir / font_path;
		files.push_back(font_path);
	}

	return files;
}

using font_index = std::unordered_multimap<uint32_t, agi::fs::path>;

font_index index_fonts(FontCollectorStatusCallback &cb) {
	font_index hash_to_path;
	auto fonts = get_installed_fonts();
	std::unique_ptr<char[]> buffer(new char[1024]);
	for (auto const& path : fonts) {
		try {
			auto stream = agi::io::Open(path, true);
			stream->read(&buffer[0], 1024);
			auto hash = murmur3(&buffer[0], stream->tellg());
			hash_to_path.emplace(hash, path);
		}
		catch (agi::Exception const& e) {
			cb(to_wx(e.GetMessage() + "\n"), 3);
		}
	}
	return hash_to_path;
}

void get_font_data(std::string& buffer, HDC dc) {
	buffer.clear();

	// For ttc files we have to ask for the "ttcf" table to get the complete file
	DWORD ttcf = 0x66637474;
	auto size = GetFontData(dc, ttcf, 0, nullptr, 0);
	if (size == GDI_ERROR) {
		ttcf = 0;
		size = GetFontData(dc, 0, 0, nullptr, 0);
	}
	if (size == GDI_ERROR || size == 0)
		return;

	buffer.resize(size);
	GetFontData(dc, ttcf, 0, &buffer[0], size);
}
}

GdiFontFileLister::GdiFontFileLister(FontCollectorStatusCallback &cb)
: dc(CreateCompatibleDC(nullptr), [](HDC dc) { DeleteDC(dc); })
{
	cb(_("Updating font cache\n"), 0);
	index = index_fonts(cb);
}

CollectionResult GdiFontFileLister::GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters) {
	CollectionResult ret;

	LOGFONTW lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	wcsncpy(lf.lfFaceName, agi::charset::ConvertW(facename).c_str(), LF_FACESIZE);
	lf.lfItalic = italic ? -1 : 0;
	lf.lfWeight = bold == 0 ? 400 :
	              bold == 1 ? 700 :
	                          bold;

	// Gather all of the styles for the given family name
	std::vector<LOGFONTW> matches;
	using type = decltype(matches);
	EnumFontFamiliesEx(dc, &lf, [](const LOGFONT *lf, const TEXTMETRIC *, DWORD, LPARAM lParam) -> int {
		reinterpret_cast<type*>(lParam)->push_back(*lf);
		return 1;
	}, (LPARAM)&matches, 0);

	if (matches.empty())
		return ret;

	// If the user asked for a non-regular style, verify that it actually exists
	if (italic || bold) {
		bool has_bold = false;
		bool has_italic = false;
		bool has_bold_italic = false;

		auto is_italic = [&](LOGFONTW const& lf) {
			return !italic || lf.lfItalic;
		};
		auto is_bold = [&](LOGFONTW const& lf) {
			return !bold
				|| (bold == 1 && lf.lfWeight >= 700)
				|| (bold > 1 && lf.lfWeight > bold);
		};

		for (auto const& match : matches) {
			has_bold = has_bold || is_bold(match);
			has_italic = has_italic || is_italic(match);
			has_bold_italic = has_bold_italic || (is_bold(match) && is_italic(match));
		}

		ret.fake_italic = !has_italic;
		ret.fake_bold = (italic && has_italic ? !has_bold_italic : !has_bold);
	}

	// Use the family name supplied by EnumFontFamiliesEx as it may be a localized version
	memcpy(lf.lfFaceName, matches[0].lfFaceName, LF_FACESIZE);

	// Open the font and get the data for it to look up in the index
	auto hfont = CreateFontIndirectW(&lf);
	SelectObject(dc, hfont);
	BOOST_SCOPE_EXIT_ALL(=) {
		SelectObject(dc, nullptr);
		DeleteObject(hfont);
	};

	get_font_data(buffer, dc);

	auto range = index.equal_range(murmur3(buffer.c_str(), std::min<size_t>(buffer.size(), 1024U)));
	if (range.first == range.second)
		return ret; // could instead write to a temp dir

	// Compare the full files for each of the fonts with the same prefix
	std::unique_ptr<char[]> file_buffer(new char[buffer.size()]);
	for (auto it = range.first; it != range.second; ++it) {
		auto stream = agi::io::Open(it->second, true);
		stream->read(&file_buffer[0], buffer.size());
		if ((size_t)stream->tellg() != buffer.size())
			continue;
		if (memcmp(&file_buffer[0], &buffer[0], buffer.size()) == 0) {
			ret.paths.push_back(it->second);
			break;
		}
	}

	// No fonts actually matched
	if (ret.paths.empty())
		return ret;

	// Convert the characters to a utf-16 string
	std::wstring utf16characters;
	utf16characters.reserve(characters.size());
	for (int chr : characters) {
		if (U16_LENGTH(chr) == 1)
			utf16characters.push_back(static_cast<wchar_t>(chr));
		else {
			utf16characters.push_back(U16_LEAD(chr));
			utf16characters.push_back(U16_TRAIL(chr));
		}
	}

	SCRIPT_CACHE cache = nullptr;
	std::unique_ptr<WORD[]> indices(new WORD[utf16characters.size()]);

	// First try to check glyph coverage with Uniscribe, since it
	// handles non-BMP unicode characters
	auto hr = ScriptGetCMap(dc, &cache, utf16characters.data(),
		utf16characters.size(), 0, indices.get());

	// Uniscribe doesn't like some types of fonts, so fall back to GDI
	if (hr == E_HANDLE) {
		GetGlyphIndicesW(dc, utf16characters.data(), utf16characters.size(),
			indices.get(), GGI_MARK_NONEXISTING_GLYPHS);
		for (size_t i = 0; i < utf16characters.size(); ++i) {
			if (U16_IS_SURROGATE(utf16characters[i]))
				continue;
			if (indices[i] == SHRT_MAX)
				ret.missing += utf16characters[i];
		}
	}
	else if (hr == S_FALSE) {
		for (size_t i = 0; i < utf16characters.size(); ++i) {
			// Uniscribe doesn't report glyph indexes for non-BMP characters,
			// so we have to call ScriptGetCMap on each individual pair to
			// determine if it's the missing one
			if (U16_IS_LEAD(utf16characters[i])) {
				hr = ScriptGetCMap(dc, &cache, &utf16characters[i], 2, 0, &indices[i]);
				if (hr == S_FALSE) {
					ret.missing += utf16characters[i];
					ret.missing += utf16characters[i + 1];
				}
				++i;
			}
			else if (indices[i] == 0) {
				ret.missing += utf16characters[i];
			}
		}
	}
	ScriptFreeCache(&cache);

	return ret;
}
