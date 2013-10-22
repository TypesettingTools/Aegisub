// Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

#include <libaegisub/path.h>

#include <libaegisub/util_osx.h>

#include <boost/filesystem.hpp>
#include <pwd.h>

namespace {
#ifndef __APPLE__
std::string home_dir() {
	const char *env = getenv("HOME");
	if (env) return env;

	if ((env = getenv("USER")) || (env = getenv("LOGNAME"))) {
		if (passwd *user_info = getpwnam(env))
			return user_info->pw_dir;
	}

	throw agi::EnvironmentError("Could not get home directory. Make sure HOME is set.");
}
#endif
}

namespace agi {

void Path::FillPlatformSpecificPaths() {
#ifndef __APPLE__
	agi::fs::path home = home_dir();
	SetToken("?user", home/".aegisub");
	SetToken("?local", home/".aegisub");
	SetToken("?data", P_DATA);
#else
	agi::fs::path app_support = agi::util::OSX_GetApplicationSupportDirectory();
	SetToken("?user", app_support/"Aegisub");
	SetToken("?local", app_support/"Aegisub");
	SetToken("?data", agi::util::OSX_GetBundleSharedSupportDirectory());
#endif
	SetToken("?temp", boost::filesystem::temp_directory_path());
	SetToken("?dictionary", "/usr/share/hunspell");
	SetToken("?docs", P_DOC);
}

}
