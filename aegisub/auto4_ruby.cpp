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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//

#include "auto4_ruby.h"
#include "auto4_auto3.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_file.h"
#include "ass_override.h"
#include "text_file_reader.h"
#include "options.h"
#include <ruby.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/window.h>
#include <assert.h>
#include <algorithm>

namespace Automation4 {

	// I'm not sure if this will work ?_?
	RubyObjects *RubyObjects::inst = NULL;
	RubyScript * RubyScript::inst = NULL; // current Ruby Script
	RubyProgressSink* RubyProgressSink::inst = NULL;
	VALUE RubyScript::RubyAegisub;
	RubyAssFile *RubyAssFile::raf = NULL;

	// RubyScriptReader
	RubyScriptReader::RubyScriptReader(const wxString &filename)
	{
#ifdef WIN32
		f = _tfopen(filename.c_str(), _T("rb"));
#else
		f = fopen(filename.fn_str(), "rb");
#endif
		first = true;
		databuf = new char[bufsize];
	}

	RubyScriptReader::~RubyScriptReader()
	{
		if (databuf)
			delete databuf;
		fclose(f);
	}

/*	const char* RubyScriptReader::reader_func( void *data, size_t *size)
	{
		return self->databuf;
	}
*/

	// RubyScript

	RubyScript::RubyScript(const wxString &filename)
		: Script(filename)
	{
		try {
			Create();
		}
		catch (wxChar *e) {
			description = e;
			loaded = false;
			throw;
		}
	}

	RubyScript::~RubyScript()
	{
	}

