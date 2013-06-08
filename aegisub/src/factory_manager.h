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

template <class func>
class FactoryBase {
protected:
	typedef std::map<std::string, std::pair<bool, func>> map;
	typedef typename map::iterator iterator;

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
		iterator factory = classes().find(name);
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

template<class Base>
class Factory0 : public FactoryBase<std::unique_ptr<Base>(*)()> {
	typedef std::unique_ptr<Base>(*func)();
	template<class T>
	static std::unique_ptr<Base> create() {
		return std::unique_ptr<Base>(new T);
	}

public:
	static std::unique_ptr<Base> Create(std::string const& name) {
		func factory = FactoryBase<func>::Find(name);
		return factory ? factory() : nullptr;
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subTypes = std::vector<std::string>()) {
		FactoryBase<func>::DoRegister(&Factory0<Base>::template create<T>, name, hide, subTypes);
	}
};

template<class Base, class Arg1>
class Factory1 : public FactoryBase<std::unique_ptr<Base>(*)(Arg1)> {
	typedef std::unique_ptr<Base>(*func)(Arg1);
	template<class T>
	static std::unique_ptr<Base> create(Arg1 a1) {
		return std::unique_ptr<Base>(new T(a1));
	}

public:
	static std::unique_ptr<Base> Create(std::string const& name, Arg1 a1) {
		func factory = FactoryBase<func>::Find(name);
		return factory ? factory(a1) : nullptr;
	}

	template<class T>
	static void Register(std::string name, bool hide = false, std::vector<std::string> subTypes = std::vector<std::string>()) {
		FactoryBase<func>::DoRegister(&Factory1<Base, Arg1>::template create<T>, name, hide, subTypes);
	}
};
