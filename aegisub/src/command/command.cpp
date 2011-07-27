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

/// @file command.cpp
/// @brief Command system base file.
/// @ingroup command

#include "command.h"
#include "icon.h"
#include <libaegisub/log.h>

namespace cmd {
	static std::map<std::string, Command*> cmd_map;
	typedef std::map<std::string, Command*>::iterator iterator;

	static iterator find_command(std::string const& name) {
		iterator it = cmd_map.find(name);
		if (it == cmd_map.end()) throw CommandNotFound("'" + name + "' is not a valid command name");
		return it;
	}

	void reg(Command *cmd) {
		cmd_map[cmd->name()] = cmd;
	}

	int id(std::string const& name) {
		return distance(cmd_map.begin(), find_command(name));
	}

	int count() {
		return cmd_map.size();
	}

	Command *get(std::string const& name) {
		return find_command(name)->second;
	}

	void call(agi::Context *c, int id) {
		std::map<std::string, Command*>::iterator index(cmd_map.begin());
		advance(index, id);

		if (index != cmd_map.end()) {
			LOG_D("event/command") << index->first << " " << "(Id: " << id << ")";
			(*index->second)(c);
		} else {
			LOG_W("event/command/not_found") << "EVENT ID NOT FOUND: " << id;
			// XXX: throw
		}
	}

	void call(std::string const& name, agi::Context*c) {
		(*find_command(name)->second)(c);
	}

	wxBitmap const& Command::Icon(int size) {
		if (size == 16) {
			return icon::get(name(), 16);
		} else if (size == 24) {
			return icon::get(name(), 24);
		} else {
			throw CommandIconInvalid("Valid icon sizes are 16 or 24.");
		}
	}

	// These forward declarations exist here since we don't want to expose
	// them in a header, they're strictly internal-use.
	void init_app();
	void init_audio();
	void init_automation();
	void init_command();
	void init_edit();
	void init_grid();
	void init_help();
	void init_keyframe();
	void init_medusa();
	void init_menu();
	void init_recent();
	void init_subtitle();
	void init_time();
	void init_timecode();
	void init_tool();
	void init_video();

	void init_builtin_commands() {
		LOG_D("command/init") << "Populating command map";
		init_app();
		init_audio();
		init_automation();
		init_edit();
		init_grid();
		init_help();
		init_keyframe();
		init_menu();
		init_recent();
		init_subtitle();
		init_time();
		init_timecode();
		init_tool();
		init_video();
	}

	void clear() {
		for (std::map<std::string, Command*>::iterator it = cmd_map.begin(); it != cmd_map.end(); ++it) {
			delete it->second;
		}
		cmd_map.clear();
	}
}
