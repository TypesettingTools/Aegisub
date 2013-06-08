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

#include <libaegisub/fs_fwd.h>

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>
#include <vector>

class AssStyle;

class AssStyleStorage {
	agi::fs::path file;
	std::vector<std::unique_ptr<AssStyle>> style;

public:
	~AssStyleStorage();

	typedef std::vector<std::unique_ptr<AssStyle>>::iterator iterator;
	typedef std::vector<std::unique_ptr<AssStyle>>::const_iterator const_iterator;
	iterator begin() { return style.begin(); }
	iterator end() { return style.end(); }
	const_iterator begin() const { return style.begin(); }
	const_iterator end() const { return style.end(); }
	void push_back(std::unique_ptr<AssStyle>&& new_style);
	AssStyle *back() { return style.back().get(); }
	AssStyle *operator[](size_t idx) const { return style[idx].get(); }
	size_t size() const { return style.size(); }
	void clear();

	/// Get the names of all styles in this storage
	std::vector<std::string> GetNames();

	/// Delete the style at the given index
	void Delete(int idx);

	/// Get the style with the given name
	/// @param name Case-insensitive style name
	/// @return Style or nullptr if the requested style is not found
	AssStyle *GetStyle(std::string const& name);

	/// Save stored styles to a file
	void Save() const;

	/// Load stored styles from a file
	/// @param filename Catalog filename. Does not have to exist.
	void Load(agi::fs::path const& filename);
};
