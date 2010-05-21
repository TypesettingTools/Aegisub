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

/// @file option_visit.h
/// @brief Cajun JSON visitor to load config values.
/// @see option_visit.cpp
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include "libaegisub/cajun/elements.h"
#include "libaegisub/cajun/visitor.h"
#endif

#pragma once

#include "libaegisub/option.h"

namespace agi {

DEFINE_BASE_EXCEPTION_NOINNER(OptionJsonValueError, Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionJsonValueArray, OptionJsonValueError, "options/value/array")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionJsonValueSingle, OptionJsonValueError, "options/value")
DEFINE_SIMPLE_EXCEPTION_NOINNER(OptionJsonValueNull, OptionJsonValueError, "options/value")

class ConfigVisitor : public json::ConstVisitor {

	OptionValueMap &values;
	std::string name;
	typedef std::pair<std::string, OptionValue*> OptionValuePair;

	void AddOptionValue(OptionValue* opt);

public:

	ConfigVisitor(OptionValueMap &val, const std::string &member_name);
	~ConfigVisitor();

	void Visit(const json::Array& array);
	void Visit(const json::Object& object);
	void Visit(const json::Number& number);
	void Visit(const json::String& string);
	void Visit(const json::Boolean& boolean);
	void Visit(const json::Null& null);
};


class PutOptionVisitor : public json::Visitor {
public:
	bool result;
	const std::string &path;
	const json::UnknownElement &value;

	PutOptionVisitor(const std::string &path, const json::UnknownElement &value)
	: result(false), path(path), value(value)
	{}

	// all of these are a fail
	virtual void Visit(json::Array& array) { }
	virtual void Visit(json::Number& number) { }
	virtual void Visit(json::String& string) { }
	virtual void Visit(json::Boolean& boolean) { }
	virtual void Visit(json::Null& null) { }

	// this one is the win
	virtual void Visit(json::Object& object) {
		result = Options::PutOption(object, path, value);
	}
};

} // namespace agi
