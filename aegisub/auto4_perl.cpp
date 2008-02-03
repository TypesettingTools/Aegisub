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


#ifdef WITH_PERL


#include "auto4_perl.h"
#include "auto4_perl_console.h"
#include "options.h"
#include "ass_style.h"


#define COLLECT_PV(buf, s, e) \
	buf = wxString(SvPV_nolen(ST(s)), pl2wx);\
	for(int ARG_i = s+1;  ARG_i <= e; ARG_i++) {\
		buf << _T(" ") << wxString(SvPV_nolen(ST(ARG_i)), pl2wx);\
	}


namespace Automation4 {

  
///////////////////////////////////
// Perl -> C++ interface (XSUBS)
//

  /* package Aegisub */
  XS(perl_log)
  {
	wxTRACE_FUNC(Aegisub::log);
	dXSARGS;
	IV level = 6;

	int start = 0;
	if(items >= 2 && SvIOK(ST(0))) {
	  level = SvIV(ST(0));
	  start = 1;
	}
	wxString msg;
	COLLECT_PV(msg, start, items-1);
	PerlLog(level, msg);
  }

  XS(perl_warning)
  {
	wxTRACE_FUNC(Aegisub::warn);
	dXSARGS;

	if(items >= 1) {
	  wxString buf;
	  COLLECT_PV(buf, 0, items-1);
	  PerlLogWarning(buf);
	}
  }


  XS(perl_text_extents)
  {
	wxTRACE_FUNC(Aegisub::text_extents);
	dXSARGS;

	// Read the parameters
	SV *style; wxString text;
	if(items >= 2) {
	  // Enough of them
	  style = sv_mortalcopy(ST(0));
	  text = wxString(SvPV_nolen(ST(1)), pl2wx);
	}
	else {
	  PerlLogWarning(_("Not enough parameters for Aegisub::text_extents()"));
	  // We needed 2 parameters at least!
	  XSRETURN_UNDEF;
	}

	// Get the AssStyle
	AssStyle *s;
	if(SvROK(style)) {
	  // Create one from the hassh
	  s = PerlAss::MakeAssStyle((HV*)SvRV(style));
	}
	else {
	  // It's the name of the style
	  wxString sn(SvPV_nolen(style), pl2wx);
	  // We get it from the AssFile::top
	  s = AssFile::top->GetStyle(sn);
	  /* TODO maybe: make it dig from the current hassh's styles */
	  if(!s)
		XSRETURN_UNDEF;
	}

	// The return parameters
	double width, height, descent, extlead;
	// The actual calculation
	if(!CalculateTextExtents(s, text, width, height, descent, extlead)) {
	  /* TODO: diagnose error */
	  XSRETURN_EMPTY;
	}

	// Returns
	switch(GIMME_V) {
	case G_SCALAR:
	  // Scalar context
	  XSRETURN_NV(width);
	  break;
	default:
	case G_ARRAY:
	  // List context
	  EXTEND(SP, 4);
	  XST_mNV(0, width);
	  XST_mNV(1, height);
	  XST_mNV(2, descent);
	  XST_mNV(3, extlead);
	  XSRETURN(4);
	}
  }

  /* Aegisub::Script */
  XS(perl_script_set_info)
  {
	wxTRACE_FUNC(Aegisub::Script::set_info);
	dXSARGS;

	PerlScript *active = PerlScript::GetScript();
	if(active) {
	  // Update the object's vars
	  active->ReadVars();

	  // We want at most 4 parameters :P
	  if(items > 4) items = 4;
	  // Set script info vars
	  switch (items) {
	  case 4:
		active->SetVersion(wxString(SvPV_nolen(ST(3)), pl2wx));
	  case 3:
		active->SetAuthor(wxString(SvPV_nolen(ST(2)), pl2wx));
	  case 2:
		active->SetDescription(wxString(SvPV_nolen(ST(1)), pl2wx));
	  case 1:
		active->SetName(wxString(SvPV_nolen(ST(0)), pl2wx));
	  }

	  // Update the package's vars
	  active->WriteVars();
	}
  }

