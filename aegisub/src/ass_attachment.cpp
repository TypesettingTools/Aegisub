// Copyright (c) 2006, Rodrigo Braz Monteiro
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
// Aegisub Project http://www.aegisub.org/

/// @file ass_attachment.cpp
/// @brief Manage files embedded in subtitles
/// @ingroup subs_storage
///

#include "config.h"

#include "ass_attachment.h"

#include <libaegisub/io.h>

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

AssAttachment::AssAttachment(std::string const& name, AssEntryGroup group)
: data(new std::vector<char>)
, filename(name)
, group(group)
{
}

AssAttachment::AssAttachment(agi::fs::path const& name, AssEntryGroup group)
: data(new std::vector<char>)
, filename(name.filename().string())
, group(group)
{
	if (boost::iends_with(filename, ".ttf"))
		filename = filename.substr(0, filename.size() - 4) + "_0" + filename.substr(filename.size() - 4);

	std::unique_ptr<std::istream> file(agi::io::Open(name, true));
	file->seekg(0, std::ios::end);
	data->resize(file->tellg());
	file->seekg(0, std::ios::beg);
	file->read(&(*data)[0], data->size());
}

AssEntry *AssAttachment::Clone() const {
	AssAttachment *clone = new AssAttachment(filename, group);
	clone->data = data;
	return clone;
}

const std::string AssAttachment::GetEntryData() const {
	size_t size = data->size();
	size_t written = 0;

	std::string entryData = (group == ENTRY_FONT ? "fontname: " : "filename: ") + filename + "\r\n";
	entryData.reserve(size * 4 / 3 + size / 80 * 2 + entryData.size() + 2);

	for (size_t pos = 0; pos < size; pos += 3) {
		unsigned char src[3] = { '\0', '\0', '\0' };
		memcpy(src, &(*data)[pos], std::min<size_t>(3u, size - pos));

		unsigned char dst[4];
		dst[0] = src[0] >> 2;
		dst[1] = ((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4);
		dst[2] = ((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6);
		dst[3] = src[2] & 0x3F;

		for (size_t i = 0; i < std::min<size_t>(size - pos + 1, 4u); ++i) {
			entryData += dst[i] + 33;

			if (++written == 80 && pos + 3 < size) {
				written = 0;
				entryData += "\r\n";
			}
		}
	}

	return entryData;
}

void AssAttachment::Extract(agi::fs::path const& filename) const {
	agi::io::Save(filename, true).Get().write(&(*data)[0], data->size());
}

std::string AssAttachment::GetFileName(bool raw) const {
	if (raw || !boost::iends_with(filename, ".ttf")) return filename;

	// Remove stuff after last underscore if it's a font
	std::string::size_type last_under = filename.rfind('_');
	if (last_under == std::string::npos)
		return filename;

	return filename.substr(0, last_under) + ".ttf";
}

void AssAttachment::Finish() {
	unsigned char src[4];
	unsigned char dst[3];

	data->reserve(buffer.size() * 3 / 4);

	for(size_t pos = 0; pos + 1 < buffer.size(); ) {
		size_t read = std::min<size_t>(buffer.size() - pos, 4);

		// Move 4 bytes from buffer to src
		for (size_t i = 0; i < read; ++i)
			src[i] = (unsigned char)buffer[pos++] - 33;
		for (size_t i = read; i < 4; ++i)
			src[i] = 0;

		// Convert the 4 bytes from source to 3 in dst
		dst[0] = (src[0] << 2) | (src[1] >> 4);
		dst[1] = ((src[1] & 0xF) << 4) | (src[2] >> 2);
		dst[2] = ((src[2] & 0x3) << 6) | (src[3]);

		copy(dst, dst + read - 1, back_inserter(*data));
	}

	buffer.clear();
	buffer.shrink_to_fit();
}
