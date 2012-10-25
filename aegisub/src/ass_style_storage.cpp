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

/// @file ass_style_storage.cpp
/// @brief Manage stores of styles
/// @ingroup style_editor
///

#include "config.h"

#include "ass_style_storage.h"

#ifndef AGI_PRE
#include <tr1/functional>
#endif

#include "ass_style.h"
#include "standard_paths.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"

AssStyleStorage::~AssStyleStorage() {
	delete_clear(style);
}

void AssStyleStorage::Save() const {
	if (storage_name.empty()) return;

	wxString dirname = StandardPaths::DecodePath("?user/catalog/");
	if (!wxDirExists(dirname) && !wxMkdir(dirname))
		throw "Failed creating directory for style catalogs";

	TextFileWriter file(StandardPaths::DecodePath("?user/catalog/" + storage_name + ".sty"), "UTF-8");
	for (const_iterator cur = begin(); cur != end(); ++cur)
		file.WriteLineToFile((*cur)->GetEntryData());
}

void AssStyleStorage::Load(wxString const& name) {
	storage_name = name;
	Clear();

	try {
		TextFileReader file(StandardPaths::DecodePath("?user/catalog/" + name + ".sty"), "UTF-8");

		while (file.HasMoreLines()) {
			wxString data = file.ReadLineFromFile();
			if (data.StartsWith("Style:")) {
				try {
					style.push_back(new AssStyle(data));
				} catch(...) {
					/* just ignore invalid lines for now */
				}
			}
		}
	}
	catch (agi::FileNotAccessibleError const&) {
		// Just treat a missing file as an empty file
	}
}

void AssStyleStorage::Clear() {
	delete_clear(style);
}

void AssStyleStorage::Delete(int idx) {
	delete style[idx];
	style.erase(style.begin() + idx);
}

wxArrayString AssStyleStorage::GetNames() {
	wxArrayString names;
	for (iterator cur = begin(); cur != end(); ++cur)
		names.Add((*cur)->name);
	return names;
}

AssStyle *AssStyleStorage::GetStyle(wxString const& name) {
	for (iterator cur = begin(); cur != end(); ++cur) {
		if ((*cur)->name.CmpNoCase(name) == 0)
			return *cur;
	}
	return 0;
}