  XS(perl_script_register_macro)
  {
	wxTRACE_FUNC(Aegisub::Script::register_macro);
	dXSARGS;

	PerlScript *active = PerlScript::GetScript();
	if(active && items >= 3) {
	  wxString name, description;
	  SV *proc_sub = NULL, *val_sub = NULL;

	  if(items > 4) items = 4;
	  switch (items) {
	  case 4:
		val_sub  = sv_mortalcopy(ST(3));
	  case 3:
		proc_sub = sv_mortalcopy(ST(2));
		description = wxString(SvPV_nolen(ST(1)), pl2wx);
		name = wxString(SvPV_nolen(ST(0)), pl2wx);
	  }
	  if(proc_sub) {
		active->AddFeature(new PerlFeatureMacro(name, description, active, proc_sub, val_sub));
		XSRETURN_YES;
	  }
	}
	XSRETURN_UNDEF;
  }

  /* Aegisub::Progress */
  XS(perl_progress_set)
  {
	wxTRACE_FUNC(Aegisub::Progress::set_progress);
	dXSARGS;

	PerlProgressSink *ps = PerlProgressSink::GetProgressSink();
	if(ps && items >= 1) {
	  NV pc = SvNV(ST(0));
	  if(pc <= 1) pc *= 100;
	  if(pc > 100) pc = 100;
	  ps->SetProgress(pc);
	  wxWakeUpIdle();
	}
  }

  XS(perl_progress_task)
  {
	wxTRACE_FUNC(Aegisub::Progress::set_task);
	dXSARGS;

	PerlProgressSink *ps = PerlProgressSink::GetProgressSink();
	if(ps && items >= 1) {
	  wxString task;
	  COLLECT_PV(task, 0, items-1);
	  ps->SetTask(task);
	  wxWakeUpIdle();
	}
  }

  XS(perl_progress_title)
  {
	wxTRACE_FUNC(Aegisub::Progress::set_title);
	dXSARGS;

	PerlProgressSink *ps = PerlProgressSink::GetProgressSink();
	if(ps && items >= 1) {
	  wxString title;
	  COLLECT_PV(title, 0, items-1);
	  ps->SetTitle(title);
	  wxWakeUpIdle();
	}
  }

  XS(perl_progress_cancelled)
  {
	wxTRACE_FUNC(Aegisub::Progress::is_cancelled);
	dMARK; dAX;

	if(PerlProgressSink *ps = PerlProgressSink::GetProgressSink()) {
	  if(ps->IsCancelled())	XSRETURN_YES;
	  else XSRETURN_NO;
	}
	else {
	  XSRETURN_UNDEF;
	}
  }

  /* Aegisub::PerlConsole */
  XS(perl_console_register)
  {
	wxTRACE_FUNC(Aegisub::PerlConsole::register_console);
#ifdef WITH_PERLCONSOLE
	dXSARGS;

	PerlScript *script = PerlScript::GetScript();
	if(script) {
	  wxString name = _T("Perl console");
	  wxString desc = _T("Show the Perl console");
	  switch (items) {
	  case 2:
		desc = wxString(SvPV_nolen(ST(1)), pl2wx);
	  case 1:
		name = wxString(SvPV_nolen(ST(0)), pl2wx);
	  }

	  if(!PerlConsole::GetConsole())
		// If there's no registered console
		script->AddFeature(new PerlConsole(name, desc, script));
	}
	XSRETURN_YES;
#else
	dMARK; dAX;
	PerlLogWarning(_("Tried to register PerlConsole, but support for it was disabled in this version."));  // Warning or Hint?
	XSRETURN_UNDEF;
#endif
  }

  XS(perl_console_echo)
  {
	wxTRACE_FUNC(Aegisub::PerlConsole::echo);
	dXSARGS;

	// We should get some parameters
	if(items == 0) return;

	// Join the params in a unique string :S
	wxString buffer = wxString(SvPV_nolen(ST(0)), pl2wx);
	for(int i = 1; i < items; i++) {
	  buffer << _T(" ") << wxString(SvPV_nolen(ST(i)), pl2wx);
	}

#ifdef WITH_PERLCONSOLE
	if(PerlConsole::GetConsole()) {
	  // If there's a console echo to it
	  PerlConsole::Echo(buffer);
	}
	else
#endif
	  // Otherwise print on stdout
	  PerlIO_printf(PerlIO_stdout(), "%s\n", buffer.mb_str(wxConvLocal).data());
	  // (through perl io system)
  }

