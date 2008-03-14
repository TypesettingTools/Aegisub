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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

#pragma once

#ifndef _AUTO4_LUA_H
#define _AUTO4_LUA_H

#include "auto4_base.h"
#include <wx/thread.h>
#include <wx/event.h>

#ifdef __WINDOWS__
#include "../lua51/src/lua.h"
#include "../lua51/src/lauxlib.h"
#else
#include "lua.hpp"
#endif

class wxWindow;

namespace Automation4 {

	// Provides access to an AssFile object (and all lines contained) for a Lua script
	class LuaAssFile {
	private:
		AssFile *ass;
		lua_State *L;

		bool can_modify;
		bool can_set_undo;
		void CheckAllowModify(); // throws an error if modification is disallowed

		// keep a cursor of last accessed item to avoid walking over the entire file on every access
		std::list<AssEntry*>::iterator last_entry_ptr;
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


	// Provides progress UI and control functions for a Lua script
	class LuaProgressSink : public ProgressSink {
	private:
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


	// Provides Config UI functions for a Lua script
	class LuaConfigDialogControl {
	public:
		wxControl *cw; // control window
		wxString name, hint;
		int x, y, width, height;

		virtual wxControl *Create(wxWindow *parent) = 0;
		virtual void ControlReadBack() = 0;
		virtual void LuaReadBack(lua_State *L) = 0;

		virtual bool CanSerialiseValue() { return false; }
		virtual wxString SerialiseValue() { return _T(""); }
		virtual void UnserialiseValue(const wxString &serialised) { }

		LuaConfigDialogControl(lua_State *L);
		virtual ~LuaConfigDialogControl() { }
	};

	class LuaConfigDialog : public ScriptConfigDialog {
	private:
		std::vector<LuaConfigDialogControl*> controls;
		std::vector<wxString> buttons;
		bool use_buttons;

		class ButtonEventHandler : public wxEvtHandler {
		public:
			int *button_pushed;
			void OnButtonPush(wxCommandEvent &evt);
		};

		ButtonEventHandler *button_event;
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


	// Second base-class for Lua implemented Features
	class LuaFeature : public virtual Feature {
	protected:
		lua_State *L;
		int myid;

		LuaFeature(lua_State *_L, ScriptFeatureClass _featureclass, const wxString &_name);

		void RegisterFeature();

		void GetFeatureFunction(int functionid);
		void CreateIntegerArray(const std::vector<int> &ints);
		void ThrowError();
	};


	// Class of Lua scripts
	class LuaScript : public Script {
		friend class LuaFeature;

	private:
		lua_State *L;

		void Create(); // load script and create internal structures etc.
		void Destroy(); // destroy internal structures, unreg features and delete environment

		static LuaScript* GetScriptObject(lua_State *L);

		static int LuaTextExtents(lua_State *L);
		static int LuaInclude(lua_State *L);
		static int LuaFrameFromMs(lua_State *L);
		static int LuaMsFromFrame(lua_State *L);

	public:
		LuaScript(const wxString &filename);
		virtual ~LuaScript();

		virtual void Reload();
	};


	// A single call to a Lua function, run inside a separate thread.
	// This object should be created on the stack in the function that does the call.
	class LuaThreadedCall : public wxThread {
	private:
		lua_State *L;
		int nargs;
		int nresults;
	public:
		LuaThreadedCall(lua_State *_L, int _nargs, int _nresults);
		virtual ExitCode Entry();
	};


	// Implementation of the Macro Feature for Lua scripts
	class LuaFeatureMacro : public FeatureMacro, LuaFeature {
	private:
		bool no_validate;
	protected:
		LuaFeatureMacro(const wxString &_name, const wxString &_description, lua_State *_L);
	public:
		static int LuaRegister(lua_State *L);
		virtual ~LuaFeatureMacro() { }

		virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active);
		virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent);
	};


	// Implementation of the Export Filter Feature for Lua scripts
	class LuaFeatureFilter : public FeatureFilter, LuaFeature {
	private:
		bool has_config;
		LuaConfigDialog *config_dialog;

	protected:
		LuaFeatureFilter(const wxString &_name, const wxString &_description, int merit, lua_State *_L);

		ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent);

		void Init();
	public:
		static int LuaRegister(lua_State *L);

		virtual ~LuaFeatureFilter() { }

		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	};

};

#endif
