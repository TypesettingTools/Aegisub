// Copyright (c) 2006, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file auto4_lua.h
/// @see auto4_lua.cpp
/// @ingroup scripting
///

#include "auto4_base.h"

#include "command/command.h"

#include <deque>

#include <wx/event.h>
#include <wx/thread.h>

class AssEntry;
class wxWindow;
struct lua_State;
namespace agi { namespace vfr { class Framerate; } }

namespace Automation4 {
	/// @class LuaAssFile
	/// @brief Object wrapping an AssFile object for modification through Lua
	class LuaAssFile {
		struct PendingCommit {
			wxString mesage;
			int modification_type;
			std::vector<AssEntry*> lines;
		};

		/// Pointer to file being modified
		AssFile *ass;

		/// Lua state the object exists in
		lua_State *L;

		/// Is the feature this object is created for read-only?
		bool can_modify;
		/// Is the feature allowed to set undo points?
		bool can_set_undo;
		/// throws an error if modification is disallowed
		void CheckAllowModify();
		/// throws an error if the line index is out of bounds
		void CheckBounds(int idx);

		/// How ass file been modified by the script since the last commit
		int modification_type;

		/// Reference count used to avoid deleting this until both lua and the
		/// calling C++ code are done with it
		int references;

		/// Set of subtitle lines being modified; initially a shallow copy of ass->Line
		std::vector<AssEntry*> lines;
		/// Commits to apply once processing completes successfully
		std::deque<PendingCommit> pending_commits;
		/// Lines to delete once processing complete successfully
		std::deque<AssEntry*> lines_to_delete;

		int ObjectIndexRead(lua_State *L);
		void ObjectIndexWrite(lua_State *L);
		int ObjectGetLen(lua_State *L);
		void ObjectDelete(lua_State *L);
		void ObjectDeleteRange(lua_State *L);
		void ObjectAppend(lua_State *L);
		void ObjectInsert(lua_State *L);
		void ObjectGarbageCollect(lua_State *L);
		int ObjectIPairs(lua_State *L);
		int IterNext(lua_State *L);

		int LuaParseKaraokeData(lua_State *L);

		void LuaSetUndoPoint(lua_State *L);

		// LuaAssFile can only be deleted by the reference count hitting zero
		~LuaAssFile() { }
	public:
		static LuaAssFile *GetObjPointer(lua_State *L, int idx);

		/// makes a Lua representation of AssEntry and places on the top of the stack
		static void AssEntryToLua(lua_State *L, AssEntry *e);
		/// assumes a Lua representation of AssEntry on the top of the stack, and creates an AssEntry object of it
		static AssEntry *LuaToAssEntry(lua_State *L);

		/// @brief Signal that the script using this file is now done running
		/// @param set_undo If there's any uncommitted changes to the file,
		///                 they will be automatically committed with this
		///                 description
		void ProcessingComplete(wxString const& undo_description = wxString());

		/// End processing without applying any changes made
		void Cancel();

		/// Constructor
		/// @param L lua state
		/// @param ass File to wrap
		/// @param can_modify Is modifying the file allowed?
		/// @param can_set_undo Is setting undo points allowed?
		LuaAssFile(lua_State *L, AssFile *ass, bool can_modify = false, bool can_set_undo = false);
	};

	class LuaProgressSink {
		lua_State *L;

		static int LuaSetProgress(lua_State *L);
		static int LuaSetTask(lua_State *L);
		static int LuaSetTitle(lua_State *L);
		static int LuaGetCancelled(lua_State *L);
		static int LuaDebugOut(lua_State *L);
		static int LuaDisplayDialog(lua_State *L);
		static int LuaDisplayOpenDialog(lua_State *L);
		static int LuaDisplaySaveDialog(lua_State *L);

	public:
		LuaProgressSink(lua_State *L, ProgressSink *ps, bool allow_config_dialog = true);
		~LuaProgressSink();