  /* Universal loader */
  EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);  

  /* XS registration */
  EXTERN_C void xs_perl_main(pTHX)
  {
	dXSUB_SYS;
	
	/* DynaLoader is a special case */
	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);

	// My XSUBS ^^
	newXS("Aegisub::log", perl_log, __FILE__);
	newXS("Aegisub::warn", perl_warning, __FILE__);
	newXS("Aegisub::text_extents", perl_text_extents, __FILE__);
	newXS("Aegisub::Script::set_info", perl_script_set_info, __FILE__);
	newXS("Aegisub::Script::register_macro", perl_script_register_macro, __FILE__);
	newXS("Aegisub::Progress::set_progress", perl_progress_set, __FILE__);
	newXS("Aegisub::Progress::set_task", perl_progress_task, __FILE__);
	newXS("Aegisub::Progress::set_title", perl_progress_title, __FILE__);
	newXS("Aegisub::Progress::is_cancelled", perl_progress_cancelled, __FILE__);
	newXS("Aegisub::PerlConsole::echo", perl_console_echo, __FILE__);
	newXS("Aegisub::PerlConsole::register_console", perl_console_register, __FILE__);
  }


/////////////
// PerlLog
//
  void PerlLog(unsigned int level, const wxString &msg)
  {
	PerlProgressSink *ps = PerlProgressSink::GetProgressSink();
	if(!(level & 0x8) && ps) {
	  wxString _msg;
	  // Prepend a description of the log line
	  switch(level) {
	  case 0: _msg = _("Fatal error: ");
		break;
	  case 1: _msg = _("Error: ");
		break;
	  case 2: _msg = _("Warning: ");
		break;
	  case 3: _msg = _("Hint: ");
		break;
	  case 4: _msg = _("Debug: ");
		break;
	  case 5: _msg = _("Trace: ");
	  }
	  // Print onto the progress window
	  ps->Log(level >= 6 ? -1 : level, _msg+msg+_T("\n"));
	}
	else {
	  level &= 0x7;
	  // Use the wx log functions
	  switch(level) {
	  case 0: wxLogFatalError(msg);
		break;
	  case 1: wxLogError(msg);
		break;
	  case 2: wxLogWarning(msg);
		break;
	  case 3: wxLogVerbose(msg);
		break;
	  case 4: wxLogDebug(msg);
		break;
	  case 5: wxLogTrace(wxTRACE_AutoPerl, msg);
		break;
	  default:
	  case 6: wxLogMessage(msg);
	  }
	}
  }


