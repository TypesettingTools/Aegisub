// Copyright (c) 2020, Myaamori <myaamori1993@gmail.com>
// Copyright (c) 2023, arch1t3cht <arch1t3cht@gmail.com>
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

#include "cli.h"

#include <libaegisub/ass/string_codec.h>
#include <libaegisub/cajun/elements.h>
#include <libaegisub/color.h>
#include <libaegisub/exception.h>
#include <libaegisub/json.h>
#include <libaegisub/log.h>
#include <libaegisub/format.h>
#include <libaegisub/split.h>
#include <libaegisub/util.h>

#include <filesystem>
#include <sstream>

std::set<int> parse_range(const std::string& s) {
	std::set<int> lines;
	for (auto tok : agi::Split(s, ',')) {
		auto item = agi::str(tok);
		int i;

		if (agi::util::try_parse(item, &i)) {
			lines.insert(i);
		} else {
			auto sep = item.find('-');
			if (sep == std::string::npos) {
				throw agi::InvalidInputException("Invalid number or range: " + item);
			}

			if (!agi::util::try_parse(item.substr(0, sep), &i)) {
				throw agi::InvalidInputException("Invalid start number in range: " + item);
			}

			int j;
			if (!agi::util::try_parse(item.substr(sep + 1), &j)) {
				throw agi::InvalidInputException("Invalid end number in range: " + item);
			}

			for (int k = i; k <= j; k++) {
				lines.insert(k);
			}
		}
	}
	return lines;
}

std::string serialize_element(const json::UnknownElement& elem) {
	try {
		auto& val_str = static_cast<json::String const&>(elem);
		return agi::ass::inline_string_encode(val_str);
	}
	catch (json::Exception const& e) {}

	try {
		auto& val_int = static_cast<json::Integer const&>(elem);
		return std::to_string(val_int);
	}
	catch (json::Exception const& e) {}

	try {
		auto& val_double = static_cast<json::Double const&>(elem);
		return std::to_string(val_double);
	}
	catch (json::Exception const& e) {}

	try {
		auto& val_bool = static_cast<json::Boolean const&>(elem);
		return val_bool ? "1" : "0";
	} catch (json::Exception const& e) {}

	auto& val_array = static_cast<json::Array const&>(elem);
	if (val_array.size() != 3 && val_array.size() != 4) {
		LOG_E("main/cli") << "Wrong array length for color";
		throw json::Exception("Wrong array length for color");
	}

	agi::Color color;
	for (int i = 0; i < val_array.size(); i++) {
		int arrelem_int = static_cast<json::Integer const&>(val_array[i]);
		if (arrelem_int < 0 || arrelem_int >= 256) {
			LOG_E("main/cli") << "Color part not within [0,255] bounds";
			throw json::Exception("Color part not within [0,255] bounds");
		}

		switch (i) {
			case 0: color.r = arrelem_int; break;
			case 1: color.g = arrelem_int; break;
			case 2: color.b = arrelem_int; break;
			case 3: color.a = arrelem_int; break;
		}
	}
	return val_array.size() == 4 ? color.GetAssStyleFormatted() : color.GetAssOverrideFormatted();
}

std::pair<int, std::string> json_to_lua(const std::string& s) {
	std::istringstream stream(s);
	auto elem = agi::json_util::parse(stream);
	auto& root = static_cast<json::Object const&>(elem);
	auto button = root.find("button");
	if (button == root.end()) {
		throw agi::InvalidInputException("No button specified in JSON: " + s);
	}

	const json::Integer* button_num = nullptr;
	try {
		button_num = &static_cast<json::Integer const&>(button->second);
	}
	catch (json::Exception const& e) {
		throw agi::InvalidInputException("'button' not an integer in JSON: " + s);
	}

	auto values = root.find("values");
	if (values == root.end()) {
		return std::make_pair(*button_num, "");
	}

	const json::Object* values_obj = nullptr;
	try {
		values_obj = &static_cast<json::Object const&>(values->second);
	}
	catch (json::Exception const& e) {
		throw agi::InvalidInputException("'values' not an object in JSON: " + s);
	}

	std::string vals = "";
	for (auto& p : *values_obj) {
		if (!vals.empty()) {
			vals += "|";
		}

		auto& key = p.first;
		try {
			vals += agi::ass::inline_string_encode(key) + ":" + serialize_element(p.second);
		}
		catch (json::Exception const& e) {
			throw agi::InvalidInputException(agi::format("Could not serialize 'values.%s' in JSON: %s", key, s));
		}
	}

	return std::make_pair(*button_num, vals);
}

std::list<std::pair<int, std::string>> parse_dialog_responses(const std::vector<std::string>& args) {
	std::list<std::pair<int, std::string>> responses;
	for (auto& s : args) {
		auto pair = json_to_lua(s);
		LOG_D("main/cli") << agi::format("Dialog response: button %d -> %s", pair.first, pair.second);
		responses.push_back(std::move(pair));
	}
	return responses;
}

std::list<std::vector<agi::fs::path>> parse_file_responses(const std::vector<std::string>& args) {
	std::list<std::vector<agi::fs::path>> responses;
	for (auto& s : args) {
		std::vector<agi::fs::path> paths;
		std::stringstream ss;
		for (const auto& tok : agi::Split(s, '|')) {
			agi::fs::path p = std::filesystem::absolute(agi::str(tok));
			ss << p << " ";
			paths.push_back(std::move(p));
		}
		LOG_E("main/cli") << "File response: " << ss.str();
		responses.push_back(std::move(paths));
	}
	return responses;
}
