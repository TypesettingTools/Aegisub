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

/// @file auto4_lua_dialog.cpp
/// @brief Lua 5.1-based scripting engine (configuration-dialogue interface)
/// @ingroup scripting
///

#include "config.h"

#include "auto4_lua.h"

#include "auto4_lua_utils.h"
#include "ass_style.h"
#include "colour_button.h"
#include "compat.h"
#include "string_codec.h"
#include "utils.h"
#include "validators.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/tokenizer.hpp>
#include <cassert>
#include <cfloat>
#include <unordered_map>

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/validate.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/window.h>

namespace {
	inline void get_if_right_type(lua_State *L, std::string &def) {
		if (lua_isstring(L, -1))
			def = lua_tostring(L, -1);
	}

	inline void get_if_right_type(lua_State *L, double &def) {
		if (lua_isnumber(L, -1))
			def = lua_tonumber(L, -1);
	}

	inline void get_if_right_type(lua_State *L, int &def) {
		if (lua_isnumber(L, -1))
			def = lua_tointeger(L, -1);
	}

	inline void get_if_right_type(lua_State *L, bool &def) {
		if (lua_isboolean(L, -1))
			def = !!lua_toboolean(L, -1);
	}

	template<class T>
	T get_field(lua_State *L, const char *name, T def) {
		lua_getfield(L, -1, name);
		get_if_right_type(L, def);
		lua_pop(L, 1);
		return def;
	}

	inline std::string get_field(lua_State *L, const char *name) {
		return get_field(L, name, std::string());
	}

	template<class T>
	void read_string_array(lua_State *L, T &cont) {
		lua_for_each(L, [&] {
			if (lua_isstring(L, -1))
				cont.push_back(lua_tostring(L, -1));
		});
	}

	int string_to_wx_id(std::string const& str) {
		static std::unordered_map<std::string, int> ids;
		if (ids.empty()) {
			ids["ok"] = wxID_OK;
			ids["yes"] = wxID_YES;
			ids["save"] = wxID_SAVE;
			ids["apply"] = wxID_APPLY;
			ids["close"] = wxID_CLOSE;
			ids["no"] = wxID_NO;
			ids["cancel"] = wxID_CANCEL;
			ids["help"] = wxID_HELP;
			ids["context_help"] = wxID_CONTEXT_HELP;
		}
		auto it = ids.find(str);
		return it == end(ids) ? -1 : it->second;
	}
}

namespace Automation4 {
	// LuaDialogControl
	LuaDialogControl::LuaDialogControl(lua_State *L)
	// Assume top of stack is a control table (don't do checking)
	: name(get_field(L, "name"))
	, hint(get_field(L, "hint"))
	, x(get_field(L, "x", 0))
	, y(get_field(L, "y", 0))
	, width(get_field(L, "width", 1))
	, height(get_field(L, "height", 1))
	{
		LOG_D("automation/lua/dialog") << "created control: '" << name << "', (" << x << "," << y << ")(" << width << "," << height << "), " << hint;
	}

	namespace LuaControl {
		/// A static text label
		class Label : public LuaDialogControl {
			std::string label;
		public:
			Label(lua_State *L) : LuaDialogControl(L), label(get_field(L, "label")) { }

			wxControl *Create(wxWindow *parent) {
				return new wxStaticText(parent, -1, to_wx(label));
			}

			int GetSizerFlags() const { return wxALIGN_CENTRE_VERTICAL | wxALIGN_LEFT; }

			void LuaReadBack(lua_State *L) {
				// Label doesn't produce output, so let it be nil
				lua_pushnil(L);
			}
		};

		/// A single-line text edit control
		class Edit : public LuaDialogControl {
		protected:
			std::string text;
			wxTextCtrl *cw;

		public:
			Edit(lua_State *L)
			: LuaDialogControl(L)
			, text(get_field(L, "value"))
			, cw(0)
			{
				// Undocumented behaviour, 'value' is also accepted as key for text,
				// mostly so a text control can stand in for other things.
				// This shouldn't be exploited and might change later.
				text = get_field(L, "text", text);
			}

			bool CanSerialiseValue() const { return true; }
			std::string SerialiseValue() const { return inline_string_encode(text); }
			void UnserialiseValue(const std::string &serialised) { text = inline_string_decode(serialised); }

