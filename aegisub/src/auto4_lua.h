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

#include "auto4_base.h"

#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lua.h"
#include "../../contrib/lua51/src/lauxlib.h"
#else
#include <lua.hpp>
#endif

class wxWindow;


/// DOCME
namespace Automation4 {


	/// DOCME
	/// @class LuaAssFile
	/// @brief DOCME
	///
	/// DOCME
	class LuaAssFile {
	private:

		/// DOCME
		AssFile *ass;

		/// DOCME
		lua_State *L;


		/// DOCME
		bool can_modify;

		/// DOCME
		bool can_set_undo;
		void CheckAllowModify(); // throws an error if modification is disallowed


		/// DOCME
		std::list<AssEntry*>::iterator last_entry_ptr;

		/// DOCME
		int last_entry_id;
		void GetAssEntry(int n); // set last_entry_ptr to point to item n

		static int ObjectIndexRead(lua_State *L);
		static int ObjectIndexWrite(lua_State *L);
		static int ObjectGetLen(lua_State *L);
		static int ObjectDelete(lua_State *L);
		static int ObjectDeleteRange(lua_State *L);
		static int ObjectAppend(lua_State *L);
		static int ObjectInsert(lua_State *L);
		static int ObjectGarbageCollect(lua_State *L);

		static int LuaParseTagData(lua_State *L);
		static int LuaUnparseTagData(lua_State *L);
		static int LuaParseKaraokeData(lua_State *L);
		static int LuaSetUndoPoint(lua_State *L);

		static LuaAssFile *GetObjPointer(lua_State *L, int idx);

		~LuaAssFile();
	public:
		static void AssEntryToLua(lua_State *L, AssEntry *e); // makes a Lua representation of AssEntry and places on the top of the stack
		static AssEntry *LuaToAssEntry(lua_State *L); // assumes a Lua representation of AssEntry on the top of the stack, and creates an AssEntry object of it

		LuaAssFile(lua_State *_L, AssFile *_ass, bool _can_modify, bool _can_set_undo);
	};



	/// DOCME
	/// @class LuaProgressSink
	/// @brief DOCME
	///
	/// DOCME
	class LuaProgressSink : public ProgressSink {
	private:

		/// DOCME
		lua_State *L;

		static int LuaSetProgress(lua_State *L);
		static int LuaSetTask(lua_State *L);
		static int LuaSetTitle(lua_State *L);
		static int LuaGetCancelled(lua_State *L);
		static int LuaDebugOut(lua_State *L);
		static int LuaDisplayDialog(lua_State *L);

	public:
		LuaProgressSink(lua_State *_L, wxWindow *parent, bool allow_config_dialog = true);
		virtual ~LuaProgressSink();

		static LuaProgressSink* GetObjPointer(lua_State *L, int idx);
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
		virtual wxString SerialiseValue() { return _T(""); }

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



	/// DOCME
	/// @class LuaScript
	/// @brief DOCME
	///
	/// DOCME
	class LuaScript : public Script {
		friend class LuaFeature;

	private:

		/// DOCME
		lua_State *L;

		void Create(); // load script and create internal structures etc.
		void Destroy(); // destroy internal structures, unreg features and delete environment

		static LuaScript* GetScriptObject(lua_State *L);

		static int LuaTextExtents(lua_State *L);
		static int LuaInclude(lua_State *L);
		static int LuaModuleLoader(lua_State *L);
		static int LuaFrameFromMs(lua_State *L);
		static int LuaMsFromFrame(lua_State *L);
		static int LuaVideoSize(lua_State *L);

	public:
		LuaScript(const wxString &filename);
		virtual ~LuaScript();

		virtual void Reload();
	};



	/// DOCME
	/// @class LuaThreadedCall
	/// @brief DOCME
	///
	/// DOCME
	class LuaThreadedCall : public wxThread {
	private:

		/// DOCME
		lua_State *L;

		/// DOCME
		int nargs;

		/// DOCME
		int nresults;
	public:
		LuaThreadedCall(lua_State *_L, int _nargs, int _nresults);
		virtual ExitCode Entry();
	};



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

};
