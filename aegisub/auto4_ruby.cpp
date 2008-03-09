// Copyright (c) 2007, Patryk Pomykalski
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
// Contact: mailto:pomyk@go2.pl
//

#ifdef WITH_RUBY

#ifdef _MSC_VER
#pragma warning(disable: 4003)
#endif

#include "auto4_ruby.h"
#include "auto4_ruby_factory.h"
#include "auto4_auto3.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_file.h"
#include "ass_override.h"
#include "text_file_reader.h"
#include "options.h"
#include "vfr.h"
#include "video_context.h"
#include "main.h"
#include "frame_main.h"
#include "subs_grid.h"
#include <ruby.h>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/window.h>
#include <assert.h>
#include <algorithm>


namespace Automation4 {

	RubyObjects *RubyObjects::inst = NULL;
	RubyScript * RubyScript::inst = NULL; // current Ruby Script
	RubyProgressSink* RubyProgressSink::inst = NULL;
	RubyThread* ruby_thread = NULL;
	wxSemaphore* ruby_thread_sem = NULL;
	wxSemaphore* ruby_script_sem = NULL;
	VALUE RubyAegisub = Qfalse;
	wxString backtrace = _T("");
	wxString error = _T("");

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

	void RubyThread::CallFunction(RubyCallArguments* arg, VALUE *res)
	{
		args = arg;
		result = res;
		action = CALL_FUNCTION;
	}

	void RubyThread::LoadFile(const char *f)
	{
		file = f;
		action = LOAD_FILE;
	}

	RubyScript::~RubyScript()
	{
	}

