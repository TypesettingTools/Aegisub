// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

// The non-unix version of ftsystem.c uses stdio functions, but freetype uses
// streams as if they had mmap's performance characteristics (as it uses mmap
// on unix), which results in mind-blowingly poor performance (35%+ of the
// fontconfig caching runtime is spent on fseek).

#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_STREAM_H
#include FT_SYSTEM_H
#include FT_ERRORS_H

#include <codecvt>
#include <fstream>

extern "C" FT_Error FT_Stream_Open(FT_Stream stream, const char *filepathname) {
	if (!stream)
		return FT_THROW(Invalid_Stream_Handle);

	stream->descriptor.pointer = nullptr;
	stream->pathname.pointer = const_cast<char *>(filepathname);
	stream->base = nullptr;
	stream->pos = 0;
	stream->read = nullptr;
	stream->close = nullptr;

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	std::ifstream file(converter.from_bytes(filepathname), std::ios::binary);
	if (!file.good())
		return FT_THROW(Cannot_Open_Resource);

	file.seekg(0, std::ios::end);
	stream->size = (unsigned long)file.tellg();
	if (!stream->size)
		return FT_THROW(Cannot_Open_Stream);
	file.seekg(0, std::ios::beg);

	stream->base = (unsigned char *)malloc(stream->size);
	file.read((char *)stream->base, stream->size);

	stream->close = [](FT_Stream stream) {
		free(stream->base);
		stream->size = 0;
		stream->base = nullptr;
	};

	return FT_Err_Ok;
}

extern "C" FT_Memory FT_New_Memory() {
	return new FT_MemoryRec_{
		nullptr,
		[](FT_Memory, long size) { return malloc(size); },
		[](FT_Memory, void *ptr) { free(ptr); },
		[](FT_Memory, long, long size, void *ptr) { return realloc(ptr, size); }};
}

extern "C" void FT_Done_Memory(FT_Memory memory) {
	delete memory;
}
