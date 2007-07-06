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

#pragma once

#ifndef _AUTO4_RUBY_H
#define _AUTO4_RUBY_H

#pragma warning(disable: 4311 4312)

#include "auto4_base.h"
#include <wx/thread.h>
#include <wx/event.h>
#include <ruby.h>

class wxWindow;

namespace Automation4 {

	
	// Provides access to an AssFile object (and all lines contained) for a Ruby script
	class RubyAssFile {
	private:
		AssFile *ass;

		bool can_modify;
		bool can_set_undo;

		// keep a cursor of last accessed item to avoid walking over the entire file on every access
		std::list<AssEntry*>::iterator last_entry_ptr;
		int last_entry_id;

		static int RubyParseTagData();
		static int RubyUnparseTagData();
		static int RubySetUndoPoint();

	public:
		void RubyUpdateAssFile(VALUE subtitles);
		static VALUE AssEntryToRuby(AssEntry *e); // makes a Ruby representation of AssEntry
		static AssEntry *RubyToAssEntry(VALUE ass_entry); // creates an AssEntry object from a Ruby representation
		RubyAssFile(AssFile *_ass, bool _can_modify, bool _can_set_undo);
		~RubyAssFile();

		static RubyAssFile *raf;
		VALUE rbAssFile;

	};


	// Provides progress UI and control functions for a Ruby script
	class RubyProgressSink : public ProgressSink {
	private:

	public:
		RubyProgressSink(wxWindow *parent, bool allow_config_dialog = true);
		virtual ~RubyProgressSink();
		static RubyProgressSink* inst;
		static VALUE RubySetProgress(VALUE self, VALUE progress);
		static VALUE RubySetTask(VALUE self, VALUE task);
		static VALUE RubySetTitle(VALUE self, VALUE title);
		static VALUE RubyGetCancelled(VALUE self);
		static VALUE RubyDebugOut(int argc, VALUE *args, VALUE self);
		static VALUE RubyDisplayDialog(VALUE self, VALUE cfg, VALUE buttons);
	};


	// Provides Config UI functions for a Ruby script
	class RubyConfigDialogControl {
	public:
		wxControl *cw; // control window
		wxString name, hint;
		int x, y, width, height;
		VALUE name_sym;

		virtual wxControl *Create(wxWindow *parent) = 0;
		virtual void ControlReadBack() = 0;
		virtual VALUE RubyReadBack() = 0;

		RubyConfigDialogControl();
		RubyConfigDialogControl(VALUE opts);
		virtual ~RubyConfigDialogControl() { }
	};

	class RubyConfigDialog : public ScriptConfigDialog {
	private:
		std::vector<RubyConfigDialogControl*> controls;
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
		RubyConfigDialog(VALUE cfg, VALUE buttons, bool show_buttons);
		virtual ~RubyConfigDialog();
		VALUE RubyReadBack(); // read back internal structure to Ruby hash

		void ReadBack(); // from auto4 base
	};


	// Second base-class for Ruby implemented Features
	class RubyFeature : public virtual Feature {
	protected:
		
		int myid;

		RubyFeature(ScriptFeatureClass _featureclass, const wxString &_name);

		void RegisterFeature();
		VALUE CreateIntegerArray(const std::vector<int> &ints);
		void ThrowError();
	};


	// Class of Ruby scripts
	class RubyScript : public Script {
		friend class RubyFeature;
		friend class RubyProgressSink;

	private:

		void Create(); // load script and create internal structures etc.
		void Destroy(); // destroy internal structures, unreg features and delete environment

		static RubyScript* GetScriptObject();
	public:
		static VALUE RubyTextExtents(VALUE self, VALUE style, VALUE text);
		static VALUE RubyFrameToTime(VALUE self, VALUE frame);
		static VALUE RubyTimeToFrame(VALUE self, VALUE time);
		static VALUE RubyKeyFrames(VALUE self);
		static VALUE backtrace_hook(VALUE self, VALUE backtr);
		
		RubyScript(const wxString &filename);
		static void RubyError();
		static wxString GetError();
		virtual ~RubyScript();
		virtual void Reload();
		static RubyScript* inst;
	};


	// Implementation of the Macro Feature for Ruby scripts
	class RubyFeatureMacro : public FeatureMacro, RubyFeature {
	private:
		bool no_validate;
		VALUE macro_fun;
		VALUE validation_fun;

	protected:
		RubyFeatureMacro(const wxString &_name, const wxString &_description, VALUE macro_function, VALUE validate_function);
	public:
		static VALUE RubyRegister(VALUE self, VALUE name, VALUE description, VALUE macro_function, VALUE validate_function);
		virtual ~RubyFeatureMacro() { }

		virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active);
		virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent);
	};


	// Implementation of the Export Filter Feature for Ruby scripts
	class RubyFeatureFilter : public FeatureFilter, RubyFeature {
	private:
		bool has_config;
		RubyConfigDialog *config_dialog;
		VALUE filter_fun;
		VALUE dialog_fun;

	protected:
		RubyFeatureFilter(const wxString &_name, const wxString &_description, int merit, VALUE function, VALUE dialog);

		ScriptConfigDialog* GenerateConfigDialog(wxWindow *parent);

		void Init();
	public:
		static VALUE RubyRegister(VALUE self, VALUE name, VALUE description, VALUE merit, VALUE macro_function, VALUE validate_function);

		void ProcessSubs(AssFile *subs, wxWindow *export_dialog);
	};

	// class for registering ruby objects created in c++
	// so garbage collector doesn't destroy them
	class RubyObjects {
	private:
		VALUE objects;
		static RubyObjects *inst;
		RubyObjects();
	public:
		~RubyObjects();
		static RubyObjects *Get();
		void Register(VALUE object);
		void Unregister(VALUE object);
	};

	// stuff for safe calling of ruby functions
	struct RubyCallArguments {
		VALUE recv;
		ID id;
		int n;
		VALUE *argv;
		RubyCallArguments(VALUE _recv, ID _id, int _n, VALUE *_argv);
	};

	// Separate thread for ruby interpreter
	class RubyThread : public wxThread {
	private:
		enum {NOTHING, CALL_FUNCTION, LOAD_FILE};
		int action;
		int status;
		RubyCallArguments *args;
		const char* file;
		VALUE *result;
		void InitRuby();
		wxPathList include_path;
	public:
		RubyThread(wxPathList include_path);
		void CallFunction(RubyCallArguments* arg, VALUE *res);
		void LoadFile(const char* file);
		int GetStatus() {return status;};
		virtual ExitCode Entry();
	};

	VALUE rbCallWrapper(VALUE arg);
	VALUE rbExecWrapper(VALUE arg);
	VALUE rbLoadWrapper(VALUE arg);
	VALUE rbGcWrapper(VALUE arg);
	VALUE rbAss2RbWrapper(VALUE arg);
	VALUE rb2AssWrapper(VALUE arg);
	VALUE rbError(VALUE arg);
	typedef VALUE (*RB_HOOK)(...);
	typedef VALUE (*RB_HOOK2)(VALUE);
	
#define STR2SYM(x) ID2SYM(rb_intern(x))
};

#endif
