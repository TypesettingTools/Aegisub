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

template <typename func>
class FactoryBase {
protected:
	typedef std::map<std::string, std::pair<bool, func>> map;

	static map& classes() {
		static map classes;
		return classes;
	}

	static void DoRegister(func function, std::string name, bool hide, std::vector<std::string> &subtypes) {
		if (subtypes.empty())
			classes().insert(std::make_pair(name, std::make_pair(hide, function)));
		else {
			for (auto const& subtype : subtypes)
				classes().insert(std::make_pair(name + '/' + subtype, std::make_pair(hide, function)));
		}
	}

	static func Find(std::string const& name) {
		auto factory = classes().find(name);
		return factory != classes().end() ? factory->second.second : nullptr;
	}

public:
	static std::vector<std::string> GetClasses(std::string favourite="") {
		std::vector<std::string> list;
		std::string cmp;
		std::transform(favourite.begin(), favourite.end(), favourite.begin(), ::tolower);
		for (auto const& cls : classes()) {
			cmp.clear();
			std::transform(cls.first.begin(), cls.first.end(), std::back_inserter(cmp), ::tolower);
			if (cmp == favourite)
				list.insert(list.begin(), cls.first);
			else if (!cls.second.first)
				list.push_back(cls.first);
		}
		return list;
	}
};

template<typename Base, typename Arg1=void, typename Arg2=void>
class Factory : public FactoryBase<Base *(*)(Arg1, Arg2)> {
	typedef Base *(*func)(Arg1, Arg2);

public:
	static std::unique_ptr<Base> Create(std::string const& name, Arg1 a1, Arg2 a2) {
		auto factory = FactoryBase<func>::Find(name);
		return factory ? std::unique_ptr<Base>(factory(a1, a2)) : nullptr;
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subTypes = std::vector<std::string>()) {
		FactoryBase<func>::DoRegister([](Arg1 a1, Arg2 a2) -> Base * { return new T(a1, a2); }, name, hide, subTypes);
	}
};

template<typename Base, typename Arg1>
class Factory<Base, Arg1, void> : public FactoryBase<Base *(*)(Arg1)> {
	typedef Base *(*func)(Arg1);

public:
	static std::unique_ptr<Base> Create(std::string const& name, Arg1 a1) {
		auto factory = FactoryBase<func>::Find(name);
		return factory ? std::unique_ptr<Base>(factory(a1)) : nullptr;
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subTypes = std::vector<std::string>()) {
		FactoryBase<func>::DoRegister([](Arg1 a1) -> Base * { return new T(a1); }, name, hide, subTypes);
	}
};

template<typename Base>
class Factory<Base, void, void> : public FactoryBase<Base *(*)()> {
	typedef Base *(*func)();

public:
	static std::unique_ptr<Base> Create(std::string const& name) {
		auto factory = FactoryBase<func>::Find(name);
		return factory ? std::unique_ptr<Base>(factory()) : nullptr;
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subTypes = std::vector<std::string>()) {
		FactoryBase<func>::DoRegister([]() -> Base * { return new T; }, name, hide, subTypes);
	}
};
