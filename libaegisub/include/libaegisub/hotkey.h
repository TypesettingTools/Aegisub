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

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <libaegisub/fs.h>
#include <libaegisub/signal.h>

namespace json {
	class UnknownElement;
	typedef std::map<std::string, UnknownElement, std::less<>> Object;
}

namespace agi::hotkey {
/// @class Combo
/// A Combo represents a linear sequence of characters set in an std::vector.
/// This makes up a single combination, or "Hotkey".
class Combo {
	std::string keys;
	std::string cmd_name;
	std::string context;
public:
	/// Constructor
	/// @param ctx Context
	/// @param cmd Command name
	Combo(std::string_view ctx, std::string_view cmd, std::string_view keys)
	: keys(keys)
	, cmd_name(cmd)
	, context(ctx)
	{
	}

	/// String representation of the Combo
	std::string_view Str() const { return keys; }

	/// Command name triggered by the combination.
	/// @return Command name
	std::string_view CmdName() const { return cmd_name; }

	/// Context this Combo is triggered in.
	std::string_view Context() const { return context; }
};

/// @class Hotkey
/// Holds the map of Combo instances and handles searching for matching key sequences.
class Hotkey {
public:
	/// Map to hold Combo instances
	typedef std::multimap<std::string, Combo, std::less<>> HotkeyMap;
private:
	HotkeyMap cmd_map;                  ///< Command name -> Combo
	std::vector<const Combo *> str_map; ///< Sorted by string representation
	const agi::fs::path config_file;    ///< Default user config location.
	bool backup_config_file = false;

	/// Build hotkey map.
	/// @param context Context being parsed.
	/// @param object  json::Object holding items for context being parsed.
	void BuildHotkey(std::string_view context, const json::Object& object);

	/// Write active Hotkey configuration to disk.
	void Flush();

	void UpdateStrMap();

	/// Announce that the loaded hotkeys have been changed
	agi::signal::Signal<> HotkeysChanged;
public:
	/// Constructor
	/// @param file           Location of user config file.
	/// @param default_config Default config.
	Hotkey(agi::fs::path const& file, std::string_view default_config);

	/// Scan for a matching key.
	/// @param context  Context requested.
	/// @param str      Hyphen separated key sequence.
	/// @param always   Enable the "Always" override context
	/// @return Name of command or "" if none match
	std::string_view Scan(std::string_view context, std::string_view str, bool always) const;

	bool HasHotkey(std::string_view context, std::string_view str) const;

	/// Get the string representation of the hotkeys for the given command
	/// @param context Context requested
	/// @param command Command name
	/// @return A vector of all hotkeys for that command in the context
	std::vector<std::string> GetHotkeys(std::string_view context, std::string_view command) const;

	/// Get a string representation of a hotkeys for the given command
	/// @param context Context requested
	/// @param command Command name
	/// @return A hotkey for the given command or "" if there are none
	std::string_view GetHotkey(std::string_view context, std::string_view command) const;

	/// Get the raw command name -> combo map for all registered hotkeys
	HotkeyMap const& GetHotkeyMap() const { return cmd_map; }

	/// Replace the loaded hotkeys with a new set
	void SetHotkeyMap(HotkeyMap new_map);

	DEFINE_SIGNAL_ADDERS(HotkeysChanged, AddHotkeyChangeListener)
};

} // namespace agi::hotkey
