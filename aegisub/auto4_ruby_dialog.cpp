// Copyright (c) 2006, 2007, Niels Martin Hansen, Patryk Pomykalski
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
#include "auto4_ruby.h"
#include <ruby.h>
#include <wx/window.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/validate.h>
#include <assert.h>

namespace Automation4 {


	// RubyConfigDialogControl

	RubyConfigDialogControl::RubyConfigDialogControl(VALUE opts)
	{
		VALUE val = rb_hash_aref(opts, STR2SYM("name"));
		name_sym = val;
		if(TYPE(val) == T_STRING) {
			name = wxString(StringValueCStr(val), wxConvUTF8);
		} else if(TYPE(val) == T_SYMBOL) {
			name = wxString(rb_id2name(SYM2ID(val)), wxConvUTF8);
		} else name = _T("");

		val = rb_hash_aref(opts, STR2SYM("x"));
		if(TYPE(val) == T_FIXNUM) {
			x = FIX2INT(val);
			if (x < 0) x = 0;
		}
		else x = 0;

		val = rb_hash_aref(opts, STR2SYM("y"));
		if(TYPE(val) == T_FIXNUM) {
			y = FIX2INT(val);
			if (y < 0) y = 0;
		}
		else y = 0;

		val = rb_hash_aref(opts, STR2SYM("width"));
		if(TYPE(val) == T_FIXNUM) {
			width = FIX2INT(val);
			if (width < 1) width = 1;
		}
		else width = 1;

		val = rb_hash_aref(opts, STR2SYM("height"));
		if(TYPE(val) == T_FIXNUM) {
			height = FIX2INT(val);
			if (height < 1) width = 1;
		}
		else height = 1;

		val = rb_hash_aref(opts, STR2SYM("hint"));
		if(TYPE(val) == T_STRING)
			hint = wxString(StringValueCStr(val), wxConvUTF8);
		else hint = _T("");

		wxLogDebug(_T("created control: '%s', (%d,%d)(%d,%d), '%s'"), name.c_str(), x, y, width, height, hint.c_str());
	}

	namespace RubyControl {

		// Label

		class Label : public RubyConfigDialogControl {
		public:
			wxString label;

			Label(){};
			Label(VALUE opts)
				: RubyConfigDialogControl(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("label"));
				if(TYPE(val) == T_STRING)
					label = wxString(StringValueCStr(val), wxConvUTF8);
				else label = _T("");
			}

			virtual ~Label() { }

			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxStaticText(parent, -1, label);
			}

			void ControlReadBack()
			{
				// Nothing here
			}

