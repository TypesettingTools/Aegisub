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

/// @file option.cpp
/// @brief Option interface.
/// @ingroup libaegisub

#include "libaegisub/option.h"

#include "libaegisub/cajun/reader.h"
#include "libaegisub/cajun/writer.h"
#include "libaegisub/cajun/elements.h"
#include "libaegisub/cajun/visitor.h"

#include "libaegisub/exception.h"
#include "libaegisub/fs.h"
#include "libaegisub/io.h"
#include "libaegisub/log.h"
#include "libaegisub/option_value.h"
#include "libaegisub/make_unique.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <cassert>
#include <memory>

namespace {
using namespace agi;

DEFINE_EXCEPTION(OptionJsonValueError, Exception);

class ConfigVisitor final : public json::ConstVisitor {
	std::vector<std::unique_ptr<OptionValue>> values;

	/// Option name prefix to add to read names
	std::string name;

	/// Log errors rather than throwing them, for when loading user config files
	/// (as a bad user config file shouldn't make the program fail to start)
	bool ignore_errors;

	void Error(const char *message) {
		if (ignore_errors)
			LOG_E("option/load/config_visitor") << "Error loading option from user configuration: " << message;
		else
			throw OptionJsonValueError(message);
	}

	template<class OptionValueType>
	void ReadArray(json::Array const& src, std::string const& array_type) {
		typename OptionValueType::value_type arr;
		arr.reserve(src.size());

		for (json::Object const& obj : src) {
			if (obj.size() != 1)
				return Error("Invalid array member");
			if (obj.begin()->first != array_type)
				return Error("Attempt to insert value into array of wrong type");

			arr.push_back((typename OptionValueType::value_type::value_type)(obj.begin()->second));
		}

		values.push_back(agi::make_unique<OptionValueType>(name, std::move(arr)));
	}

	void Visit(const json::Object& object) {
		auto old_name = name;
		for (auto const& obj : object) {
			name = old_name + (name.empty() ? "" : "/") + obj.first;
			obj.second.Accept(*this);
		}
		name = old_name;
	}

	void Visit(const json::Array& array) {
		if (array.empty())
			return Error("Cannot infer the type of an empty array");

		json::Object const& front = array.front();
		if (front.size() != 1)
			return Error("Invalid array member");

		auto const& array_type = front.begin()->first;
		if (array_type == "string")
			ReadArray<OptionValueListString>(array, array_type);
		else if (array_type == "int")
			ReadArray<OptionValueListInt>(array, array_type);
		else if (array_type == "double")
			ReadArray<OptionValueListDouble>(array, array_type);
		else if (array_type == "bool")
			ReadArray<OptionValueListBool>(array, array_type);
		else if (array_type == "color")
			ReadArray<OptionValueListColor>(array, array_type);
		else
			Error("Array type not handled");
	}

	void Visit(int64_t number) {
		values.push_back(agi::make_unique<OptionValueInt>(name, number));
	}

	void Visit(double number) {
		values.push_back(agi::make_unique<OptionValueDouble>(name, number));
	}

	void Visit(const json::String& string) {
		size_t size = string.size();
		if ((size == 4 && string[0] == '#') ||
			(size == 7 && string[0] == '#') ||
			(size >= 10 && boost::starts_with(string, "rgb(")) ||
			((size == 9 || size == 10) && boost::starts_with(string, "&H")))
		{
			values.push_back(agi::make_unique<OptionValueColor>(name, string));
		} else {
			values.push_back(agi::make_unique<OptionValueString>(name, string));
		}
	}

	void Visit(bool boolean) {
		values.push_back(agi::make_unique<OptionValueBool>(name, boolean));
	}

	void Visit(const json::Null& null) {
		Error("Attempt to read null value");
	}

public:
	ConfigVisitor(bool ignore_errors) : ignore_errors(ignore_errors) { }
	std::vector<std::unique_ptr<OptionValue>> Values() { return std::move(values); }
};

/// @brief Write an option to a json object
/// @param[out] obj  Parent object
/// @param[in] path  Path option should be stored in.
/// @param[in] value Value to write.
void put_option(json::Object &obj, const std::string &path, const json::UnknownElement &value) {
	std::string::size_type pos = path.find('/');
	// Not having a '/' denotes it is a leaf.
	if (pos == std::string::npos) {
		assert(obj.find(path) == obj.end());
		obj[path] = value;
	}
	else
		put_option(obj[path.substr(0, pos)], path.substr(pos + 1), value);
}

template<class T>
void put_array(json::Object &obj, const std::string &path, const char *element_key, std::vector<T> const& value) {
	json::Array array;
	for (T const& elem : value) {
		array.push_back(json::Object());
		static_cast<json::Object&>(array.back())[element_key] = (json::UnknownElement)elem;
	}

	put_option(obj, path, array);
}

struct option_name_cmp {
	bool operator()(std::unique_ptr<OptionValue> const& a, std::unique_ptr<OptionValue> const& b) const {
		return a->GetName() < b->GetName();
	}

