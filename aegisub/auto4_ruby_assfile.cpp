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

#ifdef WITH_RUBY

#ifdef _MSC_VER
#pragma warning(disable: 4003)
#endif

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
		rb_hash_aset(ass_entry, STR2SYM("section"), rb_str_new2(e->group.mb_str(wxConvUTF8)));
		wxString raw(e->GetEntryData());
		if(!raw.IsEmpty())
			rb_hash_aset(ass_entry, STR2SYM("raw"), rb_str_new2(e->GetEntryData().mb_str(wxConvUTF8)));
		VALUE entry_class;

		if (raw.Trim().IsEmpty()) {
			entry_class = STR2SYM("clear");

		} else if (raw[0] == _T(';')) {
			// "text" field, same as "raw" but with semicolon stripped
			wxString text(raw, 1, raw.size()-1);
			rb_hash_aset(ass_entry, STR2SYM("text"), rb_str_new2(text.mb_str(wxConvUTF8)));
			entry_class = STR2SYM("comment");
		} else if (raw[0] == _T('[')) {
			entry_class = STR2SYM("head");

		} else if (section.Lower() == _T("[script info]")) {
			// assumed "info" class
			// first "key"
			wxString key = raw.BeforeFirst(_T(':'));
			rb_hash_aset(ass_entry, STR2SYM("key"), rb_str_new2(key.mb_str(wxConvUTF8)));

			// then "value"
			wxString value = raw.AfterFirst(_T(':'));
			value.Trim(false);
			rb_hash_aset(ass_entry, STR2SYM("value"), rb_str_new2(value.mb_str(wxConvUTF8)));
			entry_class = STR2SYM("info");

		} else if (raw.Left(7).Lower() == _T("format:")) {
			// TODO: parse the format line; just use a tokenizer
			entry_class = STR2SYM("format");

		} else if (e->GetType() == ENTRY_DIALOGUE) {
			AssDialogue *dia = e->GetAsDialogue(e);

			rb_hash_aset(ass_entry, STR2SYM("comment"), dia->Comment ? Qtrue : Qfalse);
			rb_hash_aset(ass_entry, STR2SYM("layer"), rb_int2inum(dia->Layer));
			rb_hash_aset(ass_entry, STR2SYM("start_time"), rb_int2inum(dia->Start.GetMS()));
			rb_hash_aset(ass_entry, STR2SYM("end_time"), rb_int2inum(dia->End.GetMS()));

			rb_hash_aset(ass_entry, STR2SYM("style"), rb_str_new2(dia->Style.mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, STR2SYM("actor"), rb_str_new2(dia->Actor.mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, STR2SYM("margin_l"), rb_int2inum(dia->Margin[0]));
			rb_hash_aset(ass_entry, STR2SYM("margin_r"), rb_int2inum(dia->Margin[1]));
			rb_hash_aset(ass_entry, STR2SYM("margin_t"), rb_int2inum(dia->Margin[2]));
			rb_hash_aset(ass_entry, STR2SYM("margin_b"), rb_int2inum(dia->Margin[3]));

			rb_hash_aset(ass_entry, STR2SYM("effect"), rb_str_new2(dia->Effect.mb_str(wxConvUTF8)));
//			rb_hash_aset(ass_entry, STR2SYM("userdata"), rb_str_new(""));
			rb_hash_aset(ass_entry, STR2SYM("text"), rb_str_new2(dia->Text.mb_str(wxConvUTF8)));

			entry_class = STR2SYM("dialogue");

		} else if (e->GetType() == ENTRY_STYLE) {
			AssStyle *sty = e->GetAsStyle(e);

			rb_hash_aset(ass_entry, STR2SYM("name"), rb_str_new2(sty->name.mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, STR2SYM("fontname"), rb_str_new2(sty->font.mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, STR2SYM("fontsize"), rb_int2inum(sty->fontsize));

			rb_hash_aset(ass_entry, STR2SYM("color1"), rb_str_new2(sty->primary.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, STR2SYM("color2"), rb_str_new2(sty->secondary.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, STR2SYM("color3"), rb_str_new2(sty->outline.GetASSFormatted(true).mb_str(wxConvUTF8)));
			rb_hash_aset(ass_entry, STR2SYM("color4"), rb_str_new2(sty->shadow.GetASSFormatted(true).mb_str(wxConvUTF8)));

			rb_hash_aset(ass_entry, STR2SYM("bold"), rb_int2inum(sty->bold));
			rb_hash_aset(ass_entry, STR2SYM("italic"), rb_int2inum(sty->italic));
			rb_hash_aset(ass_entry, STR2SYM("underline"), rb_int2inum(sty->underline));
			rb_hash_aset(ass_entry, STR2SYM("strikeout"), rb_int2inum(sty->strikeout));

			rb_hash_aset(ass_entry, STR2SYM("scale_x"), rb_int2inum(sty->scalex));
			rb_hash_aset(ass_entry, STR2SYM("scale_y"), rb_int2inum(sty->scaley));

			rb_hash_aset(ass_entry, STR2SYM("spacing"), rb_int2inum(sty->spacing));
			rb_hash_aset(ass_entry, STR2SYM("angle"), rb_int2inum(sty->angle));

			rb_hash_aset(ass_entry, STR2SYM("borderstyle"), rb_int2inum(sty->borderstyle));
			rb_hash_aset(ass_entry, STR2SYM("outline"), rb_int2inum(sty->outline_w));
			rb_hash_aset(ass_entry, STR2SYM("shadow"), rb_int2inum(sty->shadow_w));
			rb_hash_aset(ass_entry, STR2SYM("align"), rb_int2inum(sty->alignment));

			rb_hash_aset(ass_entry, STR2SYM("margin_l"), rb_int2inum(sty->Margin[0]));
			rb_hash_aset(ass_entry, STR2SYM("margin_r"), rb_int2inum(sty->Margin[1]));
			rb_hash_aset(ass_entry, STR2SYM("margin_t"), rb_int2inum(sty->Margin[2]));
			rb_hash_aset(ass_entry, STR2SYM("margin_b"), rb_int2inum(sty->Margin[3]));

			rb_hash_aset(ass_entry, STR2SYM("encoding"), rb_int2inum(sty->encoding));

			// From STS.h: "0: window, 1: video, 2: undefined (~window)"
			rb_hash_aset(ass_entry, STR2SYM("relative_to"), rb_int2inum(2));
			rb_hash_aset(ass_entry, STR2SYM("vertical"), Qfalse);

			entry_class = STR2SYM("style");

		} else {
			entry_class = STR2SYM("unknown");
		}
		// store class of item; last thing done for each class specific code must be pushing the class name
		rb_hash_aset(ass_entry, STR2SYM("class"), entry_class);
	
		return ass_entry;
	}

	AssEntry *RubyAssFile::RubyToAssEntry(VALUE ass_entry)
	{
		VALUE entry_class = rb_hash_aref(ass_entry, STR2SYM("class"));
		
		wxString lclass(rb_id2name(SYM2ID(entry_class)), wxConvUTF8);
		lclass.MakeLower();

		VALUE _section = rb_hash_aref(ass_entry, STR2SYM("section"));
		wxString section(StringValueCStr(_section), wxConvUTF8);

		AssEntry *result;
		if (lclass == _T("clear")) {
			result = new AssEntry(_T(""));
			result->group = section;

		} else if (lclass == _T("comment")) {
//			GETSTRING(raw, "text", "comment")
			VALUE _text = rb_hash_aref(ass_entry, STR2SYM("text"));
			wxString raw(StringValueCStr(_text), wxConvUTF8);
			raw.Prepend(_T(";"));
			result = new AssEntry(raw);
			result->group = section;

		} else if (lclass == _T("head")) {
			result = new AssEntry(section);
			result->group = section;

		} else if (lclass == _T("info")) {
			VALUE _key = rb_hash_aref(ass_entry, STR2SYM("key"));
			wxString key(StringValueCStr(_key), wxConvUTF8);
			VALUE _value = rb_hash_aref(ass_entry, STR2SYM("value"));
			wxString value(StringValueCStr(_value), wxConvUTF8);
			result = new AssEntry(wxString::Format(_T("%s: %s"), key.c_str(), value.c_str()));
			result->group = _T("[Script Info]"); // just so it can be read correctly back

		} else if (lclass == _T("format")) {
			// ohshi- ...
			// *FIXME* maybe ignore the actual data and just put some default stuff based on section?
			result = new AssEntry(_T("Format: Auto4,Is,Broken"));
			result->group = section;

		} else if (lclass == _T("style")) {
			VALUE _name = rb_hash_aref(ass_entry, STR2SYM("name"));
			wxString name(StringValueCStr(_name), wxConvUTF8);
			VALUE _fontname = rb_hash_aref(ass_entry, STR2SYM("fontname"));
			wxString fontname(StringValueCStr(_fontname), wxConvUTF8);
			VALUE _fontsize = rb_hash_aref(ass_entry, STR2SYM("fontsize"));
			float fontsize = rb_num2dbl(_fontsize);
			VALUE _color1 = rb_hash_aref(ass_entry, STR2SYM("color1"));
			wxString color1(StringValueCStr(_color1), wxConvUTF8);
			VALUE _color2 = rb_hash_aref(ass_entry, STR2SYM("color2"));
			wxString color2(StringValueCStr(_color2), wxConvUTF8);
			VALUE _color3 = rb_hash_aref(ass_entry, STR2SYM("color3"));
			wxString color3(StringValueCStr(_color3), wxConvUTF8);
			VALUE _color4 = rb_hash_aref(ass_entry, STR2SYM("color4"));
			wxString color4(StringValueCStr(_color4), wxConvUTF8);
			VALUE _bold = rb_hash_aref(ass_entry, STR2SYM("bold"));
			bool bold = rb_num2long(_bold) == 1;
			VALUE _italic = rb_hash_aref(ass_entry, STR2SYM("italic"));
			bool italic = rb_num2long(_italic) == 1;
			VALUE _underline = rb_hash_aref(ass_entry, STR2SYM("underline"));
			bool underline = rb_num2long(_underline) == 1;
			VALUE _strikeout = rb_hash_aref(ass_entry, STR2SYM("strikeout"));
			bool strikeout = rb_num2long(_strikeout) == 1;
			VALUE _scale_x = rb_hash_aref(ass_entry, STR2SYM("scale_x"));
			float scale_x = rb_num2dbl(_scale_x);
			VALUE _scale_y = rb_hash_aref(ass_entry, STR2SYM("scale_y"));
			float scale_y = rb_num2dbl(_scale_y);
			VALUE _spacing = rb_hash_aref(ass_entry, STR2SYM("spacing"));
			int spacing = rb_num2long(_spacing);
			VALUE _angle = rb_hash_aref(ass_entry, STR2SYM("angle"));
			float angle = rb_num2dbl(_angle);
			VALUE _borderstyle = rb_hash_aref(ass_entry, STR2SYM("borderstyle"));
			int borderstyle = rb_num2long(_borderstyle);
			VALUE _outline = rb_hash_aref(ass_entry, STR2SYM("outline"));
			float outline = rb_num2dbl(_outline);
			VALUE _shadow = rb_hash_aref(ass_entry, STR2SYM("shadow"));
			float shadow = rb_num2dbl(_shadow);
			VALUE _align = rb_hash_aref(ass_entry, STR2SYM("align"));
			int align = rb_num2long(_align);
			VALUE _margin_l = rb_hash_aref(ass_entry, STR2SYM("margin_l"));
			int margin_l = rb_num2long(_margin_l);
			VALUE _margin_r = rb_hash_aref(ass_entry, STR2SYM("margin_r"));
			int margin_r = rb_num2long(_margin_r);
			VALUE _margin_t = rb_hash_aref(ass_entry, STR2SYM("margin_t"));
			int margin_t = rb_num2long(_margin_t);
			VALUE _margin_b = rb_hash_aref(ass_entry, STR2SYM("margin_b"));
			int margin_b = rb_num2long(_margin_b);
			VALUE _encoding = rb_hash_aref(ass_entry, STR2SYM("encoding"));
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

		} else if (lclass == _T("dialogue")) {
			VALUE _comment = rb_hash_aref(ass_entry, STR2SYM("comment"));
			bool comment = _comment == Qfalse ? false : true;
			VALUE _layer = rb_hash_aref(ass_entry, STR2SYM("layer"));
			int layer = rb_num2long(_layer);
			VALUE _start_time = rb_hash_aref(ass_entry, STR2SYM("start_time"));
			int start_time = rb_num2long(_start_time);
			VALUE _end_time = rb_hash_aref(ass_entry, STR2SYM("end_time"));
			int end_time = rb_num2long(_end_time);
			VALUE _style = rb_hash_aref(ass_entry, STR2SYM("style"));
			wxString style(StringValueCStr(_style), wxConvUTF8);
			VALUE _actor = rb_hash_aref(ass_entry, STR2SYM("actor"));
			wxString actor(StringValueCStr(_actor), wxConvUTF8);
			VALUE _margin_l = rb_hash_aref(ass_entry, STR2SYM("margin_l"));
			int margin_l = rb_num2long(_margin_l);
			VALUE _margin_r = rb_hash_aref(ass_entry, STR2SYM("margin_r"));
			int margin_r = rb_num2long(_margin_r);
			VALUE _margin_t = rb_hash_aref(ass_entry, STR2SYM("margin_t"));
			int margin_t = rb_num2long(_margin_t);
			VALUE _margin_b = rb_hash_aref(ass_entry, STR2SYM("margin_b"));
			int margin_b = rb_num2long(_margin_b);
			VALUE _effect = rb_hash_aref(ass_entry, STR2SYM("effect"));
			wxString effect(StringValueCStr(_effect), wxConvUTF8);
			VALUE _text = rb_hash_aref(ass_entry, STR2SYM("text"));
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
		}

		if (can_set_undo) {
			AssFile::top->FlagAsModified(_T(""));
		}
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
		// I can think of two things to do here:
		// One is to read in all of the subs from Ruby and cobvert back to AssEntry, inserting it into the file, then set undo point
		// Another is to just scrap it and only support one undo point per macro execution, and simply save the message, using it
		// to set the description when the actual undo point it set after execution.
		//  -jfs
		return 0;
	}

	RubyAssFile::~RubyAssFile()
	{
		RubyObjects::Get()->Unregister(rbAssFile);
	}

	RubyAssFile::RubyAssFile(AssFile *_ass, bool _can_modify, bool _can_set_undo)
		: ass(_ass)
		, can_modify(_can_modify)
		, can_set_undo(_can_set_undo)
	{
		rb_gc_disable();
		rbAssFile = rb_ary_new2(ass->Line.size());
		RubyObjects::Get()->Register(rbAssFile);

		std::list<AssEntry*>::iterator entry;
		int status;
		for(entry = ass->Line.begin(); entry != ass->Line.end(); ++entry)
		{
			VALUE res = rb_protect(rbAss2RbWrapper, reinterpret_cast<VALUE>(*entry), &status);
			if(status == 0) rb_ary_push(rbAssFile, res);
		}
		rb_gc_enable();
		// TODO
		//rb_define_module_function(RubyScript::RubyAegisub, "parse_tag_data",reinterpret_cast<RB_HOOK>(&RubyParseTagData), 1);
		//rb_define_module_function(RubyScript::RubyAegisub, "unparse_tag_data",reinterpret_cast<RB_HOOK>(&RubyUnparseTagData), 1);
		//rb_define_module_function(RubyScript::RubyAegisub, "set_undo_point",reinterpret_cast<RB_HOOK>(&RubySetUndoPoint), 1);

	}

};

#endif // WITH_RUBY
