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
#include "version.h"
#include "standard_paths.h"
#include <wx/filename.h>
#include <wx/utils.h>


#ifdef __VISUALC__
#pragma warning(disable: 4800)
#pragma warning(disable: 4706)
#endif


namespace Automation4 {


//////////////////////
// PerlScript class
//
  PerlScript *PerlScript::active = NULL;

  PerlScript::PerlScript(const wxString &filename):
	Script(filename)
  {
	// Create a package name for the script
	package.Printf(_T("Aegisub::Script::p%lx"), this);

	// local @INC; # lol
	inc_saved = newAV();

	// Buggy
	reload = false;
	mtime = 0;

	// Load the script
	load();
  }

  PerlScript::~PerlScript()
  {
	unload();
  }

  void PerlScript::Reload()
  {
	unload();
	reload = false;
	load();
  }

  void PerlScript::load()
  {
	wxTRACE_METH(load);
	wxLogTrace(wxTRACE_AutoPerl, _T("filename = '%s', package = '%s'"), GetFilename().c_str(), package.c_str());

	// Feed some defaults into the script info
	name = GetPrettyFilename().BeforeLast(_T('.'));
	description = _("Perl script");
	author = wxGetUserId();
	version = GetAegisubShortVersionString();

	wxFileName fn(GetFilename());
	wxDateTime mod;
	fn.GetTimes(NULL,&mod,NULL);
	mtime = mod.GetTicks();

	// Create the script's package
	gv_stashpv(package.mb_str(wx2pl), 1);
	// Set this script as active
	activate(this);

	// 'Enclose' the script into its package
	wxString _script = _T("package ") + package + _T(";\n")
	  _T("require Aegisub; require Aegisub::Script; require Aegisub::Progress;") // Core modules
	  _T("our ($_script_reload, $_script_path, $_script_package);\n") // Internal vars
	  _T("our ($script_name, $script_description, $script_author, $script_version);\n") // Package info
	  _T("open SCRIPT, $_script_path;\n")  // Open the script file
	  _T("local @_source = <SCRIPT>;\n")   // read the source
	  _T("close SCRIPT;\n")                // close the file
	  _T("eval \"@{_source}\n1;\" || die $@;"); // eval the source

	// Let's eval the 'boxed' script
	eval_pv(_script.mb_str(wx2pl), 0);
	SV *_err = newSVsv(ERRSV);  // We need this later
	// Done running
	deactivate();
	// and check on errors
	if(SvTRUE(_err)) {
	  description = wxString(SvPV_nolen(_err), pl2wx);
	  loaded = false;
	}
	else {
	  loaded = true;
	}

	wxTRACE_RET(load);
  }
  
  void PerlScript::unload() {
	wxTRACE_METH(unload);
	wxLogTrace(wxTRACE_AutoPerl, _T("name = '%s' package = '%s'"), name.c_str(), package.c_str());

	// Deinstantiate(?) all features and clear the vector
	for(; !features.empty(); features.pop_back()) {
	  delete (Feature*) features.back();
	}
	features.clear();

	// Dismiss the package's stash
	hv_undef((HV*)gv_stashpv(package.mb_str(wx2pl), 0));

	// Officially finished with unloading
	wxLogDebug(_T("'%s' (%s) unloaded"), name.c_str(), package.c_str());
	loaded = false;
	wxTRACE_RET(unload);
  }

  void PerlScript::activate(PerlScript *script)
  {
	wxTRACE_FUNC(PerlScript::activate);
	wxLogTrace(wxTRACE_AutoPerl, _T("name = '%s',  package = '%s'"), script->GetName().c_str(), script->GetPackage().c_str());

	// Hooking $SIG{__WARN__}
	wxLogTrace(wxTRACE_AutoPerl, _T("$SIG{__WARN__} = \\&Aegisub::warn"));
	eval_pv("$SIG{__WARN__} = \\&Aegisub::warn", 1);

	// Add the script's includes to @INC
	AV *inc_av = get_av("main::INC", 0);
	if(inc_av) {
	  dAV;

	  // Save the previous includes
	  AV_COPY(inc_av, script->inc_saved);

	  // Make room in @INC
	  I32 inc_count = script->include_path.GetCount();
	  av_unshift(inc_av, inc_count);
	  // Add the automation include paths
	  for(I32 i = 0; i < inc_count; i++) {
		wxLogTrace(wxTRACE_AutoPerl, _T("$INC[%d] = '%s'"), i, script->include_path.Item(i).c_str());
		AV_TOUCH(inc_av, i)
		  AV_STORE(newSVpv(script->include_path.Item(i).mb_str(wx2pl), 0));
	  }
	  wxLogDebug(_T("@INC = ( %s )"), wxString(SvPV_nolen(eval_pv("\"@INC\"", 1)), pl2wx).c_str());
	}
	else {
	  PerlLogWarning(_("Unable to add the automation include path(s) to @INC: the script's code may not compile or execute properly."));
	}

	// Set the values of script vars
	script->WriteVars();

	active = script;
	wxLogDebug(_T("'%s' (%p) activated"), active->GetName().c_str(), active);
  }

