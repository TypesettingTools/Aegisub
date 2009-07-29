// Copyright (c) 2007, Niels Martin Hansen, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file font_file_lister_freetype.cpp
/// @brief FreeType based font collector
/// @ingroup font_collector
///


////////////
// Includes

#include "config.h"

#ifdef WITH_FREETYPE2
#include "font_file_lister_freetype.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SFNT_NAMES_H
#ifdef WIN32
# include <shlobj.h>
#endif
#include <wx/dir.h>
#include "charset_conv.h"



/// @brief Constructor 
///
FreetypeFontFileLister::FreetypeFontFileLister() {
	// Initialize freetype2
	FT_Init_FreeType(&ft2lib);
}



/// @brief Destructor 
///
FreetypeFontFileLister::~FreetypeFontFileLister() {
}



/// @brief Get name from face 
/// @param face 
/// @param id   
/// @return 
///
wxArrayString GetName(FT_Face &face,int id) {
	// Get name
	wxArrayString final;
	int count = FT_Get_Sfnt_Name_Count(face);

	for (int i=0;i<count;i++) {
		FT_SfntName name;
		FT_Get_Sfnt_Name(face,i,&name);
		if (name.name_id == id) {
			char *str = new char[name.string_len+2];
			memcpy(str,name.string,name.string_len);
			str[name.string_len] = 0;
			str[name.string_len+1] = 0;
			if (name.encoding_id == 0) final.Add(wxString(str, csConvLocal));
			else if (name.encoding_id == 1) {
				wxMBConvUTF16BE conv;
				wxString string(str,conv);
				final.Add(string);
			}
			delete [] str;
		}
	}

	return final;
}



/// @brief Gather data from system 
///
void FreetypeFontFileLister::DoInitialize() {
	// Load cache
	LoadCache();

	// Get fonts folder
	wxString source;
#ifdef WIN32
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS,NULL,0,szPath))) {
		source = wxString(szPath);
	}
	else source = wxGetOSDirectory() + _T("\\fonts");
	source += _T("\\");
#else
# ifdef __APPLE__
	// XXXHACK: Is this always a correct assumption?
	// Fonts might be instaled in more places, I think...
	source = _T("/Library/Fonts/");
# endif
#endif

	// Get the list of fonts in the fonts folder
	wxArrayString fontfiles;
	wxDir::GetAllFiles(source, &fontfiles, wxEmptyString, wxDIR_FILES);

	// Loop through each file
	int fterr;
	for (unsigned int i=0;i<fontfiles.Count(); i++) {
		// Check if it's cached
		if (IsFilenameCached(fontfiles[i])) continue;

		// Loop through each face in the file
		for (int facenum=0;true;facenum++) {
			// Get font face
			FT_Face face;
			fterr = FT_New_Face(ft2lib, fontfiles[i].mb_str(*wxConvFileName), facenum, &face);
			if (fterr) break;

			// Special names for TTF and OTF
			int nameCount = 0;
			wxString ext = fontfiles[i].Right(4).Lower();
			if (ext == _T(".otf") || ext == _T(".ttf") || ext == _T(".ttc_")) nameCount = FT_Get_Sfnt_Name_Count(face);
			if (nameCount > 0) {
				wxArrayString family = GetName(face,1);
				wxArrayString subFamily = GetName(face,2);
				wxArrayString fullName = GetName(face,4);
				for (size_t j=0;j<family.Count() && j<subFamily.Count();j++) {
					if (subFamily[j] != _T("Regular")) {
						AddFont(fontfiles[i],family[j] + _T(" ") + subFamily[j]);
						AddFont(fontfiles[i],_T("*")+family[j]);
					}
					else AddFont(fontfiles[i],family[j]);
				}
				for (size_t j=0;j<fullName.Count();j++) AddFont(fontfiles[i],fullName[j]);
			}

			// Ordinary fonts
			else {
				if (face->style_name) {
					AddFont(fontfiles[i],wxString(face->family_name, csConvLocal) + _T(" ") + wxString(face->style_name, csConvLocal));
					AddFont(fontfiles[i],_T("*")+wxString(face->family_name, csConvLocal));
				}
				else AddFont(fontfiles[i],wxString(face->family_name, csConvLocal));
			}
			FT_Done_Face(face);
		}
	}

	// Save cache
	SaveCache();
}

#endif WITH_FREETYPE2


