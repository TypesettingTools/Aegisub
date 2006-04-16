// Copyright (c) 2005, 2006, Niels Martin Hansen
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
// Contact: mailto:zeratul@cellosoft.com
//

extern "C" {
#include <lua.h>
}

#include <wx/wxprec.h>
#include <wx/filefn.h>
#include <vector>
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_override.h"
#include "ass_style.h"

#include <setjmp.h>

#pragma once


class AutomationError {
public:
	wxString message;
	AutomationError(wxString msg);
};


enum AutomationScriptConfigurationOptionKind {
	COK_INVALID = 0,
	COK_LABEL,
	COK_TEXT,
	COK_INT,
	COK_FLOAT,
	COK_BOOL,
	COK_COLOUR,
	COK_STYLE
};

struct AutomationScriptConfigurationOption {
	wxString name;
	AutomationScriptConfigurationOptionKind kind;
	wxString label;
	wxString hint;
	union {
		bool isset;
		int intval;
		double floatval;
	} min, max;
	struct {
		wxString stringval;
		int intval;
		double floatval;
		bool boolval;
		AssColor colourval;
	} default_val, value;
};

class AutomationScriptConfiguration {
public:

	bool present; // is there any configuration option set at all?

	std::vector<AutomationScriptConfigurationOption> options;

	wxString serialize(); // make a string from the option name+value pairs
	void unserialize(wxString &settings); // set the option values from a serialized string

	void load_from_lua(lua_State *L); // top of the stack must be a table in the format of the global "configuration" table, or nil; fill the options vector from that table. (the current options vector will be cleared. all values will be set to the defaults.)
	void store_to_lua(lua_State *L); // create a process_lines@config style table from the options name:value pairs. after the call, the top of the stack will be such a table.
};


struct AutomationScriptFile {
	bool utf8bom;
	char *scriptdata;
	size_t scriptlen;
	wxString filename;

	~AutomationScriptFile();
	static AutomationScriptFile *CreateFromString(wxString &script);
	static AutomationScriptFile *CreateFromFile(wxString filename);
};


class AutomationScript {
protected:
	lua_State *L;
	jmp_buf panicjmp;

	static int L_panicfunc(lua_State *L);

public:
	AutomationScript(AutomationScriptFile *script);
	virtual ~AutomationScript();

	// Reporting functions. These do nothing in the base class.
	// They should be overridden in a derived class.
	virtual void OutputDebugString(wxString str, bool isdebug = false);
	virtual void ReportProgress(float progress);

	// stuff corresponding to globals in the script
	float version;
	wxString kind;
	wxString name;
	wxString description;
	AutomationScriptConfiguration configuration;

	// filename the script was loaded from
	wxString filename;
	// include path for scripts
	wxPathList include_path;

	volatile bool force_cancel;
	void *progress_target;
	void *debug_target;
	typedef void (*progress_reporter_t)(float progress, AutomationScript *script, void *target);
	typedef void (*debug_reporter_t)(wxString &str, bool isdebug, AutomationScript *script, void *target);
	progress_reporter_t progress_reporter;
	debug_reporter_t debug_reporter;

	virtual void process_lines(AssFile *input);
};

