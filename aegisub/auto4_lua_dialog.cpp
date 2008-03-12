// Copyright (c) 2006, 2007, Niels Martin Hansen
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


#ifdef WITH_AUTO4_LUA

#include "auto4_lua.h"

#ifdef __WINDOWS__
#include "../lua51/src/lualib.h"
#include "../lua51/src/lauxlib.h"
#else
#include "lualib.h"
#include "lauxlib.h"
#endif

#include "string_codec.h"
#include "utils.h"
#include <wx/window.h>
#include <wx/spinctrl.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/validate.h>
#include <wx/tokenzr.h>
#include <assert.h>
#include "colour_button.h"

namespace Automation4 {


	// LuaConfigDialogControl

	LuaConfigDialogControl::LuaConfigDialogControl(lua_State *L)
	{
		// Assume top of stack is a control table (don't do checking)

		lua_getfield(L, -1, "name");
		if (lua_isstring(L, -1)) {
			name = wxString(lua_tostring(L, -1), wxConvUTF8);
		} else {
			name = _T("");
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "x");
		if (lua_isnumber(L, -1)) {
			x = lua_tointeger(L, -1);
			if (x < 0) x = 0;
		} else {
			x = 0;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "y");
		if (lua_isnumber(L, -1)) {
			y = lua_tointeger(L, -1);
			if (y < 0) y = 0;
		} else {
			y = 0;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "width");
		if (lua_isnumber(L, -1)) {
			width = lua_tointeger(L, -1);
			if (width < 1) width = 1;
		} else {
			width = 1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "height");
		if (lua_isnumber(L, -1)) {
			height = lua_tointeger(L, -1);
			if (height < 1) height = 1;
		} else {
			height = 1;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "hint");
		if (lua_isstring(L, -1)) {
			hint = wxString(lua_tostring(L, -1), wxConvUTF8);
		} else {
			hint = _T("");
		}
		lua_pop(L, 1);

		wxLogDebug(_T("created control: '%s', (%d,%d)(%d,%d), '%s'"), name.c_str(), x, y, width, height, hint.c_str());
	}

	namespace LuaControl {

		// Label

		class Label : public LuaConfigDialogControl {
		public:
			wxString label;

			Label(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "label");
				label = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}

			virtual ~Label() { }

			// Doesn't have a serialisable value so don't implement that sub-interface

			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxStaticText(parent, -1, label);
			}

			void ControlReadBack()
			{
				// Nothing here
			}

