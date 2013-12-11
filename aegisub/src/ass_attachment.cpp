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

#include "config.h"

#include "ass_attachment.h"

#include <libaegisub/ass/uuencode.h>
#include <libaegisub/io.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/iterator_range.hpp>
#include <fstream>

AssAttachment::AssAttachment(AssAttachment const& rgt)
: entry_data(rgt.entry_data)
, filename(rgt.filename)
, group(rgt.group)
{
}

AssAttachment::AssAttachment(std::string const& header, AssEntryGroup group)
: entry_data(header + "\r\n")
, filename(header.substr(10))
, group(group)
{
}

AssAttachment::AssAttachment(agi::fs::path const& name, AssEntryGroup group)
: filename(name.filename().string())
, group(group)
{
	// SSA stuffs some information about the font in the embedded filename, but
	// nothing else uses it so just do the absolute minimum (0 is the encoding)
	if (boost::iends_with(filename.get(), ".ttf"))
		filename = filename.get().substr(0, filename.get().size() - 4) + "_0" + filename.get().substr(filename.get().size() - 4);

	std::vector<char> data;
	std::unique_ptr<std::istream> file(agi::io::Open(name, true));
	file->seekg(0, std::ios::end);
	data.resize(file->tellg());
	file->seekg(0, std::ios::beg);
	file->read(&data[0], data.size());

	entry_data = (group == AssEntryGroup::FONT ? "fontname: " : "filename: ") + filename.get() + "\r\n";
	entry_data = entry_data.get() + agi::ass::UUEncode(data);
}

AssEntry *AssAttachment::Clone() const {
	return new AssAttachment(*this);
}

const std::string AssAttachment::GetEntryData() const {
	return entry_data;
}

size_t AssAttachment::GetSize() const {
	auto header_end = entry_data.get().find('\n');
	return entry_data.get().size() - header_end - 1;
}

void AssAttachment::Extract(agi::fs::path const& filename) const {
	auto header_end = entry_data.get().find('\n');
	auto decoded = agi::ass::UUDecode(boost::make_iterator_range(entry_data.get().begin() + header_end + 1, entry_data.get().end()));
	agi::io::Save(filename, true).Get().write(&decoded[0], decoded.size());
}

std::string AssAttachment::GetFileName(bool raw) const {
	if (raw || !boost::iends_with(filename.get(), ".ttf")) return filename;

	// Remove stuff after last underscore if it's a font
	std::string::size_type last_under = filename.get().rfind('_');
	if (last_under == std::string::npos)
		return filename;

	return filename.get().substr(0, last_under) + ".ttf";
}
