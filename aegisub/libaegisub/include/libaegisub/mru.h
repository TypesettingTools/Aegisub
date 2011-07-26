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
//
// $Id$

/// @file mru.h
/// @brief Public interface for MRU (Most Recently Used) lists.
/// @ingroup libaegisub

#ifndef LAGI_PRE
#include <fstream>
#include <list>
#include <map>
#endif

#include <libaegisub/exception.h>

namespace json {
	class Array;
}

namespace agi {

DEFINE_BASE_EXCEPTION_NOINNER(MRUError,Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(MRUErrorInvalidKey, MRUError, "mru/invalid")
DEFINE_SIMPLE_EXCEPTION_NOINNER(MRUErrorIndexOutOfRange, MRUError, "mru/invalid")

/// @class MRUManager
/// @brief Most Recently Used (MRU) list handling
///
/// Add() should be called anytime a file is opened, this will either add the
/// entry or update it if it already exists.
///
/// If a file fails to open, Remove() should be called.
///
class MRUManager {

public:
	/// @brief Map for time->value pairs.
	typedef std::list<std::string> MRUListMap;

	/// @brief Constructor
	/// @param config File to load MRU values from
	MRUManager(const std::string &config, const std::string &default_config);

	/// Destructor
	~MRUManager();

	/// @brief Add entry to the list.
	/// @param key List name
	/// @param entry Entry to add
	/// @exception MRUErrorInvalidKey thrown when an invalid key is used.
	void Add(const std::string &key, const std::string &entry);

	/// @brief Remove entry from the list.
	/// @param key List name
	/// @param entry Entry to add
	/// @exception MRUErrorInvalidKey thrown when an invalid key is used.
	void Remove(const std::string &key, const std::string &entry);

	/// @brief Return list
	/// @param key List name
	/// @exception MRUErrorInvalidKey thrown when an invalid key is used.
	const MRUListMap* Get(const std::string &key);

	/// @brief Return A single entry in a list.
	/// @param key List name
	/// @param entry 0-base position of entry
	/// @exception MRUErrorInvalidKey thrown when an invalid key is used.
	std::string const& GetEntry(const std::string &key, size_t entry);

	/// Write MRU lists to disk.
	void Flush();

private:

	/// Internal name of the config file, set during object construction.
	const std::string config_name;

	/// @brief Map for MRUListMap values.
	/// @param std::string Name
	/// @param MRUListMap instance.
	typedef std::map<std::string, MRUListMap> MRUMap;

	/// Internal MRUMap values.
	MRUMap mru;

	void Load(const std::string &key, const ::json::Array& array);
	inline void Prune(MRUListMap& map);
	MRUListMap &Find(std::string const& key);
};

} // namespace agi