	void RubyScript::Create()
	{
		Destroy();

		try {
#if defined(NT)
			int argc = 0;
			char **argv = 0;
			NtInitialize(&argc, &argv);
#endif
			ruby_init();
			ruby_init_loadpath();
			RubyScript::inst = this;
			RubyAegisub = rb_define_module("Aegisub");
			rb_define_module_function(RubyAegisub, "register_macro",reinterpret_cast<RB_HOOK>(&RubyFeatureMacro::RubyRegister), 4);
			rb_define_module_function(RubyAegisub, "register_filter",reinterpret_cast<RB_HOOK>(&RubyFeatureFilter::RubyRegister), 5);
			rb_define_module_function(RubyAegisub, "text_extents",reinterpret_cast<RB_HOOK>(&RubyTextExtents), 2);
			rb_define_module_function(RubyScript::RubyAegisub, "progress_set",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetProgress), 1);
			rb_define_module_function(RubyScript::RubyAegisub, "progress_task",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetTask), 1);
			rb_define_module_function(RubyScript::RubyAegisub, "progress_title",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetTitle), 1);
			rb_define_module_function(RubyScript::RubyAegisub, "debug_out",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubyDebugOut), -1);
			rb_define_module_function(RubyScript::RubyAegisub, "get_cancelled",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubyGetCancelled), 0);
			VALUE paths = rb_gv_get("$:");
			for(int i = 0; i < include_path.GetCount(); i++)
			{
				rb_ary_push(paths, rb_str_new2(include_path[i].mb_str(wxConvISO8859_1)));
			}

			int status = 0;
			wxCharBuffer buf = GetFilename().mb_str(wxConvISO8859_1);
			const char *t = buf.data();

			rb_protect(rbLoadWrapper, rb_str_new2(t), &status);
			if(status > 0)	// something bad happened (probably parsing error)
			{
				throw StringValueCStr(ruby_errinfo);
			}

			VALUE global_var = rb_gv_get("$script_name");
			if(TYPE(global_var) == T_STRING)
				name = wxString(StringValueCStr(global_var), wxConvUTF8);
			global_var = rb_gv_get("$script_description");
			if(TYPE(global_var) == T_STRING)
				description = wxString(StringValueCStr(global_var), wxConvUTF8);
			global_var = rb_gv_get("$script_author");
			if(TYPE(global_var) == T_STRING)
				author = wxString(StringValueCStr(global_var), wxConvUTF8);
			global_var = rb_gv_get("$script_version");
			if(TYPE(global_var) == T_STRING)
				version = wxString(StringValueCStr(global_var), wxConvUTF8);
			loaded = true;
		}
		catch (const char* e) {
			Destroy();
			loaded = false;
			wxString *err = new wxString(e, wxConvUTF8);
			throw err->c_str();
		}
	}

	void RubyScript::Destroy()
	{
		if(loaded) {
			ruby_finalize();
			ruby_cleanup(0);
		}
		// TODO: would be nice to implement this
		// RubyObjects::Get()->UnregisterAll();

		// remove features
		for (int i = 0; i < (int)features.size(); i++) {
			Feature *f = features[i];
			delete f;
		}
		features.clear();
		loaded = false;
		RubyScript::inst = NULL;
	}

	void RubyScript::Reload()
	{
		Destroy();
		Create();
	}

	RubyScript* RubyScript::GetScriptObject()
	{
		return RubyScript::inst;
	}


	VALUE RubyScript::RubyTextExtents(VALUE self, VALUE _style, VALUE _text)
	{
		if(TYPE(_style) != T_HASH)
			rb_raise(rb_eRuntimeError, "text_extents: Style parameter must be a hash");

		AssEntry *et = RubyAssFile::RubyToAssEntry(_style);
		AssStyle *st = dynamic_cast<AssStyle*>(et);
		if (!st) {
			delete et; // Make sure to delete the "live" pointer
			rb_raise(rb_eRuntimeError, "Not a style entry");
		}

		wxString text(StringValueCStr(_text), wxConvUTF8);

		double width, height, descent, extlead;
		if (!CalculateTextExtents(st, text, width, height, descent, extlead)) {
			delete st;
			rb_raise(rb_eRuntimeError, "Some internal error occurred calculating text_extents");
		}
		delete st;

		VALUE result = rb_ary_new3(4, rb_float_new(width), rb_float_new(height), rb_float_new(descent), rb_float_new(extlead));
		return result;
	}


	// RubyFeature

	RubyFeature::RubyFeature(ScriptFeatureClass _featureclass, const wxString &_name)
		: Feature(_featureclass, _name)
	{
	}

	void RubyFeature::RegisterFeature()
	{
		RubyScript::GetScriptObject()->features.push_back(this);

		// get the index+1 it was pushed into
		myid = (int)RubyScript::GetScriptObject()->features.size()-1;
	}

	VALUE RubyFeature::CreateIntegerArray(const std::vector<int> &ints)
	{
		VALUE res = rb_ary_new2(ints.size());
		// create an array-style table with an integer vector in it
		// leave the new table on top of the stack
		for (int i = 0; i != ints.size(); ++i) {
			int k = ints[i];
			rb_ary_push(res, rb_int2inum(k));
		}
		return res;
	}

	void RubyFeature::ThrowError()
	{
/*		wxString err(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 1);
		wxLogError(err);
*/	}


	// RubyFeatureMacro

	VALUE RubyFeatureMacro::RubyRegister(VALUE self, VALUE name, VALUE description, VALUE macro_function, VALUE validate_function)
	{
		wxString _name(StringValueCStr(name), wxConvUTF8);
		wxString _description(StringValueCStr(description), wxConvUTF8);
		RubyFeatureMacro *macro = new RubyFeatureMacro(_name, _description, macro_function, validate_function);
		return Qtrue;
	}

	RubyFeatureMacro::RubyFeatureMacro(const wxString &_name, const wxString &_description, VALUE macro_function, VALUE validate_function)
		: Feature(SCRIPTFEATURE_MACRO, _name)
		, FeatureMacro(_name, _description)
		, RubyFeature(SCRIPTFEATURE_MACRO, _name)
		, macro_fun(macro_function)
		, validation_fun(validate_function)
	{
		no_validate = validate_function == Qnil;
		RegisterFeature();
	}

	bool RubyFeatureMacro::Validate(AssFile *subs, const std::vector<int> &selected, int active)
	{
		if (no_validate)
			return true;

		try {
			RubyAssFile *subsobj = new RubyAssFile(subs, true, true);
			VALUE sel = CreateIntegerArray(selected); // selected items
			RubyObjects::Get()->Register(sel);
			VALUE result = rbFunCall(rb_mKernel, rb_to_id(validation_fun), 3, subsobj->rbAssFile, sel, rb_int2inum(active));
			RubyObjects::Get()->Unregister(sel);
			if(result != Qnil && result != Qfalse)
				return true;
		}catch (const char* e) {
			wxString *err = new wxString(e, wxConvUTF8);
			wxMessageBox(*err, _T("Error running validation function"),wxICON_ERROR | wxOK);			
		}

		return false;
	}

	void RubyFeatureMacro::Process(AssFile *subs, const std::vector<int> &selected, int active, wxWindow * const progress_parent)
	{
		try {
			delete RubyProgressSink::inst;
			RubyProgressSink::inst = new RubyProgressSink(progress_parent, false);
			RubyProgressSink::inst->SetTitle(GetName());
			RubyProgressSink::inst->Show(true);

			// do call
			RubyAssFile *subsobj = new RubyAssFile(subs, true, true);
			VALUE sel = CreateIntegerArray(selected); // selected items
			RubyObjects::Get()->Register(sel);
			VALUE result = rbFunCall(rb_mKernel, rb_to_id(macro_fun), 3, subsobj->rbAssFile, sel, rb_int2inum(active));
			RubyObjects::Get()->Unregister(sel);
			if(result != Qnil && result != Qfalse)
			{
				subsobj->RubyUpdateAssFile(result);
			}
		} catch (const char* e) {
			wxString *err = new wxString(e, wxConvUTF8);
			wxMessageBox(*err, _T("Error running macro"),wxICON_ERROR | wxOK);			
		}
		RubyProgressSink::inst->script_finished = true;
}


	// RubyFeatureFilter
	RubyFeatureFilter::RubyFeatureFilter(const wxString &_name, const wxString &_description, 
		int merit, VALUE _filter_fun, VALUE _dialog_fun)
		: Feature(SCRIPTFEATURE_FILTER, _name)
		, FeatureFilter(_name, _description, merit)
		, RubyFeature(SCRIPTFEATURE_FILTER, _name)
		, filter_fun(_filter_fun)
		, dialog_fun(_dialog_fun)
	{
		has_config = _dialog_fun != Qnil;
		// Works the same as in RubyFeatureMacro
		RegisterFeature();
	}

	void RubyFeatureFilter::Init()
	{
		// Don't think there's anything to do here... (empty in auto3)
	}

	VALUE RubyFeatureFilter::RubyRegister(VALUE self, VALUE name, VALUE description, VALUE merit, VALUE function, VALUE dialog)
	{
		wxString _name(StringValueCStr(name), wxConvUTF8);
		wxString _description(StringValueCStr(description), wxConvUTF8);
		int _merit = rb_num2long(merit);
		RubyFeatureFilter *filter = new RubyFeatureFilter(_name, _description, _merit, function, dialog);
		return Qtrue;
	}

	void RubyFeatureFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{

		// TODO: configuration dialog

		try {
			if (has_config && config_dialog) {
				assert(config_dialog->RubyReadBack() == 1);
				// TODO, write back stored options here
			}
			delete RubyProgressSink::inst;
			RubyProgressSink::inst = new RubyProgressSink(export_dialog, false);
			RubyProgressSink::inst->SetTitle(GetName());
			RubyProgressSink::inst->Show(true);
	
			RubyAssFile *subsobj = new RubyAssFile(subs, true/*modify*/, false/*undo*/);
			VALUE result = rbFunCall(rb_mKernel, rb_to_id(filter_fun), 2, subsobj->rbAssFile, Qnil /* config */);
			if(result != Qnil && result != Qfalse)
			{
				subsobj->RubyUpdateAssFile(result);
			}
		} catch (const char* e) {
			wxString *err = new wxString(e, wxConvUTF8);
			wxMessageBox(*err, _T("Error running filter"),wxICON_ERROR | wxOK);			
		}
		RubyProgressSink::inst->script_finished = true;
	}

	ScriptConfigDialog* RubyFeatureFilter::GenerateConfigDialog(wxWindow *parent)
	{
		if (!has_config)
			return 0;

		//GetFeatureFunction(2); // 2 = config dialog function

		// prepare function call
		// subtitles (don't allow any modifications during dialog creation, ideally the subs aren't even accessed)
//		RubyAssFile *subsobj = new RubyAssFile(AssFile::top, false/*allow modifications*/, false/*disallow undo*/);
		// stored options

/*		if(RubyProgressSink::inst)
		{
			delete RubyProgressSink::inst;
			RubyProgressSink::inst = NULL;
		}
		RubyProgressSink::inst = new RubyProgressSink(parent, false);
		RubyProgressSink::inst->SetTitle(GetName());

		// do call TODO
		RubyProgressSink::inst->ShowModal();

*/		return config_dialog = new RubyConfigDialog(false);
	}


	// RubyProgressSink

	RubyProgressSink::RubyProgressSink(wxWindow *parent, bool allow_config_dialog)
		: ProgressSink(parent)
	{
	}

	RubyProgressSink::~RubyProgressSink()
	{
		// remove progress reporting stuff
		// TODO
	}

	VALUE RubyProgressSink::RubySetProgress(VALUE self, VALUE progress)
	{
		float _progr = rb_num2dbl(progress);
		RubyProgressSink::inst->SetProgress(_progr);
		RubyProgressSink::inst->DoUpdateDisplay();
		wxSafeYield(RubyProgressSink::inst);
		return Qtrue;
	}

	VALUE RubyProgressSink::RubySetTask(VALUE self, VALUE task)
	{
		wxString _t(StringValueCStr(task), wxConvUTF8);
		RubyProgressSink::inst->SetTask(_t);
		RubyProgressSink::inst->DoUpdateDisplay();
		return Qtrue;
	}

	VALUE RubyProgressSink::RubySetTitle(VALUE self, VALUE title)
	{
		wxString _t(StringValueCStr(title), wxConvUTF8);
		RubyProgressSink::inst->SetTitle(_t);
		//wxSafeYield(RubyProgressSink::inst);
		RubyProgressSink::inst->DoUpdateDisplay();
		return Qtrue;
	}

	VALUE RubyProgressSink::RubyGetCancelled(VALUE self)
	{
		if(RubyProgressSink::inst->cancelled)
			return Qtrue;
		return Qfalse;
	}

	VALUE RubyProgressSink::RubyDebugOut(int argc, VALUE *args, VALUE self)
	{
		if(argc > 1 && TYPE(args[0]) == T_FIXNUM) 
		{
			if(FIX2INT(args[0]) > RubyProgressSink::inst->trace_level)
				return Qnil;
		}
		else args[1] = args[0];
		wxString _m(StringValueCStr(args[1]), wxConvUTF8);
		RubyProgressSink::inst->AddDebugOutput(_m);
		RubyProgressSink::inst->DoUpdateDisplay();
		wxSafeYield(RubyProgressSink::inst);
		return Qtrue;
	}

	int RubyProgressSink::RubyDisplayDialog()
	{
		return 0;
	}


	// Factory class for Ruby scripts
	// Not declared in header, since it doesn't need to be accessed from outside
	// except through polymorphism
	class RubyScriptFactory : public ScriptFactory {
	public:
		RubyScriptFactory()
		{
			engine_name = _T("Ruby");
			filename_pattern = _T("*.rb");
			Register(this);
		}

		~RubyScriptFactory() 
		{ 
		}

		virtual Script* Produce(const wxString &filename) const
		{
			// Just check if file extension is .rb
			// Reject anything else
			if (filename.Right(3).Lower() == _T(".rb")) {
				return new RubyScript(filename);
			} else {
				return 0;
			}
		}
	};
	RubyScriptFactory _ruby_script_factory;

	RubyObjects::RubyObjects()
	{
		objects = rb_ary_new();
		rb_gc_register_address(&objects);
	}

	RubyObjects::~RubyObjects()
	{
		rb_gc_unregister_address(&objects);
	}

	RubyObjects *RubyObjects::Get()
	{
		if(inst)
			return inst;
		else
			inst = new RubyObjects;
		return inst;
	}

	void RubyObjects::Register(VALUE obj) {
		rb_ary_push(objects, obj);
	}

	void RubyObjects::Unregister(VALUE obj) {
		rb_ary_delete(objects, obj);
	}

	RubyCallArguments::RubyCallArguments(VALUE _recv, ID _id, int _n, VALUE *_argv)
		:id(_id), n(_n), argv(_argv)
	{
		recv = _recv;
	};

	VALUE rbCallWrapper(VALUE arg)
	{
		RubyCallArguments &a = *reinterpret_cast<RubyCallArguments*>(arg);
		return rb_funcall2(a.recv, a.id, a.n, a.argv);
	}
	VALUE rbExecWrapper(VALUE arg){return  ruby_exec();}
	VALUE rbLoadWrapper(VALUE arg){rb_load(arg, 0); return Qtrue;}
	VALUE rbAss2RbWrapper(VALUE arg){return RubyAssFile::AssEntryToRuby(reinterpret_cast<AssEntry*>(arg));}
	VALUE rb2AssWrapper(VALUE arg){return reinterpret_cast<VALUE>(RubyAssFile::RubyToAssEntry(arg));}

	VALUE rbFunCall(VALUE recv, ID id, int n, ...) 
	{
		VALUE *argv = 0;
		if (n > 0) {
			argv = ALLOCA_N(VALUE, n);
			va_list ar;
			va_start(ar, n);
			int i;
			for(i=0;i<n;i++) {
				argv[i] = va_arg(ar, VALUE);
			}
			va_end(ar);
		} 

		RubyCallArguments arg(recv, id, n, argv);
		int error = 0;
		VALUE result;
		result = rb_protect(rbCallWrapper, reinterpret_cast<VALUE>(&arg), &error);
		if(error) {
			throw StringValueCStr(ruby_errinfo);
		}
		return result;
	}
};
