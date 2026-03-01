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
#include <libaegisub/util_osx.h>

#include <pwd.h>

#ifndef __APPLE__
#include <fstream>
#include <stdlib.h>
#include <libgen.h>
#endif

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

std::string xdg_dir(std::string_view token, agi::fs::path const& fallback) {
	const char *env = getenv(token.data());
	if (env) return env;
	return fallback.string();
} 

#ifdef APPIMAGE_BUILD
std::string exe_dir() {
	char *exe, *dir;
	std::string data = "";

#ifdef __FreeBSD__
	exe = realpath("/proc/self/file", NULL);
#else
	exe = realpath("/proc/self/exe", NULL);
#endif

	if (!exe) return "";

	if ((dir = dirname(exe)) && strlen(dir) > 0) {
		data = dir;
	}

	free(exe);

	return data;
}
#endif  /* APPIMAGE_BUILD */
#endif  /* !__APPLE__ */
}

namespace agi {
void Path::FillPlatformSpecificPaths() {
#ifndef __APPLE__
	agi::fs::path home = home_dir();
	agi::fs::path old_root = home/".aegisub";

	agi::fs::path xdg_config = xdg_dir("XDG_CONFIG_HOME", home/".config");
	agi::fs::path xdg_cache = xdg_dir("XDG_CACHE_HOME", home/".cache");
	agi::fs::path xdg_state = xdg_dir("XDG_STATE_HOME", home/".local/state");
	agi::fs::path xdg_data = xdg_dir("XDG_DATA_HOME", home/".local/share");

	if (agi::fs::DirectoryExists(old_root)) {
		SetToken("?user", old_root);
		SetToken("?local", old_root);
		SetToken("?state", old_root);
	} else {
		SetToken("?user", xdg_config/"aegisub");
		SetToken("?local", xdg_cache/"aegisub");
		SetToken("?state", xdg_state/"aegisub");
	}

#ifdef APPIMAGE_BUILD
	agi::fs::path data = exe_dir();
	if (data == "") {
		if (agi::fs::DirectoryExists(old_root)) {
		    data = old_root;
		} else {
			data = xdg_data/"aegisub"
		}
	}
	SetToken("?data", data);
	SetToken("?dictionary", Decode("?data/dictionaries"));
#else
	SetToken("?data", P_DATA);
	SetToken("?dictionary", "/usr/share/hunspell");
#endif

#else
	agi::fs::path app_support = agi::util::GetApplicationSupportDirectory();
	SetToken("?user", app_support/"Aegisub");
	SetToken("?local", app_support/"Aegisub");
	SetToken("?state", app_support/"Aegisub");
	SetToken("?data", agi::util::GetBundleSharedSupportDirectory());
	SetToken("?dictionary", Decode("?data/dictionaries"));
#endif
	SetToken("?temp", agi::fs::path(std::filesystem::temp_directory_path()));
}

}
