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

#include <libaegisub/fs.h>

namespace agi::acs {
enum Type {
	FileRead,
	DirRead,
	FileWrite,
	DirWrite
};

void Check(agi::fs::path const& file, acs::Type);

static inline void CheckFileRead(agi::fs::path const& file) { Check(file, acs::FileRead); }
static inline void CheckFileWrite(agi::fs::path const& file) { Check(file, acs::FileWrite); }

static inline void CheckDirRead(agi::fs::path const& dir) { Check(dir, acs::DirRead); }
static inline void CheckDirWrite(agi::fs::path const& dir) { Check(dir, acs::DirWrite); }
}
