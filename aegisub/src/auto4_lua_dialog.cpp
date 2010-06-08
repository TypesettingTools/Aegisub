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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file auto4_lua_dialog.cpp
/// @brief Lua 5.1-based scripting engine (configuration-dialogue interface)
/// @ingroup scripting
///


#include "config.h"

#ifdef WITH_AUTO4_LUA

#ifndef AGI_PRE
#include <assert.h>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/log.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/tokenzr.h>
#include <wx/validate.h>
#include <wx/window.h>
#endif

#include <libaegisub/log.h>

#include "ass_style.h"
#include "auto4_lua.h"
#include "colour_button.h"
#include "string_codec.h"
#include "utils.h"

// These must be after the headers above.
#ifdef __WINDOWS__
#include "../../contrib/lua51/src/lualib.h"
#include "../../contrib/lua51/src/lauxlib.h"
#else
#include <lualib.h>
#include <lauxlib.h>
#endif


/// DOCME
namespace Automation4 {


	// LuaConfigDialogControl


	/// @brief DOCME
	/// @param L 
	///
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

		LOG_D("automation/lua/dialog") << "created control: '" << name.c_str() << "', (" << x << "," << y << ")(" << width << "," << height << ", "<< hint.c_str();
	}


	/// DOCME
	namespace LuaControl {

		// Label


		/// DOCME
		/// @class Label
		/// @brief DOCME
		///
		/// DOCME
		class Label : public LuaConfigDialogControl {
		public:

			/// DOCME
			wxString label;


			/// @brief DOCME
			/// @param L 
			///
			Label(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "label");
				label = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}


			/// @brief DOCME
			///
			virtual ~Label() { }

			// Doesn't have a serialisable value so don't implement that sub-interface


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				return cw = new wxStaticText(parent, -1, label);
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				// Nothing here
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				// Label doesn't produce output, so let it be nil
				lua_pushnil(L);
			}
		};


		// Basic edit


		/// DOCME
		/// @class Edit
		/// @brief DOCME
		///
		/// DOCME
		class Edit : public LuaConfigDialogControl {
		public:

			/// DOCME
			wxString text;


			/// @brief DOCME
			/// @param L 
			///
			Edit(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				// Undocumented behaviour, 'value' is also accepted as key for text,
				// mostly so a text control can stand in for other things.
				// This shouldn't be exploited and might change later.
				lua_getfield(L, -1, "value");
				if (lua_isnil(L, -1))
				{
					lua_pop(L, 1);
					lua_getfield(L, -1, "text");
				}
				text = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}


			/// @brief DOCME
			///
			virtual ~Edit() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return inline_string_encode(text);
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				text = inline_string_decode(serialised);
			}


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, 0);
				cw->SetToolTip(hint);
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				text = ((wxTextCtrl*)cw)->GetValue();
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, text.mb_str(wxConvUTF8));
			}

		};



		/// DOCME
		/// @class Color
		/// @brief DOCME
		///
		/// DOCME
		class Color : public LuaConfigDialogControl {
		public:

			/// DOCME
			wxString text;


			/// @brief DOCME
			/// @param L 
			///
			Color(lua_State *L)
				: LuaConfigDialogControl(L)
			{
				lua_getfield(L, -1, "value");
				text = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			}


			/// @brief DOCME
			///
			virtual ~Color() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return inline_string_encode(text);
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				text = inline_string_decode(serialised);
			}


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				AssColor colour;
				colour.Parse(text);
				cw = new ColourButton(parent, -1, wxSize(50*width,10*height), colour.GetWXColor());
				cw->SetToolTip(hint);
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				text = ((ColourButton*)cw)->GetColour().GetAsString(wxC2S_HTML_SYNTAX);
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, text.mb_str(wxConvUTF8));
			}

		};


		
		// Multiline edit


		/// DOCME
		/// @class Textbox
		/// @brief DOCME
		///
		/// DOCME
		class Textbox : public Edit {
		public:


			/// @brief DOCME
			/// @param L 
			///
			Textbox(lua_State *L)
				: Edit(L)
			{
				// Nothing more
			}


			/// @brief DOCME
			///
			virtual ~Textbox() { }

			// Same serialisation interface as single-line edit


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
				cw->SetMinSize(wxSize(0, 30));
				cw->SetToolTip(hint);
				return cw;
			}

		};


		// Integer only edit


		/// DOCME
		/// @class IntEdit
		/// @brief DOCME
		///
		/// DOCME
		class IntEdit : public Edit {
		public:

			/// DOCME
			int value;

			/// DOCME
			bool hasspin;

			/// DOCME

			/// DOCME
			int min, max;


			/// @brief DOCME
			/// @param L 
			///
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
					// Assuming we are using a binary machine with finite word length,
					// that represents integers as two's complement, this will get us
					// the largest and smallest values representable by the int type.
					max = (int)((~(unsigned int)0) >> 1);
					min = ~max;
				}
			}


			/// @brief DOCME
			///
			virtual ~IntEdit() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return wxString::Format(_T("%d"), value);
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				long tmp;
				if (serialised.ToLong(&tmp))
					value = tmp;
			}


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				wxSpinCtrl *scw = new wxSpinCtrl(parent, -1, wxString::Format(_T("%d"), value), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, value);
				scw->SetToolTip(hint);
				cw = scw;
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				value = ((wxSpinCtrl*)cw)->GetValue();
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushinteger(L, value);
			}

		};


		// Float only edit


		/// DOCME
		/// @class FloatEdit
		/// @brief DOCME
		///
		/// DOCME
		class FloatEdit : public Edit {
		public:

			/// DOCME
			float value;

			/// DOCME
			bool hasspin;

			/// DOCME

			/// DOCME
			float min, max;
			// FIXME: Can't support spin button atm


			/// @brief DOCME
			/// @param L 
			///
			FloatEdit(lua_State *L)
				: Edit(L)
			{
				lua_getfield(L, -1, "value");
				value = (float)lua_tonumber(L, -1);
				lua_pop(L, 1);

				// TODO: spin button support
			}


			/// @brief DOCME
			///
			virtual ~FloatEdit() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return AegiFloatToString(value);
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				double tmp;
				if (serialised.ToDouble(&tmp))
					value = (float)tmp;
			}


			/// DOCME
			typedef wxValidator FloatTextValidator;

			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxTextCtrl(parent, -1, AegiFloatToString(value), wxDefaultPosition, wxDefaultSize, 0); //, FloatTextValidator());
				cw->SetToolTip(hint);
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				double newval;
				text = ((wxTextCtrl*)cw)->GetValue();
				if (text.ToDouble(&newval)) {
					value = newval;
				}
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushnumber(L, value);
			}

		};


		// Dropdown


		/// DOCME
		/// @class Dropdown
		/// @brief DOCME
		///
		/// DOCME
		class Dropdown : public LuaConfigDialogControl {
		public:

			/// DOCME
			wxArrayString items;

			/// DOCME
			wxString value;


			/// @brief DOCME
			/// @param L 
			///
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


			/// @brief DOCME
			///
			virtual ~Dropdown() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return inline_string_encode(value);
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				value = inline_string_decode(serialised);
			}


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxComboBox(parent, -1, value, wxDefaultPosition, wxDefaultSize, items, wxCB_READONLY);
				cw->SetToolTip(hint);
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				value = ((wxComboBox*)cw)->GetValue();
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushstring(L, value.mb_str(wxConvUTF8));
			}
			
		};


		// Checkbox


		/// DOCME
		/// @class Checkbox
		/// @brief DOCME
		///
		/// DOCME
		class Checkbox : public LuaConfigDialogControl {
		public:

			/// DOCME
			wxString label;

			/// DOCME
			bool value;


			/// @brief DOCME
			/// @param L 
			///
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


			/// @brief DOCME
			///
			virtual ~Checkbox() { }


			/// @brief DOCME
			/// @return 
			///
			bool CanSerialiseValue()
			{
				return true;
			}


			/// @brief DOCME
			/// @return 
			///
			wxString SerialiseValue()
			{
				return value ? _T("1") : _T("0");
			}


			/// @brief DOCME
			/// @param serialised 
			///
			void UnserialiseValue(const wxString &serialised)
			{
				// fixme? should this allow more different "false" values?
				value = (serialised == _T("0")) ? false : true;
			}


			/// @brief DOCME
			/// @param parent 
			/// @return 
			///
			wxControl *Create(wxWindow *parent)
			{
				cw = new wxCheckBox(parent, -1, label);
				cw->SetToolTip(hint);
				static_cast<wxCheckBox*>(cw)->SetValue(value);
				return cw;
			}


			/// @brief DOCME
			///
			void ControlReadBack()
			{
				value = ((wxCheckBox*)cw)->GetValue();
			}


			/// @brief DOCME
			/// @param L 
			///
			void LuaReadBack(lua_State *L)
			{
				lua_pushboolean(L, value);
			}

		};

	};


	// LuaConfigDialog


	/// @brief DOCME
	/// @param L               
	/// @param include_buttons 
	///
	LuaConfigDialog::LuaConfigDialog(lua_State *L, bool include_buttons)
		: use_buttons(include_buttons)
	{
		LOG_D("automation/lua/dialog") << "creating LuaConfigDialoug, addr: " << this;
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


	/// @brief DOCME
	///
	LuaConfigDialog::~LuaConfigDialog()
	{
		for (size_t i = 0; i < controls.size(); ++i)
			delete controls[i];
	}


	/// @brief DOCME
	/// @param parent 
	/// @return 
	///
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
				LOG_D("automation/lua/dialog") << "creating user buttons";
				for (size_t i = 0; i < buttons.size(); ++i) {
					LOG_D("automation/lua/dialog") << "button '" << buttons[i].c_str() << "' gets id " << 1001+(wxWindowID)i;

					bs->Add(new wxButton(w, 1001+(wxWindowID)i, buttons[i]));
				}
			} else {
				LOG_D("automation/lua/dialog") << "creating default buttons";
				bs->Add(new wxButton(w, wxID_OK));
				bs->Add(new wxButton(w, wxID_CANCEL));
			}
			bs->Realize();

			button_event = new ButtonEventHandler();
			button_event->button_pushed = &button_pushed;
			// passing button_event as userdata because wx will then delete it
			w->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LuaConfigDialog::ButtonEventHandler::OnButtonPush), button_event, button_event);
			LOG_D("automation/lua/dialog") << "set event handler, addr: " << this;

			wxBoxSizer *ms = new wxBoxSizer(wxVERTICAL);
			ms->Add(s, 0, wxBOTTOM, 5);
			ms->Add(bs);
			w->SetSizerAndFit(ms);
		} else {
			w->SetSizerAndFit(s);
		}

		return w;
	}


	/// @brief DOCME
	/// @param L 
	/// @return 
	///
	int LuaConfigDialog::LuaReadBack(lua_State *L)
	{
		// First read back which button was pressed, if any
		if (use_buttons) {
			LOG_D("automation/lua/dialog") << "reading back button_pushed";
			int btn = button_pushed;
			if (btn == 0) {
				LOG_D("automation/lua/dialog") << "was zero, cancelled";
				// Always cancel/closed
				lua_pushboolean(L, 0);
			} else if (buttons.size() == 0 && btn == 1) {
				LOG_D("automation/lua/dialog") << "default buttons, button 1 bushed, Ok button";
				lua_pushboolean(L, 1);
			} else {
				LOG_D("automation/lua/dialog") << "user button: " << buttons.at(btn-1).c_str();
				// button_pushed is index+1 to reserve 0 for Cancel
				lua_pushstring(L, buttons.at(btn-1).mb_str(wxConvUTF8));
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


	/// @brief DOCME
	/// @return 
	///
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


	/// @brief DOCME
	/// @param serialised 
	///
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


	/// @brief DOCME
	///
	void LuaConfigDialog::ReadBack()
	{
		for (size_t i = 0; i < controls.size(); ++i) {
			controls[i]->ControlReadBack();
		}
	}


	/// @brief DOCME
	/// @param evt 
	///
	void LuaConfigDialog::ButtonEventHandler::OnButtonPush(wxCommandEvent &evt)
	{
		// Let button_pushed == 0 mean "cancelled", such that pushing Cancel or closing the dialog
		// will both result in button_pushed == 0
		if (evt.GetId() == wxID_OK) {
			LOG_D("automation/lua/dialog") << "was wxID_OK";
			*button_pushed = 1;
		} else if (evt.GetId() == wxID_CANCEL) {
			LOG_D("automation/lua/dialog") << "was wxID_CANCEL";
			*button_pushed = 0;
		} else {
			LOG_D("automation/lua/dialog") << "was user button";
			// Therefore, when buttons are numbered from 1001 to 1000+n, make sure to set it to i+1
			*button_pushed = evt.GetId() - 1000;

			// hack to make sure the dialog will be closed
			// only do this for non-colour buttons
			ColourButton *button = dynamic_cast<ColourButton*> (evt.GetEventObject());
			if (button) return;
			evt.SetId(wxID_OK);
		}
		LOG_D("automation/lua/dialog") << "button_pushed set to: " << *button_pushed;
		evt.Skip();
	}

};

#endif // WITH_AUTO4_LUA


