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

#include <wx/filename.h>

#include <istream>

#include "ass_attachment.h"

#include "compat.h"

#include <libaegisub/io.h>
#include <libaegisub/scoped_ptr.h>

AssAttachment::AssAttachment(wxString const& name, AssEntryGroup group)
: data(new std::vector<char>)
, filename(name)
, group(group)
{
	wxFileName fname(filename);
	wxString ext = fname.GetExt().Lower();
	if (ext == "ttf")
		filename = fname.GetName() + "_0." + ext;
}

AssEntry *AssAttachment::Clone() const {
	AssAttachment *clone = new AssAttachment(filename, group);
	clone->data = data;
	return clone;
}

const wxString AssAttachment::GetEntryData() const {
	size_t pos = 0;
	size_t size = data->size();
	size_t written = 0;
	unsigned char src[3];
	unsigned char dst[4];

	// Write header
	wxString entryData = (group == ENTRY_FONT ? "fontname: " : "filename: ") + filename + "\r\n";

	// Read three bytes
	while (pos < size) {
		// Number to read
		size_t read = size - pos;
		if (read > 3) read = 3;

		// Read source
		src[0] = (*data)[pos];
		if (read >= 2) src[1] = (*data)[pos+1];
		else src[1] = 0;
		if (read == 3) src[2] = (*data)[pos+2];
		else src[2] = 0;
		pos += read;

		// Codify
		dst[0] = src[0] >> 2;
		dst[1] = ((src[0] & 0x3) << 4) | ((src[1] & 0xF0) >> 4);
		dst[2] = ((src[1] & 0xF) << 2) | ((src[2] & 0xC0) >> 6);
		dst[3] = src[2] & 0x3F;

		// Number to write
		size_t toWrite = read+1;

		// Convert to text
		for (size_t i=0;i<toWrite;i++) {
			entryData += dst[i]+33;
			written++;

			// Line break
			if (written == 80 && pos < size) {
				written = 0;
				entryData += "\r\n";
			}
		}
	}

	return entryData;
}

void AssAttachment::Extract(wxString const& filename) const {
	agi::io::Save(from_wx(filename), true).Get().write(&(*data)[0], data->size());
}

void AssAttachment::Import(wxString const& filename) {
	agi::scoped_ptr<std::istream> file(agi::io::Open(from_wx(filename), true));
	file->seekg(0, std::ios::end);
	data->resize(file->tellg());
	file->seekg(0, std::ios::beg);
	file->read(&(*data)[0], data->size());
}

wxString AssAttachment::GetFileName(bool raw) const {
	if (raw || filename.Right(4).Lower() != ".ttf") return filename;

	// Remove stuff after last underscore if it's a font
	wxString::size_type last_under = filename.rfind('_');
	if (last_under == wxString::npos)
		return filename;

	return filename.Left(last_under) + ".ttf";
}

void AssAttachment::Finish() {
	// Source and dest buffers
	unsigned char src[4];
	unsigned char dst[3];

	data->reserve(buffer.size() * 3 / 4);

	// Read buffer
	for(size_t pos = 0; pos + 1 < buffer.size(); ) {
		// Find characters left
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

		// Push into vector
		copy(dst, dst + read - 1, back_inserter(*data));
	}

	// Clear buffer
	buffer.clear();
	buffer.Shrink();
}
