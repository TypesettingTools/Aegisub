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
//
// $Id$

/// @file auto4_lua.h
/// @see auto4_lua.cpp
/// @ingroup scripting
///

#ifndef AGI_PRE
#include <wx/event.h>
#include <wx/thread.h>
#endif

#include "compat.h"
#include "auto4_base.h"

#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lua.h"
#else
#include <lua.hpp>
#endif

class AssEntry;
class wxWindow;
namespace agi { namespace vfr { class Framerate; } }

namespace Automation4 {
	/// @class LuaAssFile
	/// @brief Object wrapping an AssFile object for modification through Lua
	class LuaAssFile {
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

		/// How ass file been modified by the script since the last commit
		int modification_type;

		/// Reference count used to avoid deleting this until both lua and the
		/// calling C++ code are done with it
		int references;

		/// Cursor for last access into file
		std::list<AssEntry*>::iterator last_entry_ptr;
		/// Index for last access into file
		int last_entry_id;
		/// Move last_entry_ptr to 1-based index n
		void SeekCursorTo(int n);

		int ObjectIndexRead(lua_State *L);
		void ObjectIndexWrite(lua_State *L);
		int ObjectGetLen(lua_State *L);
		void ObjectDelete(lua_State *L);
		void ObjectDeleteRange(lua_State *L);
		void ObjectAppend(lua_State *L);
		void ObjectInsert(lua_State *L);
		void ObjectGarbageCollect(lua_State *L);

		int LuaParseTagData(lua_State *L);
		int LuaUnparseTagData(lua_State *L);
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
		void ProcessingComplete(wxString const& undo_description = "");

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

	public:
		LuaProgressSink(lua_State *L, ProgressSink *ps, bool allow_config_dialog = true);
		~LuaProgressSink();

		static ProgressSink* GetObjPointer(lua_State *L, int idx);
	};



	/// DOCME
	/// @class LuaConfigDialogControl
	/// @brief DOCME
	///
	/// DOCME
	class LuaConfigDialogControl {
	public:

		/// DOCME
		wxControl *cw; // control window

		/// DOCME

		/// DOCME
		wxString name, hint;

		/// DOCME

		/// DOCME

		/// DOCME

		/// DOCME
		int x, y, width, height;

		virtual wxControl *Create(wxWindow *parent) = 0;
		virtual void ControlReadBack() = 0;
		virtual void LuaReadBack(lua_State *L) = 0;


		/// @brief DOCME
		/// @return 
		///
		virtual bool CanSerialiseValue() { return false; }

		/// @brief DOCME
		/// @return 
		///
		virtual wxString SerialiseValue() { return ""; }

		/// @brief DOCME
		/// @param serialised 
		///
		virtual void UnserialiseValue(const wxString &serialised) { }

		LuaConfigDialogControl(lua_State *L);

		/// @brief DOCME
		///
		virtual ~LuaConfigDialogControl() { }
	};


	/// DOCME
	/// @class LuaConfigDialog
	/// @brief DOCME
	///
	/// DOCME
	class LuaConfigDialog : public ScriptConfigDialog {
	private:

		/// DOCME
		std::vector<LuaConfigDialogControl*> controls;

		/// DOCME
		std::vector<wxString> buttons;

		/// DOCME
		bool use_buttons;


		/// DOCME
		/// @class ButtonEventHandler
		/// @brief DOCME
		///
		/// DOCME
		class ButtonEventHandler : public wxEvtHandler {
		public:

			/// DOCME
			int *button_pushed;
			void OnButtonPush(wxCommandEvent &evt);
		};


		/// DOCME
		ButtonEventHandler *button_event;

		/// DOCME
		int button_pushed;

	protected:
		wxWindow* CreateWindow(wxWindow *parent);

	public:
		LuaConfigDialog(lua_State *_L, bool include_buttons);
		virtual ~LuaConfigDialog();
		int LuaReadBack(lua_State *L); // read back internal structure to lua structures

		wxString Serialise();
		void Unserialise(const wxString &serialised);

		void ReadBack(); // from auto4 base
	};



	/// DOCME
	/// @class LuaFeature
	/// @brief DOCME
	///
	/// DOCME
	class LuaFeature : public virtual Feature {
	protected:

		/// DOCME
		lua_State *L;

		/// DOCME
		int myid;

		LuaFeature(lua_State *_L, ScriptFeatureClass _featureclass, const wxString &_name);

		void RegisterFeature();

		void GetFeatureFunction(int functionid);
		void CreateIntegerArray(const std::vector<int> &ints);
		void ThrowError();
	};

	void LuaThreadedCall(lua_State *L, int nargs, int nresults, wxString const& title, wxWindow *parent, bool can_open_config);

	/// DOCME
	/// @class LuaFeatureMacro
	/// @brief DOCME
	///
	/// DOCME
	class LuaFeatureMacro : public FeatureMacro, LuaFeature {
	private:

		/// DOCME
		bool no_validate;
	protected:
		LuaFeatureMacro(const wxString &_name, const wxString &_description, lua_State *_L);
	public:
		static int LuaRegister(lua_State *L);

		/// @brief DOCME
		///
		virtual ~LuaFeatureMacro() { }

		virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active);
		virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent);
	};

	/// DOCME
	/// @class LuaFeatureFilter
	/// @brief DOCME
	///
	/// DOCME
	class LuaFeatureFilter : public FeatureFilter, LuaFeature {
	private:

		/// DOCME
		bool has_config;

		/// DOCME
		LuaConfigDialog *config_dialog;

	protected:
		LuaFeatureFilter(const wxString &_name, const wxString &_description, int merit, lua_State *_L);

		ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent);

		void Init();
	public:
		static int LuaRegister(lua_State *L);


		/// @brief DOCME
		///
		virtual ~LuaFeatureFilter() { }

		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	};

	class LuaScript : public Script {
		lua_State *L;

		wxString name;
		wxString description;
		wxString author;
		wxString version;

		std::vector<Feature*> features;

		/// load script and create internal structures etc.
		void Create();
		/// destroy internal structures, unreg features and delete environment
		void Destroy();

		static int LuaTextExtents(lua_State *L);
		static int LuaInclude(lua_State *L);
		static int LuaModuleLoader(lua_State *L);
		static int LuaFrameFromMs(lua_State *L);
		static int LuaMsFromFrame(lua_State *L);
		static int LuaVideoSize(lua_State *L);

	public:
		LuaScript(const wxString &filename);
		~LuaScript();

		static LuaScript* GetScriptObject(lua_State *L);

		int RegisterFeature(Feature *feature);

		// Script implementation
		void Reload();

		wxString GetName() const { return name; }
		wxString GetDescription() const { return description; }
		wxString GetAuthor() const { return author; }
		wxString GetVersion() const { return version; }
		bool GetLoadedState() const { return L != 0; }

		std::vector<Feature*> GetFeatures() const { return features; }
	};
};
