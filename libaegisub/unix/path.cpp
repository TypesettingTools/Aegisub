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
agi::fs::path home_dir() {
	const char *env = getenv("HOME");
	if (env) return agi::fs::path(sfs::path(env));

	if ((env = getenv("USER")) || (env = getenv("LOGNAME"))) {
		if (passwd *user_info = getpwnam(env))
			return agi::fs::path(sfs::path(user_info->pw_dir));
	}

	throw agi::EnvironmentError("Could not get home directory. Make sure HOME is set.");
}

#ifdef APPIMAGE_BUILD
agi::fs::path data_dir() {
	char *exe = realpath("/proc/self/exe", NULL);
	if (!exe) return "";

	sfs::path p = sfs::path(exe).parent_path();
	free(exe);

	if (p.filename() == "bin") {
		// assume unix prefix layout
		return agi::fs::path(p.parent_path()/"share");
	}

	return agi::fs::path(p);
}
#endif
}

namespace agi {
void Path::FillPlatformSpecificPaths() {
	agi::fs::path dotdir = home_dir()/".aegisub";
	SetToken("?user", dotdir);
	SetToken("?local", dotdir);

#ifdef APPIMAGE_BUILD
	agi::fs::path data = data_dir();
	SetToken("?data", (data == "") ? dotdir : data);
	SetToken("?dictionary", Decode("?data/dictionaries"));
#else
	SetToken("?data", P_DATA);
	SetToken("?dictionary", "/usr/share/hunspell");
#endif

	SetToken("?temp", agi::fs::path(sfs::temp_directory_path()));
}
}
