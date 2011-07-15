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

		/// Check whether or not it makes sense to call this command at this time
		/// @param c Project context
		///
		/// This function should be very fast, as it is called whenever a menu
		/// containing this command is opened and is called periodically for
		/// any commands used in a toolbar
		///
		/// Note that it is still legal to call commands when this returns
		/// false. In this situation, commands should do nothing.
		virtual bool Validate(const agi::Context *c) { return true; }

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
} // namespace cmd
