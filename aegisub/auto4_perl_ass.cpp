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
#include "ass_entry.h"
#include "ass_style.h"
#include "ass_dialogue.h"
#include "ass_attachment.h"


// Disable warning
#ifdef __VISUALC__
#pragma warning(disable: 4800)
#endif

// For wxString::Trim
#define right  true
#define left  false


#define HASSH_BASIC_INIT(ae, he) \
	HV_TOUCH(he, "raw", 3)\
		HV_STORE(newSVpv(ae->GetEntryData().mb_str(wx2pl), 0));\
	HV_TOUCH(he, "section", 7)\
		HV_STORE(newSVpv(ae->group.mb_str(wx2pl), 0));\
	wxString he ## _class = GetEntryClass(ae);\
	HV_TOUCH(he, "class", 5)\
		HV_STORE(newSVpv(he ## _class.mb_str(wx2pl), 0))

#define ASS_BASIC_INIT(he, ae) \
	HV_FETCH(he, "raw", 3)\
		ae->SetEntryData(wxString(SvPV_nolen(HV_VAL), pl2wx));\
	HV_FETCH(he, "section", 7)\
		ae->group = wxString(SvPV_nolen(HV_VAL), pl2wx)


namespace Automation4 {

  wxString PerlAss::GetEntryClass(AssEntry *entry)
  {
	wxString data = entry->GetEntryData();

	if(entry->GetType() == ENTRY_DIALOGUE) return _T("dialogue");

	if(entry->GetType() == ENTRY_STYLE) {
	  return _T("style");
	  /* TODO: add stylex recognition */
	}

	if(entry->GetType() == ENTRY_BASE) {
	  if(entry->group == _T("[Script Info]") && data.Matches(_T("*:*"))) return _T("info"); 

	  if(data == entry->group) return _T("head");

	  if(data.StartsWith(_T("Format:"))) return _T("format");

	  if(data.IsEmpty()) return _T("clear");

	  if(data.Trim(left).StartsWith(_T(";"))) return _T("comment");
	}

	// Fallback
	return _T("unknown");
  }


  HV *PerlAss::MakeHasshEntry(AssEntry *e)
  {
	switch((int)e->GetType()) {
	case ENTRY_DIALOGUE:
	  return MakeHasshDialogue(AssEntry::GetAsDialogue(e));

	case ENTRY_STYLE:
	  return MakeHasshStyle(AssEntry::GetAsStyle(e));

	default:
	case ENTRY_BASE:
	  dHV;
	  HV *entry = newHV();
	  HASSH_BASIC_INIT(e, entry);
	  
	  if(entry_class == _T("info")) {
		// Info
		HV_TOUCH(entry, "key", 3) {
		  wxString _text = e->GetEntryData().BeforeFirst(_T(':')).Strip(wxString::both);
		  HV_STORE(newSVpv(_text.mb_str(wx2pl), 0));
		}

		HV_TOUCH(entry, "value", 5) {
		  wxString _text = e->GetEntryData().AfterFirst(_T(':')).Strip(wxString::both);
		  HV_STORE(newSVpv(_text.mb_str(wx2pl), 0));
		}
	  }
	  else if(entry_class == _T("format")) {
		// Format °_°
		HV_TOUCH(entry, "fields", 6) {
		  AV *fields_av = newAV();
		  HV_STORE(newRV_noinc((SV*)fields_av));
		  for(wxString fields_buf = e->GetEntryData().AfterFirst(_T(':')).Trim(left);
			  !fields_buf.IsEmpty();
			  fields_buf = fields_buf.AfterFirst(_T(',')).Trim(left)) {
			av_push(fields_av, newSVpv(fields_buf.BeforeFirst(_T(',')).Trim(right).mb_str(wx2pl), 0));
		  }
		}
	  }
	  else if(entry_class == _T("comment")) {
		// Comment
		HV_TOUCH(entry, "text", 4) {
		  wxString _text = e->GetEntryData().AfterFirst(_T(';'));
		  HV_STORE(newSVpv(_text.mb_str(wx2pl), 0));
		}
	  }
	  return entry;
	}
  }

  HV *PerlAss::MakeHasshStyle(AssStyle *s)
  {
	dHV;

	// Create new empty hash
	HV *style = newHV();

	// Add fields
	HASSH_BASIC_INIT(s, style);
	
	HV_TOUCH(style, "name", 4)
	  HV_STORE(newSVpv(s->name.mb_str(wx2pl), 0));

	HV_TOUCH(style, "font", 4)
	  HV_STORE(newSVpv(s->font.mb_str(wx2pl), 0));

	HV_FAS(style, "fontsize", 8, nv, s->fontsize);

	HV_TOUCH(style, "color1", 6)
	  HV_STORE(newSVpv(s->primary.GetASSFormatted(true).mb_str(wx2pl), 0));
	HV_TOUCH(style, "color2", 6)
	  HV_STORE(newSVpv(s->secondary.GetASSFormatted(true).mb_str(wx2pl), 0));
	HV_TOUCH(style, "color3", 6)
	  HV_STORE(newSVpv(s->outline.GetASSFormatted(true).mb_str(wx2pl), 0));
	HV_TOUCH(style, "color4", 6)
	  HV_STORE(newSVpv(s->shadow.GetASSFormatted(true).mb_str(wx2pl), 0));
		
	HV_TOUCH(style, "bold", 4) HV_STORE(newSViv(s->bold));

	HV_FAS(style, "italic", 6, iv, s->italic);
	HV_FAS(style, "underline", 9, iv, s->underline);
	HV_FAS(style, "strikeout", 9, iv, s->strikeout);

	HV_FAS(style, "scale_x", 7, nv, s->scalex);
	HV_FAS(style, "scale_y", 7, nv, s->scaley);

	HV_FAS(style, "spacing", 7, nv, s->spacing);

	HV_FAS(style, "angle", 5, nv, s->angle);

	HV_FAS(style, "borderstyle", 11, iv, s->borderstyle);

	HV_FAS(style, "outline", 7, nv, s->outline_w);

	HV_FAS(style, "shadow", 6, nv, s->shadow_w);

	HV_FAS(style, "align", 5, iv, s->alignment);

	HV_FAS(style, "margin_l", 8, iv, s->Margin[0]);
	HV_FAS(style, "margin_r", 8, iv, s->Margin[1]);
	HV_FAS(style, "margin_t", 8, iv, s->Margin[2]);
	HV_FAS(style, "margin_b", 8, iv, s->Margin[3]);
		
	HV_FAS(style, "encoding", 8, iv, s->encoding);
	HV_FAS(style, "relative_to", 11, iv, s->relativeTo);

	// Return the hassh style
	return style;
  }

  HV *PerlAss::MakeHasshDialogue(AssDialogue *d)
  {
	dHV;

	// Create new hash
	HV *diag = newHV();
	
	// Copy the values from the AssDialogue
	HASSH_BASIC_INIT(d, diag);

	HV_FAS(diag, "comment", 7, iv, d->Comment);

	HV_FAS(diag, "layer", 5, iv, d->Layer);

	HV_FAS(diag, "start_time", 10, iv, d->Start.GetMS());
	HV_FAS(diag, "end_time", 8, iv, d->End.GetMS());

	HV_TOUCH(diag, "style", 5)
	  HV_STORE(newSVpv(d->Style.mb_str(wx2pl), 0));

	HV_TOUCH(diag, "actor", 5)
	  HV_STORE(newSVpv(d->Actor.mb_str(wx2pl), 0));

	HV_FAS(diag, "margin_l", 8, iv, d->Margin[0]);
	HV_FAS(diag, "margin_r", 8, iv, d->Margin[1]);
	HV_FAS(diag, "margin_t", 8, iv, d->Margin[2]);
	HV_FAS(diag, "margin_b", 8, iv, d->Margin[3]);

	HV_TOUCH(diag, "effect", 6)
	  HV_STORE(newSVpv(d->Effect.mb_str(wx2pl), 0));

	HV_TOUCH(diag, "text", 4)
	  HV_STORE(newSVpv(d->Text.mb_str(wx2pl), 0));

	// Return the dialogue
	return diag;
  }

  AV *PerlAss::MakeHasshLines(AV *lines, AssFile *ass)
  {
	if(!lines) {
	  lines = newAV();
	}

	dAV;
	I32 i = 0; I32 lines_len = av_len(lines);
	for(std::list<AssEntry*>::iterator it = ass->Line.begin(); it != ass->Line.end(); it++) {
	  if(i <= lines_len && av_exists(lines, i))
		av_delete(lines, i, G_DISCARD);
	  AV_TOUCH(lines, i++)
		AV_STORE(newRV_noinc((SV*)MakeHasshEntry(*it)));
	}
	for(; i <= lines_len; i++) {
	  if(av_exists(lines, i))
		av_delete(lines, i, G_DISCARD);
	}

	return lines;
  }


  AssEntry *PerlAss::MakeAssEntry(HV *entry)
  {
	dHV;

	if(!entry) {
	  // Create an empty line, if NULL
	  entry = newHV();
	}

	// The fallback class
	wxString cl(_T("unknown"));
	// Let's get the actual class of the hassh
	HV_FETCH(entry, "class", 5)
	  cl = wxString(SvPV_nolen(HV_VAL), pl2wx);

	// We trust the value of entry{class}
	if(cl == _T("dialogue")) {
	  // It seems to be a dialogue, let's call the specialized function
	  return MakeAssDialogue(entry);
	}
	else if(cl == _T("style")) {
	  // It seems to be a style, let's call the specialized function
	  return MakeAssStyle(entry);
	}
	else {
	  AssEntry *e = new AssEntry();

	  ASS_BASIC_INIT(entry, e);

	  // A base entry
	  if(cl == _T("info")) {
		wxString key, value;
		HV_FETCH(entry, "key", 3) {
		  key = wxString(SvPV_nolen(HV_VAL), pl2wx);
		  HV_FETCH(entry, "value", 5) {
			value = wxString(SvPV_nolen(HV_VAL), pl2wx);
			e->SetEntryData(key + _T(": ") + value);
		  }
		}
	  }
	  // Not necessary, format customization isn't even supported by aegi (atm? °_°)
	  /*else if(cl == _T("format")) {
		HV_FETCH(entry, "fields", 6) {
		  AV *fields = (AV*)SvRV(HV_VAL);
		  for(int i = 0; i < av_len(fields); i++) {
			SV ** field = av_fetch(fields, i, 0);
			if(field) {
			  wxString field(SvPV_nolen(*field), pl2wx);
			}
		  }
		}
	  }*/
	  else if(cl == _T("comment")) {
		HV_FETCH(entry, "text", 4) {
		  e->SetEntryData(_T(";") + wxString(SvPV_nolen(HV_VAL), pl2wx));
		}
	  }
	  return e;
	}
  }

  AssStyle *PerlAss::MakeAssStyle(HV *style)
  {
	dHV;

	// Create a default style
	AssStyle *s = new AssStyle();

	// Fill it with the values from the hassh
	ASS_BASIC_INIT(style, s);

	HV_FETCH(style, "name", 4)
	  s->name = wxString(SvPV_nolen(HV_VAL), pl2wx);
	
	HV_FETCH(style, "font", 4)
	  s->font = wxString(SvPV_nolen(HV_VAL), pl2wx);

	HV_FAA(style, "fontsize", 8, NV, s->fontsize);
	
	HV_FETCH(style, "color1", 6)
	  s->primary.Parse(wxString(SvPV_nolen(HV_VAL), pl2wx));
	HV_FETCH(style, "color2", 6)
	  s->secondary.Parse(wxString(SvPV_nolen(HV_VAL), pl2wx));
	HV_FETCH(style, "color3", 6)
	  s->outline.Parse(wxString(SvPV_nolen(HV_VAL), pl2wx));
	HV_FETCH(style, "color4", 6)
	  s->shadow.Parse(wxString(SvPV_nolen(HV_VAL), pl2wx));
	
	HV_FAA(style, "bold", 4, IV, s->bold);
	
	HV_FAA(style, "italic", 6, IV, s->italic);
	HV_FAA(style, "underline", 9, IV, s->underline);
	HV_FAA(style, "strikeout", 9, IV, s->strikeout);
	
	HV_FAA(style, "scale_x", 7, NV, s->scalex);
	HV_FAA(style, "scale_y", 7, NV, s->scaley);

	HV_FAA(style, "spacing", 7, NV, s->spacing);
	
	HV_FAA(style, "angle", 5, NV, s->angle);
	
	HV_FAA(style, "borderstyle", 11, IV, s->borderstyle);
	
	HV_FAA(style, "outline", 7, NV, s->outline_w);
	
	HV_FAA(style, "shadow", 6, NV, s->shadow_w);
	
	HV_FAA(style, "align", 5, IV, s->alignment);
		
	HV_FAA(style, "margin_l", 8, IV, s->Margin[0]);
	HV_FAA(style, "margin_r", 8, IV, s->Margin[1]);
	HV_FAA(style, "margin_t", 8, IV, s->Margin[2]);
	HV_FAA(style, "margin_b", 8, IV, s->Margin[3]);
		
	HV_FAA(style, "encoding", 8, IV, s->encoding);
	HV_FAA(style, "relative_to", 11, IV, s->relativeTo);

	// Return the style
	return s;
  }

  AssDialogue *PerlAss::MakeAssDialogue(HV *diag)
  {
	dHV;

	// Create a default dialogue
	AssDialogue *d = new AssDialogue();

	ASS_BASIC_INIT(diag, d);

	HV_FAA(diag, "comment", 7, IV, d->Comment);
		
	HV_FAA(diag, "layer", 5, IV, d->Layer);
		
	HV_FETCH(diag, "start_time", 10)
	  d->Start.SetMS(SvIV(HV_VAL));
	HV_FETCH(diag, "end_time", 8)
	  d->End.SetMS(SvIV(HV_VAL));
		
	HV_FETCH(diag, "style", 5)
	  d->Style = wxString(SvPV_nolen(HV_VAL), pl2wx);
		  
	HV_FETCH(diag, "actor", 5)
	  d->Actor = wxString(SvPV_nolen(HV_VAL), pl2wx);
		
	HV_FAA(diag, "margin_l", 8, IV, d->Margin[0]);
	HV_FAA(diag, "margin_r", 8, IV, d->Margin[1]);
	HV_FAA(diag, "margin_t", 8, IV, d->Margin[2]);
	HV_FAA(diag, "margin_b", 8, IV, d->Margin[3]);
		
	HV_FETCH(diag, "effect", 6)
	  d->Effect = wxString(SvPV_nolen(HV_VAL), pl2wx);
		
	HV_FETCH(diag, "text", 4)
	  d->Text = wxString(SvPV_nolen(HV_VAL), pl2wx);

	// Return the dialogue
	return d;
  }

  AssFile *PerlAss::MakeAssLines(AssFile *ass, AV *lines)
  {
	if(!ass) {
	  /* TODO: create new AssFile if NULL */
	  return NULL;
	}

	dAV;
	std::list<AssEntry*>::iterator it = ass->Line.begin();
	for(I32 i = 0; i <= av_len(lines); i++) {
	  if(!av_exists(lines, i)) continue;
	  if(i < (I32)ass->Line.size()) {
		if(*it) delete *it;
		AV_FETCH(lines, i)
		  *it++ = MakeAssEntry((HV*)SvRV(AV_VAL));
	  }
	  else {
		AV_FETCH(lines, i)
		  ass->Line.push_back(MakeAssEntry((HV*)SvRV(AV_VAL)));
	  }
	}

	for(; it != ass->Line.end();) {
	  it = ass->Line.erase(it);
	}

	return ass;
  }

  
};


#endif //WITH_PERL
