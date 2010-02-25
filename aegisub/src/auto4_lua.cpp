// Copyright (c) 2006, 2007, Niels Martin Hansen
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

/// @file auto4_lua.cpp
/// @brief Lua 5.1-based scripting engine
/// @ingroup scripting
///

#include "config.h"

#ifdef WITH_AUTO4_LUA

#ifndef AGI_PRE
#include <assert.h>

#include <algorithm>

#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>
#include <wx/window.h>
#endif

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_override.h"
#include "ass_style.h"
#include "auto4_lua.h"
#include "auto4_lua_factory.h"
#include "auto4_lua_scriptreader.h"
#include "options.h"
#include "standard_paths.h"
#include "text_file_reader.h"
#include "vfr.h"
#include "video_context.h"

// This must be below the headers above.
#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lualib.h"
#include "../../contrib/lua51/src/lauxlib.h"
#else
#include <lualib.h>
#include <lauxlib.h>
#endif


/// DOCME
namespace Automation4 {

	// LuaStackcheck

//#ifdef _DEBUG
#if 0
	struct LuaStackcheck {
		lua_State *L;
		int startstack;
		void check_stack(int additional)
		{
			int top = lua_gettop(L);
			if (top - additional != startstack) {
				wxLogDebug(_T("Lua stack size mismatch."));
				dump();
				assert(top - additional == startstack);
			}
		}
		void dump()
		{
			int top = lua_gettop(L);
			wxLogDebug(_T("Dumping Lua stack..."));
			for (int i = top; i > 0; i--) {
				lua_pushvalue(L, i);
				wxString type(lua_typename(L, lua_type(L, -1)), wxConvUTF8);
				if (lua_isstring(L, i)) {
					wxLogDebug(type + _T(": ") + wxString(lua_tostring(L, -1), wxConvUTF8));
				} else {
					wxLogDebug(type);
				}
				lua_pop(L, 1);
			}
			wxLogDebug(_T("--- end dump"));
		}
		LuaStackcheck(lua_State *_L) : L(_L) { startstack = lua_gettop(L); }
		~LuaStackcheck() { check_stack(0); }
	};
#else

	/// DOCME
	struct LuaStackcheck {

		/// @brief DOCME
		/// @param additional 
		///
		void check_stack(int additional) { }

		/// @brief DOCME
		///
		void dump() { }

		/// @brief DOCME
		/// @param L 
		///
		LuaStackcheck(lua_State *L) { }

		/// @brief DOCME
		///
		~LuaStackcheck() { }
	};
#endif


	// LuaScript


	/// @brief DOCME
	/// @param filename 
	///
	LuaScript::LuaScript(const wxString &filename)
		: Script(filename)
		, L(0)
	{
		Create();
	}


	/// @brief DOCME
	///
	LuaScript::~LuaScript()
	{
		if (L) Destroy();
	}


