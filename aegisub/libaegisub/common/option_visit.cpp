// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

/// @file option_visit.cpp
/// @brief Cajun JSON visitor to load config values.
/// @see option_visit.h
/// @ingroup libaegisub

#include "../config.h"

#include "option_visit.h"

#include <cassert>
#include <cmath>

#include <libaegisub/color.h>
#include <libaegisub/log.h>
#include <libaegisub/option_value.h>
#include <libaegisub/util.h>

#include <boost/algorithm/string/predicate.hpp>

namespace agi {

ConfigVisitor::ConfigVisitor(OptionValueMap &val, const std::string &member_name, bool ignore_errors, bool replace)
: values(val)
, name(member_name)
, ignore_errors(ignore_errors)
, replace(replace)
{
}

template<class ErrorType>
void ConfigVisitor::Error(const char *message) {
	if (ignore_errors)
		LOG_E("option/load/config_visitor") << "Error loading option from user configuration: " << message;
	else
		throw ErrorType(message);
}

void ConfigVisitor::Visit(const json::Object& object) {
	if (!name.empty())
		name += "/";

	for (auto const& obj : object) {
		ConfigVisitor config_visitor(values, name + obj.first, ignore_errors, replace);
		obj.second.Accept(config_visitor);
	}
}

template<class OptionValueType, class ValueType>
std::unique_ptr<OptionValue> ConfigVisitor::ReadArray(json::Array const& src, std::string const& array_type, void (OptionValueType::*)(const std::vector<ValueType>&)) {
	std::vector<ValueType> arr;
	arr.reserve(src.size());

	for (json::Object const& obj : src) {
		if (obj.size() != 1) {
			Error<OptionJsonValueArray>("Invalid array member");
			return 0;
		}
		if (obj.begin()->first != array_type) {
			Error<OptionJsonValueArray>("Attempt to insert value into array of wrong type");
			return 0;
		}

		arr.push_back(ValueType(obj.begin()->second));
	}

	return util::make_unique<OptionValueType>(name, arr);
}

void ConfigVisitor::Visit(const json::Array& array) {
	if (array.empty()) {
		Error<OptionJsonValueArray>("Cannot infer the type of an empty array");
		return;
	}

	json::Object const& front = array.front();
	if (front.size() != 1) {
		Error<OptionJsonValueArray>("Invalid array member");
		return;
	}

	const std::string& array_type = front.begin()->first;

	if (array_type == "string")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListString::SetListString));
	else if (array_type == "int")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListInt::SetListInt));
	else if (array_type == "double")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListDouble::SetListDouble));
	else if (array_type == "bool")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListBool::SetListBool));
	else if (array_type == "color")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListColor::SetListColor));
	else
		Error<OptionJsonValueArray>("Array type not handled");
}

void ConfigVisitor::Visit(const json::Integer& number) {
	AddOptionValue(util::make_unique<OptionValueInt>(name, number));
}

void ConfigVisitor::Visit(const json::Double& number) {
	AddOptionValue(util::make_unique<OptionValueDouble>(name, number));
}

void ConfigVisitor::Visit(const json::String& string) {
	size_t size = string.size();
	if ((size == 4 && string[0] == '#') ||
		(size == 7 && string[0] == '#') ||
		(size >= 10 && boost::starts_with(string, "rgb(")) ||
		((size == 9 || size == 10) && boost::starts_with(string, "&H")))
	{
		AddOptionValue(util::make_unique<OptionValueColor>(name, string));
	} else {
		AddOptionValue(util::make_unique<OptionValueString>(name, string));
	}
}

void ConfigVisitor::Visit(const json::Boolean& boolean) {
	AddOptionValue(util::make_unique<OptionValueBool>(name, boolean));
}

void ConfigVisitor::Visit(const json::Null& null) {
	Error<OptionJsonValueNull>("Attempt to read null value");
}

void ConfigVisitor::AddOptionValue(std::unique_ptr<OptionValue>&& opt) {
	if (!opt) {
		assert(ignore_errors);
		return;
	}

	auto it = values.find(name);
	if (it == values.end())
		values[name] = std::move(opt);
	else if (replace)
		it->second = std::move(opt);
	else {
		try {
			values[name]->Set(opt.get());
		}
		catch (agi::OptionValueError const& e) {
			if (ignore_errors)
				LOG_E("option/load/config_visitor") << "Error loading option from user configuration: " << e.GetChainedMessage();
			else
				throw;
		}
	}
}
} // namespace agi
