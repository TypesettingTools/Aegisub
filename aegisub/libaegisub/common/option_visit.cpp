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
#include <math.h>

#include <memory>
#endif

#include <libaegisub/colour.h>
#include <libaegisub/option_value.h>
#include "option_visit.h"

namespace agi {

ConfigVisitor::ConfigVisitor(OptionValueMap &val, const std::string &member_name)
: values(val)
, name(member_name + "/")
{
}

void ConfigVisitor::Visit(const json::Object& object) {
	json::Object::const_iterator index(object.Begin()), index_end(object.End());

	for (; index != index_end; ++index) {
		const json::Object::Member& member = *index;
		const std::string &member_name = member.name;
		const json::UnknownElement& element = member.element;

		ConfigVisitor config_visitor(values, name + member_name);

		element.Accept(config_visitor);
	}
}


void ConfigVisitor::Visit(const json::Array& array) {
	bool init = false;

	OptionValueList *array_list = NULL;

	json::Array::const_iterator index(array.Begin()), indexEnd(array.End());

	for (; index != indexEnd; ++index) {

		const json::Object& index_array = *index;

		json::Object::const_iterator index_object(index_array.Begin()), index_objectEnd(index_array.End());

		for (; index_object != index_objectEnd; ++index_object) {
			const json::Object::Member& member = *index_object;
			const std::string& member_name = member.name;


			// This can only happen once since a list must always be of the same
			// type, if we try inserting another type into it we want it to fail.
			if (!init) {
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
				init = true;
			}

			try {
				if (member_name == "string") {
					std::string val = (json::String)member.element;
					array_list->InsertString(val);

				} else if (member_name == "int") {
					int64_t val = (int64_t)(json::Number)member.element;
					array_list->InsertInt(val);

				} else if (member_name == "double") {
					double val = (json::Number)member.element;
					array_list->InsertDouble(val);

				} else if (member_name == "bool") {
					bool val = (json::Boolean)member.element;
					array_list->InsertBool(val);

				} else if (member_name == "colour") {
					std::string val = (json::String)member.element;
					Colour col(val);
					array_list->InsertColour(col);
				}
			} catch (agi::Exception&) {
				delete array_list;
				throw OptionJsonValueArray("Attempt to insert value into array of wrong type");
			}

		} // for index_object

	} // for index

	if (array_list) AddOptionValue(array_list);

}


void ConfigVisitor::Visit(const json::Number& number) {
	double val = number.Value();

	if (int64_t(val) == ceil(val)) {
		OptionValue *opt = new OptionValueInt(name, int64_t(val));
		AddOptionValue(opt);
	} else {
		OptionValue *opt = new OptionValueDouble(name, val);
		AddOptionValue(opt);
	}

}


void ConfigVisitor::Visit(const json::String& string) {
	OptionValue *opt;
	if (string.Value().find("rgb(") == 0) {
		opt = new OptionValueColour(name, string.Value());
	} else {
		opt = new OptionValueString(name, string.Value());
	}
	AddOptionValue(opt);
}


void ConfigVisitor::Visit(const json::Boolean& boolean) {
	OptionValue *opt = new OptionValueBool(name, boolean.Value());
	AddOptionValue(opt);
}


void ConfigVisitor::Visit(const json::Null& null) {
	throw OptionJsonValueNull("Attempt to read null value");
}


void ConfigVisitor::AddOptionValue(OptionValue* opt) {
	// Corresponding code is in the constuctor.
	std::string stripped = name.substr(1, name.rfind("/")-1);
	OptionValue *opt_cur;

	OptionValueMap::iterator index;

	if ((index = values.find(stripped)) != values.end()) {
		opt_cur = index->second;
	} else {
		values.insert(OptionValuePair(stripped, opt));
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
