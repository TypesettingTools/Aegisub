// Copyright (c) 2010-2011, Amar Takhar <verm@aegisub.org>
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

/// @file path.cpp
/// @brief Common paths.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <vector>
#endif

#include "libaegisub/access.h"
#include "libaegisub/log.h"
#include "libaegisub/path.h"
#include "libaegisub/option.h"

namespace agi {

Path::Path(const std::string &file, const std::string& default_path)
: path_file(file),
path_default(default_path) {
	opt = new agi::Options(file, default_path);
	opt->ConfigUser();
	LOG_D("agi/path") << "New Path object";
}

Path::~Path() {
	opt->Flush();
}


std::string Path::Get(const char *name) {
	std::string path;
	try {
		path = std::string(opt->Get(name)->GetString());
	} catch (OptionErrorNotFound&) {
		throw PathErrorNotFound("Invalid path key");
	}

	Decode(path);
	return path;
}


void Path::Set(const char *name, const std::string &path) {
	std::string set(path);

	if (path[0] == 94) {
		std::string tmp(path);
		// Check that the used cookie exists.
		Decode(tmp);
		agi::acs::CheckDirWrite(path);
	}

	try {
		opt->Get(name)->SetString(set);
	} catch (OptionErrorNotFound&) {
		throw PathErrorNotFound("Invalid path key");
	}

}


void Path::ListGet(const char *name, std::vector<std::string> &out) {
	opt->Get(name)->GetListString(out);
}


void Path::ListSet(const char *name, std::vector<std::string> list) {
	opt->Get(name)->SetListString(list);
}


void Path::Decode(std::string &path) {
	if (path[0] != 94) // "^"
		return;
	try {
		if (path.find("^CONFIG") == 0) {
			path.replace(0, 7, Config());
			return;
		}

		if (path.find("^USER") == 0) {
			path.replace(0, 5, User());
			return;
		}

		if (path.find("^DATA") == 0) {
			path.replace(0, 5, Data());
			return;
		}

		if (path.find("^DOC") == 0) {
			path.replace(0, 4, Doc());
			return;
		}

		if (path.find("^TEMP") == 0) {
			path.replace(0, 5, Temp());
			return;
		}

		if (path.find("^AUDIO") == 0) {
			std::string path_str(opt->Get("Last/Audio")->GetString());
			Decode(path_str);
			path.replace(0, 6, path_str);
			return;
		}

		if (path.find("^VIDEO") == 0) {
			std::string path_str(opt->Get("Last/Video")->GetString());
			Decode(path_str);
			path.replace(0, 6, path_str);
			return;
		}

		if (path.find("^SUBTITLE") == 0) {
			std::string path_str(opt->Get("Last/Subtitle")->GetString());
			Decode(path_str);
			path.replace(0, 5, path_str);
			return;
		}

		throw PathErrorInvalid("Invalid cookie used");

	} catch (OptionErrorNotFound&) {
		throw PathErrorInternal("Failed to find key in Decode");
	}
}

void Path::Encode(std::string &path) {
}


} // namespace agi