			wxControl *Create(wxWindow *parent) {
				cw = new wxTextCtrl(parent, -1, to_wx(text));
				cw->SetValidator(StringBinder(&text));
				cw->SetToolTip(to_wx(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushstring(L, text.c_str());
			}
		};

		/// A color-picker button
		class Color : public LuaDialogControl {
			std::string text;
			bool alpha;

			struct ColorValidator : public wxValidator {
				std::string *text;
				ColorValidator(std::string *text) : text(text) { }
				wxValidator *Clone() const { return new ColorValidator(text); }
				bool Validate(wxWindow*) { return true; }
				bool TransferToWindow() { return true; }

				bool TransferFromWindow() {
					*text = static_cast<ColourButton*>(GetWindow())->GetColor().GetHexFormatted();
					return true;
				}
			};

		public:
			Color(lua_State *L, bool alpha)
			: LuaDialogControl(L)
			, text(get_field(L, "value"))
			, alpha(alpha)
			{
			}

			bool CanSerialiseValue() const { return true; }
			std::string SerialiseValue() const { return inline_string_encode(text); }
			void UnserialiseValue(const std::string &serialised) { text = inline_string_decode(serialised); }

			wxControl *Create(wxWindow *parent) {
				agi::Color colour(text);
				wxControl *cw = new ColourButton(parent, wxSize(50*width,10*height), alpha, colour, ColorValidator(&text));
				cw->SetToolTip(to_wx(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushstring(L, text.c_str());
			}
		};

		/// A multiline text edit control
		class Textbox : public Edit {
		public:
			Textbox(lua_State *L) : Edit(L) { }

			// Same serialisation interface as single-line edit
			wxControl *Create(wxWindow *parent) {
				cw = new wxTextCtrl(parent, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, StringBinder(&text));
				cw->SetMinSize(wxSize(0, 30));
				cw->SetToolTip(to_wx(hint));
				return cw;
			}
		};


		/// Integer only edit
		class IntEdit : public Edit {
			wxSpinCtrl *cw;
			int value;
			int min, max;

		public:
			IntEdit(lua_State *L)
			: Edit(L)
			, cw(0)
			, value(get_field(L, "value", 0))
			, min(get_field(L, "min", INT_MIN))
			, max(get_field(L, "max", INT_MAX))
			{
				if (min >= max) {
					max = INT_MAX;
					min = INT_MIN;
				}
			}

			bool CanSerialiseValue() const  { return true; }
			std::string SerialiseValue() const { return std::to_string(value); }
			void UnserialiseValue(const std::string &serialised) { value = atoi(serialised.c_str()); }

			wxControl *Create(wxWindow *parent) {
				cw = new wxSpinCtrl(parent, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, value);
				cw->SetValidator(wxGenericValidator(&value));
				cw->SetToolTip(to_wx(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushinteger(L, value);
			}
		};

		// Float only edit
		class FloatEdit : public Edit {
			double value;
			double min;
			double max;
			double step;
			wxSpinCtrlDouble *scd;

			struct DoubleValidator : public wxValidator {
				double *value;
				DoubleValidator(double *value) : value(value) { }
				wxValidator *Clone() const { return new DoubleValidator(value); }
				bool Validate(wxWindow*) { return true; }

				bool TransferToWindow() {
					static_cast<wxSpinCtrlDouble*>(GetWindow())->SetValue(*value);
					return true;
				}

				bool TransferFromWindow() {
					*value = static_cast<wxSpinCtrlDouble*>(GetWindow())->GetValue();
					return true;
				}
			};

		public:
			FloatEdit(lua_State *L)
			: Edit(L)
			, value(get_field(L, "value", 0.0))
			, min(get_field(L, "min", -DBL_MAX))
			, max(get_field(L, "max", DBL_MAX))
			, step(get_field(L, "step", 0.0))
			, scd(0)
			{
				if (min >= max) {
					max = DBL_MAX;
					min = -DBL_MAX;
				}
			}

			bool CanSerialiseValue() const { return true; }
			std::string SerialiseValue() const { return std::to_string(value); }
			void UnserialiseValue(const std::string &serialised) { value = atof(serialised.c_str()); }

			wxControl *Create(wxWindow *parent) {
				if (step > 0) {
					scd = new wxSpinCtrlDouble(parent, -1, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, value, step);
					scd->SetValidator(DoubleValidator(&value));
					scd->SetToolTip(to_wx(hint));
					return scd;
				}

				wxFloatingPointValidator<double> val(4, &value, wxNUM_VAL_NO_TRAILING_ZEROES);
				val.SetRange(min, max);

				cw = new wxTextCtrl(parent, -1, "", wxDefaultPosition, wxDefaultSize, 0, val);
				cw->SetToolTip(to_wx(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushnumber(L, value);
			}
		};

		/// A dropdown list
		class Dropdown : public LuaDialogControl {
			std::vector<std::string> items;
			std::string value;
			wxComboBox *cw;

		public:
			Dropdown(lua_State *L)
			: LuaDialogControl(L)
			, value(get_field(L, "value"))
			, cw(0)
			{
				lua_getfield(L, -1, "items");
				read_string_array(L, items);
			}

			bool CanSerialiseValue() const { return true; }
			std::string SerialiseValue() const { return inline_string_encode(value); }
			void UnserialiseValue(const std::string &serialised) { value = inline_string_decode(serialised); }

			wxControl *Create(wxWindow *parent) {
				cw = new wxComboBox(parent, -1, to_wx(value), wxDefaultPosition, wxDefaultSize, to_wx(items), wxCB_READONLY, StringBinder(&value));
				cw->SetToolTip(to_wx(hint));
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushstring(L, value.c_str());
			}
		};

		class Checkbox : public LuaDialogControl {
			std::string label;
			bool value;
			wxCheckBox *cw;

		public:
			Checkbox(lua_State *L)
			: LuaDialogControl(L)
			, label(get_field(L, "label"))
			, value(get_field(L, "value", false))
			, cw(0)
			{
			}

			bool CanSerialiseValue() const { return true; }
			std::string SerialiseValue() const { return value ? "1" : "0"; }
			void UnserialiseValue(const std::string &serialised) { value = serialised != "0"; }

			wxControl *Create(wxWindow *parent) {
				cw = new wxCheckBox(parent, -1, to_wx(label));
				cw->SetValidator(wxGenericValidator(&value));
				cw->SetToolTip(to_wx(hint));
				cw->SetValue(value);
				return cw;
			}

			void LuaReadBack(lua_State *L) {
				lua_pushboolean(L, value);
			}
		};
	}

	// LuaDialog
	LuaDialog::LuaDialog(lua_State *L, bool include_buttons)
	: use_buttons(include_buttons)
	, button_pushed(0)
	, window(0)
	{
		LOG_D("automation/lua/dialog") << "creating LuaDialoug, addr: " << this;

		// assume top of stack now contains a dialog table
		if (!lua_istable(L, 1))
			luaL_error(L, "Cannot create config dialog from something non-table");

		// Ok, so there is a table with controls
		lua_pushvalue(L, 1);
		lua_for_each(L, [&] {
			if (!lua_istable(L, -1))
				luaL_error(L, "bad control table entry");

			std::string controlclass = get_field(L, "class");
			boost::to_lower(controlclass);

			LuaDialogControl *ctl;

			// Check control class and create relevant control
			if (controlclass == "label")
				ctl = new LuaControl::Label(L);
			else if (controlclass == "edit")
				ctl = new LuaControl::Edit(L);
			else if (controlclass == "intedit")
				ctl = new LuaControl::IntEdit(L);
			else if (controlclass == "floatedit")
				ctl = new LuaControl::FloatEdit(L);
			else if (controlclass == "textbox")
				ctl = new LuaControl::Textbox(L);
			else if (controlclass == "dropdown")
				ctl = new LuaControl::Dropdown(L);
			else if (controlclass == "checkbox")
				ctl = new LuaControl::Checkbox(L);
			else if (controlclass == "color")
				ctl = new LuaControl::Color(L, false);
			else if (controlclass == "coloralpha")
				ctl = new LuaControl::Color(L, true);
			else if (controlclass == "alpha")
				// FIXME
				ctl = new LuaControl::Edit(L);
			else
				luaL_error(L, "bad control table entry");

			controls.push_back(ctl);
		});

		if (include_buttons && lua_istable(L, 2)) {
			lua_pushvalue(L, 2);
			lua_for_each(L, [&]{
				// String key: key is button ID, value is button label
				// lua_isstring actually checks "is convertible to string"
				if (lua_type(L, -2) == LUA_TSTRING)
					buttons.emplace_back(
						string_to_wx_id(lua_tostring(L, -2)),
						luaL_checkstring(L, -1));

				// Number key, string value: value is label
				else if (lua_isstring(L, -1))
					buttons.emplace_back(-1, lua_tostring(L, -1));

				// Table value: Is a subtable that needs to be flatten.
				// Used for ordered key-value pairs
				else if (lua_istable(L, -1)) {
					lua_pushvalue(L, -1);
					lua_for_each(L, [&]{
						buttons.emplace_back(
							string_to_wx_id(luaL_checkstring(L, -2)),
							luaL_checkstring(L, -1));
					});
				}
				else
					luaL_error(L, "Invalid entry in buttons table");
			});
		}
	}

	LuaDialog::~LuaDialog() {
		delete_clear(controls);
	}

	wxWindow* LuaDialog::CreateWindow(wxWindow *parent) {
		window = new wxPanel(parent);

		auto s = new wxGridBagSizer(4, 4);
		for (auto c : controls)
			s->Add(c->Create(window), wxGBPosition(c->y, c->x),
				wxGBSpan(c->height, c->width), c->GetSizerFlags());

		if (!use_buttons) {
			window->SetSizerAndFit(s);
			return window;
		}

		if (buttons.size() == 0) {
			buttons.emplace_back(wxID_OK, "");
			buttons.emplace_back(wxID_CANCEL, "");
		}

		auto dialog = static_cast<wxDialog *>(parent);
		auto bs = new wxStdDialogButtonSizer;

		auto make_button = [&](wxWindowID id, int button_pushed, wxString const& text) -> wxButton *{
			auto button = new wxButton(window, id, text);
			button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &evt) {
				this->button_pushed = button_pushed;
				dialog->TransferDataFromWindow();
				dialog->EndModal(0);
			});

			if (id == wxID_OK || id == wxID_YES || id == wxID_SAVE) {
				button->SetFocus();
				button->SetDefault();
				dialog->SetAffirmativeId(id);
			}

			if (id == wxID_CLOSE || id == wxID_NO)
				dialog->SetEscapeId(id);

			return button;
		};

		if (boost::count(buttons | boost::adaptors::map_keys, -1) == 0) {
			for (size_t i = 0; i < buttons.size(); ++i)
				bs->AddButton(make_button(buttons[i].first, i, wxEmptyString));
			bs->Realize();
		}
		else {
			for (size_t i = 0; i < buttons.size(); ++i)
				bs->Add(make_button(buttons[i].first, i, buttons[i].second));
		}

		auto ms = new wxBoxSizer(wxVERTICAL);
		ms->Add(s, 0, wxBOTTOM, 5);
		ms->Add(bs);
		window->SetSizerAndFit(ms);

		return window;
	}

	int LuaDialog::LuaReadBack(lua_State *L) {
		// First read back which button was pressed, if any
		if (use_buttons) {
			if (buttons[button_pushed].first == wxID_CANCEL)
				lua_pushboolean(L, false);
			else
				lua_pushstring(L, buttons[button_pushed].second.c_str());
		}

		// Then read controls back
		lua_newtable(L);
		for (auto control : controls) {
			control->LuaReadBack(L);
			lua_setfield(L, -2, control->name.c_str());
		}

		return use_buttons ? 2 : 1;
	}

	std::string LuaDialog::Serialise() {
		std::string res;

		// Format into "name1:value1|name2:value2|name3:value3"
		for (auto control : controls) {
			if (control->CanSerialiseValue()) {
				if (!res.empty())
					res += "|";
				res += inline_string_encode(control->name) + ":" + control->SerialiseValue();
			}
		}

		return res;
	}

	void LuaDialog::Unserialise(const std::string &serialised) {
		boost::char_separator<char> psep("|"), csep(":");
		for (auto const& cur : boost::tokenizer<boost::char_separator<char>>(serialised, psep)) {
			size_t pos = cur.find(':');
			if (pos == std::string::npos) continue;

			std::string name = inline_string_decode(cur.substr(0, pos));
			std::string value = cur.substr(pos + 1);

			// Hand value to all controls matching name
			for (auto control : controls) {
				if (control->name == name && control->CanSerialiseValue())
					control->UnserialiseValue(value);
			}
		}
	}
}
