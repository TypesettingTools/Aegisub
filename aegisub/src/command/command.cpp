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
#include <libaegisub/log.h>

namespace cmd {

CommandManager *cm;

int id(std::string name) { return cm->id(name); }
void call(agi::Context *c, const int id) { return cm->call(c, id); }
int count() { return cm->count(); }
Command* get(std::string name) { return cm->get(name); }


wxBitmap* Command::Icon(int size) {
	if (size == 16) {
		return icon::get(name(), 16);
	} else if (size == 24) {
		return icon::get(name(), 24);
	} else {
		throw CommandIconInvalid("Valid icon sizes are 16 or 24.");
	}
}


int CommandManager::id(std::string name) {

	cmdMap::iterator index;

	if ((index = map.find(name)) != map.end()) {
		int id = std::distance(map.begin(), index);
		return id;
	}
	// XXX: throw
	printf("cmd::id NOT FOUND (%s)\n", name.c_str());
	return 60003;
}


Command* CommandManager::get(std::string name) {
	cmdMap::iterator index;

	if ((index = map.find(name)) != map.end()) {
		return index->second;
	}
	// XXX: throw
	printf("cmd::id NOT FOUND (%s)\n", name.c_str());
}




void CommandManager::call(agi::Context *c, const int id) {
	cmdMap::iterator index(map.begin());
	std::advance(index, id);

	if (index != map.end()) {
		LOG_D("event/command") << index->first << " " << "(Id: " << id << ")";
		(*index->second)(c);
	} else {
		LOG_W("event/command/not_found") << "EVENT ID NOT FOUND: " << id;
		// XXX: throw
	}
}


void CommandManager::reg(Command *cmd) {
	map.insert(cmdPair(cmd->name(), cmd));
}


// These forward declarations exist here since we don't want to expose
// them in a header, they're strictly internal-use.
void init_app(CommandManager *cm);
void init_audio(CommandManager *cm);
void init_automation(CommandManager *cm);
void init_command(CommandManager *cm);
void init_edit(CommandManager *cm);
void init_grid(CommandManager *cm);
void init_help(CommandManager *cm);
void init_keyframe(CommandManager *cm);
void init_medusa(CommandManager *cm);
void init_menu(CommandManager *cm);
void init_recent(CommandManager *cm);
void init_subtitle(CommandManager *cm);
void init_time(CommandManager *cm);
void init_timecode(CommandManager *cm);
void init_tool(CommandManager *cm);
void init_video(CommandManager *cm);

void init_command(CommandManager *cm) {
	LOG_D("command/init") << "Populating command map";
	init_app(cm);
	init_audio(cm);
	init_automation(cm);
	init_edit(cm);
	init_grid(cm);
	init_help(cm);
	init_keyframe(cm);
	init_medusa(cm);
	init_menu(cm);
	init_recent(cm);
	init_subtitle(cm);
	init_time(cm);
	init_timecode(cm);
	init_tool(cm);
	init_video(cm);
}

} // namespace cmd
