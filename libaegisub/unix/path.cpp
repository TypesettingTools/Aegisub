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
#include <libaegisub/exception.h>

#include <pwd.h>
#include <stdlib.h>

namespace sfs = std::filesystem;

namespace {
std::string home_dir() {
	const char *env = getenv("HOME");
	if (env) return env;

	if ((env = getenv("USER")) || (env = getenv("LOGNAME"))) {
		if (passwd *user_info = getpwnam(env))
			return user_info->pw_dir;
	}

	throw agi::EnvironmentError("Could not get home directory. Make sure HOME is set.");
}

#ifdef APPIMAGE_BUILD
sfs::path data_dir() {
	char *exe = realpath("/proc/self/exe", NULL);
	if (!exe) return "";

	sfs::path p = sfs::path(exe).parent_path();
	free(exe);

	if (p.filename() == "bin") {
		// assume unix prefix layout
		return p.parent_path()/"share";
	}

	return p;
}
#endif
}

namespace agi {
void Path::FillPlatformSpecificPaths() {
	sfs::path home = home_dir();
	SetToken("?user", home/".aegisub");
	SetToken("?local", home/".aegisub");

#ifdef APPIMAGE_BUILD
	sfs::path data = data_dir();
	if (data == "") data = home/".aegisub";
	SetToken("?data", data);
	SetToken("?dictionary", Decode("?data/dictionaries"));
#else
	SetToken("?data", P_DATA);
	SetToken("?dictionary", "/usr/share/hunspell");
#endif

	SetToken("?temp", sfs::temp_directory_path());
}
}
