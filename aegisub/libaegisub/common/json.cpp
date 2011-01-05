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


#ifdef LAGI_PRE
#include <fstream>
#include <strstream>
#endif

#include "libaegisub/access.h"
#include "libaegisub/io.h"
#include "libaegisub/json.h"


namespace agi {
	namespace json_util {

json::UnknownElement parse(std::istream *stream) {
	json::UnknownElement root;

	try {
		json::Reader::Read(root, *stream);
	} catch (json::Reader::ParseException& e) {
		std::cout << "json::ParseException: " << e.what() << ", Line/offset: " << e.m_locTokenBegin.m_nLine + 1 << '/' << e.m_locTokenBegin.m_nLineOffset + 1 << std::endl << std::endl;
	} catch (json::Exception& e) {
		/// @todo Do something better here, maybe print the exact error
		std::cout << "json::Exception: " << e.what() << std::endl;
	}

	delete stream;
	return root;
}


json::UnknownElement file(const std::string file) {
	return parse(io::Open(file));
}


json::UnknownElement file(const std::string file, const std::string &default_config) {

	try {
		return parse(io::Open(file));

	// We only want to catch this single error as anything else could
	// reflect a deeper problem.  ie, failed i/o, wrong permissions etc.
	} catch (const acs::AcsNotFound&) {

		std::istringstream stream(default_config);
		return parse(&stream);
	}
}


	} // namespace json_util
} // namespace agi
