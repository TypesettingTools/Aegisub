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

/// @file command.h
/// @brief Command base class and main header.
/// @ingroup command

#include "../include/aegisub/context.h"
#include "icon.h"

DEFINE_BASE_EXCEPTION_NOINNER(CommandError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(CommandIconNone, CommandError, "command/icon")
DEFINE_SIMPLE_EXCEPTION_NOINNER(CommandIconInvalid, CommandError, "command/icon/invalid")

#define CMD_NAME(a) const char* name() { return a; }
#define STR_MENU(a) wxString StrMenu() const { return a; }
#define STR_DISP(a) wxString StrDisplay() const { return a; }
#define STR_HELP(a) wxString StrHelp() const { return a; }

/// Commands
namespace cmd {
	class CommandManager;
	class Command;

	/// CommandManager instance.
	extern CommandManager *cm;

	/// Init all commands.
	/// @param cm CommandManager instance.
	void init_command(CommandManager *cm);

	// The following are nothing more than glorified macros.
	int id(std::string name);					///< @see CommandManager::id
	void call(agi::Context *c, const int id);	///< @see CommandManager::call
	int count();								///< @see CommandManager::count
	Command* get(std::string name);				///< @see CommandManager::get


	/// Holds an individual Command
	class Command {
	public:
		virtual const char* name()=0;				///< Command name.
		virtual wxString StrMenu() const=0;			///< String for menu purposes including accelerators.
		virtual wxString StrDisplay() const=0;		///< Plain string for display purposes.
		virtual wxString StrHelp() const=0;			///< Short help string descripting the command purpose.

		/// Request icon.
		/// @param size Icon size.
		wxBitmap* Icon(int size);

		/// Command function
		virtual void operator()(agi::Context *c)=0;

		/// Destructor
		virtual ~Command() {};
	};


	/// Manager for commands
	class CommandManager {
		typedef std::map<std::string, Command*> cmdMap;		///< Map to hold commands.
		typedef std::pair<std::string, Command*> cmdPair;	///< Pair for command insertion.
		cmdMap map;											///< Actual map.

	public:
		/// Register a command.
		/// @param cmd Command object.
		void reg(Command *cmd);

		/// Retrieve an ID for event usage or otherwise
		/// @param name Command name
		/// @return Command ID
		/// @note This is guaranteed to be unique.
		int id(std::string name);

		/// Call a command.
		/// @param c  Current Context.
		/// @param id ID for Command to call.
		void call(agi::Context *c, const int id);

		/// Count number of commands.
		/// @return ID number.
		int count() { return map.size(); }

		/// Retrieve a Command object.
		/// @param Command object.
		Command* get(std::string name);
	};
} // namespace cmd
