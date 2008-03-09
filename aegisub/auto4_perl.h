// Copyright (c) 2008, Simone Cociancich
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
#ifndef _AUTO4_PERL_H
#define _AUTO4_PERL_H


#include "auto4_base.h"
#include <wx/window.h>
#include <wx/string.h>

#include "ass_file.h"

#undef _
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "auto4_perldata.inc"  // Perl variables manipulation macros
#undef bool

// the fucking perl.h redefines _() -.-
#undef _
#define _(s)	wxGetTranslation(_T(s))


// String conversions between wxWidgets and Perl
#define wx2pl	wxConvUTF8
#define pl2wx	wxConvUTF8


#define PERL_SCRIPT_EXTENSION ".pl"  /* TODO maybe: make it multi-extension */


// Debug support
/* define the following to activate tracing for the perl engine */
#define WXTRACE_AUTOPERL
#define wxTRACE_AutoPerl _T("auto4_perl")

#define wxTRACE_METH(name) \
	wxLogTrace(wxTRACE_AutoPerl, _T("\t=== %p::%s() ==="), this, _T(#name))

#define wxTRACE_FUNC(name) \
	wxLogTrace(wxTRACE_AutoPerl, _T("\t=== %s() ==="), _T(#name))

#define wxTRACE_RET(name) \
	wxLogTrace(wxTRACE_AutoPerl, _T("\t___ %s() returned ___"), _T(#name))


namespace Automation4 {


/////////////
// PerlLog
//
#define LOG_FATAL	0
#define LOG_ERROR	1
#define LOG_WARNING	2
#define LOG_HINT	3
#define LOG_DEBUG	4
#define LOG_TRACE	5
#define LOG_MESSAGE	6

#define LOG_WX		8

#define PerlLogFatal(str)		PerlLog(LOG_FATAL, str)
#define PerlLogFatalError(str)	PerlLog(LOG_FATAL, str)
#define PerlLogError(str)		PerlLog(LOG_ERROR, str)
#define PerlLogWarning(str)		PerlLog(LOG_WARNING, str)
#define PerlLogHint(str)		PerlLog(LOG_HINT, str)
#define PerlLogVerbose(str)		PerlLog(LOG_HINT, str)
#define PerlLogDebug(str)		PerlLog(LOG_DEBUG, str)
#define PerlLogTrace(str)		PerlLog(LOG_TRACE, str)
#define PerlLogMessage(str)		PerlLog(LOG_MESSAGE, str)

  void PerlLog(unsigned int level, const wxString &msg);


////////////////
// PerlThread
//
  class PerlThread : public wxThread {
  private:
	const char *pv;
	SV *sv;
	I32 flags;

	bool type;

	wxThreadError launch();

  public:
	enum { EVAL = 0, CALL = 1 };

	PerlThread();
	PerlThread(const char *sub_name, I32 flags, bool type = CALL);
	PerlThread(SV *sv, I32 flags, bool type = CALL);

	wxThreadError Call(const char *sub_name, I32 flags);
	wxThreadError Call(SV *sv, I32 flags);
	wxThreadError Eval(const char* p, I32 croak_on_error);
	wxThreadError Eval(SV* sv, I32 flags);

	virtual ExitCode Entry();
  };


///////////////////
// Script object
//
  class PerlScript : public Script {
  private:
	static PerlScript *active;	// The active script (at any given time)

	AV *inc_saved;
	wxString package;	// Every script resides in a package named at random

	bool reload;	// Automatically reload if source file has changed
	time_t mtime;	// The mtime of the loaded source file
	
	void load();	// It doas all the script initialization
	void unload();	// It does all the script disposing

	static void activate(PerlScript *script);	// Set the active script
	static void deactivate();	// Unset the active script

  public:
	PerlScript(const wxString &filename);
	virtual ~PerlScript();
	static PerlScript *GetScript() { return active; }		// Query the value of the active script

	virtual void Reload();	// Reloading of a loaded script

	void Activate()       { activate(this); }	// Set the script as active
	void Deactivate() const { deactivate(); }	// Unset the active script

	/* TODO maybe: move to tied scalars */
	void ReadVars();		// Sync the script's vars from perl package to script object
	void WriteVars() const;	// Sync the script's vars from script object to perl package

	void AddFeature(Feature *feature);
	void DeleteFeature(Feature *feature);

	const wxString& GetPackage() const { return package; }	// The perl package containing script code
	void SetName(const wxString &str) { name = str; }
	void SetDescription(const wxString &str) { description = str; }
	void SetAuthor(const wxString &str) { author = str; }
	void SetVersion(const wxString &str) { version = str; }
  };


//////////////////
// Macro object
//
  class PerlFeatureMacro : public FeatureMacro {
  private:
	SV *processing_sub;	// May be reference or name of sub
	SV *validation_sub;	// here too
	
  protected:
	PerlScript *script;	// The owner script
	
  public:
	PerlFeatureMacro(const wxString &name, const wxString &description, PerlScript *perl_script, SV *proc_sub, SV *val_sub);
	virtual ~PerlFeatureMacro();
	
	virtual bool Validate(AssFile *subs, const std::vector<int> &selected, int active);
	virtual void Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent);
  };


//////////////////////
// PerlProgressSink
//
  class PerlProgressSink : public ProgressSink {
  private:
	static PerlProgressSink *sink;
  public:
	PerlProgressSink(wxWindow *parent, const wxString &title = _T("..."));
	~PerlProgressSink();
	static PerlProgressSink *GetProgressSink() { return sink; }

	bool IsCancelled() const { return cancelled; }
	void Log(int level, const wxString &message) { if(level <= trace_level) AddDebugOutput(message); }
  };


///////////////////////////////////////////////////
// Conversion between aegisub data and perl data
//
  class PerlAss {
  private:

  public:
	static wxString GetEntryClass(AssEntry *entry);

	static HV *MakeHasshEntry(AssEntry *entry);
	static HV *MakeHasshStyle(AssStyle *style);
	static HV *MakeHasshDialogue(AssDialogue *diag);
	static AV *MakeHasshLines(AV *lines, AssFile *ass);

	static AssEntry *MakeAssEntry(HV *entry);
	static AssStyle *MakeAssStyle(HV *style);
	static AssDialogue *MakeAssDialogue(HV *diag);
	static AssFile *MakeAssLines(AssFile *ass, AV *lines);
  };


///////////////////////
// PerlScriptFactory
//
  class PerlScriptFactory : public ScriptFactory {
  private:
	PerlInterpreter *parser;
	bool loaded;

  public:
	PerlScriptFactory();
	~PerlScriptFactory();
	
	virtual Script* Produce(const wxString &filename) const
	{
	  if(filename.EndsWith(_T(PERL_SCRIPT_EXTENSION))) {
		return new PerlScript(filename);
	  }
	  else {
		return 0;
	  }
	}
  };

};


#endif
