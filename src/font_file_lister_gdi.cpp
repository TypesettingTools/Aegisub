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

#include <libaegisub/charset_conv_win.h>
#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/log.h>

#include <ShlObj.h>
#include <boost/scope_exit.hpp>

namespace {
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

std::string read_file(agi::fs::path const& path) {
	std::string data;
	auto stream = agi::io::Open(path, true);
	stream->seekg(0, std::ios::end);
	data.resize(stream->tellg());
	stream->seekg(0, std::ios::beg);
	stream->read(&data[0], data.size());
	return data;
}

using font_index = std::unordered_map<std::string, agi::fs::path>;

font_index index_fonts() {
	font_index hash_to_path;
	auto fonts = get_installed_fonts();
	for (auto const& path : fonts) {
		hash_to_path[read_file(path)] = path;
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
	index = index_fonts();
}

CollectionResult GdiFontFileLister::GetFontPaths(std::string const& facename, int bold, bool italic, std::vector<int> const& characters) {
	CollectionResult ret;

	LOGFONTW lf{};
	lf.lfCharSet = DEFAULT_CHARSET;
	wcsncpy(lf.lfFaceName, agi::charset::ConvertW(facename).c_str(), LF_FACESIZE);
	lf.lfItalic = italic;
	lf.lfWeight = bold == 0 ? 400 :
	              bold == 1 ? 700 :
	                          bold;

	auto cb = [&](LOGFONTW const& actual) {
		auto hfont = CreateFontIndirectW(&actual);
		SelectObject(dc, hfont);
		BOOST_SCOPE_EXIT_ALL(=) {
			SelectObject(dc, nullptr);
			DeleteObject(hfont);
		};

		get_font_data(buffer, dc);

		auto it = index.find(buffer);
		if (it == end(index))
			return false; // could instead write to a temp dir

		ret.paths.push_back(it->second);
		if (actual.lfWeight < lf.lfWeight)
			ret.fake_bold = true;
		if (actual.lfItalic < lf.lfItalic)
			ret.fake_italic = true;

		// Convert the characters to a utf-16 string
		std::wstring utf16characters;
		utf16characters.reserve(characters.size());
		for (int chr : characters) {
			// GetGlyphIndices does not support surrogate pairs, so only check BMP characters
			if (chr < std::numeric_limits<wchar_t>::max())
				utf16characters.push_back(static_cast<wchar_t>(chr));
		}

		std::unique_ptr<WORD[]> indices(new WORD[utf16characters.size()]);
		GetGlyphIndicesW(dc, utf16characters.data(), utf16characters.size(),
		                 indices.get(), GGI_MARK_NONEXISTING_GLYPHS);

		for (size_t i = 0; i < utf16characters.size(); ++i) {
			if (indices[i] == 0xFFFF)
				ret.missing += utf16characters[i];
		}

		return true;
	};

	using type = decltype(cb);
	bool found = !EnumFontFamiliesEx(dc, &lf,
		[](const LOGFONT *lf, const TEXTMETRIC *, DWORD, LPARAM lParam) -> int {
			return !(*reinterpret_cast<type*>(lParam))(*lf);
		}, (LPARAM)&cb, 0);

	return ret;
}
