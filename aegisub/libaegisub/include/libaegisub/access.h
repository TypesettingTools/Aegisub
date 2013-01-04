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

/// @file access.h
/// @brief Public interface for access methods.
/// @ingroup libaegisub

#include <libaegisub/exception.h>
#include <libaegisub/fs_fwd.h>

namespace agi {
	namespace acs {

enum Type {
	FileRead,
	DirRead,
	FileWrite,
	DirWrite
};

void Check(fs::path const& file, acs::Type);

inline void CheckFileRead(fs::path const& file) { Check(file, acs::FileRead); }
inline void CheckFileWrite(fs::path const& file) { Check(file, acs::FileWrite); }

inline void CheckDirRead(fs::path const& dir) { Check(dir, acs::DirRead); }
inline void CheckDirWrite(fs::path const& dir) { Check(dir, acs::DirWrite); }

	} // namespace axs
} // namespace agi
