// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "libaegisub/thesaurus.h"

#include "libaegisub/charset_conv.h"
#include "libaegisub/file_mapping.h"
#include "libaegisub/line_iterator.h"
#include "libaegisub/split.h"

#include <boost/interprocess/streams/bufferstream.hpp>

namespace agi {

Thesaurus::Thesaurus(agi::fs::path const& dat_path, agi::fs::path const& idx_path)
: dat(std::make_unique<read_file_mapping>(dat_path))
{
	read_file_mapping idx_file(idx_path);
	boost::interprocess::ibufferstream idx(idx_file.read(), static_cast<size_t>(idx_file.size()));

	std::string encoding_name;
	getline(idx, encoding_name);
	std::string unused_entry_count;
	getline(idx, unused_entry_count);

	conv = std::make_unique<charset::IconvWrapper>(encoding_name.c_str(), "utf-8");

	// Read the list of words and file offsets for those words
	for (auto const& line : line_iterator<std::string>(idx, encoding_name.c_str())) {
		auto pos = line.find('|');
		if (pos != line.npos && line.find('|', pos + 1) == line.npos)
			offsets[line.substr(0, pos)] = static_cast<size_t>(atoi(line.c_str() + pos + 1));
	}
}

Thesaurus::~Thesaurus() = default;

std::vector<Thesaurus::Entry> Thesaurus::Lookup(std::string_view word) {
	std::vector<Entry> out;
	if (!dat) return out;

	auto it = offsets.find(word);
	if (it == offsets.end()) return out;
	if (it->second >= dat->size()) return out;

	auto len = dat->size() - it->second;
	auto buff = dat->read(it->second, len);
	auto buff_end = buff + len;

	std::string temp;
	auto read_line = [&]() -> std::string_view {
		auto start = buff;
		auto end = std::find(buff, buff_end, '\n');
		buff = end < buff_end ? end + 1 : buff_end;
		if (end > start && end[-1] == '\r') --end;
		temp.clear();
		conv->Convert(std::string_view(start, end - start), temp);
		return temp;
	};

	// First line is the word and meaning count
	std::vector<std::string> header;
	agi::Split(header, read_line(), '|');
	if (header.size() != 2) return out;
	int meanings = atoi(header[1].c_str());

	out.reserve(meanings);
	std::vector<std::string> line;
	for (int i = 0; i < meanings; ++i) {
		agi::Split(line, read_line(), '|');
		if (line.size() < 2)
			continue;

		Entry e;
		// The "definition" is just the part of speech (which may be empty)
		// plus the word it's giving synonyms for (which may not be the passed word)
		if (!line[0].empty())
			e.first = line[0] + ' ';
		e.first += line[1];
		e.second.reserve(line.size() - 2);

		for (size_t i = 2; i < line.size(); ++i) {
			if (line[i].size())
				e.second.emplace_back(std::move(line[i]));
		}

		out.emplace_back(std::move(e));
	}

	return out;
}

}
