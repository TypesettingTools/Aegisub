// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file font_file_lister.h
/// @see font_file_lister.cpp
/// @ingroup font_collector
///


////////////
// Includes
#ifndef AGI_PRE
#include <map>

#include <wx/arrstr.h>
#include <wx/string.h>
#endif


////////////
// Typedefs
#if defined(__WINDOWS__) || defined(__APPLE__)

/// DOCME
typedef struct FT_LibraryRec_ *FT_Library;
#endif

/// DOCME
typedef std::map<wxString,wxArrayString> FontMap;



/// DOCME
/// @class FontFileLister
/// @brief DOCME
///
/// DOCME
class FontFileLister {
private:

	/// DOCME
	static FontFileLister *instance;
	static void GetInstance();

	/// DOCME
	FontMap fontTable;

	/// DOCME
	wxArrayString fontFiles;

protected:

	/// @brief DOCME
	/// @param facename 
	/// @return 
	///
	virtual wxArrayString DoGetFilesWithFace(wxString facename) { return CacheGetFilesWithFace(facename); }
	virtual void DoInitialize()=0;

	/// @brief DOCME
	///
	virtual void DoClearData() { ClearCache(); }

	FontFileLister();
	virtual ~FontFileLister();

	wxArrayString CacheGetFilesWithFace(wxString facename);
	bool IsFilenameCached(wxString filename);
	void AddFont(wxString filename,wxString facename);
	void SaveCache();
	void LoadCache();
	void ClearCache();

public:
	static wxArrayString GetFilesWithFace(wxString facename);
	static void Initialize();
	static void ClearData();
};