			void LuaReadBack(lua_State *L)
			{
				// Label doesn't produce output, so let it be nil
				lua_pushnil(L);
			}
		};


		// Basic edit

		class Edit : public LuaConfigDialogControl {
		public:
			wxString text;

			Edit(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "text");
				text = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}

			virtual ~Edit() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return inline_string_encode(text);
			}

			void UnserialiseValue(const wxString &serialised)
			{
				text = inline_string_decode(serialised);
			}

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0);
				cw->SetToolTip(hint);
				return cw;
			}

			void ControlReadBack()
			{
				text = ((wxTextCtrl*)cw)->GetValue();
			}

			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, text.mb_str(wxConvUTF8));
			}

		};


		class Color : public LuaConfigDialogControl {
		public:
			wxString text;

			Color(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "value");
				text = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}

			virtual ~Color() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return inline_string_encode(text);
			}

			void UnserialiseValue(const wxString &serialised)
			{
				text = inline_string_decode(serialised);
			}

			wxControl *Create(wxWindow *parent)
			{
				cw = new ColourButton(parent, -1, wxSize(50*width,10*height), wxColour(text));
				cw->SetToolTip(hint);
				return cw;
			}

			void ControlReadBack()
			{
				text = ((ColourButton*)cw)->GetColour().GetAsString(wxC2S_HTML_SYNTAX);
			}

			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, text.mb_str(wxConvUTF8));
			}

		};


		
		// Multiline edit

		class Textbox : public Edit {
		public:

			Textbox(lua_State *L)
				: Edit(L)
			{
				// Nothing more
			}

			virtual ~Textbox() { }

			// Same serialisation interface as single-line edit

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
				cw->SetMinSize(wxSize(0, 30));
				cw->SetToolTip(hint);
				return cw;
			}

		};


		// Integer only edit

		class IntEdit : public Edit {
		public:
			int value;
			bool hasspin;
			int min, max;

			IntEdit(lua_State *L)
				: Edit(L)
			{
				lua_getfield(L, -1, "value");
				value = lua_tointeger(L, -1);
				lua_pop(L, 1);

				hasspin = false;

				lua_getfield(L, -1, "min");
				if (!lua_isnumber(L, -1))
					goto nospin;
				min = lua_tointeger(L, -1);
				lua_pop(L, 1);

				lua_getfield(L, -1, "max");
				if (!lua_isnumber(L, -1))
					goto nospin;
				max = lua_tointeger(L, -1);
				lua_pop(L, 1);

				hasspin = true;
nospin:
				if (!hasspin) {
					lua_pop(L, 1);
				}
			}

			virtual ~IntEdit() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return wxString::Format(_T("%d"), value);
			}

			void UnserialiseValue(const wxString &serialised)
			{
				long tmp;
				if (serialised.ToLong(&tmp))
					value = tmp;
			}

			typedef wxValidator IntTextValidator; // TODO
			wxControl *Create(wxWindow *parent)
			{
				if (hasspin) {
					wxSpinCtrl *scw = new wxSpinCtrl(parent, -1, wxString::Format(_T("%d"), value), wxDefaultPosition, wxDefaultSize, min, max, value);
					scw->SetRange(min, max);
					scw->SetValue(value);
					cw = scw;
				} else {
					cw = new wxTextCtrl(parent, -1, wxString::Format(_T("%d"), value), wxDefaultPosition, wxDefaultSize, 0); //, IntTextValidator());
				}
				cw->SetToolTip(hint);
				return cw;
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

			void LuaReadBack(lua_State *L)
			{
				lua_pushinteger(L, value);
			}

		};


		// Float only edit

		class FloatEdit : public Edit {
		public:
			float value;
			bool hasspin;
			float min, max;
			// FIXME: Can't support spin button atm

			FloatEdit(lua_State *L)
				: Edit(L)
			{
				lua_getfield(L, -1, "value");
				value = lua_tointeger(L, -1);
				lua_pop(L, 1);

				// TODO: spin button support
			}

			virtual ~FloatEdit() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return PrettyFloatF(value);
			}

			void UnserialiseValue(const wxString &serialised)
			{
				double tmp;
				if (serialised.ToDouble(&tmp))
					value = (float)tmp;
			}

			typedef wxValidator FloatTextValidator;
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0); //, FloatTextValidator());
				cw->SetToolTip(hint);
				return cw;
			}

			void ControlReadBack()
			{
				double newval;
				text = ((wxTextCtrl*)cw)->GetValue();
				if (text.ToDouble(&newval)) {
					value = newval;
				}
			}

			void LuaReadBack(lua_State *L)
			{
				lua_pushnumber(L, value);
			}

		};


		// Dropdown

		class Dropdown : public LuaConfigDialogControl {
		public:
			wxArrayString items;
			wxString value;

			Dropdown(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "value");
				value = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);

				lua_getfield(L, -1, "items");
				lua_pushnil(L);
				while (lua_next(L, -2)) {
					if (lua_isstring(L, -1)) {
						items.Add(wxString(lua_tostring(L, -1), wxConvUTF8));
					}
					lua_pop(L, 1);
				}
				lua_pop(L, 1);
			}

			virtual ~Dropdown() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return inline_string_encode(value);
			}

			void UnserialiseValue(const wxString &serialised)
			{
				value = inline_string_decode(serialised);
			}

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxComboBox(parent, -1, value, wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
				cw->SetToolTip(hint);
				return cw;
			}

			void ControlReadBack()
			{
				value = ((wxComboBox*)cw)->GetValue();
			}

			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, value.mb_str(wxConvUTF8));
			}
			
		};


		// Checkbox

		class Checkbox : public LuaConfigDialogControl {
		public:
			wxString label;
			bool value;

			Checkbox(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "label");
				label = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);

				lua_getfield(L, -1, "value");
				value = lua_toboolean(L, -1) != 0;
				lua_pop(L, 1);
			}

			virtual ~Checkbox() { }

			bool CanSerialiseValue()
			{
				return true;
			}

			wxString SerialiseValue()
			{
				return value ? _T("1") : _T("0");
			}

			void UnserialiseValue(const wxString &serialised)
			{
				// fixme? should this allow more different "false" values?
				value = (serialised == _T("0")) ? false : true;
			}

			wxControl *Create(wxWindow *parent)
			{
				cw = new wxCheckBox(parent, -1, label);
				cw->SetToolTip(hint);
				return cw;
			}

			void ControlReadBack()
			{
				value = ((wxCheckBox*)cw)->GetValue();
			}

			void LuaReadBack(lua_State *L)
			{
				lua_pushboolean(L, value);
			}

		};

	};


	// LuaConfigDialog

	LuaConfigDialog::LuaConfigDialog(lua_State *L, bool include_buttons)
		: use_buttons(include_buttons)
	{
		wxLogDebug(_T("creating LuaConfigDialog, this addr is %p"), this);
		button_pushed = 0;
		if (include_buttons) {

			if (!lua_istable(L, -1))
				// Just to avoid deeper indentation...
				goto skipbuttons;
			// Iterate over items in table
			lua_pushnil(L); // initial key
			while (lua_next(L, -2)) {
				// Simply skip invalid items... FIXME, warn here?
				if (lua_isstring(L, -1)) {
					wxString s(lua_tostring(L, -1), wxConvUTF8);
					buttons.push_back(s);
				}
				lua_pop(L, 1);
			}
skipbuttons:
			lua_pop(L, 1);
		}

		// assume top of stack now contains a dialog table
		if (!lua_istable(L, -1)) {
			lua_pushstring(L, "Cannot create config dialog from something non-table");
			lua_error(L);
			assert(false);
		}

		// Ok, so there is a table with controls
		lua_pushnil(L); // initial key
		while (lua_next(L, -2)) {
			if (lua_istable(L, -1)) {
				// Get control class
				lua_getfield(L, -1, "class");
				if (!lua_isstring(L, -1))
					goto badcontrol;
				wxString controlclass(lua_tostring(L, -1), wxConvUTF8);
				controlclass.LowerCase();
				lua_pop(L, 1);

				LuaConfigDialogControl *ctl;

				// Check control class and create relevant control
				if (controlclass == _T("label")) {
					ctl = new LuaControl::Label(L);
				} else if (controlclass == _T("edit")) {
					ctl = new LuaControl::Edit(L);
				} else if (controlclass == _T("intedit")) {
					ctl = new LuaControl::IntEdit(L);
				} else if (controlclass == _T("floatedit")) {
					ctl = new LuaControl::FloatEdit(L);
				} else if (controlclass == _T("textbox")) {
					ctl = new LuaControl::Textbox(L);
				} else if (controlclass == _T("dropdown")) {
					ctl = new LuaControl::Dropdown(L);
				} else if (controlclass == _T("checkbox")) {
					ctl = new LuaControl::Checkbox(L);
				} else if (controlclass == _T("color")) {
					ctl = new LuaControl::Color(L);
				} else if (controlclass == _T("coloralpha")) {
					// FIXME
					ctl = new LuaControl::Edit(L);
				} else if (controlclass == _T("alpha")) {
					// FIXME
					ctl = new LuaControl::Edit(L);
				} else {
					goto badcontrol;
				}

				controls.push_back(ctl);

			} else {
badcontrol:
				// not a control...
				// FIXME, better error reporting?
				lua_pushstring(L, "bad control table entry");
				lua_error(L);
			}
			lua_pop(L, 1);
		}
	}

	LuaConfigDialog::~LuaConfigDialog()
	{
		for (size_t i = 0; i < controls.size(); ++i)
			delete controls[i];
	}

	wxWindow* LuaConfigDialog::CreateWindow(wxWindow *parent)
	{
		wxWindow *w = new wxPanel(parent);
		wxGridBagSizer *s = new wxGridBagSizer(4, 4);

		for (size_t i = 0; i < controls.size(); ++i) {
			LuaConfigDialogControl *c = controls[i];
			c->Create(w);
			if (dynamic_cast<LuaControl::Label*>(c)) {
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
			w->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LuaConfigDialog::ButtonEventHandler::OnButtonPush), button_event, button_event);
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

	int LuaConfigDialog::LuaReadBack(lua_State *L)
	{
		// First read back which button was pressed, if any
		if (use_buttons) {
			wxLogDebug(_T("reading back button_pushed"));
			int btn = button_pushed;
			if (btn == 0) {
				wxLogDebug(_T("was zero, cancelled"));
				// Always cancel/closed
				lua_pushboolean(L, 0);
			} else {
				wxLogDebug(_T("nonzero, something else: %d"), btn);
				if (buttons.size() > 0) {
					wxLogDebug(_T("user button: %s"), buttons[btn-1].c_str());
					// button_pushed is index+1 to reserve 0 for Cancel
					lua_pushstring(L, buttons[btn-1].mb_str(wxConvUTF8));
				} else {
					wxLogDebug(_T("default button, must be Ok"));
					// Cancel case already covered, must be Ok then
					lua_pushboolean(L, 1);
				}
			}
		}

		// Then read controls back
		lua_newtable(L);
		for (size_t i = 0; i < controls.size(); ++i) {
			controls[i]->LuaReadBack(L);
			lua_setfield(L, -2, controls[i]->name.mb_str(wxConvUTF8));
		}

		if (use_buttons) {
			return 2;
		} else {
			return 1;
		}
	}

	wxString LuaConfigDialog::Serialise()
	{
		if (controls.size() == 0)
			return _T("");

		wxString res;

		// Format into "name1:value1|name2:value2|name3:value3|"
		for (size_t i = 0; i < controls.size(); ++i) {
			if (controls[i]->CanSerialiseValue()) {
				wxString sn = inline_string_encode(controls[i]->name);
				wxString sv = controls[i]->SerialiseValue();
				res += wxString::Format(_T("%s:%s|"), sn.c_str(), sv.c_str());
			}
		}

		// Remove trailing pipe
		if (!res.IsEmpty())
			res.RemoveLast();

		return res;
	}

	void LuaConfigDialog::Unserialise(const wxString &serialised)
	{
		// Split by pipe
		wxStringTokenizer tk(serialised, _T("|"));
		while (tk.HasMoreTokens()) {
			// Split by colon
			wxString pair = tk.GetNextToken();
			wxString name = inline_string_decode(pair.BeforeFirst(_T(':')));
			wxString value = pair.AfterFirst(_T(':'));

			// Hand value to all controls matching name
			for (size_t i = 0; i < controls.size(); ++i) {
				if (controls[i]->name == name && controls[i]->CanSerialiseValue()) {
					controls[i]->UnserialiseValue(value);
				}
			}
		}
	}

	void LuaConfigDialog::ReadBack()
	{
		for (size_t i = 0; i < controls.size(); ++i) {
			controls[i]->ControlReadBack();
		}
	}

	void LuaConfigDialog::ButtonEventHandler::OnButtonPush(wxCommandEvent &evt)
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

#endif // WITH_AUTO4_LUA