  void PerlScript::deactivate()
  {
	wxTRACE_FUNC(PerlScript::deactivate);
	wxLogTrace(wxTRACE_AutoPerl, _T("name = '%s', package = '%s'"), active->GetName().c_str(), active->GetPackage().c_str());

	// Revert @INC to its value before the script activation
	AV *inc_av = get_av("main::INC", 0);
	if(inc_av) {
	  dAV;

	  // Reset @INC
	  if(av_len(active->inc_saved) >= 0) {
		// If there's a saved one
		AV_COPY(active->inc_saved, inc_av);
		wxLogDebug(_T("@INC = ( %s )"), wxString(SvPV_nolen(eval_pv("\"@INC\"", 1)), pl2wx).c_str());
		av_clear(active->inc_saved);
	  }
	}
	
	// Read the values of script vars
	active->ReadVars();

	// If reload flag is set...
	/* STILL BROKEN :< */
	if(active->reload) {
	  // check if the source file on disk changed
	  wxFileName fn(active->GetFilename());
	  wxDateTime mod;
	  fn.GetTimes(NULL,&mod,NULL);
	  if(active->mtime != mod.GetTicks()) {
		// and reload the script
		PerlLogVerbose(wxString::Format(_("Reloading %s because the file on disk (%s) changed."), active->GetName().c_str(), active->GetFilename().c_str()));
		active->Reload();
	  }
	}

	// Unhooking $SIG{__WARN__}
	wxLogTrace(wxTRACE_AutoPerl, _T("undef $SIG{__WARN__}"));
	eval_pv("undef $SIG{__WARN__}", 1);

	wxLogDebug(_T("%s(%p) deactivated"), active->GetName().c_str(), active);
	active = NULL;
  }
  
  void PerlScript::AddFeature(Feature *feature)
  {
	wxTRACE_METH(AddFeature);
	features.push_back(feature);
	wxLogDebug(_T("Added '%s' to '%s'(%s)'s features"), feature->GetName().c_str(), name.c_str(), package.c_str());
  }

  void PerlScript::DeleteFeature(Feature *feature)
  {
	wxTRACE_METH(DeleteFeature);
	for(std::vector<Feature*>::iterator it = features.begin(); it != features.end(); it++)
	  if(*it == feature) {
		delete feature;
		wxLogDebug(_T("Deleted '%s' from '%s'(%s)'s features"), feature->GetName().c_str(), name.c_str(), package.c_str());
		features.erase(it);
	  }
  }

  void PerlScript::ReadVars()
  {
	wxTRACE_METH(ReadVars);
	// This will get anything inside it °_°
	SV *whore = NULL;
	// All the vars' names will stick to it #_#
	wxString bitch;

	bitch = package + _T("::script_name");
	whore = get_sv(bitch.mb_str(wx2pl), 0);
	if(whore) name = wxString(SvPV_nolen(whore), pl2wx);

	bitch = package + _T("::script_description");
	whore = get_sv(bitch.mb_str(wx2pl), 0);
	if(whore) description = wxString(SvPV_nolen(whore), pl2wx);

	bitch = package + _T("::script_author");
	whore = get_sv(bitch.mb_str(wx2pl), 0);
	if(whore) author = wxString(SvPV_nolen(whore), pl2wx);

	bitch = package + _T("::script_version");
	whore = get_sv(bitch.mb_str(wx2pl), 0);
	if(whore) version = wxString(SvPV_nolen(whore), pl2wx);

	//bitch = package + _T("::_script_reload");
	//whore = get_sv(bitch.mb_str(wx2pl), 0);
	//if(whore) reload = SvTRUE(whore);
  }

  void PerlScript::WriteVars() const
  {
	wxTRACE_METH(WriteVars);
	// Somewhat as above
	SV *whore = NULL;
	wxString bitch;

	bitch = package + _T("::_script_package");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, package.mb_str(wx2pl));

	bitch = package + _T("::_script_path");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, GetFilename().mb_str(wx2pl));

	bitch = package + _T("::_script_reload");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setiv(whore, int(reload));

	bitch = package + _T("::script_name");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, name.mb_str(wx2pl));

	bitch = package + _T("::script_description");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, description.mb_str(wx2pl));

	bitch = package + _T("::script_author");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, author.mb_str(wx2pl));

	bitch = package + _T("::script_version");
	whore = get_sv(bitch.mb_str(wx2pl), 1);
	sv_setpv(whore, version.mb_str(wx2pl));
  }

  
