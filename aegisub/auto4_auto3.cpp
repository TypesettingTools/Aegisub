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
#include "../lua51/src/lualib.h"
#include "../lua51/src/lauxlib.h"
#include "options.h"
#include "string_codec.h"
#include "vfr.h"
#include "ass_override.h"

namespace Automation4 {

	// Helper functions for reading/writing data

	static inline void L_settable(lua_State *L, int table, const char *key, lua_Number val)
	{
		lua_pushstring(L, key);
		lua_pushnumber(L, val);
		if (table > 0 || table < -100) {
		  lua_settable(L, table);
		} else {
			lua_settable(L, table-2);
		}
	}

	static inline void L_settable(lua_State *L, int table, const char *key, wxString val)
	{
		//wxLogMessage(_T("Adding string at index '%s': %s"), wxString(key, wxConvUTF8), val);
		lua_pushstring(L, key);
		lua_pushstring(L, val.mb_str(wxConvUTF8));
		if (table > 0 || table < -100) {
		  lua_settable(L, table);
		} else {
			lua_settable(L, table-2);
		}
	}

	static inline void L_settable_bool(lua_State *L, int table, const char *key, bool val)
	{
		lua_pushstring(L, key);
		lua_pushboolean(L, val?1:0);
		if (table > 0 || table < -100) {
		  lua_settable(L, table);
		} else {
			lua_settable(L, table-2);
		}
	}

	static inline void L_settable(lua_State *L, int table, wxString &key, lua_Number val)
	{
		L_settable(L, table, key.mb_str(wxConvUTF8), val);
	}

	static inline void L_settable(lua_State *L, int table, wxString &key, wxString val)
	{
		L_settable(L, table, key.mb_str(wxConvUTF8), val);
	}

	static inline void L_settable_bool(lua_State *L, int table, wxString &key, bool val)
	{
		L_settable_bool(L, table, key.mb_str(wxConvUTF8).data(), val);
	}

	static inline void L_settable_kara(lua_State *L, int table, int index, int duration, wxString &kind, wxString &text, wxString &text_stripped)
	{
		lua_newtable(L);
		L_settable(L, -1, "duration", duration);
		L_settable(L, -1, "kind", kind);
		L_settable(L, -1, "text", text);
		L_settable(L, -1, "text_stripped", text_stripped);
		if (table > 0 || table < -100) {
			lua_rawseti(L, table, index);
		} else {
			lua_rawseti(L, table-1, index);
		}
	}

	static inline lua_Number L_gettableN(lua_State *L, const char *key)
	{
		lua_pushstring(L, key);
		lua_gettable(L, -2);
		lua_Number res = lua_tonumber(L, -1);
		lua_settop(L, -2);
		return res;
	}

	static inline wxString L_gettableS(lua_State *L, const char *key)
	{
		lua_pushstring(L, key);
		lua_gettable(L, -2);
		wxString res(lua_tostring(L, -1), wxConvUTF8);
		lua_settop(L, -2);
		return res;
	}

	static inline bool L_gettableB(lua_State *L, const char *key)
	{
		lua_pushstring(L, key);
		lua_gettable(L, -2);
		bool res = lua_toboolean(L, -1) != 0;
		lua_settop(L, -2);
		return res;
	}


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
		ps->AddDebugOutput(_T("\n"));
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
		if (options.size() == 0)
			return 0;

		wxPanel *res = new wxPanel(parent, -1);

		wxFlexGridSizer *sizer = new wxFlexGridSizer(2, 5, 5);

