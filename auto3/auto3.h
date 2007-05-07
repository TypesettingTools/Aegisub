// Copyright (c) 2007, Niels Martin Hansen
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

#include <stdlib.h>
#ifdef AUTO3LIB
#include "lua/include/lua.h"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


// On Win32, filenames are wide, but whatever encoding the system uses everywhere else
#ifdef WIN32
typedef wchar_t* filename_t;
#else
typedef char* filename_t;
#endif

// All other strings involved are UTF-8, no need to do conversion back and forth here
// So just hardcode char* as type for everything else


#ifndef AUTO3LIB
// Definitions used when building Aegisub (ie. importing the symbols)
// I believe GCC also knows about __declspec(dllimport) etc. and does something sensible with it
# ifdef _MSC_VER
#  define AUTO3_API __declspec(dllimport)
# else
#  define AUTO3_API
# endif
#else
// Otherwise we're exporting the symbols
# ifdef _MSC_VER
#  define AUTO3_API __declspec(dllexport)
# else
#  define AUTO3_API
# endif
#endif


// Stuff for configuration dialogs
enum Auto3ConfigOptionKind {
	COK_INVALID = 0,
	COK_LABEL, // static text
	COK_TEXT, // textbox
	COK_INT, // integer entry, should get spin-button is possible
	COK_FLOAT, // float entry, also try for spin-button on this
	COK_BOOL, // bool entry, use a checkbox
	COK_COLOUR, // colour entry, use a picker if possible, otherwise text box
	COK_STYLE // style name, listbox with names of styles defined in subs
};
struct Auto3ConfigOption {
	char *name;
	enum Auto3ConfigOptionKind kind;
	char *label;
	char *hint;
	struct {
		int valid; // non-zero if the value is present
		int intval;
		float floatval;
	} min, max;
	union {
		char *stringval; // text, colour (vb-hex style-line format), style
		int intval; // also bool, nonzero is true
		float floatval;
	} default_val, value;
};


// Callback interface
// The application should provide ALL of these functions
struct Auto3Callbacks {
	// Logging and status
	// pointer passed to logging/status callbacks
	void *logdata;
	// log error during script execution
	void (*log_error)(void *cbdata, const char *msg);
	// log message during script execution
	void (*log_message)(void *cbdata, const char *msg);
	// set progress during script execution
	void (*set_progress)(void *cbdata, float progress);
	// set status message during script execution
	void (*set_status)(void *cbdata, const char* msg);

	// Reading/writing subtitles and related information
	// pointer passed to read/write data callbacks
	void *rwdata;
	// application sets *res_x and *res_y to appropriate values
	void (*get_meta_info)(void *cbdata, int *res_x, int *res_y);
	// set style pointer to point at first style
	void (*reset_style_pointer)(void *cbdata);
	// Get the next style, the application must fill the data into its own buffers, which it then fill in pointers to
	// (Ie. the application owns all strings allocated for this callback.)
	// Return non-zero if a style was found and values filled, otherwise return zero
	int (*get_next_style)(
		void *cbdata, char **name, char **fontname, int *fontsize, char **color1, char **color2, char **color3, char **color4,
		int *bold, int *italic, int *underline, int *strikeout, float *scale_x, float *scale_y, float *spacing, float *angle,
		int *borderstyle, float *outline, float *shadow, int *align, int *margin_l, int *margin_r, int *margin_v, int *encoding);
	// set subtitle pointer to point at first subtitle line
	void (*reset_subs_pointer)(void *cbdata);
	// Get next subtitle line, the application must fill the data into its own buffers, and then fill in pointers to those
	// Return non-zero if a style was found and values filled, otherwise return zero
	int (*get_next_sub)(
		void *cbdata, int *layer, int *start_time, int *end_time, char **style, char **actor,
		int *margin_l, int *margin_r, int *margin_v, char **effect, char **text, int *comment);
	// start writing back new subtitles, application must clear all subtitle lines and be ready to write
	void (*start_subs_write)(void *cbdata);
	// Write a subtitle line back to subtitle file, char pointers are owned by the lib
	void (*write_sub)(void *cbdata, int layer, int start_time, int end_time, const char *style, const char *actor,
		int margin_l, int margin_r, int margin_v, const char *effect, const char *text, int comment);

	// Getting various environment information during runtime
	// pointer passed to runtime data callbacks
	void *rundata;
	// Resolve a filename passed to the include function
	// The result must be allocated with Auto3Malloc and will be free'd by the lib
	filename_t (*resolve_include)(void *cbdata, const char *incname);
	// Get sizing information for a text string given a style
	void (*text_extents)(void *cbdata, const char *text, const char *fontname, int fontsize, int bold, int italic,
		int spacing, float scale_x, float scale_y, int encoding,
		float *out_width, float *out_height, float *out_descent, float *out_extlead);
	// Convert a time in milliseconds to a video frame number
	int (*frame_from_ms)(void *cbdata, int ms);
	// Convert a video frame number to a time in milliseconds
	int (*ms_from_frame)(void *cbdata, int frame);
};


// Describes an interpreter
struct Auto3Interpreter {
	// Public attributes, treat them as read-only
	char *name;
	char *description;
	
	// Configuration dialog options
	// End of list marked with name==NULL
	// You may change the "value" field of these (in fact, do so)
	struct Auto3ConfigOption *config;

	// Callbacks
	// This is filled in from the 'cb' argument to the Create function,
	// but may be modified later by the application
	struct Auto3Callbacks cb;

#ifdef AUTO3LIB
	// Private data
	lua_State *L;
#endif
};


// Create a new interpreter
// filename is name of script file
// prettyname is a UTF-8 string used as identifier for the script in error messages
// cb should point to an Auto3Callbacks struct filled in; a copy of this struct will be made
// error will be filled with any error message on fail, the application is responsible for freeing this string (use Auto3Free)
// Returns pointer to interpreter object if successful, otherwise NULL
AUTO3_API struct Auto3Interpreter *CreateAuto3Script(const filename_t filename, const char *prettyname, struct Auto3Callbacks *cb, char **error);
// Release an interpreter
AUTO3_API void DestroyAuto3Script(struct Auto3Interpreter *script);

// Our "malloc" function, allocate memory for strings with this
AUTO3_API void *Auto3Malloc(size_t amount);
// Convenience function, use this for duplicating strings this lib should own
AUTO3_API char *Auto3Strdup(const char *str);
// Our "free" function, free strings with this
AUTO3_API void Auto3Free(void *ptr);

// Start the script execution
// script->logcbdata and log->rwcbdata should be set to sensible values before this call.
// The value fields in the config dialog should also be set to values entered by the user here.
// This will first call get_meta_info,
// then reset_style pointer followed by a number of calls to get_next_style,
// then a call to reset_subs_pointer followed by a number of calls to get_next_sub,
// then actual processing will take place.
// After processing, start_subs_write will be called, followed by a number of calls to write_sub.
// Any number of calls to the logging/status functions can take place during script execution
// Returns non-zero on error
AUTO3_API int RunAuto3Script(struct Auto3Interpreter *script);


#ifdef __cplusplus
}; // extern "C"
#endif
