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
#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"

#include <boost/algorithm/string.hpp>
#include <boost/phoenix/operator/comparison.hpp>
#include <boost/phoenix/core/argument.hpp>

#include <fstream>

using boost::phoenix::placeholders::_1;

namespace agi {

Thesaurus::Thesaurus(agi::fs::path const& dat_path, agi::fs::path const& idx_path)
: dat(io::Open(dat_path))
{
	std::unique_ptr<std::ifstream> idx(io::Open(idx_path));

	std::string encoding_name;
	getline(*idx, encoding_name);
	std::string unused_entry_count;
	getline(*idx, unused_entry_count);

	// Read the list of words and file offsets for those words
	for (line_iterator<std::string> iter(*idx, encoding_name), end; iter != end; ++iter) {
		std::vector<std::string> chunks;
		boost::split(chunks, *iter, _1 == '|');
		if (chunks.size() == 2)
			offsets[chunks[0]] = atoi(chunks[1].c_str());
	}

	conv.reset(new charset::IconvWrapper(encoding_name.c_str(), "utf-8"));
}

Thesaurus::~Thesaurus() { }

std::vector<Thesaurus::Entry> Thesaurus::Lookup(std::string const& word) {
	std::vector<Entry> out;
	if (!dat.get()) return out;

	auto it = offsets.find(word);
	if (it == offsets.end()) return out;

	dat->seekg(it->second, std::ios::beg);
	if (!dat->good()) return out;

	// First line is the word and meaning count
	std::string temp;
	getline(*dat, temp);
	std::vector<std::string> header;
	std::string converted(conv->Convert(temp));
	boost::split(header, converted, _1 == '|');
	if (header.size() != 2) return out;
	int meanings = atoi(header[1].c_str());

	out.reserve(meanings);
	for (int i = 0; i < meanings; ++i) {
		getline(*dat, temp);
		auto converted = conv->Convert(temp);
		std::vector<std::string> line;
		boost::split(line, converted, _1 == '|');

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
