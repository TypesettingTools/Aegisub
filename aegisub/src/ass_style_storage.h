// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file ass_style_storage.h
/// @see ass_style_storage.cpp
/// @ingroup style_editor
///

#ifndef AGI_PRE
#include <deque>

#include <wx/arrstr.h>
#endif

class AssStyle;

/// DOCME
/// @class AssStyleStorage
/// @brief DOCME
///
/// DOCME
class AssStyleStorage {
	wxString storage_name;
	std::deque<AssStyle*> style;

public:
	~AssStyleStorage();

	typedef std::deque<AssStyle*>::iterator iterator;
	typedef std::deque<AssStyle*>::const_iterator const_iterator;
	iterator begin() { return style.begin(); }
	iterator end() { return style.end(); }
	const_iterator begin() const { return style.begin(); }
	const_iterator end() const { return style.end(); }
	void push_back(AssStyle *new_style) { style.push_back(new_style); }
	AssStyle *back() { return style.back(); }
	AssStyle *operator[](size_t idx) const { return style[idx]; }
	size_t size() const { return style.size(); }

	/// Get the names of all styles in this storage
	wxArrayString GetNames();

	/// Delete all styles in this storage
	void Clear();

	/// Delete the style at the given index
	void Delete(int idx);

	/// Get the style with the given name
	/// @param name Case-insensitive style name
	/// @return Style or NULL if the requested style is not found
	AssStyle *GetStyle(wxString const& name);

	/// Save stored styles to a file
	void Save() const;

	/// Load stored styles from a file
	/// @param name Catalog name (note: not file name)
	void Load(wxString const& name);
};