			VALUE RubyReadBack()
			{
				return Qnil;
			}
		};


		// Basic edit

		class Edit : public RubyConfigDialogControl {
		public:
			wxString text;

			Edit(){};
			Edit(VALUE opts)
				: RubyConfigDialogControl(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("text"));
				if(TYPE(val) == T_STRING)
					text = wxString(StringValueCStr(val), wxConvUTF8);
				else text = _T("");
			}

			virtual ~Edit() { }

			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0);
			}

			void ControlReadBack()
			{
				text = ((wxTextCtrl*)cw)->GetValue();
			}

			VALUE RubyReadBack()
			{
		//		if(text.IsEmpty()) return rb_str_new("", 0);
				return rb_str_new2(text.mb_str(wxConvUTF8));
			}

		};

		
		// Multiline edit

		class Textbox : public Edit {
		public:

			Textbox(){};
			Textbox(VALUE opts)
				: Edit(opts)
			{
				// Nothing more
			}

			virtual ~Textbox() { }

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
				cw->SetMinSize(wxSize(0, 30));
				return cw;
			}

		};


		// Integer only edit

		class IntEdit : public Edit {
		public:
			int value;
			bool hasspin;
			int min, max;

			IntEdit(){};
			IntEdit(VALUE opts)
				: Edit(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("value"));
				if(TYPE(val) == T_FIXNUM) {
					value = FIX2INT(val);
				}

				hasspin = false;
				val = rb_hash_aref(opts, STR2SYM("min"));
				if(TYPE(val) == T_FIXNUM) {
					min = FIX2INT(val);
				}
				else return;

				val = rb_hash_aref(opts, STR2SYM("max"));
				if(TYPE(val) == T_FIXNUM) {
					max = FIX2INT(val);
					hasspin = true;
				}
			}

			virtual ~IntEdit() { }

			typedef wxValidator IntTextValidator; // TODO
			wxControl *Create(wxWindow *parent)
			{
				if (hasspin) {
					return cw = new wxSpinCtrl(parent, -1, wxString::Format(_T("%d"), value), wxDefaultPosition, wxDefaultSize, min, max, value);
				} else {
					return cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0); //, IntTextValidator());
				}
			}

			void ControlReadBack()
			{
				if (hasspin) {
					value = ((wxSpinCtrl*)cw)->GetValue();
				} else {
					long newval;
					text = ((wxTextCtrl*)cw)->GetValue();
					if (text.ToLong(&newval)) {
						value = newval;
					}
				}
			}

			VALUE RubyReadBack()
			{
				return INT2FIX(value);
			}

		};


		// Float only edit

		class FloatEdit : public Edit {
		public:
			float value;
			// FIXME: Can't support spin button atm

			FloatEdit(){};
			FloatEdit(VALUE opts)
				: Edit(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("value"));
				if(TYPE(val) == T_FLOAT) {
					value = NUM2DBL(val);
				} else if (TYPE(val) == T_FIXNUM) {
					value = FIX2INT(val);
				}
				// TODO: spin button support
			}

			virtual ~FloatEdit() { }

			typedef wxValidator FloatTextValidator;
			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0); //, FloatTextValidator());
			}

			void ControlReadBack()
			{
				double newval;
				text = ((wxTextCtrl*)cw)->GetValue();
				if (text.ToDouble(&newval)) {
					value = newval;
				}
			}

			VALUE RubyReadBack()
			{
				return rb_float_new(value);
			}

		};


		// Dropdown

		class Dropdown : public RubyConfigDialogControl {
		public:
			wxArrayString items;
			wxString value;

			Dropdown(){};
			Dropdown(VALUE opts)
				: RubyConfigDialogControl(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("value"));
				if(TYPE(val) == T_STRING)
					value = wxString(StringValueCStr(val), wxConvUTF8);
				
				val = rb_hash_aref(opts, STR2SYM("items"));
				if(TYPE(val) == T_ARRAY)
				{
					long len = RARRAY(val)->len;
					VALUE *ptr = RARRAY(val)->ptr;
					for(int i = 0; i < len; i++)
					{
						if(TYPE(ptr[i]) == T_STRING)
							items.Add(wxString(StringValueCStr(ptr[i]), wxConvUTF8));
					}
				}
			}

			virtual ~Dropdown() { }

			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxComboBox(parent, -1, value, wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
			}

			void ControlReadBack()
			{
				value = ((wxComboBox*)cw)->GetValue();
			}

			VALUE RubyReadBack()
			{
				return rb_str_new2(value.mb_str(wxConvUTF8));
			}
		};


		// Checkbox

		class Checkbox : public RubyConfigDialogControl {
		public:
			wxString label;
			bool value;

			Checkbox(){};
			Checkbox(VALUE opts)
				: RubyConfigDialogControl(opts)
			{
				VALUE val = rb_hash_aref(opts, STR2SYM("label"));
				if(TYPE(val) == T_STRING)
					label = wxString(StringValueCStr(val), wxConvUTF8);

				val = rb_hash_aref(opts, STR2SYM("value"));
				if(val == Qtrue) value = true;
				else value = false;
			}

			virtual ~Checkbox() { }

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxCheckBox(parent, -1, label);
				((wxCheckBox*)cw)->SetValue(value);
				return cw;
			}

			void ControlReadBack()
			{
				value = ((wxCheckBox*)cw)->GetValue();
			}

			VALUE RubyReadBack()
			{
				if(value) return Qtrue;
				return Qfalse;
			}

		};

	};


	// RubyConfigDialog

	RubyConfigDialog::RubyConfigDialog(VALUE config, VALUE btn_data, bool include_buttons)
		: use_buttons(include_buttons)
	{
		wxLogDebug(_T("creating RubyConfigDialog, this addr is %p"), this);
		button_pushed = 0;
	
		if(include_buttons && TYPE(btn_data) == T_ARRAY) 
		{
			long len = RARRAY(btn_data)->len;
			VALUE *ptr = RARRAY(btn_data)->ptr;
			for(int i = 0; i < len; i++)
			{			
				if(rb_respond_to(ptr[i], rb_intern("to_s")))
				{
					ptr[i] = rb_funcall(ptr[i], rb_intern("to_s"), 0);
					wxString s(StringValueCStr(ptr[i]), wxConvUTF8);
					buttons.push_back(s);
				}
			}
		}

		if(TYPE(config) != T_ARRAY)	{
			if(rb_respond_to(config, rb_intern("to_ary")))
				config = rb_funcall(config, rb_intern("to_ary"), 0);
			else throw "Cannot create config dialog from something non-table";
		}

		long len = RARRAY(config)->len;
		VALUE *ptr = RARRAY(config)->ptr;
		for(int i = 0; i < len; i++)
		{
			if(TYPE(ptr[i]) != T_HASH)
				continue;	// skip invalid entry

			VALUE ctrlclass = rb_hash_aref(ptr[i], STR2SYM("class"));

			const char *cls_name;
			if (TYPE(ctrlclass) == T_SYMBOL) {
					cls_name = rb_id2name(SYM2ID(ctrlclass));
			} else if (TYPE(ctrlclass) == T_STRING) {
				cls_name = StringValueCStr(ctrlclass);
			} else continue;
			wxString controlclass(cls_name, wxConvUTF8);

			RubyConfigDialogControl *ctl;

			// Check control class and create relevant control
			if (controlclass == _T("label")) {
				ctl = new RubyControl::Label(ptr[i]);
			} else if (controlclass == _T("edit")) {
				ctl = new RubyControl::Edit(ptr[i]);
			} else if (controlclass == _T("intedit")) {
				ctl = new RubyControl::IntEdit(ptr[i]);
			} else if (controlclass == _T("floatedit")) {
				ctl = new RubyControl::FloatEdit(ptr[i]);
			} else if (controlclass == _T("textbox")) {
				ctl = new RubyControl::Textbox(ptr[i]);
			} else if (controlclass == _T("dropdown")) {
				ctl = new RubyControl::Dropdown(ptr[i]);
			} else if (controlclass == _T("checkbox")) {
				ctl = new RubyControl::Checkbox(ptr[i]);
			} else if (controlclass == _T("color")) {
				// FIXME
				ctl = new RubyControl::Edit(ptr[i]);
			} else if (controlclass == _T("coloralpha")) {
				// FIXME
				ctl = new RubyControl::Edit(ptr[i]);
			} else if (controlclass == _T("alpha")) {
				// FIXME
				ctl = new RubyControl::Edit(ptr[i]);
			} else continue;	// skip

			controls.push_back(ctl);
		}
	}

	RubyConfigDialog::~RubyConfigDialog()
	{
		for (size_t i = 0; i < controls.size(); ++i)
			delete controls[i];
	}

	wxWindow* RubyConfigDialog::CreateWindow(wxWindow *parent)
	{
		wxWindow *w = new wxPanel(parent);
		wxGridBagSizer *s = new wxGridBagSizer(4, 4);

		for (size_t i = 0; i < controls.size(); ++i) {
			RubyConfigDialogControl *c = controls[i];
			c->Create(w);
			if (dynamic_cast<RubyControl::Label*>(c)) {
				s->Add(c->cw, wxGBPosition(c->y, c->x), wxGBSpan(c->height, c->width), wxALIGN_CENTRE_VERTICAL|wxALIGN_LEFT);
			} else {
				s->Add(c->cw, wxGBPosition(c->y, c->x), wxGBSpan(c->height, c->width), wxEXPAND);
			}
		}

		if (use_buttons) {
			wxStdDialogButtonSizer *bs = new wxStdDialogButtonSizer();
			if (buttons.size() > 0) {
				wxLogDebug(_T("creating user buttons"));
				for (size_t i = 0; i < buttons.size(); ++i) {
					wxLogDebug(_T("button '%s' gets id %d"), buttons[i].c_str(), 1001+(wxWindowID)i);
					bs->Add(new wxButton(w, 1001+(wxWindowID)i, buttons[i]));
				}
			} else {
				wxLogDebug(_T("creating default buttons"));
				bs->Add(new wxButton(w, wxID_OK));
				bs->Add(new wxButton(w, wxID_CANCEL));
			}
			bs->Realize();

			button_event = new ButtonEventHandler();
			button_event->button_pushed = &button_pushed;
			// passing button_event as userdata because wx will then delete it
			w->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RubyConfigDialog::ButtonEventHandler::OnButtonPush), button_event, button_event);
			wxLogDebug(_T("set event handler, this addr is %p"), this);

			wxBoxSizer *ms = new wxBoxSizer(wxVERTICAL);
			ms->Add(s, 0, wxBOTTOM, 5);
			ms->Add(bs);
			w->SetSizerAndFit(ms);
		} else {
			w->SetSizerAndFit(s);
		}

		return w;
	}

	VALUE RubyConfigDialog::RubyReadBack()
	{
		VALUE cfg = rb_hash_new();

		for (size_t i = 0; i < controls.size(); ++i) {
			VALUE v = controls[i]->RubyReadBack();
			if(v != Qnil)
				rb_hash_aset(cfg, controls[i]->name_sym, v);	
		}
		if (use_buttons) {
			VALUE res = rb_ary_new();

			wxLogDebug(_T("reading back button_pushed"));
			int btn = button_pushed;
			if (btn == 0) {
				wxLogDebug(_T("was zero, cancelled"));
				// Always cancel/closed
				rb_ary_push(res, Qfalse);
			} else {
				wxLogDebug(_T("nonzero, something else: %d"), btn);
				if (buttons.size() > 0) {
					wxLogDebug(_T("user button: %s"), buttons[btn-1].c_str());
					// button_pushed is index+1 to reserve 0 for Cancel
					rb_ary_push(res, rb_str_new2(buttons[btn-1].mb_str(wxConvUTF8)));					
				} else {
					wxLogDebug(_T("default button, must be Ok"));
					// Cancel case already covered, must be Ok then
					rb_ary_push(res, Qtrue);					
				}
			}
			rb_ary_push(res, cfg);	// return array [button, hash with config]
			return res;
		}

		return cfg;	// if no buttons return only hash with config
	}

	void RubyConfigDialog::ReadBack()
	{
		for (size_t i = 0; i < controls.size(); ++i) {
			controls[i]->ControlReadBack();
		}
	}

	void RubyConfigDialog::ButtonEventHandler::OnButtonPush(wxCommandEvent &evt)
	{
		// Let button_pushed == 0 mean "cancelled", such that pushing Cancel or closing the dialog
		// will both result in button_pushed == 0
		if (evt.GetId() == wxID_OK) {
			wxLogDebug(_T("was wxID_OK"));
			*button_pushed = 1;
		} else if (evt.GetId() == wxID_CANCEL) {
			wxLogDebug(_T("was wxID_CANCEL"));
			*button_pushed = 0;
		} else {
			wxLogDebug(_T("was user button"));
			// Therefore, when buttons are numbered from 1001 to 1000+n, make sure to set it to i+1
			*button_pushed = evt.GetId() - 1000;
			evt.SetId(wxID_OK); // hack to make sure the dialog will be closed
		}
		wxLogDebug(_T("button_pushed set to %d"), *button_pushed);
		evt.Skip();
	}

};

#endif // WITH_RUBY
