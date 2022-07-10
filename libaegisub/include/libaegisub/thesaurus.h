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

#include "fs_fwd.h"

#include <boost/container/flat_map.hpp>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace agi {

class read_file_mapping;
namespace charset {
class IconvWrapper;
}

class Thesaurus {
	/// Map of word -> byte position in the data file
	boost::container::flat_map<std::string, size_t> offsets;
	/// Read handle to the data file
	std::unique_ptr<read_file_mapping> dat;
	/// Converter from the data file's charset to UTF-8
	std::unique_ptr<charset::IconvWrapper> conv;

  public:
	/// A pair of a word and synonyms for that word
	typedef std::pair<std::string, std::vector<std::string>> Entry;

	/// Constructor
	/// @param dat_path Path to data file
	/// @param idx_path Path to index file
	Thesaurus(agi::fs::path const& dat_path, agi::fs::path const& idx_path);
	~Thesaurus();

	/// Look up synonyms for a word
	/// @param word Word to look up
	std::vector<Entry> Lookup(std::string const& word);
};

} // namespace agi
