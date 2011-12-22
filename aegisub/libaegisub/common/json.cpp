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

/// @file json.cpp
/// @brief Parse JSON files and return json::UnknownElement
/// @ingroup libaegisub io

#include "../config.h"

#ifndef LAGI_PRE
#include <fstream>
#include <sstream>
#endif

#include "libaegisub/access.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"
#include "libaegisub/log.h"
#include "libaegisub/scoped_ptr.h"


namespace agi {
	namespace json_util {

json::UnknownElement parse(std::istream *stream) {
	try {
		agi::scoped_ptr<std::istream> stream_deleter(stream);

		json::UnknownElement root;
		json::Reader::Read(root, *stream);
		return root;
	} catch (json::Reader::ParseException& e) {
		LOG_E("json/parse") << "json::ParseException: " << e.what() << ", Line/offset: " << e.m_locTokenBegin.m_nLine + 1 << '/' << e.m_locTokenBegin.m_nLineOffset + 1;
		throw;
	} catch (json::Exception& e) {
		LOG_E("json/parse") << "json::Exception: " << e.what();
		throw;
	}
}

json::UnknownElement file(const std::string &file) {
	return parse(io::Open(file));
}

json::UnknownElement file(const std::string &file, const std::string &default_config) {
	try {
		return parse(io::Open(file));
	}
	catch (const acs::AcsNotFound&) {
		// Not an error
		return parse(new std::istringstream(default_config));
	}
	catch (json::Exception&) {
		// Already logged in parse
		return parse(new std::istringstream(default_config));
	}
	catch (agi::Exception& e) {
		LOG_E("json/file") << "Unexpted error when reading config file " << file << ": " << e.GetMessage();
		return parse(new std::istringstream(default_config));
	}
}

	} // namespace json_util
} // namespace agi
