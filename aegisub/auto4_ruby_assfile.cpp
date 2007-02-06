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

#include "auto4_ruby.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "ass_file.h"
#include "ass_override.h"
#include <assert.h>
#include <algorithm>
#include <ruby.h>


namespace Automation4 {

	// LuaAssFile

	VALUE RubyAssFile::AssEntryToRuby(AssEntry *e)
	{
		VALUE ass_entry;
		ass_entry = rb_hash_new();
		
		wxString section(e->group);
		rb_hash_aset(ass_entry, rb_str_new("section", 7), rb_str_new2(e->group.mb_str(wxConvUTF8)));
		wxString raw(e->GetEntryData());
		if(!raw.IsEmpty())
			rb_hash_aset(ass_entry, rb_str_new("raw", 3), rb_str_new2(e->GetEntryData().mb_str(wxConvUTF8)));
		VALUE entry_class;

		if (raw.Trim().IsEmpty()) {
			entry_class = rb_str_new("clear", 5);

		} else if (raw[0] == _T(';')) {
			// "text" field, same as "raw" but with semicolon stripped
			wxString text(raw, 1, raw.size()-1);
			rb_hash_aset(ass_entry, rb_str_new("text", 4), rb_str_new2(text.mb_str(wxConvUTF8)));
			entry_class = rb_str_new("comment", 7);
		} else if (raw[0] == _T('[')) {
			entry_class = rb_str_new("head", 4);

		} else if (section.Lower() == _T("[script info]")) {
			// assumed "info" class
			// first "key"
			wxString key = raw.BeforeFirst(_T(':'));
			rb_hash_aset(ass_entry, rb_str_new("key", 3), rb_str_new2(key.mb_str(wxConvUTF8)));

			// then "value"
			wxString value = raw.AfterFirst(_T(':'));
			rb_hash_aset(ass_entry, rb_str_new("value", 5), rb_str_new2(value.mb_str(wxConvUTF8)));
			entry_class = rb_str_new("info", 4);

		} else if (raw.Left(7).Lower() == _T("format:")) {
			// TODO: parse the format line; just use a tokenizer
			entry_class = rb_str_new("format", 6);

		} else if (e->GetType() == ENTRY_DIALOGUE) {
			AssDialogue *dia = e->GetAsDialogue(e);

			rb_hash_aset(ass_entry, rb_str_new("comment", 7), dia->Comment ? Qtrue : Qfalse);
			rb_hash_aset(ass_entry, rb_str_new("layer", 5), rb_int2inum(dia->Layer));
			rb_hash_aset(ass_entry, rb_str_new("start_time", 10), rb_int2inum(dia->Start.GetMS()));
			rb_hash_aset(ass_entry, rb_str_new("end_time", 8), rb_int2inum(dia->End.GetMS()));

			rb_hash_aset(ass_entry, rb_str_new("style", 5), rb_str_new2(dia->Style.mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, rb_str_new("actor", 5), rb_str_new2(dia->Actor.mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, rb_str_new("margin_l", 8), rb_int2inum(dia->Margin[0]));
			rb_hash_aset(ass_entry, rb_str_new("margin_r", 8), rb_int2inum(dia->Margin[1]));
			rb_hash_aset(ass_entry, rb_str_new("margin_t", 8), rb_int2inum(dia->Margin[2]));
			rb_hash_aset(ass_entry, rb_str_new("margin_b", 8), rb_int2inum(dia->Margin[3]));

			rb_hash_aset(ass_entry, rb_str_new("effect", 6), rb_str_new2(dia->Effect.mb_str(wxConvUTF8)));
//			rb_hash_aset(ass_entry, rb_str_new("userdata", 8), rb_str_new(""));
			rb_hash_aset(ass_entry, rb_str_new("text", 4), rb_str_new2(dia->Text.mb_str(wxConvUTF8)));

			entry_class = rb_str_new("dialogue", 8);

		} else if (e->GetType() == ENTRY_STYLE) {
			AssStyle *sty = e->GetAsStyle(e);

			rb_hash_aset(ass_entry, rb_str_new("name", 4), rb_str_new2(sty->name.mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, rb_str_new("fontname", 8), rb_str_new2(sty->font.mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, rb_str_new("fontsize", 8), rb_int2inum(sty->fontsize));

			rb_hash_aset(ass_entry, rb_str_new("color1", 6), rb_str_new2(sty->primary.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, rb_str_new("color2", 6), rb_str_new2(sty->secondary.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, rb_str_new("color3", 6), rb_str_new2(sty->outline.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, rb_str_new("color4", 6), rb_str_new2(sty->shadow.GetASSFormatted(true).mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, rb_str_new("bold", 4), rb_int2inum(sty->bold));
			rb_hash_aset(ass_entry, rb_str_new("italic", 6), rb_int2inum(sty->italic));
			rb_hash_aset(ass_entry, rb_str_new("underline", 9), rb_int2inum(sty->underline));
			rb_hash_aset(ass_entry, rb_str_new("strikeout", 9), rb_int2inum(sty->strikeout));

			rb_hash_aset(ass_entry, rb_str_new("scale_x", 7), rb_int2inum(sty->scalex));
			rb_hash_aset(ass_entry, rb_str_new("scale_y", 7), rb_int2inum(sty->scaley));

			rb_hash_aset(ass_entry, rb_str_new("spacing", 7), rb_int2inum(sty->spacing));
			rb_hash_aset(ass_entry, rb_str_new("angle", 5), rb_int2inum(sty->angle));

			rb_hash_aset(ass_entry, rb_str_new("borderstyle", 11), rb_int2inum(sty->borderstyle));
			rb_hash_aset(ass_entry, rb_str_new("outline", 7), rb_int2inum(sty->outline_w));
			rb_hash_aset(ass_entry, rb_str_new("shadow", 6), rb_int2inum(sty->shadow_w));
			rb_hash_aset(ass_entry, rb_str_new("align", 5), rb_int2inum(sty->alignment));

			rb_hash_aset(ass_entry, rb_str_new("margin_l", 8), rb_int2inum(sty->Margin[0]));
			rb_hash_aset(ass_entry, rb_str_new("margin_r", 8), rb_int2inum(sty->Margin[1]));
			rb_hash_aset(ass_entry, rb_str_new("margin_t", 8), rb_int2inum(sty->Margin[2]));
			rb_hash_aset(ass_entry, rb_str_new("margin_b", 8), rb_int2inum(sty->Margin[3]));

			rb_hash_aset(ass_entry, rb_str_new("encoding", 8), rb_int2inum(sty->encoding));

			// From STS.h: "0: window, 1: video, 2: undefined (~window)"
			rb_hash_aset(ass_entry, rb_str_new("relative_to", 11), rb_int2inum(2));
			rb_hash_aset(ass_entry, rb_str_new("vertical", 8), Qfalse);

			entry_class = rb_str_new("style", 5);

		} else {
			entry_class = rb_str_new("unknown", 7);
		}
		// store class of item; last thing done for each class specific code must be pushing the class name
		rb_hash_aset(ass_entry, rb_str_new("class", 5), entry_class);
	
		return ass_entry;
	}

	AssEntry *RubyAssFile::RubyToAssEntry(VALUE ass_entry)
	{
		VALUE entry_class = rb_hash_aref(ass_entry, rb_str_new("class", 5));
		
		wxString lclass(StringValueCStr(entry_class), wxConvUTF8);
		lclass.MakeLower();

		VALUE _section = rb_hash_aref(ass_entry, rb_str_new("section", 7));
		wxString section(StringValueCStr(_section), wxConvUTF8);

		AssEntry *result;
		if (lclass == _T("clear")) {
			result = new AssEntry(_T(""));
			result->group = section;

		} else if (lclass == _T("comment")) {
//			GETSTRING(raw, "text", "comment")
			VALUE _text = rb_hash_aref(ass_entry, rb_str_new("text", 4));
			wxString raw(StringValueCStr(_text), wxConvUTF8);
			raw.Prepend(_T(";"));
			result = new AssEntry(raw);
			result->group = section;

		} else if (lclass == _T("head")) {
			result = new AssEntry(section);
			result->group = section;

		} else if (lclass == _T("info")) {
			VALUE _key = rb_hash_aref(ass_entry, rb_str_new("key", 3));
			wxString key(StringValueCStr(_key), wxConvUTF8);
			VALUE _value = rb_hash_aref(ass_entry, rb_str_new("value", 5));
			wxString value(StringValueCStr(_value), wxConvUTF8);
			result = new AssEntry(wxString::Format(_T("%s: %s"), key.c_str(), value.c_str()));
			result->group = _T("[Script Info]"); // just so it can be read correctly back

		} else if (lclass == _T("format")) {
			// ohshi- ...
			// *FIXME* maybe ignore the actual data and just put some default stuff based on section?
			result = new AssEntry(_T("Format: Auto4,Is,Broken"));
			result->group = section;

		} else if (lclass == _T("style")) {
			VALUE _name = rb_hash_aref(ass_entry, rb_str_new("name", 4));
			wxString name(StringValueCStr(_name), wxConvUTF8);
			VALUE _fontname = rb_hash_aref(ass_entry, rb_str_new("fontname", 8));
			wxString fontname(StringValueCStr(_fontname), wxConvUTF8);
			VALUE _fontsize = rb_hash_aref(ass_entry, rb_str_new("fontsize", 8));
			float fontsize = rb_num2dbl(_fontsize);
			VALUE _color1 = rb_hash_aref(ass_entry, rb_str_new("color1", 6));
			wxString color1(StringValueCStr(_color1), wxConvUTF8);
			VALUE _color2 = rb_hash_aref(ass_entry, rb_str_new("color2", 6));
			wxString color2(StringValueCStr(_color2), wxConvUTF8);
			VALUE _color3 = rb_hash_aref(ass_entry, rb_str_new("color3", 6));
			wxString color3(StringValueCStr(_color3), wxConvUTF8);
			VALUE _color4 = rb_hash_aref(ass_entry, rb_str_new("color4", 6));
			wxString color4(StringValueCStr(_color4), wxConvUTF8);
			VALUE _bold = rb_hash_aref(ass_entry, rb_str_new("bold", 4));
			bool bold = (bool)rb_num2long(_bold);
			VALUE _italic = rb_hash_aref(ass_entry, rb_str_new("italic", 6));
			bool italic = (bool)rb_num2long(_italic);
			VALUE _underline = rb_hash_aref(ass_entry, rb_str_new("underline", 9));
			bool underline = (bool)rb_num2long(_underline);
			VALUE _strikeout = rb_hash_aref(ass_entry, rb_str_new("strikeout", 9));
			bool strikeout = (bool)rb_num2long(_strikeout);
			VALUE _scale_x = rb_hash_aref(ass_entry, rb_str_new("scale_x", 7));
			float scale_x = rb_num2dbl(_scale_x);
			VALUE _scale_y = rb_hash_aref(ass_entry, rb_str_new("scale_y", 7));
			float scale_y = rb_num2dbl(_scale_y);
			VALUE _spacing = rb_hash_aref(ass_entry, rb_str_new("spacing", 7));
			int spacing = rb_num2long(_spacing);
			VALUE _angle = rb_hash_aref(ass_entry, rb_str_new("angle", 5));
			float angle = rb_num2dbl(_angle);
			VALUE _borderstyle = rb_hash_aref(ass_entry, rb_str_new("borderstyle", 11));
			int borderstyle = rb_num2long(_borderstyle);
			VALUE _outline = rb_hash_aref(ass_entry, rb_str_new("outline", 7));
			float outline = rb_num2dbl(_outline);
			VALUE _shadow = rb_hash_aref(ass_entry, rb_str_new("shadow", 6));
			float shadow = rb_num2dbl(_shadow);
			VALUE _align = rb_hash_aref(ass_entry, rb_str_new("align", 5));
			int align = rb_num2long(_align);
			VALUE _margin_l = rb_hash_aref(ass_entry, rb_str_new("margin_l", 8));
			int margin_l = rb_num2long(_margin_l);
			VALUE _margin_r = rb_hash_aref(ass_entry, rb_str_new("margin_r", 8));
			int margin_r = rb_num2long(_margin_r);
			VALUE _margin_t = rb_hash_aref(ass_entry, rb_str_new("margin_t", 8));
			int margin_t = rb_num2long(_margin_t);
			VALUE _margin_b = rb_hash_aref(ass_entry, rb_str_new("margin_b", 8));
			int margin_b = rb_num2long(_margin_b);
			VALUE _encoding = rb_hash_aref(ass_entry, rb_str_new("encoding", 8));
			int encoding = rb_num2long(_encoding);
			// leaving out relative_to and vertical

			AssStyle *sty = new AssStyle();
			sty->name = name;
			sty->font = fontname;
			sty->fontsize = fontsize;
			sty->primary.Parse(color1);
			sty->secondary.Parse(color2);
			sty->outline.Parse(color3);
			sty->shadow.Parse(color4);
			sty->bold = bold;
			sty->italic = italic;
			sty->underline = underline;
			sty->strikeout = strikeout;
			sty->scalex = scale_x;
			sty->scaley = scale_y;
			sty->spacing = spacing;
			sty->angle = angle;
			sty->borderstyle = borderstyle;
			sty->outline_w = outline;
			sty->shadow_w = shadow;
			sty->alignment = align;
			sty->Margin[0] = margin_l;
			sty->Margin[1] = margin_r;
			sty->Margin[2] = margin_t;
			sty->Margin[3] = margin_b;
			sty->encoding = encoding;
			sty->UpdateData();

			result = sty;
		} else if (lclass == _T("styleex")) {
			rb_raise(rb_eRuntimeError, "Found line with class 'styleex' which is not supported. Wait until AS5 is a reality.");			
			return 0;

		} else if (lclass == _T("dialogue")) {
			VALUE _comment = rb_hash_aref(ass_entry, rb_str_new("comment", 7));
			bool comment = _comment == Qfalse ? false : true;
			VALUE _layer = rb_hash_aref(ass_entry, rb_str_new("layer", 5));
			int layer = rb_num2long(_layer);
			VALUE _start_time = rb_hash_aref(ass_entry, rb_str_new("start_time", 10));
			int start_time = rb_num2long(_start_time);
			VALUE _end_time = rb_hash_aref(ass_entry, rb_str_new("end_time", 8));
			int end_time = rb_num2long(_end_time);
			VALUE _style = rb_hash_aref(ass_entry, rb_str_new("style", 5));
			wxString style(StringValueCStr(_style), wxConvUTF8);
			VALUE _actor = rb_hash_aref(ass_entry, rb_str_new("actor", 5));
			wxString actor(StringValueCStr(_actor), wxConvUTF8);
			VALUE _margin_l = rb_hash_aref(ass_entry, rb_str_new("margin_l", 8));
			int margin_l = rb_num2long(_margin_l);
			VALUE _margin_r = rb_hash_aref(ass_entry, rb_str_new("margin_r", 8));
			int margin_r = rb_num2long(_margin_r);
			VALUE _margin_t = rb_hash_aref(ass_entry, rb_str_new("margin_t", 8));
			int margin_t = rb_num2long(_margin_t);
			VALUE _margin_b = rb_hash_aref(ass_entry, rb_str_new("margin_b", 8));
			int margin_b = rb_num2long(_margin_b);
			VALUE _effect = rb_hash_aref(ass_entry, rb_str_new("effect", 6));
			wxString effect(StringValueCStr(_effect), wxConvUTF8);
			VALUE _text = rb_hash_aref(ass_entry, rb_str_new("text", 4));
			wxString text(StringValueCStr(_text), wxConvUTF8);
			//GETSTRING(userdata, "userdata", "dialogue")

			AssDialogue *dia = new AssDialogue();
			dia->Comment = comment;
			dia->Layer = layer;
			dia->Start.SetMS(start_time);
			dia->End.SetMS(end_time);
			dia->Style = style;
			dia->Actor = actor;
			dia->Margin[0] = margin_l;
			dia->Margin[1] = margin_r;
			dia->Margin[2] = margin_t;
			dia->Margin[3] = margin_b;
			dia->Effect = effect;
			dia->Text = text;
			dia->UpdateData();

			result = dia;

		} else {
			rb_raise(rb_eRuntimeError, "Found line with unknown class: %s", lclass.mb_str(wxConvUTF8).data());			
			return 0;
		}

		return result;
	}

	// Updates the AssFile with data returned by macro/filter
	// If the first line is dialogue we leave header from the original (styles, info, etc)
	void RubyAssFile::RubyUpdateAssFile(VALUE subtitles)
	{
		int size = RARRAY(subtitles)->len;

		if(size <= 0) return; // empty - leave the original
		
		VALUE rbEntry;
		AssEntry* new_entry;
		int status = 0;
		do {
			rbEntry = rb_ary_shift(subtitles);
			new_entry = reinterpret_cast<AssEntry*>(rb_protect(rb2AssWrapper, rbEntry, &status));
			--size;
		}while(status != 0);	// broken lines at the beginning?
		rb_set_errinfo(Qnil);;	// just in case

		entryIter e = ass->Line.begin();
		if(new_entry->GetType() == ENTRY_DIALOGUE)	// check if the first line is a dialogue
		{
			while(e != ass->Line.end() && (*e)->GetType() != ENTRY_DIALOGUE) ++e;
		}
		while(e != ass->Line.end()) // delete the old lines backwards
		{		
			delete (*e);
			e = ass->Line.erase(e);
		}
		ass->Line.push_back(new_entry);
		for(int i = 0; i < size; i++) // insert new lines
		{
			rbEntry = rb_ary_shift(subtitles);
			new_entry = reinterpret_cast<AssEntry*>(rb_protect(rb2AssWrapper, rbEntry, &status));
			if(status == 0)	ass->Line.push_back(new_entry);
			else 
			{
				if(RubyProgressSink::inst)
				{
					VALUE err = rb_errinfo();
					RubyProgressSink::inst->RubyDebugOut(1, &err, Qnil);
				}
				rb_set_errinfo(Qnil);
			}
		}
		AssFile::top->FlagAsModified(_T(""));
	}
	
	int RubyAssFile::RubyParseTagData()
	{
		// TODO
		return 1;
	}

	int RubyAssFile::RubyUnparseTagData()
	{
		// TODO
		return 1;
	}


	int RubyAssFile::RubySetUndoPoint()
	{
		return 0;
	}

	RubyAssFile::~RubyAssFile()
	{
		RubyObjects::Get()->Unregister(rbAssFile);
		int status;
		rb_protect(rbGcWrapper, Qnil, &status);	// run the gc
		if(status != 0)
		{
			wxMessageBox(_T("Error"), _T("Error while running Ruby garbage collection"));
		}
	}

	RubyAssFile::RubyAssFile(AssFile *_ass, bool _can_modify, bool _can_set_undo)
		: ass(_ass)
		, can_modify(_can_modify)
		, can_set_undo(_can_set_undo)
	{

		rbAssFile = rb_ary_new2(ass->Line.size());
		RubyObjects::Get()->Register(rbAssFile);

		std::list<AssEntry*>::iterator entry;
		int status;
		for(entry = ass->Line.begin(); entry != ass->Line.end(); ++entry)
		{
			VALUE res = rb_protect(rbAss2RbWrapper, reinterpret_cast<VALUE>(*entry), &status);
			if(status == 0) rb_ary_push(rbAssFile, res);
			else 
			{
				if(RubyProgressSink::inst)
				{
					VALUE err = rb_errinfo();
					RubyProgressSink::inst->RubyDebugOut(1, &err, Qnil);
				}
				rb_set_errinfo(Qnil);
			}
		}

		// TODO
		//rb_define_module_function(RubyScript::RubyAegisub, "parse_tag_data",reinterpret_cast<RB_HOOK>(&RubyParseTagData), 1);
		//rb_define_module_function(RubyScript::RubyAegisub, "unparse_tag_data",reinterpret_cast<RB_HOOK>(&RubyUnparseTagData), 1);
		//rb_define_module_function(RubyScript::RubyAegisub, "set_undo_point",reinterpret_cast<RB_HOOK>(&RubySetUndoPoint), 1);

	}

};
