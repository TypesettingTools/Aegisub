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

/// @file thesaurus.cpp
/// @brief Thesaurus implementation
/// @ingroup thesaurus
///

#include "config.h"

#include "thesaurus.h"

#include "options.h"

#include <boost/format.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/thesaurus.h>

Thesaurus::Thesaurus()
: lang_listener(OPT_SUB("Tool/Thesaurus/Language", &Thesaurus::OnLanguageChanged, this))
, dict_path_listener(OPT_SUB("Path/Dictionary", &Thesaurus::OnPathChanged, this))
{
	OnLanguageChanged();
}

Thesaurus::~Thesaurus() {
	// Explicit empty destructor needed for scoped_ptr with incomplete types
}

std::vector<Thesaurus::Entry> Thesaurus::Lookup(std::string word) {
	if (!impl.get()) return std::vector<Entry>();
	boost::to_lower(word);
	return impl->Lookup(word);
}

std::vector<std::string> Thesaurus::GetLanguageList() const {
	if (!languages.empty()) return languages;

	std::vector<std::string> idx, dat;

	// Get list of dictionaries
	auto path = config::path->Decode("?data/dictionaries/");
	agi::fs::DirectoryIterator(path, "th_*.idx").GetAll(idx);
	agi::fs::DirectoryIterator(path, "th_*.dat").GetAll(dat);

	path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString());
	agi::fs::DirectoryIterator(path, "th_*.idx").GetAll(idx);
	agi::fs::DirectoryIterator(path, "th_*.dat").GetAll(dat);

	if (idx.empty() || dat.empty()) return languages;

	sort(begin(idx), end(idx));
	sort(begin(dat), end(dat));

	// Drop extensions and the th_ prefix
	for (auto& fn : idx) fn = fn.substr(3, fn.size() - 7);
	for (auto& fn : dat) fn = fn.substr(3, fn.size() - 7);

	// Verify that each idx has a dat
	for (size_t i = 0, j = 0; i < idx.size() && j < dat.size(); ) {
		int cmp = idx[i].compare(dat[j]);
		if (cmp < 0) ++i;
		else if (cmp > 0) ++j;
		else {
			// Don't insert a language twice if it's in both the user dir and
			// the app's dir
			if (languages.empty() || dat[j] != languages.back())
				languages.push_back(dat[j]);
			++i;
			++j;
		}
	}
	return languages;
}

void Thesaurus::OnLanguageChanged() {
	impl.reset();

	auto language = OPT_GET("Tool/Thesaurus/Language")->GetString();
	if (language.empty()) return;

	auto path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString() + "/");

	// Get index and data paths
	auto idxpath = path/str(boost::format("th_%s.idx") % language);
	auto datpath = path/str(boost::format("th_%s.dat") % language);

	// If they aren't in the user dictionary path, check the application directory
	if (!agi::fs::FileExists(idxpath) || !agi::fs::FileExists(datpath)) {
		path = config::path->Decode("?data/dictionaries/");
		idxpath = path/str(boost::format("th_%s.idx") % language);
		datpath = path/str(boost::format("th_%s.dat") % language);

		if (!agi::fs::FileExists(idxpath) || !agi::fs::FileExists(datpath)) return;
	}

	LOG_I("thesaurus/file") << "Using thesaurus: " << datpath;

	impl.reset(new agi::Thesaurus(datpath, idxpath));
}

void Thesaurus::OnPathChanged() {
	languages.clear();
}
