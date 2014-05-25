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

/// @file thesaurus.cpp
/// @brief MyThes-compatible thesaurus implementation
/// @ingroup libaegisub thesaurus

#include "libaegisub/thesaurus.h"

#include "libaegisub/charset_conv.h"
#include "libaegisub/file_mapping.h"
#include "libaegisub/line_iterator.h"
#include "libaegisub/make_unique.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

namespace agi {

Thesaurus::Thesaurus(agi::fs::path const& dat_path, agi::fs::path const& idx_path)
: dat(make_unique<read_file_mapping>(dat_path))
{
	read_file_mapping idx_file(idx_path);
	boost::interprocess::ibufferstream idx(idx_file.read(), static_cast<size_t>(idx_file.size()));

	std::string encoding_name;
	getline(idx, encoding_name);
	std::string unused_entry_count;
	getline(idx, unused_entry_count);

	conv.reset(new charset::IconvWrapper(encoding_name.c_str(), "utf-8"));

	// Read the list of words and file offsets for those words
	for (auto const& line : line_iterator<std::string>(idx, encoding_name)) {
		std::vector<std::string> chunks;
		boost::split(chunks, line, [](char c) { return c == '|'; });
		if (chunks.size() == 2)
			offsets[chunks[0]] = static_cast<size_t>(atoi(chunks[1].c_str()));
	}
}

Thesaurus::~Thesaurus() { }

std::vector<Thesaurus::Entry> Thesaurus::Lookup(std::string const& word) {
	std::vector<Entry> out;
	if (!dat.get()) return out;

	auto it = offsets.find(word);
	if (it == offsets.end()) return out;
	if (it->second >= dat->size()) return out;

	auto len = dat->size() - it->second;
	auto buff = dat->read(it->second, len);
	auto buff_end = buff + len;

	std::string temp;
	auto read_line = [&] () -> std::string const& {
		auto start = buff;
		auto end = std::find(buff, buff_end, '\n');
		buff = end < buff_end ? end + 1 : buff_end;
		if (end > start && end[-1] == '\r') --end;
		temp.clear();
		conv->Convert(start, end - start, temp);
		return temp;
	};

	// First line is the word and meaning count
	std::vector<std::string> header;
	boost::split(header, read_line(), [](char c) { return c == '|'; });
	if (header.size() != 2) return out;
	int meanings = atoi(header[1].c_str());

	out.reserve(meanings);
	for (int i = 0; i < meanings; ++i) {
		std::vector<std::string> line;
		boost::split(line, read_line(), [](char c) { return c == '|'; });

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
