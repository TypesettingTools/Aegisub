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


#ifndef AGI_PRE
#include <map>
#include <string>

#include <wx/bitmap.h>
#include <wx/string.h>
#endif

#include <libaegisub/exception.h>

namespace agi { struct Context; }

DEFINE_BASE_EXCEPTION_NOINNER(CommandError, agi::Exception)
DEFINE_SIMPLE_EXCEPTION_NOINNER(CommandNotFound, CommandError, "command/notfound")
DEFINE_SIMPLE_EXCEPTION_NOINNER(CommandIconNone, CommandError, "command/icon")
DEFINE_SIMPLE_EXCEPTION_NOINNER(CommandIconInvalid, CommandError, "command/icon/invalid")

#define CMD_NAME(a) const char* name() { return a; }
#define STR_MENU(a) wxString StrMenu() const { return a; }
#define STR_DISP(a) wxString StrDisplay() const { return a; }
#define STR_HELP(a) wxString StrHelp() const { return a; }
#define CMD_TYPE(a) int Type() const { using namespace cmd; return a; }

#define COMMAND_GROUP(cname, cmdname, menu, disp, help) \
struct cname : public Command {                         \
	CMD_NAME(cmdname)                                   \
	STR_MENU(menu)                                      \
	STR_DISP(disp)                                      \
	STR_HELP(help)                                      \
	void operator()(agi::Context *) { }                 \
}

/// Commands
namespace cmd {
	enum CommandFlags {
		/// Default command type
		COMMAND_NORMAL       = 0,

		/// Invoking this command toggles a setting of some sort. Any command
		/// of this type should have IsActive implemented to signal the
		/// current state of the thing being toggled, and invoking the command
		/// twice should be a no-op
		///
		/// This is mutually exclusive with COMMAND_RADIO
		COMMAND_TOGGLE       = 1,

		/// Invoking this command sets a setting to a specific value. Any
		/// command of this type should have IsActive implemented, and if
		/// IsActive returns true, invoking the command should have no effect
		///
		/// This is mutually exclusive with COMMAND_TOGGLE
		COMMAND_RADIO        = 2,

		/// This command has an overridden Validate method
		COMMAND_VALIDATE     = 4,

		/// This command's name may change based on the state of the project
		COMMAND_DYNAMIC_NAME = 8,

		/// This command's icon may change based on the state of the project
		COMMAND_DYNAMIC_ICON = 16
	};

	/// Holds an individual Command
	class Command {
	public:
		virtual const char* name()=0;				///< Command name.
		virtual wxString StrMenu() const=0;			///< String for menu purposes including accelerators.
		virtual wxString StrDisplay() const=0;		///< Plain string for display purposes.
		virtual wxString StrHelp() const=0;			///< Short help string descripting the command purpose.

		/// Get this command's type flags
		/// @return Bitmask of CommandFlags
		virtual int Type() const { return COMMAND_NORMAL; }

		/// Request icon.
		/// @param size Icon size.
		wxBitmap const& Icon(int size);

		/// Command function
		virtual void operator()(agi::Context *c)=0;

		/// Check whether or not it makes sense to call this command at this time
		/// @param c Project context
		///
		/// This function should be very fast, as it is called whenever a menu
		/// containing this command is opened and is called periodically for
		/// any commands used in a toolbar
		///
		/// Note that it is still legal to call commands when this returns
		/// false. In this situation, commands should do nothing.
		///
		/// This function should be overridden iff the command's type flags
		/// include COMMAND_VALIDATE
		virtual bool Validate(const agi::Context *c) { return true; }

		/// Is the selectable value represented by this command currently selected?
		/// @param c Project context
		///
		/// As with Validate, this function should be very fast.
		///
		/// This function should be overridden iff the command's type flags
		/// include COMMAND_TOGGLE or COMMAND_RADIO
		virtual bool IsActive(const agi::Context *c) { return false; }

		/// Destructor
		virtual ~Command() { };
	};

	/// Init all builtin commands.
	void init_builtin_commands();

	/// Register a command.
	/// @param cmd Command object.
	void reg(Command *cmd);

	/// Retrieve an ID for event usage or otherwise
	/// @param name Command name
	/// @return Command ID
	/// @note This is guaranteed to be unique.
	int id(std::string const& name);

	/// Call a command.
	/// @param c  Current Context.
	/// @param id ID for Command to call.
	void call(agi::Context *c, int id);

	/// Count number of commands.
	/// @return ID number.
	int count();

	/// Retrieve a Command object.
	/// @param Command object.
	Command* get(std::string const& name);

	/// Unregister all commands
	void clear();
} // namespace cmd