	void RubyScript::Create()
	{
		Destroy();
		RubyScript::inst = this;

		try {
			if(ruby_thread == NULL)
			{
				ruby_thread_sem = new wxSemaphore(0, 1);
				ruby_script_sem = new wxSemaphore(0, 1);
				ruby_thread = new RubyThread(include_path);
				ruby_script_sem->Wait();
			}

			wxCharBuffer buf = GetFilename().mb_str(wxConvISO8859_1);
			const char *t = buf.data();
			ruby_thread->LoadFile(t);
			ruby_thread_sem->Post();
			ruby_script_sem->Wait();
			if(ruby_thread->GetStatus())
				RubyScript::RubyError();

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
		catch (...) {
			Destroy();
			loaded = false;
			throw;
		}
	}

	void RubyScript::Destroy()
	{

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


	VALUE RubyScript::RubyTextExtents(VALUE /*self*/, VALUE _style, VALUE _text)
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

	VALUE RubyScript::RubyFrameToTime(VALUE /*self*/, VALUE frame)
	{
		if(TYPE(frame) == T_FIXNUM && VFR_Output.IsLoaded())
		{
			return INT2FIX(VFR_Output.GetTimeAtFrame(FIX2INT(frame), true));
		}
		return Qnil;
	}

	VALUE RubyScript::RubyTimeToFrame(VALUE /*self*/, VALUE time)
	{
		if(TYPE(time) == T_FIXNUM && VFR_Output.IsLoaded())
		{
			return INT2FIX(VFR_Output.GetFrameAtTime(FIX2INT(time), true));
		}
		return Qnil;
	}

	//////////////////////////////////////////////////////////////////////////
	// output: [[keyframe indices], [keyframe times in ms]]
	VALUE RubyScript::RubyKeyFrames(VALUE /*self*/)
	{
		if(!VideoContext::Get()->KeyFramesLoaded())
			return Qnil;

		wxArrayInt key_frames = VideoContext::Get()->GetKeyFrames();

		VALUE frames = rb_ary_new();
		VALUE times = rb_ary_new();

		for(unsigned int i = 0; i < key_frames.size(); ++i)
		{
			rb_ary_push(frames, INT2FIX(key_frames[i]));
			rb_ary_push(times, INT2FIX(VFR_Output.GetTimeAtFrame(key_frames[i], true)));
		}
		VALUE res = rb_ary_new();
		rb_ary_push(res, frames);
		rb_ary_push(res, times);
		return res;
	}

	wxString RubyScript::GetError()
	{
		return wxString(error + _T("\n") + backtrace);
	}

	void RubyScript::RubyError()
	{
		wxMessageBox(RubyScript::inst->GetError(), _T("Error"),wxICON_ERROR | wxOK);
		error = _T("");
		backtrace = _T("");
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
		for (unsigned int i = 0; i < ints.size(); ++i) {
			int k = ints[i];
			rb_ary_push(res, rb_int2inum(k));
		}
		return res;
	}

	void RubyFeature::ThrowError()
	{
	//	wxString err(_T("Error running script") + RubyScript::inst->GetError());
	//	wxLogError(err);
	}


	// RubyFeatureMacro

	VALUE RubyFeatureMacro::RubyRegister(VALUE /*self*/, VALUE name, VALUE description, VALUE macro_function, VALUE validate_function)
	{
		wxString _name(StringValueCStr(name), wxConvUTF8);
		wxString _description(StringValueCStr(description), wxConvUTF8);
		RubyFeatureMacro *macro = new RubyFeatureMacro(_name, _description, macro_function, validate_function);
		(void)macro;
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

		RubyProgressSink::inst = NULL;
		RubyAssFile *subsobj = new RubyAssFile(subs, true, true);
		VALUE *argv = ALLOCA_N(VALUE, 3);
		argv[0] = subsobj->rbAssFile;
		argv[1] = CreateIntegerArray(selected); // selected items;
		argv[2] = INT2FIX(active);
		RubyCallArguments arg(rb_mKernel, rb_to_id(validation_fun), 3, argv);
		VALUE result;
		ruby_thread->CallFunction(&arg, &result);
		ruby_thread_sem->Post();
		ruby_script_sem->Wait();
		if(ruby_thread->GetStatus())
			RubyScript::RubyError();
		if(result != Qnil && result != Qfalse) {
			return true;
		}

		return false;
	}

	void RubyFeatureMacro::Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent)
	{
		delete RubyProgressSink::inst;
		RubyProgressSink::inst = new RubyProgressSink(progress_parent, false);
		RubyProgressSink::inst->SetTitle(GetName());

		// do call
		RubyAssFile *subsobj = new RubyAssFile(subs, true, true);
		VALUE *argv = ALLOCA_N(VALUE, 3);
		argv[0] = subsobj->rbAssFile;
		argv[1] = CreateIntegerArray(selected); // selected items;
		argv[2] = INT2FIX(active);
		RubyCallArguments arg(rb_mKernel, rb_to_id(macro_fun), 3, argv);
		VALUE result;
		ruby_thread->CallFunction(&arg, &result);
		ruby_thread_sem->Post();
		RubyProgressSink::inst->ShowModal();
		ruby_script_sem->Wait();
		delete RubyProgressSink::inst;
		RubyProgressSink::inst = NULL;
		if(ruby_thread->GetStatus())
			RubyScript::RubyError();
		else if(TYPE(result) == T_ARRAY)
		{
			rb_gc_disable();
			bool end = false;
			for(int i = 0; i < RARRAY(result)->len && !end; ++i)
			{
				VALUE p = RARRAY(result)->ptr[i];		// some magic in code below to allow variable output
				if(TYPE(p) != T_ARRAY) {
					p = result;
					end = true;
				}

				switch(TYPE(RARRAY(p)->ptr[0])) {

				case T_HASH:		// array of hashes = subs
					subsobj->RubyUpdateAssFile(p);
					break;

				case T_FIXNUM:		// array of ints = selection
					// i hope this works, can't test it  -jfs
					int num = RARRAY(p)->len;
					selected.clear();
					selected.reserve(num);
					for(int i = 0; i < num; ++i) {
						selected.push_back(FIX2INT(RARRAY(p)->ptr[i]));
					}
					break;
				}				
			}
			rb_gc_enable();
		}
		delete subsobj;
	}

	// RubyThread
	void RubyThread::InitRuby()
	{
	#if defined(NT)
		int argc = 0;
		char **argv = 0;
		NtInitialize(&argc, &argv);
	#endif
		ruby_init();
		ruby_init_loadpath();
		error = _T("");
		backtrace = _T("");
		if(!RubyAegisub) {
			RubyAegisub = rb_define_module("Aegisub");
			rb_define_module_function(RubyAegisub, "register_macro",reinterpret_cast<RB_HOOK>(&RubyFeatureMacro::RubyRegister), 4);
			rb_define_module_function(RubyAegisub, "register_filter",reinterpret_cast<RB_HOOK>(&RubyFeatureFilter::RubyRegister), 5);
			rb_define_module_function(RubyAegisub, "text_extents",reinterpret_cast<RB_HOOK>(&RubyScript::RubyTextExtents), 2);
			rb_define_module_function(RubyAegisub, "frame_to_time",reinterpret_cast<RB_HOOK>(&RubyScript::RubyFrameToTime), 1);
			rb_define_module_function(RubyAegisub, "time_to_frame",reinterpret_cast<RB_HOOK>(&RubyScript::RubyTimeToFrame), 1);
			rb_define_module_function(RubyAegisub, "key_frames",reinterpret_cast<RB_HOOK>(&RubyScript::RubyKeyFrames), 0);
			rb_define_module_function(rb_eException, "set_backtrace",reinterpret_cast<RB_HOOK>(&RubyScript::backtrace_hook), 1);
			rb_define_module_function(RubyAegisub, "progress_set",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetProgress), 1);
			rb_define_module_function(RubyAegisub, "progress_task",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetTask), 1);
			rb_define_module_function(RubyAegisub, "progress_title",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubySetTitle), 1);
			rb_define_module_function(RubyAegisub, "debug_out",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubyDebugOut), -1);
			rb_define_module_function(RubyAegisub, "get_cancelled",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubyGetCancelled), 0);
			rb_define_module_function(RubyAegisub, "display_dialog",reinterpret_cast<RB_HOOK>(&RubyProgressSink::RubyDisplayDialog), 2);
		}
		VALUE paths = rb_gv_get("$:");
		for(unsigned int i = 0; i < include_path.GetCount(); i++)
		{
			rb_ary_push(paths, rb_str_new2(include_path[i].mb_str(wxConvISO8859_1)));
		}

	}

	RubyThread::RubyThread(wxPathList paths)
		: wxThread(wxTHREAD_JOINABLE)
		,include_path(paths)
		,action(NOTHING)
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

	wxThread::ExitCode RubyThread::Entry()
	{
		InitRuby();
		ruby_script_sem->Post();
		do {
			ruby_thread_sem->Wait();
			status = 0;
			switch(action)
			{
			case LOAD_FILE:
				rb_protect(rbLoadWrapper, rb_str_new2(file), &status);
				break;
			case CALL_FUNCTION:
				*result = rb_protect(rbCallWrapper, reinterpret_cast<VALUE>(args), &status);
				if(RubyProgressSink::inst)
				{
					RubyProgressSink::inst->script_finished = true;
					wxWakeUpIdle();
				}
				break;
			}
			ruby_script_sem->Post();
		}while(1);
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

	VALUE RubyFeatureFilter::RubyRegister(VALUE /*self*/, VALUE name, VALUE description, VALUE merit, VALUE function, VALUE dialog)
	{
		wxString _name(StringValueCStr(name), wxConvUTF8);
		wxString _description(StringValueCStr(description), wxConvUTF8);
		int _merit = rb_num2long(merit);
		RubyFeatureFilter *filter = new RubyFeatureFilter(_name, _description, _merit, function, dialog);
		(void)filter;
		return Qtrue;
	}

	void RubyFeatureFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog)
	{

		try {
			VALUE cfg = 0;
			if (has_config && config_dialog) {
				cfg = config_dialog->RubyReadBack();
				// TODO, write back stored options here
			}
			RubyProgressSink::inst = new RubyProgressSink(export_dialog, false);
			RubyProgressSink::inst->SetTitle(GetName());
	
			RubyAssFile *subsobj = new RubyAssFile(subs, true/*modify*/, false/*undo*/);
			VALUE *argv = ALLOCA_N(VALUE, 2);
			argv[0] = subsobj->rbAssFile;
			argv[1] = cfg; // config
			RubyCallArguments arg(rb_mKernel, rb_to_id(filter_fun), 2, argv);
			VALUE result;
			ruby_thread->CallFunction(&arg, &result);
			ruby_thread_sem->Post();
			RubyProgressSink::inst->ShowModal();
			ruby_script_sem->Wait();
			if(ruby_thread->GetStatus())
				RubyScript::RubyError();
			RubyProgressSink::inst = NULL;
			delete RubyProgressSink::inst;
			if(TYPE(result) == T_ARRAY)
			{
				rb_gc_disable();
				subsobj->RubyUpdateAssFile(result);
				rb_gc_enable();
			}
			delete subsobj;
		} catch (const char* e) {
			wxString *err = new wxString(e, wxConvUTF8);
			wxMessageBox(*err, _T("Error running filter"),wxICON_ERROR | wxOK);
		}
	}

	ScriptConfigDialog* RubyFeatureFilter::GenerateConfigDialog(wxWindow *parent)
	{
		if (!has_config)
			return 0;

		delete RubyProgressSink::inst;
		RubyProgressSink::inst = new RubyProgressSink(parent, false);
		RubyProgressSink::inst->SetTitle(GetName());

		// prepare function call
		// subtitles (don't allow any modifications during dialog creation, ideally the subs aren't even accessed)
		RubyAssFile *subsobj = new RubyAssFile(AssFile::top, false/*allow modifications*/, false/*disallow undo*/);

		VALUE *argv = ALLOCA_N(VALUE, 2);
		argv[0] = subsobj->rbAssFile;
		argv[1] = Qnil; // TODO: stored options
		RubyCallArguments arg(rb_mKernel, rb_to_id(dialog_fun), 2, argv);
		VALUE dialog_data;
		ruby_thread->CallFunction(&arg, &dialog_data);
		ruby_thread_sem->Post();
		RubyProgressSink::inst->ShowModal();
		ruby_script_sem->Wait();
		if(ruby_thread->GetStatus())
			RubyScript::RubyError();
		delete RubyProgressSink::inst;
		RubyProgressSink::inst = NULL;

		return config_dialog = new RubyConfigDialog(dialog_data, Qnil, false);
	}


	// RubyProgressSink

	RubyProgressSink::RubyProgressSink(wxWindow *parent, bool /*allow_config_dialog*/)
		: ProgressSink(parent)
	{
	}

	RubyProgressSink::~RubyProgressSink()
	{
	}

	VALUE RubyProgressSink::RubySetProgress(VALUE /*self*/, VALUE progress)
	{
		float _progr = rb_num2dbl(progress);
		RubyProgressSink::inst->SetProgress(_progr);
		return Qtrue;
	}

	VALUE RubyProgressSink::RubySetTask(VALUE /*self*/, VALUE task)
	{
		wxString _t(StringValueCStr(task), wxConvUTF8);
		RubyProgressSink::inst->SetTask(_t);
		return Qtrue;
	}

	VALUE RubyProgressSink::RubySetTitle(VALUE /*self*/, VALUE title)
	{
		wxString _t(StringValueCStr(title), wxConvUTF8);
		RubyProgressSink::inst->SetTitle(_t);
		return Qtrue;
	}

	VALUE RubyProgressSink::RubyGetCancelled(VALUE /*self*/)
	{
		if(RubyProgressSink::inst->cancelled)
			return Qtrue;
		return Qfalse;
	}

	VALUE RubyProgressSink::RubyDebugOut(int argc, VALUE *args, VALUE /*self*/)
	{
		if(argc > 1 && TYPE(args[0]) == T_FIXNUM) 
		{
			if(FIX2INT(args[0]) > RubyProgressSink::inst->trace_level)
				return Qnil;
		}
		else args[1] = args[0];
		wxString _m(StringValueCStr(args[1]), wxConvUTF8);
		RubyProgressSink::inst->AddDebugOutput(_m);
		return Qtrue;
	}

	VALUE RubyProgressSink::RubyDisplayDialog(VALUE /*self*/, VALUE dialog_data, VALUE buttons)
	{
		// Send the "show dialog" event
		ShowConfigDialogEvent evt;

		RubyConfigDialog dlg(dialog_data, buttons, true); // magically creates the config dialog structure etc
		evt.config_dialog = &dlg;

		wxSemaphore sema(0, 1);
		evt.sync_sema = &sema;
		RubyProgressSink::inst->AddPendingEvent(evt);
		sema.Wait();
		return dlg.RubyReadBack();
	}

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
	VALUE rbExecWrapper(VALUE /*arg*/){return  ruby_exec();}
	VALUE rbLoadWrapper(VALUE arg){rb_load(arg, 0); return Qtrue;}
	VALUE rbGcWrapper(VALUE /*arg*/){rb_gc_start(); return Qtrue;}
	VALUE rbAss2RbWrapper(VALUE arg){return RubyAssFile::AssEntryToRuby(reinterpret_cast<AssEntry*>(arg));}
	VALUE rb2AssWrapper(VALUE arg){return reinterpret_cast<VALUE>(RubyAssFile::RubyToAssEntry(arg));}

	VALUE RubyScript::backtrace_hook(VALUE self, VALUE backtr)
	{
		int len = RARRAY(backtr)->len;
		VALUE err = rb_funcall(self, rb_intern("to_s"), 0);
		error = wxString(StringValueCStr(err), wxConvUTF8);
		for(int i = 0; i < len; ++i)
		{
			VALUE str = RARRAY(backtr)->ptr[i];
			wxString line(StringValueCStr(str), wxConvUTF8);
			backtrace.Append(line + _T("\n"));
		}
		return backtr;
	}

	Script* RubyScriptFactory::Produce(const wxString &filename) const
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

#endif // WITH_RUBY
