// Copyright (c) 2005, 2006, 2007, Niels Martin Hansen
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

#include "auto4_auto3.h"
#include "auto4_lua.h"

namespace Automation4 {


	// Auto3ProgressSink

	int Auto3ProgressSink::LuaSetStatus(lua_State *L)
	{
		Auto3ProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString task(lua_tostring(L, 1), wxConvUTF8);
		ps->SetTask(task);
		return 0;
	}

	int Auto3ProgressSink::LuaOutputDebug(lua_State *L)
	{
		Auto3ProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		wxString msg(lua_tostring(L, 1), wxConvUTF8);
		ps->AddDebugOutput(msg);
		return 0;
	}

	int Auto3ProgressSink::LuaReportProgress(lua_State *L)
	{
		Auto3ProgressSink *ps = GetObjPointer(L, lua_upvalueindex(1));
		float progress = lua_tonumber(L, 1);
		ps->SetProgress(progress);
		return 0;
	}

	Auto3ProgressSink::Auto3ProgressSink(lua_State *_L, wxWindow *parent)
		: ProgressSink(parent)
		, L(_L)
	{
		Auto3ProgressSink **ud = (Auto3ProgressSink**)lua_newuserdata(L, sizeof(Auto3ProgressSink*));
		*ud = this;

		// register progress reporting stuff
		lua_getglobal(L, "aegisub");

		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaReportProgress, 1);
		lua_setfield(L, -2, "report_progress");

		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaOutputDebug, 1);
		lua_setfield(L, -2, "output_debug");

		lua_pushvalue(L, -2);
		lua_pushcclosure(L, LuaSetStatus, 1);
		lua_setfield(L, -2, "set_status");

		// reference so other objects can also find the progress sink
		lua_pushvalue(L, -2);
		lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");

		// Remove aegisub table and userdata object from stack
		lua_pop(L, 2);
	}

	Auto3ProgressSink::~Auto3ProgressSink()
	{
		// remove progress reporting stuff
		lua_getglobal(L, "aegisub");
		lua_pushnil(L);
		lua_setfield(L, -2, "report_progress");
		lua_pushnil(L);
		lua_setfield(L, -2, "output_debug");
		lua_pushnil(L);
		lua_setfield(L, -2, "set_status");
		lua_pop(L, 1);
		lua_pushnil(L);
		lua_setfield(L, LUA_REGISTRYINDEX, "progress_sink");
	}

	Auto3ProgressSink* Auto3ProgressSink::GetObjPointer(lua_State *L, int idx)
	{
		assert(lua_type(L, idx) == LUA_TUSERDATA);
		void *ud = lua_touserdata(L, idx);
		return *((Auto3ProgressSink**)ud);
	}


	// Auto3ConfigDialog

	wxWindow* Auto3ConfigDialog::CreateWindow(wxWindow *parent)
	{
		// TODO
		return 0;
	}

	Auto3ConfigDialog::Auto3ConfigDialog(lua_State *_L, bool include_buttons)
	{
		// TODO
	}

	Auto3ConfigDialog::~Auto3ConfigDialog()
	{
		// TODO
	}

	int Auto3ConfigDialog::LuaReadBack(lua_State *L)
	{
		// TODO
		return 0;
	}

	void Auto3ConfigDialog::ReadBack()
	{
		// TODO
	}


	// Auto3Filter

	Auto3Filter::Auto3Filter(const wxString &_name, const wxString &_description, lua_State *_L)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, FeatureFilter(_name, _description, 0)
	{
		// TODO
	}

	ScriptConfigDialog* Auto3Filter::GenerateConfigDialog(wxWindow *parent)
	{
		// TODO
		return 0;
	}

	void Auto3Filter::Init()
	{
		// Nothing to do here
	}

	void Auto3Filter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{
		// TODO
	}


	// Auto3ThreadedCall

	Auto3ThreadedCall::Auto3ThreadedCall(lua_State *_L, int _nargs, int _nresults)
	{
		// TODO
	}

	wxThread::ExitCode Auto3ThreadedCall::Entry()
	{
		// TODO
		return (wxThread::ExitCode)0;
	}


	// Auto3Script

	Auto3Script::Auto3Script(const wxString &filename)
		: Script(filename)
		, L(0)
	{
		try {
			Create();
		}
		catch (wxChar *e) {
			description = e;
			loaded = false;
		}
	}

	Auto3Script::~Auto3Script()
	{
		if (L) Destroy();
	}

	void Auto3Script::Create()
	{
		// TODO
	}

	void Auto3Script::Destroy()
	{
		// TODO
	}

	void Auto3Script::Reload()
	{
		// TODO
	}


	// Auto3ScriptFactory

	class Auto3ScriptFactory : public ScriptFactory {
	public:
		Auto3ScriptFactory()
		{
			engine_name = _T("Legacy Automation 3");
			filename_pattern = _T("*.auto3");
			Register(this);
		}

		~Auto3ScriptFactory() { }

		virtual Script* Produce(const wxString &filename) const
		{
			if (filename.Right(4).Lower() == _T(".auto3")) {
				return new Auto3Script(filename);
			} else {
				return 0;
			}
		}
	};
	Auto3ScriptFactory _script_factory;

};