		for (std::vector<Auto3ScriptConfigurationOption>::iterator opt = options.begin(); opt != options.end(); opt++) {
			if (opt->kind == COK_INVALID)
				continue;

			Control control;
			control.option = &*opt;

			switch (opt->kind) {
				case COK_LABEL:
					control.control = new wxStaticText(res, -1, opt->label);
					break;

				case COK_TEXT:
					control.control = new wxTextCtrl(res, -1, opt->value.stringval);
					break;

				case COK_INT:
					control.control = new wxSpinCtrl(res, -1);
					if (opt->min.isset && opt->max.isset) {
						((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, opt->max.intval);
					} else if (opt->min.isset) {
						((wxSpinCtrl*)control.control)->SetRange(opt->min.intval, 0x7fff);
					} else if (opt->max.isset) {
						((wxSpinCtrl*)control.control)->SetRange(-0x7fff, opt->max.intval);
					} else {
						((wxSpinCtrl*)control.control)->SetRange(-0x7fff, 0x7fff);
					}
					((wxSpinCtrl*)control.control)->SetValue(opt->value.intval);
					break;

				case COK_FLOAT:
					control.control = new wxTextCtrl(res, -1, wxString::Format(_T("%f"), opt->value.floatval));
					break;

				case COK_BOOL:
					control.control = new wxCheckBox(res, -1, opt->label);
					((wxCheckBox*)control.control)->SetValue(opt->value.boolval);
					break;

				case COK_COLOUR:
					// *FIXME* what to do here?
					// just put a stupid edit box for now
					control.control = new wxTextCtrl(res, -1, opt->value.colourval.GetASSFormatted(false));
					break;

				case COK_STYLE:
					control.control = new wxChoice(res, -1, wxDefaultPosition, wxDefaultSize, AssFile::top->GetStyles());
					((wxChoice*)control.control)->Insert(_T(""), 0);
					break;

			}

			if (opt->kind != COK_LABEL && opt->kind != COK_BOOL) {
				control.label = new wxStaticText(res, -1, opt->label);
				sizer->Add(control.label, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
			} else {
				control.label = 0;
				sizer->AddSpacer(0);
			}
			control.control->SetToolTip(opt->hint);
			sizer->Add(control.control, 1, wxEXPAND);

			controls.push_back(control);
		}

		res->SetSizerAndFit(sizer);

		return res;
	}

	Auto3ConfigDialog::Auto3ConfigDialog(lua_State *L, const wxString &_ident)
		: ident(_ident)
	{
		present = false;
		if (!lua_istable(L, -1)) {
			return;
		}

		int i = 1;
		while (true) {
			// get an element from the array
			lua_pushnumber(L, i);
			lua_gettable(L, -2);

			// check if it was a table
			if (!lua_istable(L, -1)) {
				lua_pop(L, 1);
				break;
			}

			// add a new config option and fill it
			{
				Auto3ScriptConfigurationOption opt;
				options.push_back(opt);
			}
			Auto3ScriptConfigurationOption &opt = options.back();

			// get the "kind"
			lua_pushstring(L, "kind");
			lua_gettable(L, -2);
			if (lua_isstring(L, -1)) {
				// use C standard lib functions here, as it's probably faster than messing around with unicode
				// lua is known to always properly null-terminate strings, and the strings are known to be pure ascii
				const char *kind = lua_tostring(L, -1);
				if (strcmp(kind, "label") == 0) {
					opt.kind = COK_LABEL;
				} else if (strcmp(kind, "text") == 0) {
					opt.kind = COK_TEXT;
				} else if (strcmp(kind, "int") == 0) {
					opt.kind = COK_INT;
				} else if (strcmp(kind, "float") == 0) {
					opt.kind = COK_FLOAT;
				} else if (strcmp(kind, "bool") == 0) {
					opt.kind = COK_BOOL;
				} else if (strcmp(kind, "colour") == 0) {
					opt.kind = COK_COLOUR;
				} else if (strcmp(kind, "style") == 0) {
					opt.kind = COK_STYLE;
				} else {
					opt.kind = COK_INVALID;
				}
			} else {
				opt.kind = COK_INVALID;
			}

			// remove "kind" string from stack again
			lua_pop(L, 1);

			// no need to check for rest if this one is already deemed invalid
			if (opt.kind != COK_INVALID) {
				// name
				lua_pushstring(L, "name");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt.name = wxString(lua_tostring(L, -1), wxConvUTF8);
					lua_pop(L, 1);
				} else {
					lua_pop(L, 1);
					// no name means invalid option
					opt.kind = COK_INVALID;
					goto continue_invalid_option;
				}

				// label
				lua_pushstring(L, "label");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt.label = wxString(lua_tostring(L, -1), wxConvUTF8);
					lua_pop(L, 1);
				} else {
					lua_pop(L, 1);
					// label is also required
					opt.kind = COK_INVALID;
					goto continue_invalid_option;
				}
				assert(opt.kind != COK_INVALID);

				// hint
				lua_pushstring(L, "hint");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					opt.hint = wxString(lua_tostring(L, -1), wxConvUTF8);
				} else {
					opt.hint = _T("");
				}
				lua_pop(L, 1);

				// min
				lua_pushstring(L, "min");
				lua_gettable(L, -2);
				if (lua_isnumber(L, -1)) {
					opt.min.isset = true;
					opt.min.floatval = lua_tonumber(L, -1);
					opt.min.intval = (int)opt.min.floatval;
				} else {
					opt.min.isset = false;
				}
				lua_pop(L, 1);

				// max
				lua_pushstring(L, "max");
				lua_gettable(L, -2);
				if (lua_isnumber(L, -1)) {
					opt.max.isset = true;
					opt.max.floatval = lua_tonumber(L, -1);
					opt.max.intval = (int)opt.max.floatval;
				} else {
					opt.max.isset = false;
				}
				lua_pop(L, 1);

				// default (this is going to kill me)
				lua_pushstring(L, "default");
				lua_gettable(L, -2);
				switch (opt.kind) {
					case COK_LABEL:
						// nothing to do, nothing expected
						break;
					case COK_TEXT:
					case COK_STYLE:
						// expect it to be a string
						if (lua_isstring(L, -1)) {
							opt.default_val.stringval = wxString(lua_tostring(L, -1), wxConvUTF8);
						} else {
							// not a string, baaaad scripter
							opt.kind = COK_INVALID;
						}
						break;
					case COK_INT:
					case COK_FLOAT:
						// expect it to be a number
						if (lua_isnumber(L, -1)) {
							opt.default_val.floatval = lua_tonumber(L, -1);
							opt.default_val.intval = (int)opt.default_val.floatval;
						} else {
							opt.kind = COK_INVALID;
						}
						break;
					case COK_BOOL:
						// expect it to be a bool
						if (lua_isboolean(L, -1)) {
							opt.default_val.boolval = lua_toboolean(L, -1)!=0;
						} else {
							opt.kind = COK_INVALID;
						}
						break;
					case COK_COLOUR:
						// expect it to be a ass hex colour formatted string
						if (lua_isstring(L, -1)) {
							opt.default_val.stringval = wxString(lua_tostring(L, -1), wxConvUTF8);
							opt.default_val.colourval.Parse(opt.default_val.stringval); // and hope this goes well!
						} else {
							opt.kind = COK_INVALID;
						}
						break;
				}
				opt.value = opt.default_val;
				lua_pop(L, 1);
			}