	bool operator()(std::unique_ptr<OptionValue> const& a, std::string const& b) const {
		return a->GetName() < b;
	}

	bool operator()(std::unique_ptr<OptionValue> const& a, const char *b) const {
		return a->GetName() < b;
	}
};

}

namespace agi {

Options::Options(agi::fs::path const& file, std::pair<const char *, size_t> default_config, const OptionSetting setting)
: config_file(file)
, setting(setting)
{
	LOG_D("agi/options") << "New Options object";
	boost::interprocess::ibufferstream stream(default_config.first, default_config.second);
	LoadConfig(stream);
}

Options::~Options() {
	if ((setting & FLUSH_SKIP) != FLUSH_SKIP)
		Flush();
}

void Options::ConfigUser() {
	try {
		LoadConfig(*io::Open(config_file), true);
	}
	catch (fs::FileNotFound const&) {
		return;
	}
}

void Options::LoadConfig(std::istream& stream, bool ignore_errors) {
	json::UnknownElement config_root;

	try {
		json::Reader::Read(config_root, stream);
	} catch (json::Reader::ParseException& e) {
		LOG_E("option/load") << "json::ParseException: " << e.what() << ", Line/offset: " << e.m_locTokenBegin.m_nLine + 1 << '/' << e.m_locTokenBegin.m_nLineOffset + 1;
		return;
	} catch (json::Exception& e) {
		LOG_E("option/load") << "json::Exception: " << e.what();
		return;
	}

	ConfigVisitor config_visitor(ignore_errors);
	config_root.Accept(config_visitor);

	auto new_values = config_visitor.Values();
	if (new_values.empty()) return;

	sort(begin(new_values), end(new_values), option_name_cmp());

	if (values.empty()) {
		values = std::move(new_values);
		return;
	}

	auto src_it = begin(new_values), src_end = end(new_values);
	auto dst_it = begin(values), dst_end = end(values);

	while (src_it != src_end && dst_it != dst_end) {
		int cmp = (*src_it)->GetName().compare((*dst_it)->GetName());
		if (cmp < 0) // Option doesn't exist in defaults so skip
			++src_it;
		else if (cmp > 0)
			++dst_it;
		else {
			if (ignore_errors) {
				try {
					(*dst_it)->Set((*src_it).get());
				}
				catch (agi::InternalError const& e) {
					LOG_E("option/load/config_visitor") << "Error loading option from user configuration: " << e.GetMessage();
				}
			}
			else {
				*dst_it = std::move(*src_it);
			}
			++src_it;
			++dst_it;
		}
	}
}

OptionValue *Options::Get(const char *name) {
	auto index = lower_bound(begin(values), end(values), name, option_name_cmp());
	if (index != end(values) && (*index)->GetName() == name)
		return index->get();

	LOG_E("option/get") << "agi::Options::Get Option not found: (" << name << ")";
	throw agi::InternalError("Option value not found: " + std::string(name));
}

void Options::Flush() const {
	json::Object obj_out;

	for (auto const& ov : values) {
		switch (ov->GetType()) {
			case OptionType::String:
				put_option(obj_out, ov->GetName(), ov->GetString());
				break;

			case OptionType::Int:
				put_option(obj_out, ov->GetName(), ov->GetInt());
				break;

			case OptionType::Double:
				put_option(obj_out, ov->GetName(), ov->GetDouble());
				break;

			case OptionType::Color:
				put_option(obj_out, ov->GetName(), ov->GetColor().GetRgbFormatted());
				break;

			case OptionType::Bool:
				put_option(obj_out, ov->GetName(), ov->GetBool());
				break;

			case OptionType::ListString:
				put_array(obj_out, ov->GetName(), "string", ov->GetListString());
				break;

			case OptionType::ListInt:
				put_array(obj_out, ov->GetName(), "int", ov->GetListInt());
				break;

			case OptionType::ListDouble:
				put_array(obj_out, ov->GetName(), "double", ov->GetListDouble());
				break;

			case OptionType::ListColor:
				put_array(obj_out, ov->GetName(), "color", ov->GetListColor());
				break;

			case OptionType::ListBool:
				put_array(obj_out, ov->GetName(), "bool", ov->GetListBool());
				break;
		}
	}

	agi::JsonWriter::Write(obj_out, io::Save(config_file).Get());
}

} // namespace agi
