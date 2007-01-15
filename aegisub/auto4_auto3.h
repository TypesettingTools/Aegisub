// Copyright (c) 2007, Niels Martin Hansen
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

// Scripting engine for legacy Automation 3 compatibility

#pragma once

#ifndef _AUTO4_AUTO3_H
#define _AUTO4_AUTO3_H

#include "auto4_base.h"
#include <wx/thread.h>
#include <wx/event.h>
#include "../lua51/src/lua.h"
#include "../lua51/src/lauxlib.h"

namespace Automation4 {

	class Auto3ProgressSink : public ProgressSink {
	private:
		lua_State *L;

		static int LuaSetStatus(lua_State *L);
		static int LuaOutputDebug(lua_State *L);
		static int LuaReportProgress(lua_State *L);

	public:
		Auto3ProgressSink(lua_State *_L, wxWindow *parent);
		virtual ~Auto3ProgressSink();

		static Auto3ProgressSink* GetObjPointer(lua_State *L, int idx);
	};


	class Auto3ConfigDialog : public ScriptConfigDialog {
		// copypasta
	protected:
		wxWindow* CreateWindow(wxWindow *parent);

	public:
		Auto3ConfigDialog(lua_State *_L, bool include_buttons);
		virtual ~Auto3ConfigDialog();
		int LuaReadBack(lua_State *L); // read back internal structure to lua structures

		void ReadBack(); // from auto4 base
	};


	class Auto3Filter : public FeatureFilter {
	protected:
		Auto3Filter(const wxString &_name, const wxString &_description, lua_State *_L);

		ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent);

		void Init();
	public:
		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	};


	class Auto3ThreadedCall : public wxThread {
		// This is pretty much copy-paste from the non-legacy version
	private:
		lua_State *L;
		int nargs;
		int nresults;
	public:
		Auto3ThreadedCall(lua_State *_L, int _nargs, int _nresults);
		virtual ExitCode Entry();
	};


	class Auto3Script : public Script {
	private:
		Auto3Filter *filter;
		lua_State *L;

		void Create();
		void Destroy();

	public:
		Auto3Script(const wxString &filename);
		virtual ~Auto3Script();

		virtual void Reload();
	};

};

#endif