////////////////
// PerlThread
//

  PerlThread::PerlThread():
	wxThread(wxTHREAD_JOINABLE)
  {
	pv = NULL; sv = NULL;
  }

  PerlThread::PerlThread(const char *sub_name, I32 flags, bool type):
	wxThread(wxTHREAD_JOINABLE)
  {
	wxTRACE_METH(PerlThread);
	if(type == CALL) Call(sub_name, flags);
	if(type == EVAL) Eval(sub_name, flags);
  }

  PerlThread::PerlThread(SV *sv, I32 flags, bool type):
	wxThread(wxTHREAD_JOINABLE)
  {
	wxTRACE_METH(PerlThread);
	if(type == CALL) Call(sv, flags);
	if(type == EVAL) Eval(sv, flags);
  }

  wxThreadError PerlThread::launch()
  {
	wxThreadError e = Create();
	if(e != wxTHREAD_NO_ERROR) return e;

	switch(Options.AsInt(_T("Automation Thread Priority"))) {
	case 2: SetPriority(10);
	  break;
	case 1: SetPriority(30);
	  break;
	default:
	case 0: SetPriority(50); // fallback normal
	}

	wxTRACE_RET(PerlThread);
	return Run();
  }

  wxThreadError PerlThread::Call(const char *sub_name, I32 _flags)
  {
	type = CALL; pv = sub_name; flags = _flags;
	wxLogTrace(wxTRACE_AutoPerl, _T("type = CALL, pv = '%s', flags = %u"), wxString(pv, pl2wx).c_str(), flags);
	return launch();
  }
  wxThreadError PerlThread::Call(SV *_sv, I32 _flags)
  {
	type = CALL; sv = _sv; flags = _flags;
	wxLogTrace(wxTRACE_AutoPerl, _T("type = CALL, sv = %p, flags = %u"), sv, flags);
	return launch();
  }

  wxThreadError PerlThread::Eval(const char* p, I32 croak_on_error)
  {
	type = EVAL; pv = p; flags = croak_on_error;
	wxLogTrace(wxTRACE_AutoPerl, _T("type = EVAL, pv = '%s', flags = %u"), wxString(pv, pl2wx).c_str(), flags);
	return launch();
  }
  wxThreadError PerlThread::Eval(SV* _sv, I32 _flags)
  {
	type = EVAL; sv = _sv; flags = _flags;
	wxLogTrace(wxTRACE_AutoPerl, _T("type = EVAL, sv = %p, flags = %u"), sv, flags);
	return launch();
  }

  wxThread::ExitCode PerlThread::Entry()
  {
	wxTRACE_METH(Entry);

	PerlProgressSink *ps;
	if(ps = PerlProgressSink::GetProgressSink()) {
	  // If there's a progress sink...
	  while(!ps->has_inited);
	  // ...wait for it to have inited
	}

	ExitCode ec = NULL;
	switch(type) {
	case CALL:
	  if(sv) ec = (ExitCode)((size_t)call_sv(sv, flags));
	  else if(pv) ec = (ExitCode)((size_t)call_pv(pv, flags));
	  break;
	case EVAL:
	  if(sv) ec = (ExitCode)((size_t)eval_sv(sv, flags));
	  else if(pv) ec = (ExitCode)((size_t)eval_pv(pv, flags));
	}

	if(SvTRUE(ERRSV)) {
	  // Log $@ in case of error
	  PerlLogError(wxString(SvPV_nolen(ERRSV), pl2wx));
	}

	if(ps) {
	  ps->script_finished = true;
	  wxWakeUpIdle();
	}

	wxTRACE_RET(Entry);
	return ec;
  }


///////////////////////
// PerlScriptFactory
//
  class PerlScriptFactory : public ScriptFactory {
  private:
	PerlInterpreter *parser;
	bool loaded;

  public:
	PerlScriptFactory()
	{
#ifdef WXTRACE_AUTOPERL
	  // Add tracing of perl engine operations
	  wxLog::AddTraceMask(wxTRACE_AutoPerl);
#endif

	  // Script engine properties
	  loaded = false;
	  engine_name = _T("Perl");
	  filename_pattern = _T("*") _T(PERL_SCRIPT_EXTENSION);

	  // On Visual Studio, first check if the dll is available
	  // This needs to be done because it is set to delay loading of this dll
#ifdef __VISUALC__
	  HMODULE dll = LoadLibrary(_T("perl510.dll"));
	  if (!dll) return;
	  FreeLibrary(dll);
#endif
	  
	  // Perl interpreter initialization (ONE FOR ALL THE SCRIPTS)
	  int argc = 3;
	  char *argv[3] = { "aegisub", "-e", "0" };
	  char** env = NULL;
	  char **argv2 = (char**) argv;  // VC++ wants this °_°
	  PERL_SYS_INIT3(&argc,&argv2,&env);
	  parser = perl_alloc();
	  perl_construct(parser);
	  perl_parse(parser, xs_perl_main,
				 argc, argv,
				 NULL);
	  //free(argv);
	  // (That was pretty magic o_O)

	  // Let's register the perl script factory \o/
	  Register(this);
	  loaded = true;
	}
	
	~PerlScriptFactory()
	{
	  // Perl interpreter deinitialization
      if (loaded) {
		perl_destruct(parser);
	    perl_free(parser);
	    PERL_SYS_TERM();
	  }
	}
	
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

  // The one and only (thank goodness ¬.¬) perl engine!!!
  PerlScriptFactory _perl_script_factory;

};


#endif //WITH_PERL
