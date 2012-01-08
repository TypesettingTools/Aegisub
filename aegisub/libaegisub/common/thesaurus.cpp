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
//
// $Id$

/// @file thesaurus.cpp
/// @brief MyThes-compatible thesaurus implementation
/// @ingroup libaegisub thesaurus

#include "libaegisub/thesaurus.h"

#include "libaegisub/charset_conv.h"
#include "libaegisub/io.h"
#include "libaegisub/line_iterator.h"

template<class String, class Char, class Container>
static void split(String const& str, Char sep, Container *out) {
	typename String::size_type pos, prev = 0;
	out->reserve(2);
	while ((pos = str.find(sep, prev)) != String::npos) {
		if (pos > prev)
			out->push_back(str.substr(prev, pos - prev));
		prev = pos + 1;
	}
	if (prev < str.size())
		out->push_back(str.substr(prev));
}

namespace agi {

Thesaurus::Thesaurus(std::string const& dat_path, std::string const& idx_path)
: dat(io::Open(dat_path))
{
	scoped_ptr<std::ifstream> idx(io::Open(idx_path));

	std::string encoding_name;
	getline(*idx, encoding_name);
	std::string unused_entry_count;
	getline(*idx, unused_entry_count);

	// Read the list of words and file offsets for those words
	for (line_iterator<std::string> iter(*idx, encoding_name), end; iter != end; ++iter) {
		std::vector<std::string> chunks;
		split(*iter, '|', &chunks);
		if (chunks.size() == 2) {
			offsets[chunks[0]] = atoi(chunks[1].c_str());
		}
	}

	conv.reset(new charset::IconvWrapper(encoding_name.c_str(), "utf-8"));
}

Thesaurus::~Thesaurus() { }

void Thesaurus::Lookup(std::string const& word, std::vector<Entry> *out) {
	out->clear();

	std::map<std::string, int>::const_iterator it = offsets.find(word);
	if (!dat.get() || it == offsets.end()) return;

	dat->seekg(it->second, std::ios::beg);
	if (!dat->good()) return;

	// First line is the word and meaning count
	std::string temp;
	getline(*dat, temp);
	std::vector<std::string> header;
	split(conv->Convert(temp), '|', &header);
	if (header.size() != 2) return;
	int meanings = atoi(header[1].c_str());

	out->resize(meanings);
	for (int i = 0; i < meanings; ++i) {
		std::vector<std::string> line;
		getline(*dat, temp);
		split(conv->Convert(temp), '|', &line);

		// The "definition" is just the part of speech plus the word it's
		// giving synonyms for (which may not be the passed word)
		(*out)[i].first = line[0] + ' ' + line[1];
		(*out)[i].second.reserve(line.size() - 2);
		copy(line.begin() + 2, line.end(), back_inserter((*out)[i].second));
	}
}

}
