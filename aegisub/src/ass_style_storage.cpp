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

#include "ass_style.h"
#include "standard_paths.h"
#include "text_file_reader.h"
#include "text_file_writer.h"
#include "utils.h"

#include <libaegisub/fs.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

AssStyleStorage::~AssStyleStorage() {
	delete_clear(style);
}

void AssStyleStorage::Save() const {
	if (storage_name.empty()) return;

	agi::fs::CreateDirectory(StandardPaths::DecodePath("?user/catalog/"));

	TextFileWriter file(StandardPaths::DecodePath("?user/catalog/" + storage_name + ".sty"), "UTF-8");
	for (const AssStyle *cur : style)
		file.WriteLineToFile(cur->GetEntryData());
}

void AssStyleStorage::Load(std::string const& name) {
	storage_name = name;
	Clear();

	try {
		TextFileReader file(StandardPaths::DecodePath("?user/catalog/" + name + ".sty"), "UTF-8");

		while (file.HasMoreLines()) {
			try {
				style.push_back(new AssStyle(file.ReadLineFromFile()));
			} catch(...) {
				/* just ignore invalid lines for now */
			}
		}
	}
	catch (agi::fs::FileNotAccessible const&) {
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

std::vector<std::string> AssStyleStorage::GetNames() {
	std::vector<std::string> names;
	for (const AssStyle *cur : style)
		names.emplace_back(cur->name);
	return names;
}

AssStyle *AssStyleStorage::GetStyle(std::string const& name) {
	for (AssStyle *cur : style) {
		if (boost::iequals(cur->name, name))
			return cur;
	}
	return 0;
}
