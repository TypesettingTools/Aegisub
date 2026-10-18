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

namespace sfs = std::filesystem;

namespace agi {
void Path::FillPlatformSpecificPaths() {
	agi::fs::path user_dir = agi::fs::path(sfs::path(agi::util::GetApplicationSupportDirectory()))/"Aegisub";
	SetToken("?user", user_dir);
	SetToken("?local", user_dir);
	SetToken("?data", agi::util::GetBundleSharedSupportDirectory());
	SetToken("?dictionary", Decode("?data/dictionaries"));
	SetToken("?temp", agi::fs::path(sfs::temp_directory_path()));
}
}
