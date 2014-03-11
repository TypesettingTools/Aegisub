// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file factory_manager.h
/// @brief Template/base-class for factory classes
/// @ingroup utility
///

#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace agi { class ProgressSink; }

template<typename Base, typename... Args>
class Factory {
	typedef Base *(*func)(Args const&...);
	typedef std::map<std::string, std::pair<bool, func>> map;

	static map& classes() {
		static map classes;
		return classes;
	}

public:
	static std::vector<std::string> GetClasses(std::string favorite="") {
		std::vector<std::string> list;
		std::string cmp;
		for (auto& c : favorite) c = ::tolower(c);
		for (auto const& cls : classes()) {
			cmp.clear();
			std::transform(cls.first.begin(), cls.first.end(), std::back_inserter(cmp), ::tolower);
			if (cmp == favorite)
				list.insert(list.begin(), cls.first);
			else if (!cls.second.first)
				list.push_back(cls.first);
		}
		return list;
	}

	static std::unique_ptr<Base> Create(std::string const& name, Args const&... args) {
		auto factory = classes().find(name);
		if (factory == classes().end()) return nullptr;
		return std::unique_ptr<Base>(factory->second.second(args...));
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subtypes = {}) {
		func factory = [](Args const&... args) -> Base * { return new T(args...); };
		if (subtypes.empty())
			classes().insert(std::make_pair(name, std::make_pair(hide, factory)));
		else {
			for (auto const& subtype : subtypes)
				classes().insert(std::make_pair(name + '/' + subtype, std::make_pair(hide, factory)));
		}
	}
};

