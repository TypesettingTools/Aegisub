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
//#include "ass_dialogue.h"

#undef _
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "auto4_perldata.inc"  // Parl variables manipulation macros


// the fucking perl.h redefines _() -.- please disregard warnings during compilation
#undef _
#define _(s)	wxGetTranslation(_T(s))


// String conversions between wxWidgets and Perl
#define wx2pl	wxConvUTF8
#define pl2wx	wxConvUTF8


#define PERL_SCRIPT_EXTENSION ".pl"  /* TODO maybe: make it multi-extension */


namespace Automation4 {


///////////
// XSUBS
//
  void xs_perl_script(pTHX);
  void xs_perl_misc(pTHX);
  void xs_perl_console(pTHX);


///////////////////
// Script object
//
  class PerlFeatureMacro;
  class PerlScript : public Script {
  private:
	static PerlScript *active;	// The active script (at any given time)

	AV *inc_saved;
	wxString package;	// Every script resides in a package named at random

	bool reload;	// Automatically reload if source file has changed
	time_t mtime;	// The mtime of the loaded source file
	
	void load();	// It doas all the script initialization
	void unload();	// It does all the script disposing

	static void activate(PerlScript *script);	// Set the active script /* TODO: add @INC hacking */
	static void deactivate();	// Unset the active script

  public:
	PerlScript(const wxString &filename);
	virtual ~PerlScript();

	virtual void Reload();	// Reloading of a loaded script

	void Activate()       { activate(this); }	// Set the script as active
	void Deactivate() const { deactivate(); }	// Unset the active script

	const wxString& GetPackage() const { return package; }	// The perl package containing script code
	static PerlScript *GetScript() { return active; }		// Query the value of the active script

	/* TODO maybe: change these into tying of scalars */
	void ReadVars();		// Sync the script's vars from perl package to script object
	void WriteVars() const;	// Sync the script's vars from script object to perl package

	/* TODO: add c++ equivalents */
	static XS(set_info);	// Aegisub::Script::set_info()
	void AddFeature(Feature *feature);
	void DeleteFeature(Feature *feature);
	static XS(register_macro);		// Aegisub::Script::register_macro()
	static XS(register_console);	// Aegisub::Script::register_console() /* TODO: move this into PerlConsole class */

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


/////////////////////////
// Misc utility functions
//
  class PerlLog {
  public:
	static XS(log_warning);	// Aegisub::warn()
  };

  XS(text_extents);	// Aegisub::text_extents()


};


#endif
