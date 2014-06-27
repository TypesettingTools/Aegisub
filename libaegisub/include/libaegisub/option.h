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

/// @file option.h
/// @brief Public interface for option values.
/// @ingroup libaegisub

#pragma once

#include <boost/filesystem/path.hpp>
#include <iosfwd>
#include <map>
#include <memory>

#include <libaegisub/fs_fwd.h>

namespace json {
	class UnknownElement;
	typedef std::map<std::string, UnknownElement> Object;
}

namespace agi {
class OptionValue;

using OptionValueMap = std::map<std::string, std::unique_ptr<OptionValue>>;

class Options {
public:
	/// Options class settings.
	enum OptionSetting {
		NONE       = 0x000,		///< Do nothing (default)
		FLUSH_SKIP = 0x001		///< Skip writing the config file to disk
	};

private:
	/// Internal OptionValueMap
	OptionValueMap values;

	/// User config (file that will be written to disk)
	const agi::fs::path config_file;

	/// Settings.
	const OptionSetting setting;

	/// @brief Load a config file into the Options object.
	/// @param config Config to load.
	/// @param ignore_errors Log invalid entires in the option file and continue rather than throwing an exception
	void LoadConfig(std::istream& stream, bool ignore_errors = false);

public:
	/// @brief Constructor
	/// @param file User config that will be loaded from and written back to.
	/// @param default_config Default configuration.
	Options(agi::fs::path const& file, std::pair<const char *, size_t> default_config, const OptionSetting setting = NONE);

	template<size_t N>
	Options(agi::fs::path const& file, const char (&default_config)[N], const OptionSetting setting = NONE)
	: Options(file, {default_config, N - 1}, setting) { }

	/// Destructor
	~Options();

	/// @brief Get an option by name.
	/// @param name Option to get.
	/// Get an option value object by name throw an internal exception if the option is not found.
	OptionValue* Get(const std::string &name);

	/// @brief Next configuration file to load.
	/// @param[in] src Stream to load from.
	/// Load next config which will supersede any values from previous configs
	/// can be called as many times as required, but only after ConfigDefault() and
	/// before ConfigUser()
	void ConfigNext(std::istream &stream);

	/// @brief Set user config file.
	/// Set the user configuration file and read options from it, closes all
	/// possible config file loading and sets the file to write to.
	void ConfigUser();

	/// Write the user configuration to disk, throws an exception if something goes wrong.
	void Flush() const;
};

} // namespace agi
