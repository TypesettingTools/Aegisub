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

/// @file option.cpp
/// @brief Option interface.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <fstream>
#include <sstream>
#include <map>
#include <memory>

#include "libaegisub/cajun/reader.h"
#include "libaegisub/cajun/writer.h"
#include "libaegisub/cajun/elements.h"
#endif

#include "libaegisub/access.h"
#include "libaegisub/io.h"
#include "libaegisub/log.h"

#include "option_visit.h"

namespace agi {

Options::Options(const std::string &file, const std::string& default_config, const OptionSetting setting):
							config_file(file), config_default(default_config), config_loaded(false), setting(setting) {
	LOG_D("agi/options") << "New Options object";
	std::istringstream stream(default_config);
	LoadConfig(stream);
}

Options::~Options() {

	if ((setting & FLUSH_SKIP) != FLUSH_SKIP) {
		Flush();
	}

	for (OptionValueMap::iterator i = values.begin(); i != values.end(); i++) {
		delete i->second;
	}
}

void Options::ConfigNext(std::istream& stream) {
	LoadConfig(stream);
}

void Options::ConfigUser() {
	std::auto_ptr<std::istream> stream;

	try {
		stream.reset(agi::io::Open(config_file));
	} catch (const acs::AcsNotFound&) {
		return;
	}

	/// @todo Handle other errors such as parsing and notifying the user.
	LoadConfig(*stream);
	config_loaded = true;
}


void Options::LoadConfig(std::istream& stream) {
	/// @todo Store all previously loaded configs in an array for bug report purposes,
	///       this is just a temp stub.
	json::UnknownElement config_root;

	try {
		json::Reader::Read(config_root, stream);
	} catch (json::Reader::ParseException& e) {
		std::cout << "json::ParseException: " << e.what() << ", Line/offset: " << e.m_locTokenBegin.m_nLine + 1 << '/' << e.m_locTokenBegin.m_nLineOffset + 1 << std::endl << std::endl;
	} catch (json::Exception& e) {
		/// @todo Do something better here, maybe print the exact error
		std::cout << "json::Exception: " << e.what() << std::endl;
	}

	ConfigVisitor config_visitor(values, std::string(""));
	config_root.Accept(config_visitor);
}




OptionValue* Options::Get(const std::string &name) {

	OptionValueMap::iterator index;

	if ((index = values.find(name)) != values.end())
		return index->second;

	std::cout << "agi::Options::Get Option not found: (" << name << ")" << std::endl;
	throw OptionErrorNotFound("Option value not found");
}



void Options::Flush() {

	json::Object obj_out;

	bool ok;

	for (OptionValueMap::const_iterator i = values.begin(); i != values.end(); ++i) {

		std::string key = i->first.substr(i->first.rfind("/")+1, i->first.size());

		int type = i->second->GetType();

		switch (type) {
			case OptionValue::Type_String: {
				ok = PutOption(obj_out, i->first, (json::String)i->second->GetString());
			}
			break;

			case OptionValue::Type_Int:
				ok = PutOption(obj_out, i->first, (json::Number)(const double)i->second->GetInt());
			break;

			case OptionValue::Type_Double:
				ok = PutOption(obj_out, i->first, (json::Number)i->second->GetDouble());
			break;

			case OptionValue::Type_Colour: {
				std::string str = std::string(i->second->GetColour());
				ok = PutOption(obj_out, i->first, (json::String)str);
			}
			break;

			case OptionValue::Type_Bool:
				ok = PutOption(obj_out, i->first, (json::Boolean)i->second->GetBool());
			break;

			case OptionValue::Type_List_String: {
				std::vector<std::string> array_string;
				i->second->GetListString(array_string);

				json::Array array;

				for (std::vector<std::string>::const_iterator i_str = array_string.begin(); i_str != array_string.end(); ++i_str) {
					json::Object obj;
					obj["string"] = json::String(*i_str);
					array.Insert(obj);
				}

				ok = PutOption(obj_out, i->first, (json::Array)array);
			}
			break;

			case OptionValue::Type_List_Int: {
				std::vector<int64_t> array_int;
				i->second->GetListInt(array_int);

				json::Array array;

				for (std::vector<int64_t>::const_iterator i_int = array_int.begin(); i_int != array_int.end(); ++i_int) {
					json::Object obj;
					obj["int"] = json::Number((const double)*i_int);
					array.Insert(obj);
				}
				ok = PutOption(obj_out, i->first, (json::Array)array);
			}
			break;

			case OptionValue::Type_List_Double: {
				std::vector<double> array_double;
				i->second->GetListDouble(array_double);

				json::Array array;

				for (std::vector<double>::const_iterator i_double = array_double.begin(); i_double != array_double.end(); ++i_double) {
					json::Object obj;
					obj["double"] = json::Number(*i_double);
					array.Insert(obj);
				}
				ok = PutOption(obj_out, i->first, (json::Array)array);
			}
			break;

			case OptionValue::Type_List_Colour: {
				std::vector<Colour> array_colour;
				i->second->GetListColour(array_colour);

				json::Array array;
				for (std::vector<Colour>::const_iterator i_colour = array_colour.begin(); i_colour != array_colour.end(); ++i_colour) {
					json::Object obj;

					Colour col = *i_colour;
					std::string str = std::string(col);

					obj["colour"] = json::String(str);

					array.Insert(obj);
				}
				ok = PutOption(obj_out, i->first, (json::Array)array);
			}
			break;

			case OptionValue::Type_List_Bool: {
				std::vector<bool> array_bool;

				json::Array array;

				i->second->GetListBool(array_bool);
				for (std::vector<bool>::const_iterator i_bool = array_bool.begin(); i_bool != array_bool.end(); ++i_bool) {
					json::Object obj;
					obj["bool"] = json::Boolean(*i_bool);
					array.Insert(obj);
				}
				ok = PutOption(obj_out, i->first, (json::Array)array);
			}
			break;
		}
	}

	io::Save file(config_file);
	json::Writer::Write(obj_out, file.Get());
}


bool Options::PutOption(json::Object &obj, const std::string &path, const json::UnknownElement &value) {
	// Having a '/' denotes it is a leaf.
	if (path.find('/') == std::string::npos) {
		json::Object::iterator pos = obj.Find(path);

		// Fail if a key of the same name already exists.
		if (pos != obj.End())
			throw OptionErrorDuplicateKey("Key already exists");

		obj.Insert(json::Object::Member(path, value));
		return true;
	} else {
		std::string thispart = path.substr(0, path.find("/"));
		std::string restpart = path.substr(path.find("/")+1, path.size());
		json::Object::iterator pos = obj.Find(thispart);

		// New key, make object.
		if (pos == obj.End())
			pos = obj.Insert(json::Object::Member(thispart, json::Object()));

		PutOptionVisitor visitor(restpart, value);
		pos->element.Accept(visitor);
		return visitor.result;
	}
}

} // namespace agi
