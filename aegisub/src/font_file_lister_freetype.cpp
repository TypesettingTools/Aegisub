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
//
// $Id$

/// @file font_file_lister_freetype.cpp
/// @brief FreeType based font collector
/// @ingroup font_collector
///

#include "config.h"

#ifdef WITH_FREETYPE2
#include "font_file_lister_freetype.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

#include <shlobj.h>

#include "charset_conv.h"
#include "standard_paths.h"
#include "text_file_reader.h"
#include "text_file_writer.h"

namespace {
	typedef std::map<wxString, std::set<wxString> > FontMap;

	wxString get_font_folder() {
		wchar_t szPath[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, szPath)))
			return wxString(szPath) + "\\";
		else
			return wxGetOSDirectory() + "\\fonts\\";
	}

	FT_UInt get_name_count(wxString const& filename, FT_Face face) {
		wxString ext = filename.Right(4).Lower();
		if (ext == ".otf" || ext == ".ttf" || ext == ".ttc_")
			return FT_Get_Sfnt_Name_Count(face);
		return 0;
	}

	std::map<FT_UShort, std::vector<wxString> > get_names(FT_Face face) {
		std::map<FT_UShort, std::vector<wxString> > final;

		FT_UInt count = FT_Get_Sfnt_Name_Count(face);
		for (FT_UInt i = 0; i < count; ++i) {
			FT_SfntName name;
			FT_Get_Sfnt_Name(face, i, &name);

			// truetype string encoding is an absolute nightmare, so just try
			// UTF-16BE and local charsets
			if (name.string_len) {
				if ((name.platform_id == TT_PLATFORM_MICROSOFT && name.encoding_id == TT_MS_ID_UNICODE_CS) ||
					name.platform_id == TT_PLATFORM_APPLE_UNICODE ||
					name.string[0])
				{
					final[name.name_id].push_back(wxString(name.string, wxMBConvUTF16BE(), name.string_len));
				}
				else
					final[name.name_id].push_back(wxString(name.string, name.string_len));
			}
		}

		return final;
	}

	void load_cache(FontMap &font_files, std::set<wxString> &indexed_files) {
		try {
			TextFileReader file(StandardPaths::DecodePath("?local/freetype_collector_index.dat"), "utf-16le");
			while (file.HasMoreLines()) {
				wxString face = file.ReadLineFromFile();
				std::set<wxString>& files = font_files[face];

				while (file.HasMoreLines()) {
					wxString filename = file.ReadLineFromFile();
					if (filename.empty()) break;
					if (wxFileExists(filename)) {
						files.insert(filename);
						indexed_files.insert(filename);
					}
				}
			}
		}
		catch (agi::FileNotAccessibleError const&) { }
	}

	void save_cache(FontMap &font_files) {
		TextFileWriter file(StandardPaths::DecodePath("?local/freetype_collector_index.dat"), "utf-16le");
		for (FontMap::iterator face_it = font_files.begin(); face_it != font_files.end(); ++face_it) {
			file.WriteLineToFile(face_it->first);
			for_each(face_it->second.begin(), face_it->second.end(),
				bind(&TextFileWriter::WriteLineToFile, &file, std::tr1::placeholders::_1, true));
			file.WriteLineToFile("");
		}
	}
}

FreetypeFontFileLister::FreetypeFontFileLister(FontCollectorStatusCallback AppendText) {
	AppendText(_("Collecting font data from system. This might take a while, depending on the number of fonts installed. Results are cached and subsequent executions will be faster...\n"), 0);
	load_cache(font_files, indexed_files);

	FT_Library ft2lib;
	FT_Init_FreeType(&ft2lib);

	wxArrayString fontfiles;
	wxDir::GetAllFiles(get_font_folder(), &fontfiles, "", wxDIR_FILES);

	for (size_t i = 0; i < fontfiles.size(); ++i) {
		if (indexed_files.count(fontfiles[i])) continue;

		FT_Face face;
		for (FT_Long i = 0; FT_New_Face(ft2lib, fontfiles[i].mb_str(*wxConvFileName), i, &face) == 0; ++i) {
			if (get_name_count(fontfiles[i], face) > 0) {
				std::map<FT_UShort, std::vector<wxString> > names = get_names(face);
				std::vector<wxString>& family = names[1];
				std::vector<wxString>& style = names[2];
				std::vector<wxString>& full_name = names[4];

				for (size_t j = 0; j < family.size() && j < style.size(); ++j) {
					if (style[j] != "Regular")
						AddFont(fontfiles[i], family[j], style[j]);
					else
						AddFont(fontfiles[i], family[j]);
				}
				for (size_t j = 0; j < full_name.size(); ++j)
					AddFont(fontfiles[i], full_name[j]);
			}
			else {
				if (face->style_name)
					AddFont(fontfiles[i], face->family_name, face->style_name);
				else
					AddFont(fontfiles[i], wxString(face->family_name));
			}
			FT_Done_Face(face);
		}
	}

	FT_Done_FreeType(ft2lib);

	AppendText(_("Done collecting font data.\n"), 0);

	save_cache(font_files);
}

void FreetypeFontFileLister::AddFont(wxString const& filename, wxString facename) {
	facename.Trim(true).Trim(false);
	if (facename.size() && !facename.Lower().StartsWith("copyright "))
		font_files[facename].insert(filename);
}

void FreetypeFontFileLister::AddFont(wxString const& filename, wxString const& family, wxString const& style) {
	AddFont(filename, family + " " + style);
	AddFont(filename, "*" + family);
}

std::vector<wxString> FreetypeFontFileLister::GetFontPaths(wxString const& facename, int, bool) {
	std::vector<wxString> ret;
	ret.insert(ret.end(), font_files[facename].begin(), font_files[facename].end());
	if (ret.empty())
		ret.insert(ret.end(), font_files["*" + facename].begin(), font_files["*" + facename].end());
	return ret;
}

#endif WITH_FREETYPE2
