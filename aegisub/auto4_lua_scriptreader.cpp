// Copyright (c) 2006, 2007, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

#ifdef WITH_AUTO4_LUA

#include "auto4_lua_scriptreader.h"

namespace Automation4 {

	// LuaScriptReader
	LuaScriptReader::LuaScriptReader(const wxString &filename)
	{
#ifdef WIN32
		f = _tfopen(filename.c_str(), _T("rb"));
#else
		f = fopen(filename.fn_str(), "rb");
#endif
		first = true;
		databuf = new char[bufsize];
	}

	LuaScriptReader::~LuaScriptReader()
	{
		if (databuf)
			delete databuf;
		fclose(f);
	}

	const char* LuaScriptReader::reader_func(lua_State *L, void *data, size_t *size)
	{
		LuaScriptReader *self = (LuaScriptReader*)(data);
		unsigned char *b = (unsigned char *)self->databuf;
		FILE *f = self->f;

		if (feof(f)) {
			*size = 0;
			return 0;
		}

		if (self->first) {
			// check if file is sensible and maybe skip bom
			if ((*size = fread(b, 1, 4, f)) == 4) {
				if (b[0] == 0xEF && b[1] == 0xBB && b[2] == 0xBF) {
					// got an utf8 file with bom
					// nothing further to do, already skipped the bom
					fseek(f, -1, SEEK_CUR);
				} else {
					// oops, not utf8 with bom
					// check if there is some other BOM in place and complain if there is...
					if ((b[0] == 0xFF && b[1] == 0xFE && b[2] == 0x00 && b[3] == 0x00) || // utf32be
						(b[0] == 0x00 && b[1] == 0x00 && b[2] == 0xFE && b[3] == 0xFF) || // utf32le
						(b[0] == 0xFF && b[1] == 0xFE) || // utf16be
						(b[0] == 0xFE && b[1] == 0xFF) || // utf16le
						(b[0] == 0x2B && b[1] == 0x2F && b[2] == 0x76) || // utf7
						(b[0] == 0x00 && b[2] == 0x00) || // looks like utf16be
						(b[1] == 0x00 && b[3] == 0x00)) { // looks like utf16le
							throw _T("The script file uses an unsupported character set. Only UTF-8 is supported.");
					}
					// assume utf8 without bom, and rewind file
					fseek(f, 0, SEEK_SET);
				}
			} else {
				// hmm, rather short file this...
				// doesn't have a bom, assume it's just ascii/utf8 without bom
				return self->databuf; // *size is already set
			}
			self->first = false;
		}

		*size = fread(b, 1, bufsize, f);

		return self->databuf;
	}

};

#endif // WITH_AUTO4_LUA
