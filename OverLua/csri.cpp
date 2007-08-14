/*
 * CSRI interface for OverLua
 *

    Copyright 2007  Niels Martin Hansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contact:
    E-mail: <jiifurusu@gmail.com>
    IRC: jfs in #aegisub on irc.rizon.net

 */
 
// CSRI interface is unmaintained for now
#if 0

#include <string.h>
#include <memory.h>

#define CSRI_OWN_HANDLES
#define CSRIAPI extern "C" __declspec(dllexport)

// Nothing special required here
typedef int csri_rend;

// Forward instance data
struct csri_inst;

#include "../csri/include/csri/csri.h"
#include "../csri/include/csri/logging.h"

#include "overlua.h"

// Instance data
struct csri_inst {
	csri_fmt frame_format;
	OverLuaScript *script;
};

// Renderer information definition
static const csri_info overlua_csri_info = {
	"overlua",
	"overlua-001",
	"OverLua Lua-based advanced effects renderer",
	"Niels Martin Hansen",
	"Copyright 2007 Niels Martin Hansen (GPLv2)"
};
// Just something we can pass a pointer to
static const csri_rend overlua_csri_rend = 0;

// Name of the "overlua" extension format
static const char *overlua_format_ext_name = "jfs.overlua";


CSRIAPI csri_inst *csri_open_file(csri_rend *renderer,
	const char *filename, struct csri_openflag *flags)
{
	if (renderer != &overlua_csri_rend) return 0;
	
	csri_inst *inst = new csri_inst;
	inst->script = new OverLuaScript(filename);
	
	return inst;
}


CSRIAPI csri_inst *csri_open_mem(csri_rend *renderer,
	const void *data, size_t length, struct csri_openflag *flags)
{
	if (renderer != &overlua_csri_rend) return 0;
	
	csri_inst *inst = new csri_inst;
	inst->script = new OverLuaScript(data, length);
	
	return inst;
}


CSRIAPI void csri_close(csri_inst *inst)
{
	delete inst->script;
	delete inst;
	return;
}


CSRIAPI int csri_request_fmt(csri_inst *inst, const struct csri_fmt *fmt)
{
	// only support RGB formats
	if (!csri_is_rgb(fmt->pixfmt))
		return 0;

	// get a private copy of it
	memcpy(&inst->frame_format, fmt, sizeof(csri_fmt));

	return 1;
}


CSRIAPI void csri_render(csri_inst *inst, struct csri_frame *frame, double time)
{
	// check for correct pixfmt
	if (frame->pixfmt != inst->frame_format.pixfmt) return;

	BaseImageAggregate *olframe = 0;
	switch (frame->pixfmt) {
#define HANDLE_RGB_FORMAT(fmtname, PixFmt) \
	case fmtname: \
		olframe = new BaseImageAggregateImpl<PixelFormat:: ## PixFmt>(inst->frame_format.width, inst->frame_format.height, frame->strides[0], frame->planes[0]); \
		break;
		HANDLE_RGB_FORMAT(CSRI_F_RGBA, RGBA)
		HANDLE_RGB_FORMAT(CSRI_F_ARGB, ARGB)
		HANDLE_RGB_FORMAT(CSRI_F_BGRA, BGRA)
		HANDLE_RGB_FORMAT(CSRI_F_ABGR, ABGR)
		HANDLE_RGB_FORMAT(CSRI_F_RGB_, RGBX)
		HANDLE_RGB_FORMAT(CSRI_F__RGB, XRGB)
		HANDLE_RGB_FORMAT(CSRI_F_BGR_, BGRX)
		HANDLE_RGB_FORMAT(CSRI_F__BGR, XBGR)
		HANDLE_RGB_FORMAT(CSRI_F_RGB, RGB)
		HANDLE_RGB_FORMAT(CSRI_F_BGR, BGR)
		default: break; // what, we don't support this!
#undef HANDLE_RGB_FORMAT
	}

	if (olframe) {
		inst->script->RenderFrameRGB(*olframe, time);
	}

}


CSRIAPI void *csri_query_ext(csri_rend *rend, csri_ext_id extname)
{
	if (rend != &overlua_csri_rend)
		return 0;
	
	// Check for the OverLua format extension
	if (strcmp(extname, overlua_format_ext_name) == 0)
		// Nonsense return
		return &overlua_format_ext_name;
	
	// TODO: support logging
	if (strcmp(extname, CSRI_EXT_LOGGING) == 0)
		return 0;
	
	return 0;
}


CSRIAPI struct csri_info *csri_renderer_info(csri_rend *rend)
{
	if (rend == &overlua_csri_rend)
		return (csri_info*)&overlua_csri_info;
	return 0;
}


CSRIAPI csri_rend *csri_renderer_byname(const char *name,
	const char *specific)
{
	if (strcmp(name, overlua_csri_info.name) == 0)
		if (!specific || strcmp(specific, overlua_csri_info.specific) == 0)
			return (csri_rend*)&overlua_csri_rend;
	return 0;
}


CSRIAPI csri_rend *csri_renderer_byext(unsigned n_ext, csri_ext_id *ext)
{
	// Check if every extension is supported
	while (n_ext-- > 0) {
		if (!csri_query_ext((csri_rend*)&overlua_csri_rend, *ext))
			return 0;
		ext++;
	}
	return (csri_rend*)&overlua_csri_rend;
}


CSRIAPI csri_rend *csri_renderer_default()
{
	return (csri_rend*)&overlua_csri_rend;
}


CSRIAPI csri_rend *csri_renderer_next(csri_rend *prev)
{
	if (prev == 0)
		return (csri_rend*)&overlua_csri_rend;
	else
		return 0;
}

#endif