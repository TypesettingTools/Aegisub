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

/// @file hotkey.h
/// @brief Hotkey handler
/// @ingroup hotkey menu event window

#ifndef LAGI_PRE
#include <math.h>

#include <memory>
#include <string>
#include <vector>
#endif

#include <libaegisub/cajun/elements.h>


namespace agi {
	namespace hotkey {

class Hotkey;
/// Hotkey instance.
extern Hotkey *hotkey;

/// @class Combo
/// A Combo represents a linear sequence of characters set in an std::vector.  This makes up
/// a single combination, or "Hotkey".
class Combo {
friend class Hotkey;

public:
	/// Linear key sequence that forms a combination or "Combo"
	typedef std::vector<std::string> ComboMap;

	/// Constructor
	/// @param ctx Context
	/// @param cmd Command name
	Combo(std::string ctx, std::string cmd): cmd_name(cmd), context(ctx) {}

	/// Destructor
	~Combo() {}

	/// String representation of the Combo
	std::string Str();

	/// String suitable for usage in a menu.
	std::string StrMenu();

	/// Get the literal combo map.
	/// @return ComboMap (std::vector) of linear key sequence.
	const ComboMap& Get() { return key_map; }

	/// Command name triggered by the combination.
	/// @return Command name
	const std::string& CmdName() { return cmd_name; }

	/// Context this Combo is triggered in.
	const std::string& Context() { return context; }

	/// Enable or disable Combo or "Hotkey".
	/// @param e Bool state.
	void Enable(bool e) { enable = e; }

	/// Check whether Combo is currently enabled or disabled.
	/// @return State.
	const bool& IsEnabled() { return enable; }

private:
	ComboMap key_map;				///< Map.
	const std::string cmd_name;		///< Command name.
	const std::string context;		///< Context
	bool enable;					///< Enable/Disable state

	/// Insert a key into the ComboMap.
	/// @param key Key to insert.
	void KeyInsert(std::string key) { key_map.push_back(key); }
};


/// @class Hotkey
/// Holds the map of Combo instances and handles searching for matching key sequences.
class Hotkey {
public:
	/// Constructor
	/// @param file           Location of user config file.
	/// @param default_config Default config.
	Hotkey(const std::string &file, const std::string &default_config);

	/// Destructor.
	~Hotkey();

	/// Scan for a matching key.
	/// @param context  Context requested.
	/// @param str      Hyphen separated key sequence.
	/// @param[out] cmd Command found.
	bool Scan(const std::string &context, const std::string &str, std::string &cmd) const;

	/// Get the string representation of the hotkeys for the given command
	/// @param context Context requested
	/// @param command Command name
	/// @return A vector of all hotkeys for that command in the context
	std::vector<std::string> GetHotkeys(const std::string &context, const std::string &command) const;

private:
	typedef std::multimap<std::string, Combo*> HotkeyMap;	///< Map to hold Combo instances.
	HotkeyMap str_map;										///< String representation -> Combo
	HotkeyMap cmd_map;										///< Command name -> Combo
	const std::string config_file;							///< Default user config location.
	const std::string config_default;						///< Default config.

	/// Build hotkey map.
	/// @param context Context being parsed.
	/// @param object  json::Object holding items for context being parsed.
	void BuildHotkey(std::string context, const ::json::Object& object);

	/// Insert Combo into HotkeyMap instance.
	/// @param combo Combo to insert.
	void ComboInsert(Combo *combo);

	/// Write active Hotkey configuration to disk.
	void Flush();
};

	} // namespace hotkey
} // namespace agi
