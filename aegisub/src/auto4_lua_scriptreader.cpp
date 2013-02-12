// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

/// @file auto4_lua_scriptreader.cpp
/// @brief Script-file reader for Lua 5.1-based scripting engine
/// @ingroup scripting
///

#include "config.h"

#ifdef WITH_AUTO4_LUA

#include "auto4_lua_scriptreader.h"

#include "charset_detect.h"

#include <libaegisub/io.h>
#include <libaegisub/charset_conv.h>

namespace Automation4 {
	LuaScriptReader::LuaScriptReader(agi::fs::path const& filename)
	: conv(new agi::charset::IconvWrapper(CharSetDetect::GetEncoding(filename).c_str(), "utf-8", false))
	, file(agi::io::Open(filename))
	{
	}

	LuaScriptReader::~LuaScriptReader() { }

	const char *LuaScriptReader::Read(size_t *bytes_read) {
		char in_buf[512];
		file.read(in_buf, sizeof(in_buf));

		const char *in = in_buf;
		char *out = buf;
		size_t in_bytes = file.gcount();
		size_t out_bytes = sizeof(buf);

		conv->Convert(&in, &in_bytes, &out, &out_bytes);
		if (in_bytes > 0 && in != in_buf)
			file.seekg(-(std::streamoff)in_bytes, std::ios_base::cur);
		*bytes_read = out - buf;

		// Skip the bom
		if (*bytes_read >= 3 && buf[0] == -17 && buf[1] == -69 && buf[2] == -65) {
			*bytes_read -= 3;
			return buf + 3;
		}
		return buf;
	}

	const char* LuaScriptReader::reader_func(lua_State *, void *data, size_t *size) {
		return static_cast<LuaScriptReader*>(data)->Read(size);
	}
}

#endif // WITH_AUTO4_LUA
