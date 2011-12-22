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
//
// $Id$

/// @file option_visit.cpp
/// @brief Cajun JSON visitor to load config values.
/// @see option_visit.h
/// @ingroup libaegisub

#include "../config.h"

#include "option_visit.h"

#ifndef LAGI_PRE
#include <cassert>
#include <cmath>
#include <memory>
#endif

#include <libaegisub/colour.h>
#include <libaegisub/log.h>
#include <libaegisub/option_value.h>

namespace agi {

ConfigVisitor::ConfigVisitor(OptionValueMap &val, const std::string &member_name, bool ignore_errors)
: values(val)
, name(member_name)
, ignore_errors(ignore_errors)
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
	json::Object::const_iterator index(object.begin()), index_end(object.end());

	if (!name.empty())
		name += "/";

	for (; index != index_end; ++index) {
		ConfigVisitor config_visitor(values, name + index->first, ignore_errors);
		index->second.Accept(config_visitor);
	}
}

template<class OptionValueType, class ValueType>
OptionValue *ConfigVisitor::ReadArray(json::Array const& src, std::string const& array_type, void (OptionValueType::*set_list)(const std::vector<ValueType>&)) {
	std::vector<ValueType> arr;
	arr.reserve(src.size());

	for (json::Array::const_iterator it = src.begin(); it != src.end(); ++it) {
		json::Object const& obj = *it;

		if (obj.size() != 1) {
			Error<OptionJsonValueArray>("Invalid array member");
			return 0;
		}
		if (obj.begin()->first != array_type) {
			Error<OptionJsonValueArray>("Attempt to insert value into array of wrong type");
			return 0;
		}

		arr.push_back(obj.begin()->second);
	}

	OptionValueType *ret = new OptionValueType(name);
	(ret->*set_list)(arr);
	return ret;
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
	else if (array_type == "colour")
		AddOptionValue(ReadArray(array, array_type, &OptionValueListColour::SetListColour));
	else
		Error<OptionJsonValueArray>("Array type not handled");
}

void ConfigVisitor::Visit(const json::Integer& number) {
	AddOptionValue(new OptionValueInt(name, number));
}

void ConfigVisitor::Visit(const json::Double& number) {
	AddOptionValue(new OptionValueDouble(name, number));
}

void ConfigVisitor::Visit(const json::String& string) {
	if (string.find("rgb(") == 0) {
		AddOptionValue(new OptionValueColour(name, string));
	} else {
		AddOptionValue(new OptionValueString(name, string));
	}
}

void ConfigVisitor::Visit(const json::Boolean& boolean) {
	AddOptionValue(new OptionValueBool(name, boolean));
}

void ConfigVisitor::Visit(const json::Null& null) {
	Error<OptionJsonValueNull>("Attempt to read null value");
}

void ConfigVisitor::AddOptionValue(OptionValue* opt) {
	if (!opt) {
		assert(ignore_errors);
		return;
	}

	if (!values.count(name)) {
		values[name] = opt;
		return;
	}

	// Ensure than opt is deleted at the end of this function even if the Set
	// method throws
	std::auto_ptr<OptionValue> auto_opt(opt);

	try {
		switch (opt->GetType()) {
			case OptionValue::Type_String:
				values[name]->SetString(opt->GetString());
				break;

			case OptionValue::Type_Int:
				values[name]->SetInt(opt->GetInt());
				break;

			case OptionValue::Type_Double:
				values[name]->SetDouble(opt->GetDouble());
				break;

			case OptionValue::Type_Colour:
				values[name]->SetColour(opt->GetColour());
				break;

			case OptionValue::Type_Bool:
				values[name]->SetBool(opt->GetBool());
				break;

			case OptionValue::Type_List_String:
				values[name]->SetListString(opt->GetListString());
				break;

			case OptionValue::Type_List_Int:
				values[name]->SetListInt(opt->GetListInt());
				break;

			case OptionValue::Type_List_Double:
				values[name]->SetListDouble(opt->GetListDouble());
				break;

			case OptionValue::Type_List_Colour:
				values[name]->SetListColour(opt->GetListColour());
				break;

			case OptionValue::Type_List_Bool:
				values[name]->SetListBool(opt->GetListBool());
				break;
		}
	}
	catch (agi::OptionValueError const& e) {
		if (ignore_errors)
			LOG_E("option/load/config_visitor") << "Error loading option from user configuration: " << e.GetChainedMessage();
		else
			throw;
	}
}
} // namespace agi
