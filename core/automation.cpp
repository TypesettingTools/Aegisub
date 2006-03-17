// Copyright (c) 2005, Niels Martin Hansen
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
// Contact: mailto:zeratul@cellosoft.com
//


#include <wx/file.h>
#include <wx/gdicmn.h>
#include <wx/dcmemory.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include "automation.h"
#include "ass_file.h"
#include "ass_entry.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "options.h"
#include "string_codec.h"
#include "vfr.h"

#ifdef WIN32
#include <windows.h>
#include <wchar.h>
#else
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

extern "C" {
#include <lualib.h>
#include <lauxlib.h>
}

int L_callfunc(lua_State *L, int nargs, int nresults);

void L_settable(lua_State *L, int table, wxString &key, lua_Number val);
void L_settable(lua_State *L, int table, wxString &key, wxString val);
void L_settable_bool(lua_State *L, int table, wxString &key, bool val);
void L_settable(lua_State *L, int table, const char *key, lua_Number val);
void L_settable(lua_State *L, int table, const char *key, wxString val);
void L_settable_bool(lua_State *L, int table, const char *key, bool val);
void L_settable_kara(lua_State *L, int table, int index, int duration, wxString &kind, wxString &text, wxString &text_stripped);

// these two assume the table to get from is on the top of the stack
lua_Number L_gettableN(lua_State *L, const char *key);
wxString L_gettableS(lua_State *L, const char *key);
bool L_gettableB(lua_State *L, const char *key);


namespace AutomationHelper {

	// helper functions for the scripts
	// expect a pointer to the automation script object to be on the private stack


	/*
		Helper function helper...
		Get the AutomationScript object associated with a Lua state.
	*/
	AutomationScript *GetScriptObject(lua_State *L)
	{
		lua_pushstring(L, "aegisub");
		lua_rawget(L, LUA_REGISTRYINDEX);
		AutomationScript *s = (AutomationScript*)(lua_touserdata(L, -1));
		if (!s) {
			lua_pushstring(L, "Unable to retrieve AutomationScript object from the registry. This should never happen!");
			lua_error(L); // never returns
		}
		lua_pop(L, 1);
		return s;
	}


	/*
		"Debug hook" function, used for checking if the interpreter has been asked to cancel.
		If it has, a Lua error is reported.
	*/
	void hookfunc(lua_State *L, lua_Debug *ar)
	{
		AutomationScript *script = GetScriptObject(L);
		if (script->force_cancel) {
			if (ar->currentline < 0) {
				lua_pushstring(L, "Script forcibly terminated at an unknown line");
			} else {
				lua_pushstring(L, "Script forcibly terminated at line ");
				lua_pushnumber(L, ar->currentline);
				lua_concat(L, 2);
			}
			lua_error(L);
		}
	}


	/*
		function aegisub.output_debug(text)

		Output text to a debug console.

		@text
		  String. The text to output.

		Returns: nothing.
	*/
	int output_debug(lua_State *L)
	{
		AutomationScript *script = GetScriptObject(L);

		// check we were passed a string
		if (lua_gettop(L) < 1) {
			// idiot user (nothing on the stack)
			lua_pushstring(L, "output_debug called with no arguments");
			lua_error(L); // never returns

		} else if (!lua_isstring(L, -1)) {
			// idiot user (didn't pass a string)
			lua_pushstring(L, "output_debug called with non string-compatible argument");
			lua_error(L); // never returns
		}

		script->OutputDebugString(wxString(lua_tostring(L, 1), wxConvUTF8), true);

		return 0;
	}


	/*
		function aegisub.set_status(text)

		Sets the current status-message. (Used for progress-reporting.)

		@text
		  String. The status message.

		Returns: nothing.
	*/
	int set_status(lua_State *L)
	{
		AutomationScript *script = GetScriptObject(L);

		// check we were passed a string
		if (lua_gettop(L) < 1) {
			// idiot user (nothing on the stack)
			lua_pushstring(L, "output_debug called with no arguments");
			lua_error(L); // never returns

		} else if (!lua_isstring(L, -1)) {
			// idiot user (didn't pass a string)
			lua_pushstring(L, "output_debug called with non string-compatible argument");
			lua_error(L); // never returns
		}

		script->OutputDebugString(wxString(lua_tostring(L, 1), wxConvUTF8), false);

		return 0;
	}


