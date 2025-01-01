// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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

#include <array>
#include <string_view>
#include <vector>

#include <libaegisub/exception.h>
#include <libaegisub/fs.h>

namespace json {
	class UnknownElement;
	typedef std::vector<UnknownElement> Array;
}

namespace agi {
class Options;

DEFINE_EXCEPTION(MRUError, Exception);

/// @class MRUManager
/// @brief Most Recently Used (MRU) list handling
///
/// Add() should be called anytime a file is opened, this will either add the
/// entry or update it if it already exists.
///
/// If a file fails to open, Remove() should be called.
class MRUManager {
public:
	/// @brief Map for time->value pairs.
	using MRUListMap = std::vector<agi::fs::path>;

	/// @brief Constructor
	/// @param config File to load MRU values from
	MRUManager(agi::fs::path const& config, std::string_view default_config, agi::Options *options = nullptr);

	/// @brief Add entry to the list.
	/// @param key List name
	/// @param entry Entry to add
	/// @exception MRUError thrown when an invalid key is used.
	void Add(std::string_view key, agi::fs::path const& entry);

	/// @brief Remove entry from the list.
	/// @param key List name
	/// @param entry Entry to add
	/// @exception MRUError thrown when an invalid key is used.
	void Remove(std::string_view key, agi::fs::path const& entry);

	/// @brief Return list
	/// @param key List name
	/// @exception MRUError thrown when an invalid key is used.
	const MRUListMap* Get(std::string_view key);

	/// @brief Return A single entry in a list.
	/// @param key List name
	/// @param entry 0-base position of entry
	/// @exception MRUError thrown when an invalid key is used.
	agi::fs::path const& GetEntry(std::string_view key, const size_t entry);

	/// Write MRU lists to disk.
	void Flush();

private:
	/// Internal name of the config file, set during object construction.
	const agi::fs::path config_name;

	/// User preferences object for maximum number of items to list
	agi::Options *const options;

	/// Internal MRUMap values.
	std::array<MRUListMap, 7> mru;

	/// @brief Load MRU Lists.
	/// @param key List name.
	/// @param array json::Array of values.
	void Load(std::string_view key, ::json::Array const& array);
	/// @brief Prune MRUListMap to the desired length.
	/// This uses the user-set values for MRU list length.
	void Prune(std::string_view key, MRUListMap& map) const;
	MRUListMap &Find(std::string_view key);
};

} // namespace agi