			// so we successfully got an option added, so at least there is a configuration present now
			present = true;
continue_invalid_option:
			// clean up and prepare for next iteration
			lua_pop(L, 1);
			i++;
		}
	}

	Auto3ConfigDialog::~Auto3ConfigDialog()
	{
		// TODO?
	}

	int Auto3ConfigDialog::LuaReadBack(lua_State *L)
	{
		lua_newtable(L);

		for (std::vector<Auto3ScriptConfigurationOption>::iterator opt = options.begin(); opt != options.end(); opt++) {
			switch (opt->kind) {
				case COK_INVALID:
				case COK_LABEL:
					break;

				case COK_TEXT:
				case COK_STYLE:
					L_settable(L, -1, opt->name, opt->value.stringval);
					break;

				case COK_INT:
					L_settable(L, -1, opt->name, opt->value.intval);
					break;

				case COK_FLOAT:
					L_settable(L, -1, opt->name, opt->value.floatval);
					break;

				case COK_BOOL:
					L_settable_bool(L, -1, opt->name, opt->value.boolval);
					break;

				case COK_COLOUR:
					L_settable(L, -1, opt->name, opt->value.colourval.GetASSFormatted(false, false));
					break;

				default:
					break;
			}
		}

		return 1;
	}

	void Auto3ConfigDialog::ReadBack()
	{
		wxString opthname = wxString::Format(_T("Automation Settings %s"), ident.c_str());

		for (std::vector<Control>::iterator ctl = controls.begin(); ctl != controls.end(); ctl++) {
			switch (ctl->option->kind) {
				case COK_TEXT:
					ctl->option->value.stringval = ((wxTextCtrl*)ctl->control)->GetValue();
					break;

				case COK_INT:
					ctl->option->value.intval = ((wxSpinCtrl*)ctl->control)->GetValue();
					break;

				case COK_FLOAT:
					if (!((wxTextCtrl*)ctl->control)->GetValue().ToDouble(&ctl->option->value.floatval)) {
						wxLogWarning(
							_T("The value entered for field '%s' (%s) could not be converted to a floating-point number. Default value (%f) substituted for the entered value."),
							ctl->option->label.c_str(),
							((wxTextCtrl*)ctl->control)->GetValue().c_str(),
							ctl->option->default_val.floatval);
						ctl->option->value.floatval = ctl->option->default_val.floatval;
					}
					break;

				case COK_BOOL:
					ctl->option->value.boolval = ((wxCheckBox*)ctl->control)->GetValue();
					break;

				case COK_COLOUR:
					// *FIXME* needs to be updated to use a proper color control
					ctl->option->value.colourval.Parse(((wxTextCtrl*)ctl->control)->GetValue());
					break;

				case COK_STYLE:
					ctl->option->value.stringval = ((wxChoice*)ctl->control)->GetStringSelection();
					break;
			}
		}

		// serialize the new settings and save them to the file
		AssFile::top->SetScriptInfo(opthname, serialize());
	}

	wxString Auto3ConfigDialog::serialize()
	{
		if (options.size() == 0)
			return _T("");

		wxString result;
		for (std::vector<Auto3ScriptConfigurationOption>::iterator opt = options.begin(); opt != options.end(); opt++) {
			switch (opt->kind) {
				case COK_TEXT:
				case COK_STYLE:
					result << wxString::Format(_T("%s:%s|"), opt->name.c_str(), inline_string_encode(opt->value.stringval).c_str());
					break;
				case COK_INT:
					result << wxString::Format(_T("%s:%d|"), opt->name.c_str(), opt->value.intval);
					break;
				case COK_FLOAT:
					result << wxString::Format(_T("%s:%e|"), opt->name.c_str(), opt->value.floatval);
					break;
				case COK_BOOL:
					result << wxString::Format(_T("%s:%d|"), opt->name.c_str(), opt->value.boolval?1:0);
					break;
				case COK_COLOUR:
					result << wxString::Format(_T("%s:%s|"), opt->name.c_str(), opt->value.colourval.GetASSFormatted(false).c_str());
					break;
				default:
					// The rest aren't stored
					break;
			}
		}
		if (!result.IsEmpty() && result.Last() == _T('|'))
			result.RemoveLast();
		return result;
	}

	void Auto3ConfigDialog::unserialize(wxString &settings)
	{
		wxStringTokenizer toker(settings, _T("|"), wxTOKEN_STRTOK);
		while (toker.HasMoreTokens()) {
			// get the parts of this setting
			wxString setting = toker.GetNextToken();

			wxString optname = setting.BeforeFirst(_T(':'));
			wxString optval = setting.AfterFirst(_T(':'));

			// find the setting in the list loaded from the script
			std::vector<Auto3ScriptConfigurationOption>::iterator opt = options.begin();
			while (opt != options.end() && opt->name != optname)
				opt ++;

			if (opt != options.end()) {
				// ok, found the option!
				switch (opt->kind) {
					case COK_TEXT:
					case COK_STYLE:
						opt->value.stringval = inline_string_decode(optval);
						break;

					case COK_INT:
						{
							long n;
							optval.ToLong(&n, 10);
							opt->value.intval = n;
						}
						break;

					case COK_FLOAT:
						optval.ToDouble(&opt->value.floatval);
						break;

					case COK_BOOL:
						opt->value.boolval = optval == _T("1");
						break;

					case COK_COLOUR:
						opt->value.colourval.Parse(optval);
						break;
				}
			}
		}
	}


	// Auto3Filter

	Auto3Filter::Auto3Filter(const wxString &_name, const wxString &_description, lua_State *_L)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, FeatureFilter(_name, _description, 0)
		, L(_L)
	{
		// check that the processing function exists
		lua_getglobal(L, "process_lines");
		if (!lua_isfunction(L, -1)) {
			throw _T("Script error: No 'process_lines' function provided");
		}

		lua_pop(L, 2);
	}

	ScriptConfigDialog* Auto3Filter::GenerateConfigDialog(wxWindow *parent)
	{
		// configuration (let the config object do all the loading)
		lua_getglobal(L, "configuration");
		config = new Auto3ConfigDialog(L, GetName());

		wxString opthname = wxString::Format(_T("Automation Settings %s"), GetName().c_str());
		wxString serialized = AssFile::top->GetScriptInfo(opthname);
		config->unserialize(serialized);

		return config;
	}

	void Auto3Filter::Init()
	{
		// Nothing to do here
	}

	void Auto3Filter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{
		Auto3ProgressSink *sink = new Auto3ProgressSink(L, export_dialog);
		sink->SetTitle(GetName());
		Auto3ThreadedProcessor thread(L, subs, config, sink);

		sink->ShowModal();
		thread.Wait();

		delete sink;
	}


	// Auto3ThreadedProcessor

	Auto3ThreadedProcessor::Auto3ThreadedProcessor(lua_State *_L, AssFile *_file, Auto3ConfigDialog *_config, Auto3ProgressSink *_sink)
		: wxThread(wxTHREAD_JOINABLE)
		, L(_L)
		, file(_file)
		, config(_config)
		, sink(_sink)
	{
		// Pure copypasta
		int prio = Options.AsInt(_T("Automation Thread Priority"));
		if (prio == 0) prio = 50; // normal
		else if (prio == 1) prio = 30; // below normal
		else if (prio == 2) prio = 10; // lowest
		else prio = 50; // fallback normal
		Create();
		SetPriority(prio);
		Run();
	}

	wxThread::ExitCode Auto3ThreadedProcessor::Entry()
	{
		bool failed = false;

		try {
			sink->SetTask(_T("Preparing subtitle data"));
			sink->SetProgress(0);

			// first put the function itself on the stack
			lua_pushstring(L, "process_lines");
			lua_gettable(L, LUA_GLOBALSINDEX);

			// now put the three arguments on the stack

			// first argument: the metadata table
			lua_newtable(L);
			L_settable(L, -1, "res_x", file->GetScriptInfoAsInt(_T("PlayResX")));
			L_settable(L, -1, "res_y", file->GetScriptInfoAsInt(_T("PlayResY")));

			// second and third arguments: styles and events tables
			lua_newtable(L);
			int styletab = lua_gettop(L);
			lua_newtable(L);
			int eventtab = lua_gettop(L);

			int numstyles = 0, numevents = 0;

			// fill the styles and events tables
			int processed_lines = 1;
			for (std::list<AssEntry*>::iterator i = file->Line.begin(); i != file->Line.end(); i++, processed_lines++) {

				AssEntry *e = *i;

				if (!e->Valid) continue;

				if (e->GetType() == ENTRY_STYLE) {

					AssStyle *style = e->GetAsStyle(e);

					// gonna need a table to put the style data into
					lua_newtable(L);
					// put the table into index N in the style table
					lua_pushvalue(L, -1);
					lua_rawseti(L, styletab, numstyles);
					// and put it into its named index
					lua_pushstring(L, style->name.mb_str(wxConvUTF8));
					lua_pushvalue(L, -2);
					lua_settable(L, styletab);

					// so now the table is regged and stuff, put some data into it
					L_settable     (L, -1, "name",        style->name);
					L_settable     (L, -1, "fontname",    style->font);
					L_settable     (L, -1, "fontsize",    style->fontsize);
					L_settable     (L, -1, "color1",      style->primary.GetASSFormatted(true, true));
					L_settable     (L, -1, "color2",      style->secondary.GetASSFormatted(true, true));
					L_settable     (L, -1, "color3",      style->outline.GetASSFormatted(true, true));
					L_settable     (L, -1, "color4",      style->shadow.GetASSFormatted(true, true));
					L_settable_bool(L, -1, "bold",        style->bold);
					L_settable_bool(L, -1, "italic",      style->italic);
					L_settable_bool(L, -1, "underline",   style->underline);
					L_settable_bool(L, -1, "strikeout",   style->strikeout);
					L_settable     (L, -1, "scale_x",     style->scalex);
					L_settable     (L, -1, "scale_y",     style->scaley);
					L_settable     (L, -1, "spacing",     style->spacing);
					L_settable     (L, -1, "angle",       style->angle);
					L_settable     (L, -1, "borderstyle", style->borderstyle);
					L_settable     (L, -1, "outline",     style->outline_w);
					L_settable     (L, -1, "shadow",      style->shadow_w);
					L_settable     (L, -1, "align",       style->alignment);
					L_settable     (L, -1, "margin_l",    style->Margin[0]);
					L_settable     (L, -1, "margin_r",    style->Margin[1]);
					L_settable     (L, -1, "margin_v",    style->Margin[2]);
					L_settable     (L, -1, "encoding",    style->encoding);

					// and get that table off the stack again
					lua_settop(L, -2);

					numstyles++;

				} else if (e->group == _T("[Events]")) {
						
					if (e->GetType() != ENTRY_DIALOGUE) {

						// not a dialogue/comment event

						// start checking for a blank line
						wxString entryData = e->GetEntryData();
						if (entryData.IsEmpty()) {
							lua_newtable(L);
							L_settable(L, -1, "kind", wxString(_T("blank")));
						} else if (entryData[0] == _T(';')) {
							// semicolon comment
							lua_newtable(L);
							L_settable(L, -1, "kind", wxString(_T("scomment")));
							L_settable(L, -1, "text", entryData.Mid(1));
						} else {
							// not a blank line and not a semicolon comment
							// just skip...
							continue;
						}

					} else {

						// ok, so it is a dialogue/comment event
						// massive handling :(

						lua_newtable(L);

						assert(e->GetType() == ENTRY_DIALOGUE);

						AssDialogue *dia = e->GetAsDialogue(e);

						// kind of line
						if (dia->Comment) {
							L_settable(L, -1, "kind", wxString(_T("comment")));
						} else {
							L_settable(L, -1, "kind", wxString(_T("dialogue")));
						}

						L_settable(L, -1, "layer", dia->Layer);
						L_settable(L, -1, "start_time", dia->Start.GetMS()/10);
						L_settable(L, -1, "end_time", dia->End.GetMS()/10);
						L_settable(L, -1, "style", dia->Style);
						L_settable(L, -1, "name", dia->Actor);
						L_settable(L, -1, "margin_l", dia->Margin[0]);
						L_settable(L, -1, "margin_r", dia->Margin[1]);
						L_settable(L, -1, "margin_v", dia->Margin[2]);
						L_settable(L, -1, "effect", dia->Effect);
						L_settable(L, -1, "text", dia->Text);

						// so that's the easy part
						// now for the stripped text and *ugh* the karaoke!

						// prepare for stripped text
						wxString text_stripped = _T("");
						L_settable(L, -1, "text_stripped", 0); // dummy item
						// prepare karaoke table
						lua_newtable(L);
						lua_pushstring(L, "karaoke");
						lua_pushvalue(L, -2);
						lua_settable(L, -4);
						// now the top of the stack is the karaoke table, and it's present in the dialogue table

						int kcount = 0;
						int kdur = 0;
						wxString kkind = _T("");
						wxString ktext = _T("");
						wxString ktext_stripped = _T("");

						dia->ParseASSTags();
						for (std::vector<AssDialogueBlock*>::iterator block = dia->Blocks.begin(); block != dia->Blocks.end(); block++) {

							switch ((*block)->type) {

								case BLOCK_BASE:
									lua_pushliteral(L, "BLOCK_BASE found processing dialogue blocks. This should never happen.");
									lua_error(L);
									break;

								case BLOCK_PLAIN:
									ktext += (*block)->text;
									ktext_stripped += (*block)->text;
									text_stripped += (*block)->text;
									break;

								case BLOCK_DRAWING:
									ktext += (*block)->text;
									break;

								case BLOCK_OVERRIDE: {
									bool brackets_open = false;
									std::vector<AssOverrideTag*> &tags = (*block)->GetAsOverride(*block)->Tags;

									for (std::vector<AssOverrideTag*>::iterator tag = tags.begin(); tag != tags.end(); tag++) {

										if (!(*tag)->Name.Mid(0,2).CmpNoCase(_T("\\k")) && (*tag)->IsValid()) {

											// it's a karaoke tag
											if (brackets_open) {
												ktext += _T("}");
												brackets_open = false;
											}
											L_settable_kara(L, -1, kcount, kdur, kkind, ktext, ktext_stripped);
											kcount++;
											kdur = (*tag)->Params[0]->AsInt(); // no error checking; this should always be int
											kkind = (*tag)->Name.Mid(1);
											ktext = _T("");
											ktext_stripped = _T("");

										} else {

											// it's something else
											// don't care if it's a valid tag or not
											if (!brackets_open) {
												ktext += _T("{");
												brackets_open = true;
											}
											ktext += (*tag)->ToString();

										}

									}

									if (brackets_open) {
										ktext += _T("}");
									}

									break;}

							}

						}
						dia->ClearBlocks();

						// add the final karaoke block to the table
						// (even if there's no karaoke in the line, there's always at least one karaoke block)
						// even if the line ends in {\k10} with no text after, an empty block should still be inserted
						// (otherwise data are lost)
						L_settable_kara(L, -1, kcount, kdur, kkind, ktext, ktext_stripped);
						kcount++;
						L_settable(L, -1, "n", kcount); // number of syllables in the karaoke
						lua_settop(L, -2); // remove karaoke table from the stack again
						L_settable(L, -1, "text_stripped", text_stripped); // store the real stripped text

					}

					// now the entry table has been created and placed on top of the stack
					// now all that's missing it to insert it into the event table
					lua_rawseti(L, eventtab, numevents);
					numevents++;

				} else {
					// not really a line type automation needs to take care of... ignore it
				}

				sink->SetProgress(100.0f * processed_lines / file->Line.size() / 3);

			}

			// finally add the counter elements to the styles and events tables
			lua_pushnumber(L, numstyles);
			lua_rawseti(L, styletab, -1);
			L_settable(L, eventtab, "n", numevents);
			// and let the config object create a table for the @config argument
			config->LuaReadBack(L);

			sink->SetTask(_T("Running script for processing"));
			sink->SetProgress(100.0f/3);
			lua_call(L, 4, 1);
			sink->SetProgress(200.0f/3);
			sink->SetTask(_T("Reading back data from script"));

			// phew, survived the call =)
			// time to read back the results

			if (!lua_istable(L, -1)) {
				throw _T("The script function did not return a table as expected. Unable to process results. (Nothing was changed.)");
			}

			// but start by removing all events
			{
				std::list<AssEntry*>::iterator cur, next;
				next = file->Line.begin();
				while (next != file->Line.end()) {
					cur = next++;
					if ((*cur)->group == _T("[Events]")) {
						wxString temp = (*cur)->GetEntryData();
						if (temp == _T("[Events]")) {
							// skip the section header
							continue;
						}
						if ((*cur)->GetType() != ENTRY_DIALOGUE && temp.Mid(0,1) != _T(";") && temp.Trim() != _T("")) {
							// skip non-dialogue non-semicolon comment lines (such as Format)
							continue;
						}
						delete (*cur);
						file->Line.erase(cur);
					}
				}
			}

			// so anyway, there is a single table on the stack now
			// that table contains a lot of events...
			// and it ought to contain an "n" key as well, telling how many events
			// but be lenient, and don't expect one to be there, but rather count from zero and let it be nil-terminated
			// if the "n" key is there, use it as a progress indicator hint, though
			int output_line_count;
			lua_pushstring(L, "n");
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1)) {
				output_line_count = (int) lua_tonumber(L, -1);
			} else {
				// assume number of output lines == number of input lines
				output_line_count = processed_lines;
			}
			lua_settop(L, -2);

			int outline = 0;
			int faketime = file->Line.back()->StartMS;

			// If there's nothing at index 0, start at index 1 instead, to support both zero and one based indexing
			lua_pushnumber(L, outline);
			lua_gettable(L, -2);
			if (!lua_istable(L, -1)) {
				outline++;
				output_line_count++;
			}
			lua_pop(L, 1);

			while (lua_pushnumber(L, outline), lua_gettable(L, -2), lua_istable(L, -1)) {
				// top of the stack is a table, hopefully with an AssEntry in it

				// start by getting the kind
				lua_pushstring(L, "kind");
				lua_gettable(L, -2);
				if (!lua_isstring(L, -1)) {
					sink->AddDebugOutput(wxString::Format(_T("The output data at index %d is mising a valid 'kind' field, and has been skipped\n"), outline));
					lua_settop(L, -2);
				} else {

					wxString kind = wxString(lua_tostring(L, -1), wxConvUTF8).Lower();
					// remove "kind" from stack again
					lua_settop(L, -2);

					if (kind == _T("dialogue") || kind == _T("comment")) {

						lua_pushstring(L, "layer");
						lua_gettable(L, -2);
						lua_pushstring(L, "start_time");
						lua_gettable(L, -3);
						lua_pushstring(L, "end_time");
						lua_gettable(L, -4);
						lua_pushstring(L, "style");
						lua_gettable(L, -5);
						lua_pushstring(L, "name");
						lua_gettable(L, -6);
						lua_pushstring(L, "margin_l");
						lua_gettable(L, -7);
						lua_pushstring(L, "margin_r");
						lua_gettable(L, -8);
						lua_pushstring(L, "margin_v");
						lua_gettable(L, -9);
						lua_pushstring(L, "effect");
						lua_gettable(L, -10);
						lua_pushstring(L, "text");
						lua_gettable(L, -11);

						if (lua_isnumber(L, -10) && lua_isnumber(L, -9) && lua_isnumber(L, -8) &&
							lua_isstring(L, -7) && lua_isstring(L, -6) && lua_isnumber(L, -5) &&
							lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isstring(L, -2) &&
							lua_isstring(L, -1))
						{
							AssDialogue *e = new AssDialogue();
							e->Layer = (int)lua_tonumber(L, -10);
							e->Start.SetMS(10*(int)lua_tonumber(L, -9));
							e->End.SetMS(10*(int)lua_tonumber(L, -8));
							e->Style = wxString(lua_tostring(L, -7), wxConvUTF8);
							e->Actor = wxString(lua_tostring(L, -6), wxConvUTF8);
							e->Margin[0] = (int)lua_tonumber(L, -5);
							e->Margin[1] = (int)lua_tonumber(L, -4);
							e->Margin[2] = e->Margin[3] = (int)lua_tonumber(L, -3);
							e->Effect = wxString(lua_tostring(L, -2), wxConvUTF8);
							e->Text = wxString(lua_tostring(L, -1), wxConvUTF8);
							e->Comment = kind == _T("comment");
							lua_settop(L, -11);
							e->StartMS = e->Start.GetMS();
							//e->ParseASSTags();
							e->UpdateData();
							file->Line.push_back(e);
						} else {
							sink->AddDebugOutput(wxString::Format(_T("The output data at index %d (kind '%s') has one or more missing/invalid fields, and has been skipped\n"), outline, kind.c_str()));
						}

					} else if (kind == _T("scomment")) {

						lua_pushstring(L, "text");
						lua_gettable(L, -2);
						if (lua_isstring(L, -1)) {
							wxString text(lua_tostring(L, -1), wxConvUTF8);
							lua_settop(L, -2);
							AssEntry *e = new AssEntry(wxString(_T(";")) + text);
							e->StartMS = faketime;
							file->Line.push_back(e);
						} else {
							sink->AddDebugOutput(wxString::Format(_T("The output data at index %d (kind 'scomment') is missing a valid 'text' field, and has been skipped\n"), outline));
						}

					} else if (kind == _T("blank")) {

						AssEntry *e = new AssEntry(_T(""));
						e->StartMS = faketime;
						file->Line.push_back(e);

					} else {
						sink->AddDebugOutput(wxString::Format(_T("The output data at index %d has an invalid value in the 'kind' field, and has been skipped\n"), outline));
					}

				}

				// remove table again
				lua_settop(L, -2);
				// progress report
				if (outline >= output_line_count) {
					sink->SetProgress(99.9f);
				} else {
					sink->SetProgress((200.0f + 100.0f*outline/output_line_count) / 3);
				}

				outline++;
			}

			sink->SetTask(_T("Completed"));
		}
		catch (const wchar_t *e) {
			failed = true;
			sink->AddDebugOutput(e);
		}
		catch (const char *e) {
			failed = true;
			wxString s(e, wxConvUTF8);
			sink->AddDebugOutput(s);
		}
		catch (...) {
			failed = true;
			if (lua_isstring(L, -1)) {
				wxString s(lua_tostring(L, -1), wxConvUTF8);
				sink->AddDebugOutput(s);
			} else {
				sink->AddDebugOutput(_T("Unknown error"));
			}
		}

		if (failed) {
			sink->SetTask(_T("Failed"));
		} else {
			sink->SetProgress(100);
		}
		sink->script_finished = true;
		wxWakeUpIdle();
		if (failed) {
			return (wxThread::ExitCode)1;
		} else {
			return (wxThread::ExitCode)0;
		}
	}


	// Auto3Script

	Auto3Script::Auto3Script(const wxString &filename)
		: Script(filename)
		, L(0)
		, filter(0)
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

	Auto3Script* Auto3Script::GetScriptObject(lua_State *L)
	{
		lua_getfield(L, LUA_REGISTRYINDEX, "aegisub");
		void *ptr = lua_touserdata(L, -1);
		lua_pop(L, 1);
		return (Auto3Script*)ptr;
	}

	int Auto3Script::LuaTextExtents(lua_State *L)
	{
		double resx, resy, resd, resl;

		wxString intext(lua_tostring(L, -1), wxConvUTF8);

		AssStyle st;

		st.font = L_gettableS(L, "fontname");
		st.fontsize = L_gettableN(L, "fontsize");
		st.bold = L_gettableB(L, "bold");
		st.italic = L_gettableB(L, "italic");
		st.underline = L_gettableB(L, "underline");
		st.strikeout = L_gettableB(L, "strikeout");
		st.scalex = L_gettableN(L, "scale_x");
		st.scaley = L_gettableN(L, "scale_y");
		st.spacing = (int)L_gettableN(L, "spacing");
		st.encoding = (int)L_gettableN(L, "encoding");

		if (!CalculateTextExtents(&st, intext, resx, resy, resd, resl)) {
			lua_pushstring(L, "Some internal error occurred calculating text_extents");
			lua_error(L);
		}

		lua_pushnumber(L, resx);
		lua_pushnumber(L, resy);
		lua_pushnumber(L, resd);
		lua_pushnumber(L, resl);
		return 4;
	}

	int Auto3Script::LuaInclude(lua_State *L)
	{
		Auto3Script *s = GetScriptObject(L);

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
			lua_pushfstring(L, "Could not find Automation 3 script for inclusion: %s", fnames.mb_str(wxConvUTF8).data());
			lua_error(L);
		}

		LuaScriptReader script_reader(fname.GetFullPath());
		if (lua_load(L, script_reader.reader_func, &script_reader, s->GetFilename().mb_str(wxConvUTF8))) {
			lua_pushfstring(L, "An error occurred loading the Automation 3 script file \"%s\":\n\n%s", fname.GetFullPath().mb_str(wxConvUTF8).data(), lua_tostring(L, -1));
			lua_error(L);
			return 0;
		}
		int pretop = lua_gettop(L) - 1; // don't count the function value itself
		lua_call(L, 0, LUA_MULTRET);
		return lua_gettop(L) - pretop;
	}

	int Auto3Script::LuaColorstringToRGB(lua_State *L)
	{
		if (lua_gettop(L) < 1) {
			lua_pushstring(L, "colorstring_to_rgb called without arguments");
			lua_error(L);
		}
		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "colorstring_to_rgb requires a string type argument");
			lua_error(L);
		}

		wxString colorstring(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 1);
		AssColor rgb;
		rgb.Parse(colorstring);
		lua_pushnumber(L, rgb.r);
		lua_pushnumber(L, rgb.g);
		lua_pushnumber(L, rgb.b);
		lua_pushnumber(L, rgb.a);
		return 4;
	}

	int Auto3Script::LuaFrameFromMs(lua_State *L)
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

	int Auto3Script::LuaMsFromFrame(lua_State *L)
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

	void Auto3Script::Create()
	{
		Destroy();

		loaded = true;

		try {
			L = lua_open();

			// register standard libs
			lua_pushcfunction(L, luaopen_base); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_package); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_string); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_table); lua_call(L, 0, 0);
			lua_pushcfunction(L, luaopen_math); lua_call(L, 0, 0);
			// dofile and loadfile are replaced with include
			lua_pushnil(L);
			lua_setglobal(L, "dofile");
			lua_pushnil(L);
			lua_setglobal(L, "loadfile");
			lua_pushcfunction(L, LuaInclude);
			lua_setglobal(L, "include");

			// reference to the script object
			lua_pushlightuserdata(L, this);
			lua_setfield(L, LUA_REGISTRYINDEX, "aegisub");

			// make "aegisub" table
			lua_newtable(L);
			// put helper functions in it
			lua_pushcfunction(L, LuaColorstringToRGB);
			lua_setfield(L, -2, "colorstring_to_rgb");
			lua_pushcfunction(L, LuaTextExtents);
			lua_setfield(L, -2, "text_extents");
			lua_pushcfunction(L, LuaFrameFromMs);
			lua_setfield(L, -2, "frame_from_ms");
			lua_pushcfunction(L, LuaMsFromFrame);
			lua_setfield(L, -2, "ms_from_frame");
			lua_pushinteger(L, 3);
			lua_setfield(L, -2, "lua_automation_version");
			// store table
			lua_setfield(L, LUA_GLOBALSINDEX, "aegisub");

			// load user script
			LuaScriptReader script_reader(GetFilename());
			if (lua_load(L, script_reader.reader_func, &script_reader, GetFilename().mb_str(wxConvUTF8))) {
				wxString *err = new wxString(lua_tostring(L, -1), wxConvUTF8);
				err->Prepend(_T("An error occurred loading the Automation 3 script file \"") + GetFilename() + _T("\":\n\n"));
				throw err->c_str();
			}
			// and run it
			{
				int err = lua_pcall(L, 0, 0, 0);
				if (err) {
					// error occurred, assumed to be on top of Lua stack
					wxString *errs = new wxString(lua_tostring(L, -1), wxConvUTF8);
					errs->Prepend(_T("An error occurred initialising the Automation 3 script file \"") + GetFilename() + _T("\":\n\n"));
					throw errs->c_str();
				}
			}

			// so, the script should be loaded
			// now try to get the script data!
			// first the version
			lua_getglobal(L, "version");
			if (!lua_isnumber(L, -1)) {
				throw _T("Script error: 'version' value not found or not a number");
			}
			double engineversion = lua_tonumber(L, -1);
			if (engineversion < 3 || engineversion > 4) {
				// invalid version
				throw _T("Script error: 'version' must be 3 for Automation 3 scripts");
			}
			version = _T("");
			// skip 'kind', it's useless
			// name
			lua_getglobal(L, "name");
			if (!lua_isstring(L, -1)) {
				name = GetFilename();
			} else {
				name = wxString(lua_tostring(L, -1), wxConvUTF8);
			}
			// description (optional)
			lua_getglobal(L, "description");
			if (lua_isstring(L, -1)) {
				description = wxString(lua_tostring(L, -1), wxConvUTF8);
			} else {
				description = _T("");
			}
			lua_pop(L, 4);

			// create filter feature object, that will check for process_lines function and configuration
			filter = new Auto3Filter(name, description, L);
		}
		catch (...) {
			Destroy();
			loaded = false;
			throw;
		}
	}

	void Auto3Script::Destroy()
	{
		if (!L) return;

		if (filter) {
			delete filter;
			filter = 0;
		}

		lua_close(L);
		L = 0;

		loaded = false;
	}

	void Auto3Script::Reload()
	{
		Destroy();
		Create();
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
	Auto3ScriptFactory _auto3_script_factory;

};