		static ProgressSink* GetObjPointer(lua_State *L, int idx);
	};

	/// Base class for controls in dialogs
	class LuaDialogControl {
	public:
		/// Name of this control in the output table
		std::string name;

		/// Tooltip of this control
		std::string hint;

		int x, y, width, height;

		/// Create the associated wxControl
		virtual wxControl *Create(wxWindow *parent) = 0;

		/// Get the default flags to use when inserting this control into a sizer
		virtual int GetSizerFlags() const { return wxEXPAND; }

		/// Push the current value of the control onto the lua stack. Must not
		/// touch the GUI as this may be called on a background thread.
		virtual void LuaReadBack(lua_State *L) = 0;

		/// Does this control have any user-changeable data that can be serialized?
		virtual bool CanSerialiseValue() const { return false; }

		/// Serialize the control's current value so that it can be stored
		/// in the script
		virtual std::string SerialiseValue() const { return ""; }

		/// Restore the control's value from a saved value in the script
		virtual void UnserialiseValue(const std::string &serialised) { }

		LuaDialogControl(lua_State *L);

		/// Virtual destructor so this can safely be inherited from
		virtual ~LuaDialogControl() { }
	};

	/// A lua-generated dialog or panel in the export options dialog
	class LuaDialog : public ScriptDialog {
		/// Controls in this dialog
		std::vector<LuaDialogControl*> controls;
		/// The names and IDs of buttons in this dialog if non-default ones were used
		std::vector<std::pair<int, std::string>> buttons;

		/// Does the dialog contain any buttons
		bool use_buttons;

		/// Id of the button pushed (once a button has been pushed)
		int button_pushed;

		wxWindow *window;

	public:
		LuaDialog(lua_State *L, bool include_buttons);
		~LuaDialog();

		/// Push the values of the controls in this dialog onto the lua stack
		/// in a single table
		int LuaReadBack(lua_State *L);

		// ScriptDialog implementation
		wxWindow* CreateWindow(wxWindow *parent);
		std::string Serialise();
		void Unserialise(const std::string &serialised);
	};

	class LuaFeature {
		int myid;
	protected:
		lua_State *L;

		void RegisterFeature();
		void UnregisterFeature();

		void GetFeatureFunction(const char *function) const;

		LuaFeature(lua_State *L);
	};

	/// Run a lua function on a background thread
	/// @param L Lua state
	/// @param nargs Number of arguments the function takes
	/// @param nresults Number of values the function returns
	/// @param title Title to use for the progress dialog
	/// @param parent Parent window for the progress dialog
	/// @param can_open_config Can the function open its own dialogs?
	/// @throws agi::UserCancelException if the function fails to run to completion (either due to cancelling or errors)
	void LuaThreadedCall(lua_State *L, int nargs, int nresults, std::string const& title, wxWindow *parent, bool can_open_config);

	class LuaCommand : public cmd::Command, private LuaFeature {
		std::string cmd_name;
		wxString display;
		wxString help;
		int cmd_type;

	public:
		LuaCommand(lua_State *L);
		~LuaCommand();

		const char* name() const { return cmd_name.c_str(); }
		wxString StrMenu(const agi::Context *) const { return display; }
		wxString StrDisplay(const agi::Context *) const { return display; }
		wxString StrHelp() const { return help; }

		int Type() const { return cmd_type; }

		void operator()(agi::Context *c);
		bool Validate(const agi::Context *c);
		virtual bool IsActive(const agi::Context *c);

		static int LuaRegister(lua_State *L);
	};

	class LuaExportFilter : public ExportFilter, private LuaFeature {
		bool has_config;
		LuaDialog *config_dialog;

	protected:
		ScriptDialog* GenerateConfigDialog(wxWindow *parent, agi::Context *c);

	public:
		LuaExportFilter(lua_State *L);
		static int LuaRegister(lua_State *L);

		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	};
}
