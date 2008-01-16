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
#include "ass_file.h"


namespace Automation4 {

  
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
	/* This is crappy and creepy at the same time */
	/* TODO: thoroughly recheck the code */
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

	// Sync script's vars with package's
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

	script->Deactivate();

	return ret;
  }

  void PerlFeatureMacro::Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent)
  {
	// Reference to the hassh (lines)
	AV *lines = PerlAss::MakeHasshLines(NULL, subs);
	// Selection array
	AV *selected_av = newAV();
	VECTOR_AV(selected, selected_av, int, iv);

	script->Activate();

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
	
	// Call back the callback :)
	call_sv(processing_sub, G_EVAL | G_VOID);

	if(SvTRUE(ERRSV)) {
	  // Error
	  wxLogError(wxString(SvPV_nolen(ERRSV), pl2wx));
	}
	else {
	  // Non-error: recreate the hassh :S
	  subs->FlagAsModified(GetName());
	  PerlAss::MakeAssLines(subs, lines);
	  // And reset selection vector
	  selected.clear();
	  AV_VECTOR(selected_av, selected, IV);
	  CHOP_SELECTED(subs, selected);
	}

	// Clean everything
	FREETMPS;
	LEAVE;

	script->Deactivate();
  }


};


#endif //WITH_PERL