//////////////////////
// PerlFeatureMacro
//

  PerlFeatureMacro::PerlFeatureMacro(const wxString &name, const wxString &description, PerlScript *own_script, SV *proc_sub, SV *val_sub):
	Feature(SCRIPTFEATURE_MACRO, name),
	FeatureMacro(name, description)
  {
	// We know what script we belong to ^_^
	script = own_script;

	// And not surprisingly we have some callbacks too
	processing_sub = newSVsv(proc_sub);
	validation_sub = newSVsv(val_sub);
  }

  PerlFeatureMacro::~PerlFeatureMacro() {
	// The macro subroutines get undefined
	CV *cv = Nullcv;
	HV *hv = NULL;
	GV *gv = NULL;
	if(processing_sub) {
	  cv = sv_2cv(processing_sub, &hv, &gv, 1);
	  cv_undef(cv);
	  if(hv) hv_undef(hv);
	}
	if(validation_sub) {
	  cv = sv_2cv(validation_sub, &hv, &gv, 1);
	  cv_undef(cv);
	  if(hv) hv_undef(hv);
	}
  };
  
  bool PerlFeatureMacro::Validate(AssFile *subs, const std::vector<int> &selected, int active)
  {
	// If there's no validation subroutine defined simply return true
	if(!validation_sub) return true;
	// otherwise...

	// Sub lines
	AV *lines = PerlAss::MakeHasshLines(NULL, subs);
	// Selection array
	AV *selected_av = newAV();
	VECTOR_AV(selected, selected_av, int, iv);

	// Activate the owner script
	script->Activate();

	bool ret = false;
	int c = 0;

	// Prepare the stack
	dSP;

	ENTER;
	SAVETMPS;

	// Push the parameters on the stack
	PUSHMARK(SP);
	XPUSHs(sv_2mortal(newRV_noinc((SV*)lines)));
	XPUSHs(sv_2mortal(newRV_noinc((SV*)selected_av)));
	XPUSHs(sv_2mortal(newSViv(active)));
	PUTBACK;

	// Call back the callback
	c = call_sv(validation_sub, G_EVAL | G_SCALAR);
	SPAGAIN;

	if(SvTRUE(ERRSV)) {
	  wxLogVerbose(wxString(SvPV_nolen(ERRSV), pl2wx));
	  ret = false;
	}
	else {
	  SV *wtf = sv_mortalcopy(POPs);
	  ret = SvTRUE(wtf);
	}

	// Tidy up everything
	PUTBACK;
	FREETMPS;
	LEAVE;

	// Deactivate the script
	script->Deactivate();

	return ret;
  }

  void PerlFeatureMacro::Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent)
  {
	/* TODO: extend the progress window 'coverage' */
	// Convert the AssFile::Line to perl stuff
	AV *lines = PerlAss::MakeHasshLines(NULL, subs);
	// Same with the selection array
	AV *selected_av = newAV();
	VECTOR_AV(selected, selected_av, int, iv);

	// Prepare the stack
	dSP;
	ENTER;
	SAVETMPS;

	// Push the arguments onto the stack
	PUSHMARK(SP);
	SV* lines_ref = sv_2mortal(newRV_noinc((SV*)lines));
	XPUSHs(lines_ref);
	SV* selected_ref = sv_2mortal(newRV_noinc((SV*)selected_av));
	XPUSHs(selected_ref);
	XPUSHs(sv_2mortal(newSViv(active)));
	PUTBACK;

	// Create a progress window
	PerlProgressSink *ps = new PerlProgressSink(progress_parent, GetName());
	// Start the callback thread
	script->Activate();
	PerlThread call(processing_sub, G_EVAL | G_VOID);
	// Show the progress window until it is dismissed
	ps->ShowModal();
	// Now wait the thread to return
	call.Wait();
	script->Deactivate();

	if(!SvTRUE(ERRSV)) {
	  // Show progress sink again
	  ps->Show(true);
	  ps->SetTask(_("Saving changes"));

	  // Recreate the ass :S
	  subs->FlagAsModified(GetName());
	  PerlAss::MakeAssLines(subs, (AV*)SvRV(lines_ref));
	  // And reset selection vector
	  selected.clear();
	  AV_VECTOR((AV*)SvRV(selected_ref), selected, IV);
	  CHOP_SELECTED(subs, selected);

	  ps->Hide();
	}
	// Delete the progress sink
	ps->Destroy();

	// Clean the call stack
	FREETMPS;
	LEAVE;
  }


};


#endif //WITH_PERL
