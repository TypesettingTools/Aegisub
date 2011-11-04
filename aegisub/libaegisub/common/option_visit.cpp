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

#ifndef LAGI_PRE
#include <cmath>
#include <memory>
#endif

#include <libaegisub/colour.h>
#include <libaegisub/option_value.h>
#include "option_visit.h"

namespace agi {

ConfigVisitor::ConfigVisitor(OptionValueMap &val, const std::string &member_name)
: values(val)
, name(member_name)
{
}

void ConfigVisitor::Visit(const json::Object& object) {
	json::Object::const_iterator index(object.begin()), index_end(object.end());

	if (!name.empty())
		name += "/";

	for (; index != index_end; ++index) {
		ConfigVisitor config_visitor(values, name + index->first);
		index->second.Accept(config_visitor);
	}
}

template<class T>
static inline T convert_unknown(json::UnknownElement const& ue) {
	return ue;
}

template<>
inline int64_t convert_unknown(json::UnknownElement const& ue) {
	return (int64_t)(double)ue;
}

template<class OptionValueType, class ValueType>
static OptionValue *read_array(json::Array const& src, std::string const& array_type, std::string const& name, void (OptionValueType::*set_list)(const std::vector<ValueType>&)) {
	std::vector<ValueType> arr;
	arr.reserve(src.size());

	for (json::Array::const_iterator it = src.begin(); it != src.end(); ++it) {
		json::Object const& obj = *it;

		if (obj.size() != 1)
			throw OptionJsonValueArray("Invalid array member");
		if (obj.begin()->first != array_type)
			throw OptionJsonValueArray("Attempt to insert value into array of wrong type");

		arr.push_back(convert_unknown<ValueType>(obj.begin()->second));
	}

	OptionValueType *ret = new OptionValueType(name);
	(ret->*set_list)(arr);
	return ret;
}

void ConfigVisitor::Visit(const json::Array& array) {
	if (array.empty())
		throw OptionJsonValueArray("Cannot infer the type of an empty array");

	json::Object const& front = array.front();
	if (front.size() != 1)
		throw OptionJsonValueArray("Invalid array member");

	const std::string& array_type = front.begin()->first;

	if (array_type == "string")
		AddOptionValue(read_array(array, array_type, name, &OptionValueListString::SetListString));
	else if (array_type == "int")
		AddOptionValue(read_array(array, array_type, name, &OptionValueListInt::SetListInt));
	else if (array_type == "double")
		AddOptionValue(read_array(array, array_type, name, &OptionValueListDouble::SetListDouble));
	else if (array_type == "bool")
		AddOptionValue(read_array(array, array_type, name, &OptionValueListBool::SetListBool));
	else if (array_type == "colour")
		AddOptionValue(read_array(array, array_type, name, &OptionValueListColour::SetListColour));
	else
		throw OptionJsonValueArray("Array type not handled");
}


void ConfigVisitor::Visit(const json::Number& number) {
	if (int64_t(number) == ceil(number)) {
		AddOptionValue(new OptionValueInt(name, int64_t(number)));
	} else {
		AddOptionValue(new OptionValueDouble(name, number));
	}
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
	throw OptionJsonValueNull("Attempt to read null value");
}

void ConfigVisitor::AddOptionValue(OptionValue* opt) {
	OptionValue *opt_cur;

	OptionValueMap::iterator index;

	if ((index = values.find(name)) != values.end()) {
		opt_cur = index->second;
	} else {
		values.insert(OptionValuePair(name, opt));
		return;
	}

	// Ensure than opt is deleted at the end of this function even if the Set
	// method throws
	std::auto_ptr<OptionValue> auto_opt(opt);

	int type = opt_cur->GetType();
	switch (type) {
		case OptionValue::Type_String:
			opt_cur->SetString(opt->GetString());
			break;

		case OptionValue::Type_Int:
			opt_cur->SetInt(opt->GetInt());
			break;

		case OptionValue::Type_Double:
			opt_cur->SetDouble(opt->GetDouble());
			break;

		case OptionValue::Type_Colour:
			opt_cur->SetColour(opt->GetColour());
			break;

		case OptionValue::Type_Bool:
			opt_cur->SetBool(opt->GetBool());
			break;

		case OptionValue::Type_List_String:
			opt_cur->SetListString(opt->GetListString());
			break;

		case OptionValue::Type_List_Int:
			opt_cur->SetListInt(opt->GetListInt());
			break;

		case OptionValue::Type_List_Double:
			opt_cur->SetListDouble(opt->GetListDouble());
			break;

		case OptionValue::Type_List_Colour:
			opt_cur->SetListColour(opt->GetListColour());
			break;

		case OptionValue::Type_List_Bool:
			opt_cur->SetListBool(opt->GetListBool());
			break;
	}
}
} // namespace agi
