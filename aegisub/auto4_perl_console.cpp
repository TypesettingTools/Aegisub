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
#ifdef WITH_PERLCONSOLE


#include "auto4_perl.h"
#include "auto4_perl_console.h"
#include "main.h"
#include "frame_main.h"
#include "subs_grid.h"


namespace Automation4 {


////////////////////////////////////
// PerlConsole::Dialog
//

  inline PerlConsole::Dialog::Dialog()
  {
	txt_out = NULL;
  }
  
  inline bool PerlConsole::Dialog::Create(wxWindow* parent, wxWindowID id, const wxString& title,
										  const wxPoint& pos, const wxSize& size,
										  long style, const wxString& name)
  {
	wxDialog::Create(parent, id, title, pos, size, style, name);
	
	// The text controls in the console
	txt_out = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxSize(300,200),
							 wxTE_MULTILINE | wxTE_READONLY | wxTE_CHARWRAP | wxTE_RICH);

	txt_hist = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
							  wxTE_MULTILINE | wxTE_READONLY | wxTE_CHARWRAP | wxTE_RICH);
	txt_in = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, wxDefaultSize,
							wxTE_MULTILINE | wxTE_CHARWRAP | wxTE_PROCESS_ENTER);
	
	// The right panel
	wxBoxSizer *rightpanel = new wxBoxSizer(wxVERTICAL);
	rightpanel->Add(txt_hist, 1, wxEXPAND);
	rightpanel->Add(txt_in, 0, wxEXPAND);
	// And the event handler for the input box
	Connect(txt_in->GetId(), wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(PerlConsole::Dialog::InputEnter));
	
	// The whole dialog
	wxBoxSizer *mainpanel = new wxBoxSizer(wxHORIZONTAL);
	mainpanel->Add(txt_out, 1, wxEXPAND | wxRIGHT, 2);
	mainpanel->Add(rightpanel, 1, wxEXPAND | wxLEFT, 2);
	
	// Getting it to work
	SetSizer(mainpanel);
	mainpanel->SetSizeHints(this);
	
	return true;
  }

  inline void PerlConsole::Dialog::InputEnter(wxCommandEvent& evt)
  {
	if(txt_in->GetInsertionPoint() == txt_in->GetLastPosition() &&
	   txt_in->GetLineLength(txt_in->GetNumberOfLines()-1) == 0) {
	  // If an empty line have been entered...
	  /* TODO: implement an actual command history */
	  *txt_hist << txt_in->GetValue() << PerlConsole::Evaluate(txt_in->GetValue()) << _T("\n");

	  // Resetting the input box
	  txt_in->ChangeValue(_T(""));
	}
	else {
	  // Just a normal line with text
	  txt_in->WriteText(_T("\n"));
	}
  }


//////////////////////
// PerlConsole
//
  PerlConsole *PerlConsole::registered = NULL;

  PerlConsole::PerlConsole(const wxString &name, const wxString &desc, PerlScript *script):
	Feature(SCRIPTFEATURE_MACRO, name),
	/*FeatureMacro(name, description),*/
	PerlFeatureMacro(name, desc, script, NULL, NULL)
  {
	parent_window = NULL;
	dialog = new Dialog();

	// Remove any previously registered console °_°
	if(registered) {
	  registered->script->DeleteFeature(registered);
	}
	registered = this;
  }

  PerlConsole::~PerlConsole()
  {
	if(dialog) dialog->Destroy();

	/* TODO: Free something? */

	// Delete the registered console
	registered = NULL;
  }

  void PerlConsole::Process(AssFile *subs, std::vector<int> &selected, int active, wxWindow * const progress_parent)
  {
	if(!parent_window) {
	  // Create the console's dialog if it doesn't already exist
	  parent_window = progress_parent;
	  dialog->Create(parent_window, -1, GetName(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	}
	// Show the console
	dialog->Show(true);
	// and return, the console will stay visible and permit running other macros
	// the console will 'just' emulate the execution of a macro whenever some code will be evaluated
  }

  wxString PerlConsole::evaluate(const wxString &str)
  {
	/* This mimics FrameMain::OnAutomationMacro */

	// Get a hold of the SubsBox
	SubtitlesGrid *sb = wxGetApp().frame->SubsBox;
	sb->BeginBatch();

	// Create the @_ (global <.<)
	AV *AT = get_av("_", 1);
	av_clear(AT);
	// $_[0]
	AV *lines = PerlAss::MakeHasshLines(NULL, AssFile::top);
	av_push(AT, newRV_noinc((SV*)lines));
	// $_[1]
	std::vector<int> selected_lines = sb->GetAbsoluteSelection();
	AV *selected_av = newAV();
	VECTOR_AV(selected_lines, selected_av, int, iv);
	av_push(AT, newRV_noinc((SV*)selected_av));
	// $_[2]
	int first_sel = sb->GetFirstSelRow();
	av_push(AT, newSViv(first_sel));

// Clear all maps from the subs grid before running the macro
// The stuff done by the macro might invalidate some of the iterators held by the grid, which will cause great crashing
	sb->Clear();

	// Here we go
	script->WriteVars();
	// Box the code into the right package
	wxString code = _T("package ") + script->GetPackage() + _T(";\n");
	// Add the user's code
	code << str;
	// Evaluate the code
	SV *e = eval_pv(code.mb_str(wx2pl), 0);
	/* TODO: use threaded calls */
	/*PerlThread eval(code.mb_str(wx2pl), 1, PerlThread::EVAL);
	  e = (SV*)eval.Wait();*/
	/* TODO: check for errors */
	script->ReadVars();

	// Recreate the top assfile from perl hassh
	//AssFile::top->FlagAsModified(GetName());
	PerlAss::MakeAssLines(AssFile::top, lines);
	av_undef(lines);
	// And reset selection vector
	selected_lines.clear();
	AV_VECTOR(selected_av, selected_lines, IV);
	CHOP_SELECTED(AssFile::top, selected_lines);
	av_undef(selected_av);

// Have the grid update its maps, this properly refreshes it to reflect the changed subs
	sb->UpdateMaps();
	sb->SetSelectionFromAbsolute(selected_lines);
	sb->CommitChanges(true, false);
	sb->EndBatch();

	// The eval's return
	return wxString(SvPV_nolen(e), pl2wx);
  }

  wxString PerlConsole::Evaluate(const wxString &str)
  {
	if(registered) {
	  return registered->evaluate(str);
	}
	else {
	  /* TODO: print error */
	  return _T("");
	}
  }

  void PerlConsole::Echo(const wxString &str)
  {
	if(registered && registered->dialog->txt_out) {
	  *(registered->dialog->txt_out) << str << _T("\n");
	}
	else {
	  PerlIO_printf(PerlIO_stdout(), "%s\n", str.c_str());
	}
  }
  
  
};


#endif //WITH_PERLCONSOLE
#endif //WITH_PERL
