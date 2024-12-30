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

#include "ass_style_storage.h"

#include "ass_file.h"
#include "ass_style.h"
#include "options.h"

#include <libaegisub/fs.h>
#include <libaegisub/io.h>
#include <libaegisub/line_iterator.h>
#include <libaegisub/path.h>
#include <libaegisub/string.h>

#include <boost/algorithm/string/predicate.hpp>

AssStyleStorage::~AssStyleStorage() { }
void AssStyleStorage::clear() { style.clear(); }
void AssStyleStorage::push_back(std::unique_ptr<AssStyle> new_style) { style.emplace_back(std::move(new_style)); }

void AssStyleStorage::Save() const {
	if (file.empty()) return;

	agi::fs::CreateDirectory(file.parent_path());

	agi::io::Save out(file);
	out.Get() << "\xEF\xBB\xBF"; // UTF-8 BOM

	for (auto const& cur : style)
		out.Get() << cur->GetEntryData() << "\n";
}

void AssStyleStorage::Load(agi::fs::path const& filename) {
	file = filename;
	clear();

	try {
		auto in = agi::io::Open(file);
		for (auto const& line : agi::line_iterator<std::string>(*in)) {
			try {
				style.emplace_back(std::make_unique<AssStyle>(line));
			} catch(...) {
				/* just ignore invalid lines for now */
			}
		}
	}
	catch (agi::fs::FileNotAccessible const&) {
		// Just treat a missing file as an empty file
	}
}

void AssStyleStorage::LoadCatalog(std::string_view catalogname) {
	Load(config::path->Decode(agi::Str("?user/catalog/", catalogname, ".sty")));
}

void AssStyleStorage::Delete(int idx) {
	style.erase(style.begin() + idx);
}

std::vector<std::string> AssStyleStorage::GetNames() {
	std::vector<std::string> names;
	for (auto const& cur : style)
		names.emplace_back(cur->name);
	return names;
}

AssStyle *AssStyleStorage::GetStyle(std::string_view name) {
	for (auto& cur : style) {
		if (boost::iequals(cur->name, name))
			return cur.get();
	}
	return nullptr;
}

std::vector<std::string> AssStyleStorage::GetCatalogs() {
	std::vector<std::string> catalogs;
	for (auto const& file : agi::fs::DirectoryIterator(config::path->Decode("?user/catalog/"), "*.sty"))
		catalogs.push_back(agi::fs::path(file).stem().string());
	return catalogs;
}

bool AssStyleStorage::CatalogExists(std::string_view catalogname) {
	if (catalogname.empty()) return false;
	auto filename = config::path->Decode(agi::Str("?user/catalog/", catalogname, ".sty"));
	return agi::fs::FileExists(filename);
}

void AssStyleStorage::ReplaceIntoFile(AssFile &file) {
	for (auto const& s : style) {
		delete file.GetStyle(s->name);
		file.Styles.push_back(*new AssStyle(*s));
	}
}

