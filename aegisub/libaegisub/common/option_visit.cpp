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
		const std::string &member_name = index->first;
		const json::UnknownElement& element = index->second;

		ConfigVisitor config_visitor(values, name + member_name);

		element.Accept(config_visitor);
	}
}

void ConfigVisitor::Visit(const json::Array& array) {
	OptionValueList *array_list = NULL;

	json::Array::const_iterator index(array.begin()), indexEnd(array.end());

	for (; index != indexEnd; ++index) {
		const json::Object& index_array = *index;

		json::Object::const_iterator it(index_array.begin()), index_objectEnd(index_array.end());

		for (; it != index_objectEnd; ++it) {
			const std::string& member_name = it->first;

			// This can only happen once since a list must always be of the same
			// type, if we try inserting another type into it we want it to fail.
			if (!array_list) {
				if (member_name == "string") {
					array_list = new OptionValueListString(name);
				} else if (member_name == "int") {
					array_list = new OptionValueListInt(name);
				} else if (member_name == "double") {
					array_list = new OptionValueListDouble(name);
				} else if (member_name == "bool") {
					array_list = new OptionValueListBool(name);
				} else if (member_name == "colour") {
					array_list = new OptionValueListColour(name);
				} else {
					throw OptionJsonValueArray("Array type not handled");
				}
			}

			try {
				if (member_name == "string")
					array_list->InsertString((json::String)it->second);
				else if (member_name == "int")
					array_list->InsertInt((int64_t)(json::Number)it->second);
				else if (member_name == "double")
					array_list->InsertDouble((json::Number)it->second);
				else if (member_name == "bool")
					array_list->InsertBool((json::Boolean)it->second);
				else if (member_name == "colour")
					array_list->InsertColour((std::string)(json::String)it->second);
			} catch (agi::Exception&) {
				delete array_list;
				throw OptionJsonValueArray("Attempt to insert value into array of wrong type");
			}
		} // for index_object
	} // for index

	if (array_list) AddOptionValue(array_list);
}


void ConfigVisitor::Visit(const json::Number& number) {
	if (int64_t(number) == ceil(number)) {
		OptionValue *opt = new OptionValueInt(name, int64_t(number));
		AddOptionValue(opt);
	} else {
		OptionValue *opt = new OptionValueDouble(name, number);
		AddOptionValue(opt);
	}
}

void ConfigVisitor::Visit(const json::String& string) {
	OptionValue *opt;
	if (string.find("rgb(") == 0) {
		opt = new OptionValueColour(name, string);
	} else {
		opt = new OptionValueString(name, string);
	}
	AddOptionValue(opt);
}

void ConfigVisitor::Visit(const json::Boolean& boolean) {
	OptionValue *opt = new OptionValueBool(name, boolean);
	AddOptionValue(opt);
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

		case OptionValue::Type_List_String: {
			std::vector<std::string> array;
 			opt->GetListString(array);
			opt_cur->SetListString(array);
			break;
		}

		case OptionValue::Type_List_Int: {
			std::vector<int64_t> array;
 			opt->GetListInt(array);
			opt_cur->SetListInt(array);
			break;
		}

		case OptionValue::Type_List_Double: {
			std::vector<double> array;
 			opt->GetListDouble(array);
			opt_cur->SetListDouble(array);
			break;
		}

		case OptionValue::Type_List_Colour: {
			std::vector<Colour> array;
 			opt->GetListColour(array);
			opt_cur->SetListColour(array);
			break;
		}

		case OptionValue::Type_List_Bool: {
			std::vector<bool> array;
 			opt->GetListBool(array);
			opt_cur->SetListBool(array);
			break;
		}
	}
}
} // namespace agi
