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

#include "thesaurus.h"

#include "options.h"

#include <libaegisub/dispatch.h>
#include <libaegisub/format.h>
#include <libaegisub/fs.h>
#include <libaegisub/log.h>
#include <libaegisub/path.h>
#include <libaegisub/thesaurus.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/algorithm.hpp>

Thesaurus::Thesaurus()
: lang_listener(OPT_SUB("Tool/Thesaurus/Language", &Thesaurus::OnLanguageChanged, this))
, dict_path_listener(OPT_SUB("Path/Dictionary", &Thesaurus::OnPathChanged, this))
{
	OnLanguageChanged();
}

Thesaurus::~Thesaurus() {
	if (cancel_load) *cancel_load = true;
}

std::vector<Thesaurus::Entry> Thesaurus::Lookup(std::string word) {
	if (!impl) return {};
	boost::to_lower(word);
	return impl->Lookup(word);
}

static std::vector<std::string> langs(const char *ext) {
	std::vector<std::string> paths;
	auto data_path = config::path->Decode("?data/dictionaries/");
	auto user_path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString());

	auto filter = std::string("th_*.") + ext;
	agi::fs::DirectoryIterator(data_path, filter).GetAll(paths);
	agi::fs::DirectoryIterator(user_path, filter).GetAll(paths);

	// Drop extensions and the th_ prefix
	for (auto& fn : paths) fn = fn.substr(3, fn.size() - filter.size() + 1);

	boost::sort(paths);
	paths.erase(unique(begin(paths), end(paths)), end(paths));

	return paths;
}

std::vector<std::string> Thesaurus::GetLanguageList() const {
	if (languages.empty())
		boost::set_intersection(langs("idx"), langs("dat"), back_inserter(languages));
	return languages;
}

static bool check_path(std::filesystem::path const& path, std::string const& language, std::filesystem::path& idx, std::filesystem::path& dat) {
	idx = path/agi::format("th_%s.idx", language);
	dat = path/agi::format("th_%s.dat", language);
	return agi::fs::FileExists(idx) && agi::fs::FileExists(dat);
}

void Thesaurus::OnLanguageChanged() {
	impl.reset();

	auto language = OPT_GET("Tool/Thesaurus/Language")->GetString();
	if (language.empty()) return;

	std::filesystem::path idx, dat;

	auto path = config::path->Decode(OPT_GET("Path/Dictionary")->GetString() + "/");
	if (!check_path(path, language, idx, dat)) {
		path = config::path->Decode("?data/dictionaries/");
		if (!check_path(path, language, idx, dat))
			return;
	}

	LOG_I("thesaurus/file") << "Using thesaurus: " << dat;

	if (cancel_load) *cancel_load = true;
	cancel_load = new bool{false};
	auto cancel = cancel_load; // Needed to avoid capturing via `this`
	agi::dispatch::Background().Async([=, this]{
		try {
			auto thes = std::make_unique<agi::Thesaurus>(dat, idx);
			agi::dispatch::Main().Sync([&thes, cancel, this]{
				if (!*cancel) {
					impl = std::move(thes);
					cancel_load = nullptr;
				}
				delete cancel;
			});
		}
		catch (agi::Exception const& e) {
			LOG_E("thesaurus") << e.GetMessage();
		}
	});
}

void Thesaurus::OnPathChanged() {
	languages.clear();
}