	/*
		function aegisub.colorstring_to_rgb(colorstring)

		Convert an ASS color-string to a set of RGB values.

		@colorstring
		  String. The color-string to convert.

		Returns: Four values, all numbers, being the color components in this
		order: Red, Green, Blue, Alpha-channel
	*/
	int colorstring_to_rgb(lua_State *L)
	{
		if (lua_gettop(L) < 1) {
			lua_pushstring(L, "colorstring_to_rgb called without arguments");
			lua_error(L);
		}
		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "colorstring_to_rgb requires a string type argument");
			lua_error(L);
		}

		wxString colorstring(lua_tostring(L, -1), wxConvUTF8);
		lua_pop(L, 1);
		AssColor rgb;
		rgb.ParseASS(colorstring);
		lua_pushnumber(L, rgb.r);
		lua_pushnumber(L, rgb.g);
		lua_pushnumber(L, rgb.b);
		lua_pushnumber(L, rgb.a);
		return 4;
	}


	/*
		function aegisub.report_progress(percent)

		Report the progress of the processing.

		@percent
		  Number. How much of the data have been processed so far.

		Returns: nothing.
	*/
	int report_progress(lua_State *L)
	{
		AutomationScript *script = GetScriptObject(L);

		// check we were passed a string
		if (lua_gettop(L) < 1) {
			// idiot user (nothing on the stack)
			lua_pushstring(L, "report_progress called with no arguments");
			lua_error(L); // never returns

		} else if (!lua_isnumber(L, -1)) {
			// idiot user (didn't pass a string)
			lua_pushstring(L, "report_progress requires a numeric argument");
			lua_error(L); // never returns
		}

		lua_Number p = lua_tonumber(L, -1);

        if (p < 0) p = 0;
		if (p > 100) p = 100;
		p = (p+100)/3;
		script->ReportProgress(p);

		return 0;
	}


	/*
		function aegisub.text_extents(style, text)

		Calculate the on-screen pixel size of the given text using the given style.

		@style
		  Table. A single style definition like those passed to process_lines.
		@text
		  String. The text to calculate the extents for. This should not contain
		  formatting codes, as they will be treated as part of the text.

		Returns 4 values:
		1: Number. Width of the text, in pixels.
		2: Number. Height of the text, in pixels.
		3: Number. Descent of the text, in pixels.
		4: Number. External leading for the text, in pixels.
	*/
	int text_extents(lua_State *L)
	{
		// vars for the result
		int resx = 0, resy = 0, resd = 0, resl = 0;
		// get the input
		// no error checking for the moment
		wxString intext(lua_tostring(L, -1), wxConvUTF8);
		// leave only style table
		lua_settop(L, -2);

		// read out the relevant parts of style
		wxString fontname(L_gettableS(L, "fontname"));
		double fontsize = L_gettableN(L, "fontsize");
		bool bold = L_gettableB(L, "bold");
		bool italic = L_gettableB(L, "italic");
		bool underline = L_gettableB(L, "underline");
		bool strikeout = L_gettableB(L, "strikeout");
		double scale_x = L_gettableN(L, "scale_x");
		double scale_y = L_gettableN(L, "scale_y");
		int spacing = (int)L_gettableN(L, "spacing");
		int charset = (int)L_gettableN(L, "encoding");

		wxLogDebug(_T("text_extents for: %s:%f:%d%d%d%d:%f:%f:%d:%d"), fontname.c_str(), fontsize, bold, italic, underline, strikeout, scale_x, scale_y, spacing, charset);

#ifdef WIN32
		HDC thedc = CreateCompatibleDC(0);
		if (!thedc) return 0;
		SetMapMode(thedc, MM_TEXT);

		HDC dczero = GetDC(0);
		fontsize = -MulDiv((int)(fontsize+0.5), GetDeviceCaps(dczero, LOGPIXELSY), 72);
		ReleaseDC(0, dczero);

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = fontsize;
		lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
		lf.lfItalic = italic;
		lf.lfUnderline = underline;
		lf.lfStrikeOut = strikeout;
		lf.lfCharSet = charset;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
		wcsncpy(lf.lfFaceName, fontname.wc_str(), 32);

		HFONT thefont = CreateFontIndirect(&lf);
		if (!thefont) return 0;
		SelectObject(thedc, thefont);
		
		SIZE sz;
		size_t thetextlen = intext.length();
		const wchar_t *thetext = intext.wc_str();
		if (spacing) {
			resx = 0;
			for (unsigned int i = 0; i < thetextlen; i++) {
				GetTextExtentPoint32(thedc, &thetext[i], 1, &sz);
				resx += sz.cx + spacing;
				resy = sz.cy;
			}
		} else {
			GetTextExtentPoint32(thedc, thetext, thetextlen, &sz);
			resx = sz.cx;
			resy = sz.cy;
		}

		// HACKISH FIX! This seems to work, but why? It shouldn't be needed?!?
		fontsize = L_gettableN(L, "fontsize");
		resx = (int)(resx * fontsize/resy + 0.5);
		resy = (int)(fontsize + 0.5);

		TEXTMETRIC tm;
		GetTextMetrics(thedc, &tm);
		resd = tm.tmDescent;
		resl = tm.tmExternalLeading;

		DeleteObject(thedc);
		DeleteObject(thefont);

#else // not WIN32
		wxMemoryDC thedc;

		// fix fontsize to be 72 DPI
		fontsize = -FT_MulDiv((int)(fontsize+0.5), 72, thedc.GetPPI().y);

		// now try to get a font!
		// use the font list to get some caching... (chance is the script will need the same font very often)
		// USING wxTheFontList SEEMS TO CAUSE BAD LEAKS!
		//wxFont *thefont = wxTheFontList->FindOrCreateFont(
		wxFont thefont(
			fontsize,
			wxFONTFAMILY_DEFAULT,
			italic ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
			bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
			underline,
			fontname,
			wxFONTENCODING_SYSTEM);
		thedc.SetFont(thefont);

		if (spacing) {
			// If there's inter-character spacing, kerning info must not be used, so calculate width per character
			for (unsigned int i = 0; i < intext.length(); i++) {
				int a, b, c, d;
				thedc.GetTextExtent(intext[i], &a, &b, &c, &d);
				resx += a + spacing;
				resy = b > resy ? b : resy;
				resd = c > resd ? c : resd;
				resl = d > resl ? d : resl;
			}
		} else {
			// If the inter-character spacing should be zero, kerning info can (and must) be used, so calculate everything in one go
			thedc.GetTextExtent(intext, &resx, &resy, &resd, &resl);
		}
#endif

		// Compensate for scaling
		resx = (int)(scale_x / 100 * resx + 0.5);
		resy = (int)(scale_y / 100 * resy + 0.5);
		resd = (int)(scale_y / 100 * resd + 0.5);
		resl = (int)(scale_y / 100 * resl + 0.5);

		lua_pushnumber(L, resx);
		lua_pushnumber(L, resy);
		lua_pushnumber(L, resd);
		lua_pushnumber(L, resl);
		return 4;
	}


	/*
		function aegisub.frame_from_ms(ms)

		Return the video frame-number for the given time.

		@ms
		  Number. Time in miliseconds to get the frame number for.

		Returns: A number, the frame numer. If there is no framerate data, returns
		nil.
	*/
	int frame_from_ms(lua_State *L)
	{
		int ms = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		if (VFR_Output.IsLoaded()) {
			lua_pushnumber(L, VFR_Output.GetFrameAtTime(ms, true));
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}


	/*
		function aegisub.ms_from_frame(frame)

		Returns the start-time for the given video frame-number.

		@frame
		  Number. Frame-number to get start-time from.

		Returns: A number, the start-time of the frame. If there is no framerate
		data, returns nil.
	*/
	int ms_from_frame(lua_State *L)
	{
		int frame = (int)lua_tonumber(L, -1);
		lua_pop(L, 1);
		if (VFR_Output.IsLoaded()) {
			lua_pushnumber(L, VFR_Output.GetTimeAtFrame(frame, true));
			return 1;
		} else {
			lua_pushnil(L);
			return 1;
		}
	}


	/*
		function include(filename)

		@filename
		  String. Name of the file to include.
  
		Returns: Depends on the script included.
	*/
	int include(lua_State *L)
	{
		AutomationScript *script = GetScriptObject(L);

		if (!lua_isstring(L, 1)) {
			lua_pushstring(L, "First argument to the include function must be a string.");
			lua_error(L);
		}
		wxString fnames(lua_tostring(L, 1), wxConvUTF8);

		wxFileName fname(fnames);
		if (fname.GetDirCount() == 0) {
			// filename only
			fname = script->include_path.FindAbsoluteValidPath(fnames);
		} else if (fname.IsRelative()) {
			// relative path
			wxFileName sfname(script->filename);
			fname.MakeAbsolute(sfname.GetPath(true));
		} else {
			// absolute path, invalid
			lua_pushstring(L, "Filename passed to include seems to have an absolute path, which is not allowed.");
			lua_error(L);
		}
		if (!fname.IsOk() || !fname.FileExists()) {
			{
				// need to make a new scope here, so the char buffer can go out of scope before lua_error() makes a longjmp
				wxCharBuffer errmsg = wxString::Format(_T("The file could not be included, not found. \"%s\""), fnames.c_str()).mb_str(wxConvUTF8);
				lua_pushstring(L, errmsg.data());
			}
			lua_error(L);
		}

		AutomationScriptFile *sfile;
		sfile = AutomationScriptFile::CreateFromFile(fname.GetFullPath());
		wxCharBuffer fnamebuf = fname.GetFullName().mb_str(wxConvUTF8);
		switch (luaL_loadbuffer(L, sfile->scriptdata, sfile->scriptlen, fnamebuf.data())) {
			// FIXME: these should be made into lua_error() things instead... probably
			case 0:
				// success!
				break;
			case LUA_ERRSYNTAX:
				throw AutomationError(wxString::Format(_T("Lua syntax error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
			case LUA_ERRMEM:
				throw AutomationError(wxString::Format(_T("Lua memory allocation error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
			default:
				throw AutomationError(wxString::Format(_T("Lua unknown error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
		}
		delete sfile;

		// top of stack before the call (correct for the function itself being on stack)
		int pretop = lua_gettop(L)-1;
		// call the loaded script
		lua_call(L, 0, LUA_MULTRET);
		// calculate the number of results the script produced
		return lua_gettop(L)-pretop;
	}

}


/*
	Call a Lua function without risking killing the entire program
	Just throw a C++ exception instead :)

	Really just a thin wrapper around lua_pcall
*/
inline int L_callfunc(lua_State *L, int nargs, int nresults)
{
	int res = lua_pcall(L, nargs, nresults, 0);
	switch (res) {
		case LUA_ERRRUN:
			throw AutomationError(wxString::Format(_T("Lua runtime error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
		case LUA_ERRMEM:
			throw AutomationError(wxString::Format(_T("Lua memory allocation error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
		case LUA_ERRERR:
			// shouldn't happen as an error handling function isn't being used
			throw AutomationError(wxString::Format(_T("Lua error calling error handler:  %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
		default:
			// success!
			return res;
	}
}


inline void L_settable(lua_State *L, int table, wxString &key, lua_Number val)
{
	L_settable(L, table, key.mb_str(wxConvUTF8), val);
}

inline void L_settable(lua_State *L, int table, wxString &key, wxString val)
{
	//wxLogMessage(_T("Wrapping adding of string at index '%s': %s"), key, val);
	L_settable(L, table, key.mb_str(wxConvUTF8), val);
}

inline void L_settable_bool(lua_State *L, int table, wxString &key, bool val)
{
	L_settable_bool(L, table, key.mb_str(wxConvUTF8), val);
}

inline void L_settable(lua_State *L, int table, const char *key, lua_Number val)
{
	lua_pushstring(L, key);
	lua_pushnumber(L, val);
	if (table > 0 || table < -100) {
	  lua_settable(L, table);
	} else {
		lua_settable(L, table-2);
	}
}

inline void L_settable(lua_State *L, int table, const char *key, wxString val)
{
	//wxLogMessage(_T("Adding string at index '%s': %s"), wxString(key, wxConvUTF8), val);
	lua_pushstring(L, key);
	lua_pushstring(L, val.mb_str(wxConvUTF8));
	if (table > 0 || table < -100) {
	  lua_settable(L, table);
	} else {
		lua_settable(L, table-2);
	}
}

inline void L_settable_bool(lua_State *L, int table, const char *key, bool val)
{
	lua_pushstring(L, key);
	lua_pushboolean(L, val?1:0);
	if (table > 0 || table < -100) {
	  lua_settable(L, table);
	} else {
		lua_settable(L, table-2);
	}
}

inline void L_settable_kara(lua_State *L, int table, int index, int duration, wxString &kind, wxString &text, wxString &text_stripped)
{
	lua_newtable(L);
	L_settable(L, -1, "duration", duration);
	L_settable(L, -1, "kind", kind);
	L_settable(L, -1, "text", text);
	L_settable(L, -1, "text_stripped", text_stripped);
	if (table > 0 || table < -100) {
		lua_rawseti(L, table, index);
	} else {
		lua_rawseti(L, table-1, index);
	}
}


lua_Number L_gettableN(lua_State *L, const char *key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	lua_Number res = lua_tonumber(L, -1);
	lua_settop(L, -2);
	return res;
}


wxString L_gettableS(lua_State *L, const char *key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	wxString res(lua_tostring(L, -1), wxConvUTF8);
	lua_settop(L, -2);
	return res;
}


bool L_gettableB(lua_State *L, const char *key)
{
	lua_pushstring(L, key);
	lua_gettable(L, -2);
	bool res = lua_toboolean(L, -1) != 0;
	lua_settop(L, -2);
	return res;
}



AutomationError::AutomationError(wxString msg)
: message(msg)
{
	// nothing to do here...
}



wxString AutomationScriptConfiguration::serialize()
{
	wxString result;
	for (std::vector<AutomationScriptConfigurationOption>::iterator opt = options.begin(); opt != options.end(); opt++) {
		switch (opt->kind) {
			case COK_TEXT:
			case COK_STYLE:
				result << wxString::Format(_T("%s:%s|"), opt->name.c_str(), inline_string_encode(opt->value.stringval).c_str());
				break;
			case COK_INT:
				result << wxString::Format(_T("%s:%d|"), opt->name.c_str(), opt->value.intval);
				break;
			case COK_FLOAT:
				result << wxString::Format(_T("%s:%e|"), opt->name.c_str(), opt->value.floatval);
				break;
			case COK_BOOL:
				result << wxString::Format(_T("%s:%d|"), opt->name.c_str(), opt->value.boolval?1:0);
				break;
			case COK_COLOUR:
				result << wxString::Format(_T("%s:%s|"), opt->name.c_str(), opt->value.colourval.GetASSFormatted(false).c_str());
				break;
			default:
				// The rest aren't stored
				break;
		}
	}
	if (result.Last() == _T('|'))
		result.RemoveLast();
	return result;
}


void AutomationScriptConfiguration::unserialize(wxString &settings)
{
	//wxLogMessage(_T("Unserializing config string: %s"), settings);
	wxStringTokenizer toker(settings, _T("|"), wxTOKEN_STRTOK);
	while (toker.HasMoreTokens()) {
		// get the parts of this setting
		wxString setting = toker.GetNextToken();
		//wxLogMessage(_T("Got token: %s"), setting);
		wxString optname = setting.BeforeFirst(_T(':'));
		wxString optval = setting.AfterFirst(_T(':'));
		//wxLogMessage(_T("Split into: \"%s\" and \"%s\""), optname, optval);
		// find the setting in the list loaded from the script
		std::vector<AutomationScriptConfigurationOption>::iterator opt = options.begin();
		while (opt != options.end() && opt->name != optname)
			opt ++;
		if (opt != options.end()) {
			//wxLogMessage(_T("Found the option!"));
			// ok, found the option!
			switch (opt->kind) {
				case COK_TEXT:
				case COK_STYLE:
					opt->value.stringval = inline_string_decode(optval);
					//wxLogMessage(_T("Decoded string to: %s"), opt->value.stringval);
					break;
				case COK_INT:
					{
						long n;
						optval.ToLong(&n, 10);
						opt->value.intval = n;
					}
					break;
				case COK_FLOAT:
					optval.ToDouble(&opt->value.floatval);
					break;
				case COK_BOOL:
					opt->value.boolval = optval == _T("1");
					break;
				case COK_COLOUR:
					opt->value.colourval.ParseASS(optval);
					break;
			}
		}
	}
}


void AutomationScriptConfiguration::load_from_lua(lua_State *L)
{
	//wxLogMessage(_T("Loading configuration options from script"));
	present = false;
	if (!lua_istable(L, -1)) {
		return;
	}
	//wxLogMessage(_T("The script does have config options, good!"));
	int i = 1;
	while (true) {
		// get an element from the array
		//wxLogMessage(_T("Getting option %d (stacktop is %d)"), i, lua_gettop(L));
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		// check if it was a table
		if (!lua_istable(L, -1)) {
			//wxLogMessage(_T("Damn! Not an option... breaking out (actual type was %d)"), lua_type(L, -1));
			lua_pop(L, 1);
			break;
		}
		//wxLogMessage(_T("Yay! It was an option, adding another blank option thing to the list"));
		// add a new config option and fill it
		{
			AutomationScriptConfigurationOption opt;
			options.push_back(opt);
		}
		AutomationScriptConfigurationOption &opt = options.back();
		// get the "kind"
		lua_pushstring(L, "kind");
		lua_gettable(L, -2);
		if (lua_isstring(L, -1)) {
			// use C standard lib functions here, as it's probably faster than messing around with unicode
			// lua is known to always properly null-terminate strings, and the strings are known to be pure ascii
			const char *kind = lua_tostring(L, -1);
			if (strcmp(kind, "label") == 0) {
				opt.kind = COK_LABEL;
			} else if (strcmp(kind, "text") == 0) {
				opt.kind = COK_TEXT;
			} else if (strcmp(kind, "int") == 0) {
				opt.kind = COK_INT;
			} else if (strcmp(kind, "float") == 0) {
				opt.kind = COK_FLOAT;
			} else if (strcmp(kind, "bool") == 0) {
				opt.kind = COK_BOOL;
			} else if (strcmp(kind, "colour") == 0) {
				opt.kind = COK_COLOUR;
			} else if (strcmp(kind, "style") == 0) {
				opt.kind = COK_STYLE;
			} else {
				opt.kind = COK_INVALID;
			}
		} else {
			opt.kind = COK_INVALID;
		}
		//wxLogMessage(_T("Got kind: %d"), opt.kind);
		// remove "kind" string from stack again
		lua_pop(L, 1);
		// no need to check for rest if this one is already deemed invalid
		if (opt.kind != COK_INVALID) {
			// name
			lua_pushstring(L, "name");
			lua_gettable(L, -2);
			if (lua_isstring(L, -1)) {
				opt.name = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			} else {
				lua_pop(L, 1);
				// no name means invalid option
				opt.kind = COK_INVALID;
				goto continue_invalid_option;
			}
			//wxLogMessage(_T("Got name: %s"), opt.name);
			// label
			lua_pushstring(L, "label");
			lua_gettable(L, -2);
			if (lua_isstring(L, -1)) {
				opt.label = wxString(lua_tostring(L, -1), wxConvUTF8);
				lua_pop(L, 1);
			} else {
				lua_pop(L, 1);
				// label is also required
				opt.kind = COK_INVALID;
				goto continue_invalid_option;
			}
			assert(opt.kind != COK_INVALID);
			// hint
			lua_pushstring(L, "hint");
			lua_gettable(L, -2);
			if (lua_isstring(L, -1)) {
				opt.hint = wxString(lua_tostring(L, -1), wxConvUTF8);
			} else {
				opt.hint = _T("");
			}
			lua_pop(L, 1);
			// min
			lua_pushstring(L, "min");
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1)) {
				opt.min.isset = true;
				opt.min.floatval = lua_tonumber(L, -1);
				opt.min.intval = (int)opt.min.floatval;
			} else {
				opt.min.isset = false;
			}
			lua_pop(L, 1);
			// max
			lua_pushstring(L, "max");
			lua_gettable(L, -2);
			if (lua_isnumber(L, -1)) {
				opt.max.isset = true;
				opt.max.floatval = lua_tonumber(L, -1);
				opt.max.intval = (int)opt.max.floatval;
			} else {
				opt.max.isset = false;
			}
			lua_pop(L, 1);
			// default (this is going to kill me)
			lua_pushstring(L, "default");
			lua_gettable(L, -2);
			switch (opt.kind) {
				case COK_LABEL:
					// nothing to do, nothing expected
					break;
				case COK_TEXT:
				case COK_STYLE:
					// expect it to be a string
					if (lua_isstring(L, -1)) {
						opt.default_val.stringval = wxString(lua_tostring(L, -1), wxConvUTF8);
					} else {
						// not a string, baaaad scripter
						opt.kind = COK_INVALID;
					}
					break;
				case COK_INT:
				case COK_FLOAT:
					// expect it to be a number
					if (lua_isnumber(L, -1)) {
						opt.default_val.floatval = lua_tonumber(L, -1);
						opt.default_val.intval = (int)opt.default_val.floatval;
					} else {
						opt.kind = COK_INVALID;
					}
					break;
				case COK_BOOL:
					// expect it to be a bool
					if (lua_isboolean(L, -1)) {
						opt.default_val.boolval = lua_toboolean(L, -1)!=0;
					} else {
						opt.kind = COK_INVALID;
					}
					break;
				case COK_COLOUR:
					// expect it to be a ass hex colour formatted string
					if (lua_isstring(L, -1)) {
						opt.default_val.stringval = wxString(lua_tostring(L, -1), wxConvUTF8);
						opt.default_val.colourval.ParseASS(opt.default_val.stringval); // and hope this goes well!
					} else {
						opt.kind = COK_INVALID;
					}
					break;
			}
			opt.value = opt.default_val;
			lua_pop(L, 1);
		}
		// so we successfully got an option added, so at least there is a configuration present now
		present = true;
continue_invalid_option:
		// clean up and prepare for next iteration
		lua_pop(L, 1);
		i++;
	}
}


void AutomationScriptConfiguration::store_to_lua(lua_State *L)
{
	// we'll always need a new table, no matter what
	lua_newtable(L);
	//wxLogMessage(_T("Created table for configuration data (top=%d)"), lua_gettop(L));

	for (std::vector<AutomationScriptConfigurationOption>::iterator opt = options.begin(); opt != options.end(); opt++) {
		//wxLogMessage(_T("Storing option named '%s' (top=%d)"), opt->name, lua_gettop(L));
		switch (opt->kind) {
			case COK_INVALID:
			case COK_LABEL:
				//wxLogMessage(_T("Nothing to store"));
				break;

			case COK_TEXT:
			case COK_STYLE:
				//wxLogMessage(_T("Storing string value"));
				L_settable(L, -1, opt->name, opt->value.stringval);
				break;

			case COK_INT:
				//wxLogMessage(_T("Storing int value"));
				L_settable(L, -1, opt->name, opt->value.intval);
				break;

			case COK_FLOAT:
				//wxLogMessage(_T("Storing float value"));
				L_settable(L, -1, opt->name, opt->value.floatval);
				break;

			case COK_BOOL:
				//wxLogMessage(_T("Storing bool value"));
				L_settable_bool(L, -1, opt->name, opt->value.boolval);
				break;

			case COK_COLOUR:
				//wxLogMessage(_T("Storing colourvalue"));
				L_settable(L, -1, opt->name, opt->value.colourval.GetASSFormatted(false, false));
				break;

			default:
				//wxLogMessage(_T("Felt into default handler?!?"));
				break;
		}
	}
	//wxLogMessage(_T("Finished storing configuration data (top=%d)"), lua_gettop(L));
}



AutomationScript::AutomationScript(AutomationScriptFile *script)
{
	force_cancel = false;
	filename = script->filename;
	progress_reporter = 0;
	debug_reporter = 0;

	// get a lua object
	L = lua_open();
	// put a pointer to this object in the registry index
	lua_pushstring(L, "aegisub");
	lua_pushlightuserdata(L, this);
	lua_rawset(L, LUA_REGISTRYINDEX);

	// set up the cancelling hook, call every 100 instructions
	lua_sethook(L, AutomationHelper::hookfunc, LUA_MASKCOUNT, 100);

	// provide some standard libraries
	luaopen_base(L);
	luaopen_string(L);
	luaopen_table(L);
	luaopen_math(L);
#ifdef _DEBUG
	luaopen_debug(L);
#endif
	// but no I/O, OS or debug facilities, those aren't safe

	// disable the dofile() function
	lua_pushstring(L, "dofile");
	lua_pushnil(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	// create an include function better suited aegisub later
	// the path object is needed for this
	include_path.EnsureFileAccessible(script->filename);
	{
		wxStringTokenizer toker(Options.AsText(_T("Automation Include Path")), _T("|"), false);
		while (toker.HasMoreTokens()) {
			wxFileName path(toker.GetNextToken());
			if (!path.IsOk()) continue;
			if (path.IsRelative()) continue;
			if (!path.DirExists()) continue;
			if (include_path.Member(path.GetLongPath())) continue;
			include_path.Add(path.GetLongPath());
		}
	}
	// add the include function to the global environment
	lua_pushstring(L, "include");
	lua_pushcfunction(L, AutomationHelper::include);
	lua_settable(L, LUA_GLOBALSINDEX);

	// create the "aegisub" table and fill it with some function pointers
	// create the table and add it to the global environment
	lua_pushstring(L, "aegisub");
	lua_newtable(L);
	lua_settable(L, LUA_GLOBALSINDEX);
	// get it back onto the stack
	lua_pushstring(L, "aegisub");
	lua_gettable(L, LUA_GLOBALSINDEX);
	// get the index of the table on the stack
	int tabid = lua_gettop(L);
	// now register some functions!
	lua_pushstring(L, "output_debug");
	lua_pushcfunction(L, AutomationHelper::output_debug);
	lua_settable(L, tabid);
	lua_pushstring(L, "set_status");
	lua_pushcfunction(L, AutomationHelper::set_status);
	lua_settable(L, tabid);
	lua_pushstring(L, "report_progress");
	lua_pushcfunction(L, AutomationHelper::report_progress);
	lua_settable(L, tabid);
	lua_pushstring(L, "colorstring_to_rgb");
	lua_pushcfunction(L, AutomationHelper::colorstring_to_rgb);
	lua_settable(L, tabid);
	lua_pushstring(L, "text_extents");
	lua_pushcfunction(L, AutomationHelper::text_extents);
	lua_settable(L, tabid);
	lua_pushstring(L, "frame_from_ms");
	lua_pushcfunction(L, AutomationHelper::frame_from_ms);
	lua_settable(L, tabid);
	lua_pushstring(L, "ms_from_frame");
	lua_pushcfunction(L, AutomationHelper::ms_from_frame);
	lua_settable(L, tabid);
	// and no more need for that tabid
	lua_settop(L, tabid-1);

	// ok, finally the environment is set up!
	// now try to load the script
	{
		switch (luaL_loadbuffer(L, script->scriptdata, script->scriptlen, "user script")) {
			case 0:
				// success!
				break;
			case LUA_ERRSYNTAX:
				throw AutomationError(wxString::Format(_T("Lua syntax error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
			case LUA_ERRMEM:
				throw AutomationError(wxString::Format(_T("Lua memory allocation error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
			default:
				throw AutomationError(wxString::Format(_T("Lua unknown error: %s"), wxString(lua_tostring(L, -1), wxConvUTF8).c_str()));
				break;
		}
	}
	// it's loaded and is now a function at the top of the stack
	// it doesn't take any arguments and doesn't return anything
	// so let's try executing it by calling!
	L_callfunc(L, 0, 0);

	// so, the script should be loaded
	// now try to get the script data!
	// first the version
	lua_pushstring(L, "version");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_isnumber(L, -1)) {
		throw AutomationError(wxString(_T("Script error: 'version' value not found or not a number")));
	}
	version = lua_tonumber(L, -1);
	lua_settop(L, -2);
	if (version < 3 || version > 4) {
		// invalid version
		throw AutomationError(wxString(_T("Script error: 'version' value invalid for this version of Automation")));
	}
	// kind
	lua_pushstring(L, "kind");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_isstring(L, -1)) {
		throw AutomationError(wxString(_T("Script error: 'kind' value not found or not a string")));
	}
	kind = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_settop(L, -2);
	// name
	lua_pushstring(L, "name");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_isstring(L, -1)) {
		throw AutomationError(wxString(_T("Script error: 'name' value not found or not a string")));
	}
	name = wxString(lua_tostring(L, -1), wxConvUTF8);
	lua_settop(L, -2);
	// description (optional)
	lua_pushstring(L, "description");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (lua_isstring(L, -1)) {
		description = wxString(lua_tostring(L, -1), wxConvUTF8);
		lua_settop(L, -2);
	} else {
		description = _T("");
	}
	// process_lines (just check if it's there, no need to save it anywhere)
	lua_pushstring(L, "process_lines");
	lua_gettable(L, LUA_GLOBALSINDEX);
	if (!lua_isfunction(L, -1)) {
		throw AutomationError(wxString(_T("Script error: No 'process_lines' function provided")));
	}
	lua_settop(L, -2);
	// configuration (let the config object do all the loading)
	lua_pushstring(L, "configuration");
	lua_gettable(L, LUA_GLOBALSINDEX);
	//wxLogMessage(_T("Calling configuration.load_from_lua()"));
	configuration.load_from_lua(L);
	lua_settop(L, -2);

	// done!
}


AutomationScript::~AutomationScript()
{
	lua_close(L);
}


void AutomationScript::OutputDebugString(wxString str, bool isdebug)
{
	//wxLogMessage(_T("automation message: ") + str);
	if (debug_reporter) {
		debug_reporter(str, isdebug, this, debug_target);
	}
	return;
}

void AutomationScript::ReportProgress(float progress)
{
	//wxLogMessage(wxString::Format(_T("automation progress: %.1f%%"), progress));
	if (progress_reporter) {
		progress_reporter(progress, this, progress_target);
	}
	return;
}


int AutomationScript::L_panicfunc(lua_State *L)
{
	wxLogError(_T("Lua produced an error. Attempting to recover."));
	longjmp(AutomationHelper::GetScriptObject(L)->panicjmp, lua_gettop(L));
}


void AutomationScript::process_lines(AssFile *input)
{
	// prepare for panic...
	if (int ret = setjmp(panicjmp)) {
		wxLogError(wxString::Format(_T("Returned out of Lua environment. Size of stack before: %d"), ret));
#ifdef _DEBUG
		wxLogError(
#else
		wxLogFatalError(
#endif
			_T("Due to an internal error in the Lua engine, the internal state of Aegisub might have become inconsistent ")
			_T("and cannot continue. If you can reproduce this error, please report it to the developers."));
		lua_close(L);
		return;
	} else {
		lua_atpanic(L, AutomationScript::L_panicfunc);
	}

	// start by generating lua representations of the data...
	// maybe it's safest to start by making plenty of space on the stack
	if (!lua_checkstack(L, 100)) {
		throw AutomationError(wxString(_T("Lua error: Unable to allocate stack space")));
	}

	OutputDebugString(wxString(_T("Preparing subtitle data")));

	// first put the function itself on the stack
	lua_pushstring(L, "process_lines");
	lua_gettable(L, LUA_GLOBALSINDEX);

	// now put the three arguments on the stack

	// first argument: the metadata table
	lua_newtable(L);
	L_settable(L, -1, "res_x", input->GetScriptInfoAsInt(_T("PlayResX")));
	L_settable(L, -1, "res_y", input->GetScriptInfoAsInt(_T("PlayResY")));

	// second and third arguments: styles and events tables
	lua_newtable(L);
	int styletab = lua_gettop(L);
	lua_newtable(L);
	int eventtab = lua_gettop(L);

	int numstyles = 0, numevents = 0;
	
	// fill the styles and events tables
	int processed_lines = 1;
	for (std::list<AssEntry*>::iterator i = input->Line.begin(); i != input->Line.end(); i++, processed_lines++) {

		AssEntry *e = *i;

		if (!e->Valid) continue;

		if (e->GetType() == ENTRY_STYLE) {

			AssStyle *style = e->GetAsStyle(e);

			// gonna need a table to put the style data into
			lua_newtable(L);
			// put the table into index N in the style table
			lua_pushvalue(L, -1);
			lua_rawseti(L, styletab, numstyles);
			// and put it into its named index
			lua_pushstring(L, style->name.mb_str(wxConvUTF8));
			lua_pushvalue(L, -2);
			lua_settable(L, styletab);

			// so now the table is regged and stuff, put some data into it
			L_settable     (L, -1, "name",        style->name);
			L_settable     (L, -1, "fontname",    style->font);
			L_settable     (L, -1, "fontsize",    style->fontsize);
			L_settable     (L, -1, "color1",      style->primary.GetASSFormatted(true, true));
			L_settable     (L, -1, "color2",      style->secondary.GetASSFormatted(true, true));
			L_settable     (L, -1, "color3",      style->outline.GetASSFormatted(true, true));
			L_settable     (L, -1, "color4",      style->shadow.GetASSFormatted(true, true));
			L_settable_bool(L, -1, "bold",        style->bold);
			L_settable_bool(L, -1, "italic",      style->italic);
			L_settable_bool(L, -1, "underline",   style->underline);
			L_settable_bool(L, -1, "strikeout",   style->strikeout);
			L_settable     (L, -1, "scale_x",     style->scalex);
			L_settable     (L, -1, "scale_y",     style->scaley);
			L_settable     (L, -1, "spacing",     style->spacing);
			L_settable     (L, -1, "angle",       style->angle);
			L_settable     (L, -1, "borderstyle", style->borderstyle);
			L_settable     (L, -1, "outline",     style->outline_w);
			L_settable     (L, -1, "shadow",      style->shadow_w);
			L_settable     (L, -1, "align",       style->alignment);
			L_settable     (L, -1, "margin_l",    style->MarginL);
			L_settable     (L, -1, "margin_r",    style->MarginR);
			L_settable     (L, -1, "margin_v",    style->MarginV);
			L_settable     (L, -1, "encoding",    style->encoding);

			// and get that table off the stack again
			lua_settop(L, -2);

			numstyles++;

		} else if (e->group == _T("[Events]")) {
				
			if (e->GetType() != ENTRY_DIALOGUE) {

				// not a dialogue/comment event

				// start checking for a blank line
				wxString entryData = e->GetEntryData();
				if (entryData.IsEmpty()) {
					lua_newtable(L);
					L_settable(L, -1, "kind", wxString(_T("blank")));
				} else if (entryData[0] == _T(';')) {
					// semicolon comment
					lua_newtable(L);
					L_settable(L, -1, "kind", wxString(_T("scomment")));
					L_settable(L, -1, "text", entryData.Mid(1));
				} else {
					// not a blank line and not a semicolon comment
					// just skip...
					continue;
				}

			} else {

				// ok, so it is a dialogue/comment event
				// massive handling :(

				lua_newtable(L);

				assert(e->GetType() == ENTRY_DIALOGUE);

				AssDialogue *dia = e->GetAsDialogue(e);

				// kind of line
				if (dia->Comment) {
					L_settable(L, -1, "kind", wxString(_T("comment")));
				} else {
					L_settable(L, -1, "kind", wxString(_T("dialogue")));
				}

				L_settable(L, -1, "layer", dia->Layer);
				L_settable(L, -1, "start_time", dia->Start.GetMS()/10);
				L_settable(L, -1, "end_time", dia->End.GetMS()/10);
				L_settable(L, -1, "style", dia->Style);
				L_settable(L, -1, "name", dia->Actor);
				L_settable(L, -1, "margin_l", dia->MarginL);
				L_settable(L, -1, "margin_r", dia->MarginR);
				L_settable(L, -1, "margin_v", dia->MarginV);
				L_settable(L, -1, "effect", dia->Effect);
				L_settable(L, -1, "text", dia->Text);

				// so that's the easy part
				// now for the stripped text and *ugh* the karaoke!

				// prepare for stripped text
				wxString text_stripped = _T("");
				L_settable(L, -1, "text_stripped", 0); // dummy item
				// prepare karaoke table
				lua_newtable(L);
				lua_pushstring(L, "karaoke");
				lua_pushvalue(L, -2);
				lua_settable(L, -4);
				// now the top of the stack is the karaoke table, and it's present in the dialogue table

				int kcount = 0;
				int kdur = 0;
				wxString kkind = _T("");
				wxString ktext = _T("");
				wxString ktext_stripped = _T("");

				dia->ParseASSTags();
				for (std::vector<AssDialogueBlock*>::iterator block = dia->Blocks.begin(); block != dia->Blocks.end(); block++) {

					switch ((*block)->type) {

						case BLOCK_BASE:
							throw wxString(_T("BLOCK_BASE found processing dialogue blocks. This should never happen."));

						case BLOCK_PLAIN:
							ktext += (*block)->text;
							ktext_stripped += (*block)->text;
							text_stripped += (*block)->text;
							break;

						case BLOCK_DRAWING:
							ktext += (*block)->text;
							break;

						case BLOCK_OVERRIDE: {
							bool brackets_open = false;
							std::vector<AssOverrideTag*> &tags = (*block)->GetAsOverride(*block)->Tags;

							for (std::vector<AssOverrideTag*>::iterator tag = tags.begin(); tag != tags.end(); tag++) {

								if (!(*tag)->Name.Mid(0,2).CmpNoCase(_T("\\k")) && (*tag)->IsValid()) {

									// it's a karaoke tag
									if (brackets_open) {
										ktext += _T("}");
										brackets_open = false;
									}
									L_settable_kara(L, -1, kcount, kdur, kkind, ktext, ktext_stripped);
									kcount++;
									kdur = (*tag)->Params[0]->AsInt(); // no error checking; this should always be int
									kkind = (*tag)->Name.Mid(1);
									ktext = _T("");
									ktext_stripped = _T("");

								} else {

									// it's something else
									// don't care if it's a valid tag or not
									if (!brackets_open) {
										ktext += _T("{");
										brackets_open = true;
									}
									ktext += (*tag)->ToString();

								}

							}

							if (brackets_open) {
								ktext += _T("}");
							}

							break;}

					}

				}
				dia->ClearBlocks();

				// add the final karaoke block to the table
				// (even if there's no karaoke in the line, there's always at least one karaoke block)
				// even if the line ends in {\k10} with no text after, an empty block should still be inserted
				// (otherwise data are lost)
				L_settable_kara(L, -1, kcount, kdur, kkind, ktext, ktext_stripped);
				kcount++;
				L_settable(L, -1, "n", kcount); // number of syllables in the karaoke
				lua_settop(L, -2); // remove karaoke table from the stack again
				L_settable(L, -1, "text_stripped", text_stripped); // store the real stripped text

			}

			// now the entry table has been created and placed on top of the stack
			// now all that's missing it to insert it into the event table
			lua_rawseti(L, eventtab, numevents);
			numevents++;

		} else {
			// not really a line type automation needs to take care of... ignore it
		}

		ReportProgress(100.0f * processed_lines / input->Line.size() / 3);

	}

	// finally add the counter elements to the styles and events tables
	lua_pushnumber(L, numstyles);
	lua_rawseti(L, styletab, -1);
	L_settable(L, eventtab, "n", numevents);
	// and let the config object create a table for the @config argument
	//wxLogMessage(_T("Calling configuration.store_to_lua()"));
	configuration.store_to_lua(L);

	// so now the metadata, styles and events tables are filled with data
	// ready to call the processing function!

	OutputDebugString(wxString(_T("Running script for processing")));
	ReportProgress(100.0f/3);
	L_callfunc(L, 4, 1);
	ReportProgress(200.0f/3);
	OutputDebugString(wxString(_T("Reading back data from script")));

	// phew, survived the call =)
	// time to read back the results

	wxLogDebug(_T("Returned from Lua script call"));

	if (!lua_istable(L, -1)) {
		throw AutomationError(wxString(_T("The script function did not return a table as expected. Unable to process results. (Nothing was changed.)")));
	}

	// but start by removing all events
	{
		std::list<AssEntry*>::iterator cur, next;
		next = input->Line.begin();
		while (next != input->Line.end()) {
			cur = next++;
			if ((*cur)->group == _T("[Events]")) {
				wxString temp = (*cur)->GetEntryData();
				if (temp == _T("[Events]")) {
					// skip the section header
					continue;
				}
				if ((*cur)->GetType() != ENTRY_DIALOGUE && temp.Mid(0,1) != _T(";") && temp.Trim() != _T("")) {
					// skip non-dialogue non-semicolon comment lines (such as Format)
					continue;
				}
				delete (*cur);
				input->Line.erase(cur);
			}
		}
	}

	wxLogDebug(_T("Finished removing old events from subtitles"));

	// so anyway, there is a single table on the stack now
	// that table contains a lot of events...
	// and it ought to contain an "n" key as well, telling how many events
	// but be lenient, and don't expect one to be there, but rather count from zero and let it be nil-terminated
	// if the "n" key is there, use it as a progress indicator hint, though
	int output_line_count;
	lua_pushstring(L, "n");
	lua_gettable(L, -2);
	if (lua_isnumber(L, -1)) {
		output_line_count = (int) lua_tonumber(L, -1);
	} else {
		// assume number of output lines == number of input lines
		output_line_count = processed_lines;
	}
	lua_settop(L, -2);

	wxLogDebug(_T("Retrieved number of lines in result: %d"), output_line_count);

	// loop through the stack and report back the type of each element
	wxLogDebug(_T("Size of Lua stack: %d"), lua_gettop(L));
	for (int si = lua_gettop(L); si > 0; si--) {
		wxString type = wxString(lua_typename(L, lua_type(L, si)), wxConvUTF8);
		wxLogDebug(_T("Stack index %d, type %s"), si, type.c_str());
	}
	wxLogDebug(_T("Stack dump finished"));

	int outline = 0;
	int faketime = input->Line.back()->StartMS;

	// If there's nothing at index 0, start at index 1 instead, to support both zero and one based indexing
	lua_pushnumber(L, outline);
	lua_gettable(L, -2);
	if (!lua_istable(L, -1)) {
		outline++;
		output_line_count++;
	}
	lua_pop(L, 1);

	while (lua_pushnumber(L, outline), lua_gettable(L, -2), lua_istable(L, -1)) {
		// top of the stack is a table, hopefully with an AssEntry in it

		wxLogDebug(_T("Processing output line %d"), outline);

		// start by getting the kind
		lua_pushstring(L, "kind");
		lua_gettable(L, -2);
		if (!lua_isstring(L, -1)) {
			OutputDebugString(wxString::Format(_T("The output data at index %d is mising a valid 'kind' field, and has been skipped"), outline));
			lua_settop(L, -2);
		} else {

			wxString kind = wxString(lua_tostring(L, -1), wxConvUTF8).Lower();
			// remove "kind" from stack again
			lua_settop(L, -2);
			wxLogDebug(_T("Kind of line: %s"), kind.c_str());

			if (kind == _T("dialogue") || kind == _T("comment")) {

				lua_pushstring(L, "layer");
				lua_gettable(L, -2);
				lua_pushstring(L, "start_time");
				lua_gettable(L, -3);
				lua_pushstring(L, "end_time");
				lua_gettable(L, -4);
				lua_pushstring(L, "style");
				lua_gettable(L, -5);
				lua_pushstring(L, "name");
				lua_gettable(L, -6);
				lua_pushstring(L, "margin_l");
				lua_gettable(L, -7);
				lua_pushstring(L, "margin_r");
				lua_gettable(L, -8);
				lua_pushstring(L, "margin_v");
				lua_gettable(L, -9);
				lua_pushstring(L, "effect");
				lua_gettable(L, -10);
				lua_pushstring(L, "text");
				lua_gettable(L, -11);

				wxLogDebug(_T("Read out all fields for dialogue event"));

				if (lua_isnumber(L, -10) && lua_isnumber(L, -9) && lua_isnumber(L, -8) &&
					lua_isstring(L, -7) && lua_isstring(L, -6) && lua_isnumber(L, -5) &&
					lua_isnumber(L, -4) && lua_isnumber(L, -3) && lua_isstring(L, -2) &&
					lua_isstring(L, -1))
				{

					AssDialogue *e = new AssDialogue();
					e->Layer = (int)lua_tonumber(L, -10);
					e->Start.SetMS(10*(int)lua_tonumber(L, -9));
					e->End.SetMS(10*(int)lua_tonumber(L, -8));
					e->Style = wxString(lua_tostring(L, -7), wxConvUTF8);
					e->Actor = wxString(lua_tostring(L, -6), wxConvUTF8);
					e->MarginL = (int)lua_tonumber(L, -5);
					e->MarginR = (int)lua_tonumber(L, -4);
					e->MarginV = (int)lua_tonumber(L, -3);
					e->Effect = wxString(lua_tostring(L, -2), wxConvUTF8);
					e->Text = wxString(lua_tostring(L, -1), wxConvUTF8);
					e->Comment = kind == _T("comment");
					lua_settop(L, -11);
					e->StartMS = e->Start.GetMS();
					//e->ParseASSTags();
					e->UpdateData();
					input->Line.push_back(e);
					wxLogDebug(_T("Produced new dialogue event in output subs"));

				} else {
					OutputDebugString(wxString::Format(_T("The output data at index %d (kind '%s') has one or more missing/invalid fields, and has been skipped"), outline, kind.c_str()));
				}

			} else if (kind == _T("scomment")) {

				lua_pushstring(L, "text");
				lua_gettable(L, -2);
				if (lua_isstring(L, -1)) {
					wxString text(lua_tostring(L, -1), wxConvUTF8);
					lua_settop(L, -2);
					AssEntry *e = new AssEntry(wxString(_T(";")) + text);
					e->StartMS = faketime;
					input->Line.push_back(e);
					wxLogDebug(_T("Produced new semicolon comment in output subs"));
				} else {
					OutputDebugString(wxString::Format(_T("The output data at index %d (kind 'scomment') is missing a valid 'text' field, and has been skipped"), outline));
				}

			} else if (kind == _T("blank")) {

				AssEntry *e = new AssEntry(_T(""));
				e->StartMS = faketime;
				input->Line.push_back(e);
				wxLogDebug(_T("Produced new blank line in output subs"));

			} else {
				OutputDebugString(wxString::Format(_T("The output data at index %d has an invalid value in the 'kind' field, and has been skipped"), outline));
			}

		}

		// remove table again
		lua_settop(L, -2);
		// progress report
		if (outline >= output_line_count) {
			ReportProgress(99.9f);
		} else {
			ReportProgress((200.0f + 100.0f*outline/output_line_count) / 3);
		}

		outline++;

		wxLogDebug(_T("Size of Lua stack: %d"), lua_gettop(L));

	}

	ReportProgress(100);
	OutputDebugString(wxString(_T("Script execution complete")));
	return;
}



AutomationScriptFile::~AutomationScriptFile()
{
	// if file had a bom
	if (utf8bom)
		scriptdata -= 3;
	delete scriptdata;
}

AutomationScriptFile *AutomationScriptFile::CreateFromString(wxString &script)
{
	wxCharBuffer rawscript = script.mb_str(wxConvUTF8);
	AutomationScriptFile *res = new AutomationScriptFile();
	res->scriptlen = strlen(rawscript);
	res->scriptdata = new char[res->scriptlen];
	res->filename = _T("");
	return res;
}

AutomationScriptFile *AutomationScriptFile::CreateFromFile(wxString filename)
{
	wxFile file(filename);
	if (!file.IsOpened()) {
		return 0;
	}

	// prepare variables
	AutomationScriptFile *res = new AutomationScriptFile();
	res->filename = filename;
	res->scriptlen = file.Length();
	res->scriptdata = new char[res->scriptlen];
	// read from file
	file.Read(res->scriptdata, res->scriptlen);
	// check for UTF-8 BOM
	res->utf8bom = ((res->scriptdata[0]&0xFF) == 0xEF) && ((res->scriptdata[1]&0xFF) == 0xBB) && ((res->scriptdata[2]&0xFF) == 0xBF);
	if (res->utf8bom) {
		// skip it if found
		res->scriptdata += 3;
		res->scriptlen -= 3;
	}
	return res;
}