	/// @brief DOCME
	///
	void LuaScript::Create()
	{
		Destroy();

		loaded = true;

		try {
			// create lua environment
			L = lua_open();
			LuaStackcheck _stackcheck(L);

			// register standard libs
			lua_pushcfunction(L, luaopen_base); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_package); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_table); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_math); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_io); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_os); lua_call(L, 0, 0);
			_stackcheck.check_stack(0);
			// dofile and loadfile are replaced with include
			lua_pushnil(L);
			lua_setglobal(L, "dofile");
			lua_pushnil(L);
			lua_setglobal(L, "loadfile");
			lua_pushcfunction(L, LuaInclude);
			lua_setglobal(L, "include");

			// add include_path to the module load path
			lua_getglobal(L, "package");
			lua_pushstring(L, "path");
			lua_pushstring(L, "path");
			lua_gettable(L, -3);

			wxStringTokenizer toker(Options.AsText(_T("Automation Include Path")), _T("|"), wxTOKEN_STRTOK);
			while (toker.HasMoreTokens()) {
				wxFileName path(StandardPaths::DecodePath(toker.GetNextToken()));
				if (path.IsOk() && !path.IsRelative() && path.DirExists()) {
					wxCharBuffer p = path.GetLongPath().utf8_str();
					lua_pushfstring(L, ";%s?.lua;%s?/init.lua", p.data(), p.data());
					lua_concat(L, 2);
				}
			}

			lua_settable(L, -3);

			// Replace the default lua module loader with our utf-8 compatible one
			lua_getfield(L, -1, "loaders");
			lua_pushcfunction(L, LuaModuleLoader);
			lua_rawseti(L, -2, 2);
			lua_pop(L, 2);
			_stackcheck.check_stack(0);

			// prepare stuff in the registry
			// reference to the script object
			lua_pushlightuserdata(L, this);
			lua_setfield(L, LUA_REGISTRYINDEX, "aegisub");
			// the "feature" table
			// integer indexed, using same indexes as "features" vector in the base Script class
			lua_newtable(L);
			lua_setfield(L, LUA_REGISTRYINDEX, "features");
			_stackcheck.check_stack(0);

			// make "aegisub" table
			lua_pushstring(L, "aegisub");
			lua_newtable(L);
			// aegisub.register_macro
			lua_pushcfunction(L, LuaFeatureMacro::LuaRegister);
			lua_setfield(L, -2, "register_macro");
			// aegisub.register_filter
			lua_pushcfunction(L, LuaFeatureFilter::LuaRegister);
			lua_setfield(L, -2, "register_filter");
			// aegisub.text_extents
			lua_pushcfunction(L, LuaTextExtents);
			lua_setfield(L, -2, "text_extents");
			// VFR handling
			lua_pushcfunction(L, LuaFrameFromMs);
			lua_setfield(L, -2, "frame_from_ms");
			lua_pushcfunction(L, LuaMsFromFrame);
			lua_setfield(L, -2, "ms_from_frame");
			lua_pushcfunction(L, LuaVideoSize);
			lua_setfield(L, -2, "video_size");
			// aegisub.lua_automation_version
			lua_pushinteger(L, 4);
			lua_setfield(L, -2, "lua_automation_version");
			// store aegisub table to globals
			lua_settable(L, LUA_GLOBALSINDEX);
			_stackcheck.check_stack(0);

			// load user script
			LuaScriptReader script_reader(GetFilename());
			if (lua_load(L, script_reader.reader_func, &script_reader, GetPrettyFilename().mb_str(wxConvUTF8))) {
				wxString err(lua_tostring(L, -1), wxConvUTF8);
				err.Prepend(_T("Error loading Lua script \"") + GetPrettyFilename() + _T("\":\n\n"));
				throw err;
			}
			_stackcheck.check_stack(1);
			// and execute it
			// this is where features are registered
			// don't thread this, as there's no point in it and it seems to break on wx 2.8.3, for some reason
			if (lua_pcall(L, 0, 0, 0)) {
				// error occurred, assumed to be on top of Lua stack
				wxString err(lua_tostring(L, -1), wxConvUTF8);
				err.Prepend(_T("Error initialising Lua script \"") + GetPrettyFilename() + _T("\":\n\n"));
				throw err;
			}
			_stackcheck.check_stack(0);
			lua_getglobal(L, "version");
			if (lua_isnumber(L, -1)) {
				if (lua_tointeger(L, -1) == 3) {
					lua_pop(L, 1); // just to avoid tripping the stackcheck in debug
					// So this is an auto3 script...
					// Throw it as an exception, the script factory manager will catch this and use the auto3 script instead of this script object
					throw _T("Attempted to load an Automation 3 script as an Automation 4 Lua script. Automation 3 is no longer supported.");
				}
			}
			lua_getglobal(L, "script_name");
			if (lua_isstring(L, -1)) {
				name = wxString(lua_tostring(L, -1), wxConvUTF8);
			} else {
				name = GetPrettyFilename();
			}
			lua_getglobal(L, "script_description");
			if (lua_isstring(L, -1)) {
				description = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_getglobal(L, "script_author");
			if (lua_isstring(L, -1)) {
				author = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_getglobal(L, "script_version");
			if (lua_isstring(L, -1)) {
				version = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			lua_pop(L, 5);
			// if we got this far, the script should be ready
			_stackcheck.check_stack(0);

		}
		catch (const char *e) {
			Destroy();
			loaded = false;
			name = GetPrettyFilename();
			description = wxString(e, wxConvUTF8);
		}
		catch (const wchar_t *e) {
			Destroy();
			loaded = false;
			name = GetPrettyFilename();
			description = e;
		}
		catch (const wxString& e) {
			Destroy();
			loaded = false;
			name = GetPrettyFilename();
			description = e;
		}
		catch (Script *s) {
			// Be sure to properly propagate any scripts throw
			throw s;
		}
		catch (...) {
			Destroy();
			loaded = false;
			name = GetPrettyFilename();
			description = _T("Unknown error initialising Lua script");
		}
	}


	/// @brief DOCME
	/// @return 
	///
	void LuaScript::Destroy()
	{
		// Assume the script object is clean if there's no Lua state
		if (!L) return;

		// remove features
		for (int i = 0; i < (int)features.size(); i++) {
			Feature *f = features[i];
			delete f;
		}
		features.clear();

		// delete environment
		lua_close(L);
		L = 0;

		loaded = false;
	}


	/// @brief DOCME
	///
	void LuaScript::Reload()
	{
		Destroy();
		Create();
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	LuaScript* LuaScript::GetScriptObject(lua_State *L)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "aegisub");
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return (LuaScript*)ptr;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaTextExtents(lua_State *L)
	{
		if (!lua_istable(L, 1)) {
			lua_pushstring(L, "First argument to text_extents must be a table");
			lua_error(L);
		}
		if (!lua_isstring(L, 2)) {
			lua_pushstring(L, "Second argument to text_extents must be a string");
			lua_error(L);
		}

		lua_pushvalue(L, 1);
		AssEntry *et = LuaAssFile::LuaToAssEntry(L);
		AssStyle *st = dynamic_cast<AssStyle*>(et);
		lua_pop(L, 1);
		if (!st) {
			delete et; // Make sure to delete the "live" pointer
			lua_pushstring(L, "Not a style entry");
			lua_error(L);
		}

		wxString text(lua_tostring(L, 2), wxConvUTF8);

		double width, height, descent, extlead;
		if (!CalculateTextExtents(st, text, width, height, descent, extlead)) {
			delete st;
			lua_pushstring(L, "Some internal error occurred calculating text_extents");
			lua_error(L);
		}
		delete st;

		lua_pushnumber(L, width);
		lua_pushnumber(L, height);
		lua_pushnumber(L, descent);
		lua_pushnumber(L, extlead);
		return 4;
	}

	/// @brief Module loader which uses our include rather than Lua's, for unicode file support
	/// @param L The Lua state
	/// @return Always 1 per loader_Lua?
	int LuaScript::LuaModuleLoader(lua_State *L)
	{
		int pretop = lua_gettop(L);
		wxString module(lua_tostring(L, -1), wxConvUTF8);
		module.Replace(".", LUA_DIRSEP);

		lua_getglobal(L, "package");
		lua_pushstring(L, "path");
		lua_gettable(L, -2);
		wxString package_paths(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 2);

		wxStringTokenizer toker(package_paths, L";", wxTOKEN_STRTOK);
		while (toker.HasMoreTokens()) {
			wxString filename = toker.GetNextToken();
			filename.Replace(L"?", module);
			if (wxFileName::FileExists(filename)) {
				LuaScriptReader script_reader(filename);
				if (lua_load(L, script_reader.reader_func, &script_reader, filename.utf8_str())) {
					lua_pushfstring(L, "Error loading Lua module \"%s\":\n\n%s", filename.utf8_str().data(), lua_tostring(L, -1));
					lua_error(L);
					return lua_gettop(L) - pretop;
				}
			}
		}
		return lua_gettop(L) - pretop;
	}

	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaInclude(lua_State *L)
	{
		LuaScript *s = GetScriptObject(L);

		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "Argument to include must be a string");
			lua_error(L);
			return 0;
		}
		wxString fnames(lua_tostring(L, 1), wxConvUTF8);

		wxFileName fname(fnames);
		if (fname.GetDirCount() == 0) {
			// filename only
			fname = s->include_path.FindAbsoluteValidPath(fnames);
		} else if (fname.IsRelative()) {
			// relative path
			wxFileName sfname(s->GetFilename());
			fname.MakeAbsolute(sfname.GetPath(true));
		} else {
			// absolute path, do nothing
		}
		if (!fname.IsOk() || !fname.FileExists()) {
			lua_pushfstring(L, "Lua include not found: %s", fnames.mb_str(wxConvUTF8).data());
			lua_error(L);
		}

		LuaScriptReader script_reader(fname.GetFullPath());
		if (lua_load(L, script_reader.reader_func, &script_reader, fname.GetFullName().mb_str(wxConvUTF8))) {
			lua_pushfstring(L, "Error loading Lua include \"%s\":\n\n%s", fname.GetFullPath().mb_str(wxConvUTF8).data(), lua_tostring(L, -1));
			lua_error(L);
			return 0;
		}
		int pretop = lua_gettop(L) - 1; // don't count the function value itself
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - pretop;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaFrameFromMs(lua_State *L)
	{
		int ms = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		if (VFR_Output.IsLoaded()) {
			lua_pushnumber(L, VFR_Output.GetFrameAtTime(ms, true));
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaMsFromFrame(lua_State *L)
	{
		int frame = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		if (VFR_Output.IsLoaded()) {
			lua_pushnumber(L, VFR_Output.GetTimeAtFrame(frame, true));
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaScript::LuaVideoSize(lua_State *L)
	{
		VideoContext *ctx = VideoContext::Get();
		if (ctx->IsLoaded()) {
			lua_pushnumber(L, ctx->GetWidth());
			lua_pushnumber(L, ctx->GetHeight());
			lua_pushnumber(L, ctx->GetAspectRatioValue());
			lua_pushnumber(L, ctx->GetAspectRatioType());
			return 4;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}


	// LuaThreadedCall


	/// @brief DOCME
	/// @param _L        
	/// @param _nargs    
	/// @param _nresults 
	///
	LuaThreadedCall::LuaThreadedCall(lua_State *_L, int _nargs, int _nresults)
		: wxThread(wxTHREAD_JOINABLE)
		, L(_L)
		, nargs(_nargs)
		, nresults(_nresults)
	{
		int prio = Options.AsInt(_T("Automation Thread Priority"));
		if (prio == 0) prio = 50; // normal
		else if (prio == 1) prio = 30; // below normal
		else if (prio == 2) prio = 10; // lowest
		else prio = 50; // fallback normal
		Create();
		SetPriority(prio);
		Run();
	}


	/// @brief DOCME
	/// @return 
	///
	wxThread::ExitCode LuaThreadedCall::Entry()
	{
		int result = lua_pcall(L, nargs, nresults, 0);

		// see if there's a progress sink window to close
		lua_getfield(L, LUA_REGISTRYINDEX, "progress_sink");
		if (lua_isuserdata(L, -1)) {
			LuaProgressSink *ps = LuaProgressSink::GetObjPointer(L, -1);

			if (result) {
				// if the call failed, log the error here
				wxString errmsg(lua_tostring(L, -2), wxConvUTF8);
				ps->AddDebugOutput(_T("\n\nLua reported a runtime error:\n"));
				ps->AddDebugOutput(errmsg);
				lua_pop(L, 1);
			}

			// don't bother protecting this with a mutex, it should be safe enough like this
			ps->script_finished = true;
			// tell wx to run its idle-events now, just to make the progress window notice earlier that we're done
			wxWakeUpIdle();
		}
		lua_pop(L, 1);

		lua_gc(L, LUA_GCCOLLECT, 0);
		if (result) return (wxThread::ExitCode) 1;
		else return 0;
	}


	// LuaFeature


	/// @brief DOCME
	/// @param _L            
	/// @param _featureclass 
	/// @param _name         
	///
	LuaFeature::LuaFeature(lua_State *_L, ScriptFeatureClass _featureclass, const wxString &_name)
		: Feature(_featureclass, _name)
		, L(_L)
	{
	}


	/// @brief DOCME
	///
	void LuaFeature::RegisterFeature()
	{
		// get the LuaScript objects
		lua_getfield(L, LUA_REGISTRYINDEX, "aegisub");
		LuaScript *s = (LuaScript*)lua_touserdata(L, -1);
		lua_pop(L, 1);

		// add the Feature object
		s->features.push_back(this);

		// get the index+1 it was pushed into
		myid = (int)s->features.size()-1;

		// create table with the functions
		// get features table
		lua_getfield(L, LUA_REGISTRYINDEX, "features");
		lua_pushvalue(L, -2);
		lua_rawseti(L, -2, myid);
		lua_pop(L, 1);
	}


	/// @brief DOCME
	/// @param functionid 
	///
	void LuaFeature::GetFeatureFunction(int functionid)
	{
		// get feature table
		lua_getfield(L, LUA_REGISTRYINDEX, "features");
		// get this feature's function pointers
		lua_rawgeti(L, -1, myid);
		// get pointer for validation function
		lua_rawgeti(L, -1, functionid);
		lua_remove(L, -2);
		lua_remove(L, -2);
	}


	/// @brief DOCME
	/// @param ints 
	///
	void LuaFeature::CreateIntegerArray(const std::vector<int> &ints)
	{
		// create an array-style table with an integer vector in it
		// leave the new table on top of the stack
		lua_newtable(L);
		for (size_t i = 0; i != ints.size(); ++i) {
			lua_pushinteger(L, ints[i]+1);
			lua_rawseti(L, -2, (int)i+1);
		}
	}


	/// @brief DOCME
	///
	void LuaFeature::ThrowError()
	{
		wxString err(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 1);
		wxLogError(err);
	}


	// LuaFeatureMacro


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaFeatureMacro::LuaRegister(lua_State *L)
	{
		wxString _name(lua_tostring(L, 1), wxConvUTF8);
		wxString _description(lua_tostring(L, 2), wxConvUTF8);

		LuaFeatureMacro *macro = new LuaFeatureMacro(_name, _description, L);
		(void)macro;

		return 0;
	}


	/// @brief DOCME
	/// @param _name        
	/// @param _description 
	/// @param _L           
	///
	LuaFeatureMacro::LuaFeatureMacro(const wxString &_name, const wxString &_description, lua_State *_L)
		: Feature(SCRIPTFEATURE_MACRO, _name)
		, FeatureMacro(_name, _description)
		, LuaFeature(_L, SCRIPTFEATURE_MACRO, _name)
	{
		// new table for containing the functions for this feature
		lua_newtable(L);
		// store processing function
		if (!lua_isfunction(L, 3)) {
			lua_pushstring(L, "The macro processing function must be a function");
			lua_error(L);
		}
		lua_pushvalue(L, 3);
		lua_rawseti(L, -2, 1);
		// and validation function
		lua_pushvalue(L, 4);
		no_validate = !lua_isfunction(L, -1);
		lua_rawseti(L, -2, 2);
		// make the feature known
		RegisterFeature();
		// and remove the feature function table again
		lua_pop(L, 1);
	}


	/// @brief DOCME
	/// @param subs     
	/// @param selected 
	/// @param active   
	/// @return 
	///
	bool LuaFeatureMacro::Validate(AssFile *subs, const std::vector<int> &selected, int active)
	{
		if (no_validate)
			return true;

		GetFeatureFunction(2);  // 2 = validation function

		// prepare function call
		LuaAssFile *subsobj = new LuaAssFile(L, subs, false, false);
		(void) subsobj;
		CreateIntegerArray(selected); // selected items
		lua_pushinteger(L, -1); // active line

		// do call
		int err = lua_pcall(L, 3, 1, 0);
		bool result;
		if (err) {
			wxString errmsg(lua_tostring(L, -1), wxConvUTF8);
			wxLogWarning(_T("Runtime error in Lua macro validation function:\n%s"), errmsg.c_str());
			result = false;
		} else {
			result = !!lua_toboolean(L, -1);
		}

		// clean up stack (result or error message)
		lua_pop(L, 1);

		return result;
	}


	/// @brief DOCME
	/// @param subs            
	/// @param selected        
	/// @param active          
	/// @param progress_parent 
	/// @return 
	///
	void LuaFeatureMacro::Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent)
	{
		GetFeatureFunction(1); // 1 = processing function

		// prepare function call
		LuaAssFile *subsobj = new LuaAssFile(L, subs, true, true);
		(void) subsobj;
		CreateIntegerArray(selected); // selected items
		lua_pushinteger(L, -1); // active line

		LuaProgressSink *ps = new LuaProgressSink(L, progress_parent);
		ps->SetTitle(GetName());

		// do call
		// 3 args: subtitles, selected lines, active line
		// 1 result: new selected lines
		LuaThreadedCall call(L, 3, 1);

		ps->ShowModal();
		wxThread::ExitCode code = call.Wait();
		(void) code; // ignore
		//if (code) ThrowError();

		// top of stack will be selected lines array, if any was returned
		if (lua_istable(L, -1)) {
			selected.clear();
			selected.reserve(lua_objlen(L, -1));
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				if (lua_isnumber(L, -1)) {
					selected.push_back(lua_tointeger(L, -1));
				}
				lua_pop(L, 1);
			}
			std::sort(selected.begin(), selected.end());
		}
		// either way, there will be something on the stack
		lua_pop(L, 1);

		delete ps;
	}


	// LuaFeatureFilter


	/// @brief DOCME
	/// @param _name        
	/// @param _description 
	/// @param merit        
	/// @param _L           
	///
	LuaFeatureFilter::LuaFeatureFilter(const wxString &_name, const wxString &_description, int merit, lua_State *_L)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, FeatureFilter(_name, _description, merit)
		, LuaFeature(_L, SCRIPTFEATURE_FILTER, _name)
	{
		// Works the same as in LuaFeatureMacro
		lua_newtable(L);
		if (!lua_isfunction(L, 4)) {
			lua_pushstring(L, "The filter processing function must be a function");
			lua_error(L);
		}
		lua_pushvalue(L, 4);
		lua_rawseti(L, -2, 1);
		lua_pushvalue(L, 5);
		has_config = lua_isfunction(L, -1);
		lua_rawseti(L, -2, 2);
		RegisterFeature();
		lua_pop(L, 1);
	}


	/// @brief DOCME
	///
	void LuaFeatureFilter::Init()
	{
		// Don't think there's anything to do here... (empty in auto3)
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaFeatureFilter::LuaRegister(lua_State *L)
	{
		wxString _name(lua_tostring(L, 1), wxConvUTF8);
		wxString _description(lua_tostring(L, 2), wxConvUTF8);
		int _merit = lua_tointeger(L, 3);

		LuaFeatureFilter *filter = new LuaFeatureFilter(_name, _description, _merit, L);
		(void) filter;

		return 0;
	}


	/// @brief DOCME
	/// @param subs          
	/// @param export_dialog 
	///
	void LuaFeatureFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{
		LuaStackcheck stackcheck(L);

		GetFeatureFunction(1); // 1 = processing function
		assert(lua_isfunction(L, -1));
		stackcheck.check_stack(1);

		// prepare function call
		// subtitles (undo doesn't make sense in exported subs, in fact it'll totally break the undo system)
		LuaAssFile *subsobj = new LuaAssFile(L, subs, true/*allow modifications*/, false/*disallow undo*/);
		assert(lua_isuserdata(L, -1));
		stackcheck.check_stack(2);
		// config
		if (has_config && config_dialog) {
			int results_produced = config_dialog->LuaReadBack(L);
			assert(results_produced == 1);
			(void) results_produced;	// avoid warning on release builds
			// TODO, write back stored options here
		} else {
			// no config so put an empty table instead
			lua_newtable(L);
		}
		assert(lua_istable(L, -1));
		stackcheck.check_stack(3);

		LuaProgressSink *ps = new LuaProgressSink(L, export_dialog, false);
		ps->SetTitle(GetName());
		stackcheck.check_stack(3);

		// do call
		LuaThreadedCall call(L, 2, 0);

		ps->ShowModal();
		wxThread::ExitCode code = call.Wait();
		(void) code;
		//if (code) ThrowError();

		stackcheck.check_stack(0);

		// Just ensure that subsobj survives until here
		(void) subsobj;

		delete ps;
	}


	/// @brief DOCME
	/// @param parent 
	/// @return 
	///
	ScriptConfigDialog* LuaFeatureFilter::GenerateConfigDialog(wxWindow *parent)
	{
		if (!has_config)
			return 0;

		GetFeatureFunction(2); // 2 = config dialog function

		// prepare function call
		// subtitles (don't allow any modifications during dialog creation, ideally the subs aren't even accessed)
		LuaAssFile *subsobj = new LuaAssFile(L, AssFile::top, false/*allow modifications*/, false/*disallow undo*/);
		(void) subsobj;
		// stored options
		lua_newtable(L); // TODO, nothing for now

		// do call
		int err = lua_pcall(L, 2, 1, 0);
		if (err) {
			wxString errmsg(lua_tostring(L, -1), wxConvUTF8);
			wxLogWarning(_T("Runtime error in Lua macro validation function:\n%s"), errmsg.c_str());
			lua_pop(L, 1); // remove error message
			return config_dialog = 0;
		} else {
			// Create config dialogue from table on top of stack
			return config_dialog = new LuaConfigDialog(L, false);
		}
	}


	// LuaProgressSink


	/// @brief DOCME
	/// @param _L                  
	/// @param parent              
	/// @param allow_config_dialog 
	///
	LuaProgressSink::LuaProgressSink(lua_State *_L, wxWindow *parent, bool allow_config_dialog)
		: ProgressSink(parent)
		, L(_L)
	{
		LuaProgressSink **ud = (LuaProgressSink**)lua_newuserdata(L, sizeof(LuaProgressSink*));
		*ud = this;

		// register progress reporting stuff
		lua_getglobal(L, "aegisub");
		lua_newtable(L);

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetProgress, 1);
		lua_setfield(L, -2, "set");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetTask, 1);
		lua_setfield(L, -2, "task");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaSetTitle, 1);
		lua_setfield(L, -2, "title");

		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaGetCancelled, 1);
		lua_setfield(L, -2, "is_cancelled");

		lua_setfield(L, -2, "progress");

		lua_newtable(L);
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, LuaDebugOut, 1);
		lua_setfield(L, -2, "out");
		lua_setfield(L, -2, "debug");
		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaDebugOut, 1);
		lua_setfield(L, -2, "log");

		if (allow_config_dialog) {
			lua_newtable(L);
			lua_pushvalue(L, -3);
			lua_pushcclosure(L, LuaDisplayDialog, 1);
			lua_setfield(L, -2, "display");
			lua_setfield(L, -2, "dialog");
		}

		// reference so other objects can also find the progress sink
		lua_pushvalue(L, -2);
		lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");

		lua_pop(L, 2);
	}


	/// @brief DOCME
	///
	LuaProgressSink::~LuaProgressSink()
	{
		// remove progress reporting stuff
		lua_getglobal(L, "aegisub");
		lua_pushnil(L);
		lua_setfield(L, -2, "progress");
		lua_pushnil(L);
		lua_setfield(L, -2, "debug");
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");
	}


	/// @brief DOCME
	/// @param L   
	/// @param idx 
	/// @return 
	///
	LuaProgressSink* LuaProgressSink::GetObjPointer(lua_State *L, int idx)
	{
		assert(lua_type(L, idx) == LUA_TUSERDATA);
		void *ud = lua_touserdata(L, idx);
		return *((LuaProgressSink**)ud);
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaSetProgress(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		float progress = lua_tonumber(L, 1);
		ps->SetProgress(progress);
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaSetTask(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString task(lua_tostring(L, 1), wxConvUTF8);
		ps->SetTask(task);
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaSetTitle(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString title(lua_tostring(L, 1), wxConvUTF8);
		ps->SetTitle(title);
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaGetCancelled(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		lua_pushboolean(L, ps->cancelled);
		return 1;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaDebugOut(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));

		// Check trace level
		if (lua_isnumber(L, 1)) {
			int level = lua_tointeger(L, 1);
			if (level > ps->trace_level)
				return 0;
			// remove trace level
			lua_remove(L, 1);
		}

		// Only do format-string handling if there's more than one argument left
		// (If there's more than one argument left, assume first is a format string and rest are format arguments)
		if (lua_gettop(L) > 1) {
			// Format the string
			lua_getglobal(L, "string");
			lua_getfield(L, -1, "format");
			// Here stack contains format string, format arguments, 'string' table, format function
			// remove 'string' table
			lua_remove(L, -2);
			// put the format function into place
			lua_insert(L, 1);
			// call format function
			lua_call(L, lua_gettop(L)-1, 1);
		}

		// Top of stack is now a string to output
		wxString msg(lua_tostring(L, 1), wxConvUTF8);
		ps->AddDebugOutput(msg);
		return 0;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaProgressSink::LuaDisplayDialog(lua_State *L)
	{
		LuaProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));

		// Check that two arguments were actually given
		// If only one, add another empty table for buttons
		if (lua_gettop(L) == 1) {
			lua_newtable(L);
		}
		// If more than two, remove the excess
		if (lua_gettop(L) > 2) {
			lua_settop(L, 2);
		}

		// Send the "show dialog" event
		// See comments in auto4_base.h for more info on this synchronisation
		ShowConfigDialogEvent evt;

		LuaConfigDialog dlg(L, true); // magically creates the config dialog structure etc
		evt.config_dialog = &dlg;

		wxSemaphore sema(0, 1);
		evt.sync_sema = &sema;

		ps->AddPendingEvent(evt);

		sema.Wait();

		// more magic: puts two values on stack: button pushed and table with control results
		return dlg.LuaReadBack(L);
	}


	/// @brief // Factory methods
	///
	LuaScriptFactory::LuaScriptFactory() {}

	/// @brief DOCME
	///
	LuaScriptFactory::~LuaScriptFactory() {}


	/// @brief DOCME
	///
	void LuaScriptFactory::RegisterFactory ()
	{
		engine_name = _T("Lua");
		filename_pattern = _T("*.lua");
		Register(this);
	}


	/// @brief DOCME
	/// @param filename 
	///
	Script* LuaScriptFactory::Produce(const wxString &filename) const
	{
		// Just check if file extension is .lua
		// Reject anything else
		if (filename.Right(4).Lower() == _T(".lua")) {
			return new LuaScript(filename);
		} else {
			return 0;
		}
	}

};

#endif // WITH_AUTO4_LUA
