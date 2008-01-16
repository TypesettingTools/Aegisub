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
#include "version.h"
#include "standard_paths.h"


namespace Automation4 {


  void xs_perl_script(pTHX)
  {
	newXS("Aegisub::Script::set_info", PerlScript::set_info, __FILE__);
	newXS("Aegisub::Script::register_macro", PerlScript::register_macro, __FILE__);
  }


//////////////////////
// PerlScript class
//
  PerlScript *PerlScript::active = NULL;

  PerlScript::PerlScript(const wxString &filename):
	Script(filename)
  {
	// Create a package name for the script
	package.Printf(_T("Aegisub::Script::p%lx"), this);

	inc_saved = newAV();

	reload = false;
	mtime = 0;

	// Load the code
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
	wxLogTrace("Loading %*s inside %s", 0, GetFilename().c_str(), package.c_str());

	// Feed some defaults into the script info
	name = GetPrettyFilename().BeforeLast(_T('.'));
	description = _("Perl script");
	author = wxString(getlogin(), wxConvLibc);
	version = GetAegisubShortVersionString();

	// Get file's mtime
	struct stat s;
	stat(GetFilename().mb_str(wxConvLibc), &s);
	mtime = s.st_mtime;

	// Create the script's package
	gv_stashpv(package.mb_str(wx2pl), 1);
	// Set this script as active
	activate(this);

	// 'Enclose' the script into its package
	wxString _script = _T("package ") + package + _T(";\n")
	  _T("our ($_script_reload, $_script_path, $_script_package);\n") // Internal vars
	  _T("our ($script_name, $script_description, $script_author, $script_version);\n") // Package info
	  _T("open SCRIPT, $_script_path;\n")  // Open the script file
	  _T("local @_source = <SCRIPT>;\n")   // read the source
	  _T("close SCRIPT;\n")                // close the file
	  _T("eval \"@{_source}\n1;\" || die $@;"); // eval the source

	// Let's eval the 'boxed' script
	eval_pv(_script.mb_str(wx2pl), 0);
	if(SvTRUE(ERRSV)) {
	  wxLogError(wxString(SvPV_nolen(ERRSV), pl2wx));
	  loaded = false;
	}
	else {
	  loaded = true;
	}

	// The script has done loading (running)
	deactivate();
  }
  
  void PerlScript::unload() {
	wxLogTrace("Unloading %*s(%s)", 0, name, package.c_str());

	// Deinstantiate(?) all features and clear the vector
	for(; !features.empty(); features.pop_back()) {
	  delete (Feature*) features.back();
	}
	features.clear();

	// Dismiss the package's stash
	hv_undef((HV*)gv_stashpv(package.mb_str(wx2pl), 0));

	// Officially finished with unloading
	loaded = false;
  }

  void PerlScript::activate(PerlScript *script)
  {
	wxLogTrace("Activating %*s(%s)", 0, script->GetName(), script->GetPackage().c_str());

	// Check if the source file is newer
	if(script->reload) {
	  struct stat s;
	  stat(script->GetFilename().mb_str(wxConvLibc), &s);
	  if(script->mtime != s.st_mtime) {
		printf("%d != %d !\n", script->mtime, s.st_mtime);
		wxLogVerbose(_("Reloading %s because the file on disk (%s) changed"), script->GetName().c_str(), script->GetFilename().c_str());
		script->Reload();
	  }
	}

	// Hooking $SIG{__WARN__}
	wxLogTrace("Hooking $SIG{__WARN__}", 0);
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
	  // Add the include paths
	  for(I32 i = 0; i < inc_count; i++) {
		wxLogDebug("Adding %d to @INC", include_path.Item(i).c_str());
		AV_TOUCH(inc_av, i)
		  AV_STORE(newSVpv(script->include_path.Item(i).mb_str(wx2pl), 0));
	  }
	  wxLogTrace("@INC = ( %*s )", 0, SvPV_nolen(eval_pv("\"@INC\"", 1)));
	}
	else {
	  wxLogWarning(_("Unable to add the automation include path(s) to @INC, you may have problems running the script."));
	}

	// Set the values of script vars
	script->WriteVars();
	active = script;
	wxLogDebug("%s(%p) activated", active->GetName().c_str(), active);
  }

  void PerlScript::deactivate()
  {
	wxLogTrace("Deactivating %*s (%s)", 0, active->GetName().c_str(), active->GetPackage().c_str());

	// Revert @INC to its value before the script activation
	AV *inc_av = get_av("main::INC", 0);
	if(inc_av) {
	  dAV;

	  // Reset @INC
	  if(av_len(active->inc_saved) >= 0) {
		// If there's a saved one
		AV_COPY(active->inc_saved, inc_av);
		wxLogTrace("@INC = ( %*s )", 0, SvPV_nolen(eval_pv("\"@INC\"", 1)));
		av_clear(active->inc_saved);
	  }
	}
	
	// Read the values of script vars
	active->ReadVars();

	// Unooking $SIG{__WARN__}
	wxLogTrace("Releasing $SIG{__WARN__} hook", 0);
	eval_pv("undef $SIG{__WARN__}", 1);

	wxLogDebug("%s(%p) deactivated", active->GetName().c_str(), active);
	active = NULL;
  }
  
  void PerlScript::AddFeature(Feature *feature)
  {
	features.push_back(feature);
	wxLogDebug("Added %s to %s(%s)'s features", feature->GetName(), name, package);
  }

  void PerlScript::DeleteFeature(Feature *feature)
  {
	for(std::vector<Feature*>::iterator it = features.begin(); it != features.end(); it++)
	  if(*it == feature) {
		delete feature;
		wxLogDebug("Deleted %s from %s(%s)'s features", feature->GetName(), name, package);
		features.erase(it);
	  }
  }

  void PerlScript::ReadVars()
  {
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

	bitch = package + _T("::_script_reload");
	whore = get_sv(bitch.mb_str(wx2pl), 0);
	if(whore) reload = SvTRUE(whore);
  }

  void PerlScript::WriteVars() const
  {
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

  XS(PerlScript::set_info)
  {
	dXSARGS;
	if(active) {
	  // Update the object's vars
	  active->ReadVars();

	  // Set script info vars
	  switch (items) {
	  case 4:
		active->version = wxString(SvPV_nolen(ST(3)), pl2wx);
	  case 3:
		active->author = wxString(SvPV_nolen(ST(2)), pl2wx);
	  case 2:
		active->description = wxString(SvPV_nolen(ST(1)), pl2wx);
	  case 1:
		active->name = wxString(SvPV_nolen(ST(0)), pl2wx);
	  }

	  // Update the package's vars
	  active->WriteVars();
	}
  }

  XS(PerlScript::register_macro)
  {
	dXSARGS;
	if(active && items >= 3) {
	  wxString name, description;
	  SV *proc_sub = NULL, *val_sub = NULL;
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


};


#endif //WITH_PERL
