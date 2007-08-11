/*
 * OverLua video frame access interface
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

#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "../lua51/src/lua.h"
#include "cairo_wrap.h"
#include <stddef.h>
#include <memory.h>
#include <stdint.h>
#include <omp.h>

// Forward
class OverLuaFrameAggregate;

// store a colour value
struct RGBPixel {
	unsigned char r, g, b;
	RGBPixel(unsigned char R, unsigned char G, unsigned char B) : r(R), g(G), b(B) { }
};

// Support any interleaved RGB format with 8 bit per channel
// You usually don't want to instance this class directly,
// look at OverLuaFrameAggregate defined below
template<ptrdiff_t Rpos, ptrdiff_t Gpos, ptrdiff_t Bpos, ptrdiff_t PixelWidth>
class OverLuaVideoFrame {
public:
	typedef OverLuaVideoFrame<Rpos,Gpos,Bpos,PixelWidth> MyType;

	// video properties
	unsigned width;
	unsigned height;
	ptrdiff_t stride;
	unsigned char *data;

	// read a pixel value
	inline const RGBPixel GetPixel(unsigned x, unsigned y)
	{
		RGBPixel res(0, 0, 0);
		unsigned char *ptr = data + y*stride + x*PixelWidth;
		res.r = ptr[Rpos];
		res.g = ptr[Gpos];
		res.b = ptr[Bpos];
		return res;
	}

	// write a pixel value
	inline void SetPixel(unsigned x, unsigned y, const RGBPixel &val)
	{
		unsigned char *ptr = data + y*stride + x*PixelWidth;
		ptr[Rpos] = val.r;
		ptr[Gpos] = val.g;
		ptr[Bpos] = val.b;
	}

	OverLuaVideoFrame(unsigned _width, unsigned _height, ptrdiff_t _stride, unsigned char *_data)
		: width(_width), height(_height), stride(_stride), data(_data)
	{
		owndata = false;
		// nothing further to init
	}

	OverLuaVideoFrame(const MyType &src, bool copydata = false)
	{
		width = src.width;
		height = src.height;
		stride = src.stride;
		owndata = copydata;
		if (copydata) {
			data = new unsigned char[height*stride];
			memcpy(data, src.data, height*stride);
		} else {
			data = src.data;
		}
	}

	~OverLuaVideoFrame()
	{
		if (owndata)
			delete[] data;
	}

	// should never be called more than once on the same C++ object
	void CreateLuaObject(lua_State *L, OverLuaFrameAggregate *aggregate = 0)
	{
		// create userdata object
		MyType **ud = (MyType**)lua_newuserdata(L, sizeof(MyType*));
		*ud = this;

		// create metatable
		lua_newtable(L);
		lua_pushcclosure(L, lua_indexget, 0);
		lua_setfield(L, -2, "__index");
		lua_pushcclosure(L, lua_indexset, 0);
		lua_setfield(L, -2, "__newindex");
		lua_pushcclosure(L, lua_getpixel, 0);
		lua_setfield(L, -2, "__call");
		lua_pushcclosure(L, lua_finalize, 0);
		lua_setfield(L, -2, "__gc");
		if (aggregate) {
			lua_pushlightuserdata(L, aggregate);
			lua_setfield(L, -2, "videoframe");
		}
		lua_setmetatable(L, -2);
	}

private:
	bool owndata;

	// set a pixel colour
	static int lua_indexset(lua_State *L)
	{
		// first arg = object
		// second arg = index
		// third arg = value
		MyType **ud = (MyType**)lua_touserdata(L, 1);

		if (lua_isnumber(L, 2)) {
			if (lua_istable(L, 3)) {
				int n = (int)lua_tointeger(L, 2);
				unsigned x = n % (*ud)->width;
				unsigned y = n / (*ud)->width;
				if (x < 0 || y < 0 || x >= (*ud)->width || y >= (*ud)->height) return 0;

				// read first three entries from table
				RGBPixel color(0,0,0);
				lua_pushnil(L);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.r = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 1);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.g = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 1);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.b = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 2);

				(*ud)->SetPixel(x, y, color);
				return 0;

			} else {
badtable:
				lua_pushliteral(L, "Value set into video frame pixel must be a table with at least 3 entries");
				lua_error(L);
				return 0;
			}
		}

		lua_pushliteral(L, "Undefined field in video frame");
		lua_error(L);
		return 0;
	}

	// get a pixel colour or some other property
	static int lua_indexget(lua_State *L)
	{
		// first arg = object
		// second arg = index
		MyType **ud = (MyType**)lua_touserdata(L, 1);

		if (lua_type(L, 2) == LUA_TSTRING) {
			const char *fieldname = lua_tostring(L, 2);
			if (strcmp(fieldname, "width") == 0) {
				lua_pushnumber(L, (*ud)->width);
			} else if (strcmp(fieldname, "height") == 0) {
				lua_pushnumber(L, (*ud)->height);
			} else if (strcmp(fieldname, "copy") == 0) {
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, lua_copyfunc, 1);
			} else if (strcmp(fieldname, "create_cairo_surface") == 0) {
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, lua_create_cairo_surface, 1);
			} else if (strcmp(fieldname, "overlay_cairo_surface") == 0) {
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, lua_overlay_cairo_surface, 1);
			} else {
				lua_pushfstring(L, "Undefined field name in video frame: %s", fieldname);
				lua_error(L);
			}

			return 1;

		}

		lua_pushfstring(L, "Unhandled field type indexing video frame: %s", lua_typename(L, lua_type(L, 2)));
		lua_error(L);
		return 0;
	}

	static int lua_getpixel(lua_State *L)
	{
		// first arg = object
		// second arg = x
		// third arg = y
		MyType **ud = (MyType**)lua_touserdata(L, 1);

		unsigned x = luaL_checkint(L, 2);
		unsigned y = luaL_checkint(L, 3);

		RGBPixel color(0,0,0);
		if (x < 0 || y < 0 || x >= (*ud)->width || y >= (*ud)->height) {
			// already black, leave it
		} else {
			// get it
			color = (*ud)->GetPixel(x, y);
		}
		lua_pushinteger(L, color.r);
		lua_pushinteger(L, color.g);
		lua_pushinteger(L, color.b);
		return 3;
	}

	static int lua_finalize(lua_State *L)
	{
		MyType **ud = (MyType**)lua_touserdata(L, 1);
		delete *ud;
		return 0;
	}

	static int lua_copyfunc(lua_State *L)
	{
		MyType **ud = (MyType**)lua_touserdata(L, lua_upvalueindex(1));
		MyType *copy = new MyType(**ud, true);
		copy->CreateLuaObject(L);
		return 1;
	}

	static int lua_create_cairo_surface(lua_State *L)
	{
		MyType **ud = (MyType**)lua_touserdata(L, lua_upvalueindex(1));

		// Create a new surface of same resolution
		// Always RGB24 format since we don't support video with alpha
		cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, (*ud)->width, (*ud)->height);
		if (!surf || cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
			lua_pushliteral(L, "Unable to create Cairo surface from video frame");
			lua_error(L);
			return 0;
		}

		// Prepare copying pixels from frame to surface
		unsigned char *surfdata = cairo_image_surface_get_data(surf);
		int surfstride = cairo_image_surface_get_stride(surf);
		
		// Copy pixels
		int height = (*ud)->height;
		int width = (*ud)->width;
#pragma omp parallel for
		for (int y = 0; y < height; y++) {
			uint32_t *opix = (uint32_t*)(surfdata + y*surfstride);
			for (int x = 0; x < width; x++) {
				// Hoping this will get optimised at least a bit by the compiler
				RGBPixel ipix = (*ud)->GetPixel(x, y);
				*opix = ipix.r << 16 | ipix.g << 8 | ipix.b;
				opix++;
			}
		}
		cairo_surface_mark_dirty(surf);

		// Create Lua object
		LuaCairoSurface *lsurf = new LuaCairoSurface(L, surf);

		// Release our reference to the surface
		cairo_surface_destroy(surf);

		// Return it to Lua
		return 1;
	}

	static int lua_overlay_cairo_surface(lua_State *L)
	{
		// Get inputs
		MyType **ud = (MyType**)lua_touserdata(L, lua_upvalueindex(1));
		LuaCairoSurface *surfwrap = LuaCairoSurface::CheckPointer(*(void**)lua_touserdata(L, 1));
		if (!surfwrap) {
			lua_pushliteral(L, "Argument to frame.overlay_cairo_surface is not a Cairo surface");
			lua_error(L);
			return 0;
		}
		int xpos = luaL_checkint(L, 2);
		int ypos = luaL_checkint(L, 3);

		// More argument checks
		cairo_surface_t *surf = surfwrap->GetSurface();
		if (cairo_surface_get_type(surf) != CAIRO_SURFACE_TYPE_IMAGE) {
			lua_pushliteral(L, "Argument to frame.overlay_cairo_surface is not a Cairo image surface");
			lua_error(L);
			return 0;
		}
		
		// Prepare some data
		int fwidth = (*ud)->width, fheight = (*ud)->height;
		int swidth = cairo_image_surface_get_width(surf), sheight = cairo_image_surface_get_height(surf);
		int sstride = cairo_image_surface_get_stride(surf);
		// Prepare and get read pointer for source image
		cairo_surface_flush(surf);
		unsigned char *sdata = cairo_image_surface_get_data(surf);

		// Check that the overlaid surface won't be entirely outside the frame
		if (xpos <= -swidth || ypos <= -sheight || xpos > fwidth || ypos > fheight)
			return 0;

		// Pick overlay algorithm
		cairo_format_t sformat = cairo_image_surface_get_format(surf);

		if (sformat == CAIRO_FORMAT_ARGB32) {
			// This has pre-multipled alpha
			int fy = ypos; // frame y
			int sy = 0; // source y
			if (fy < 0) fy = 0, sy = -ypos;
			int slines_to_compose = sheight-sy, flines_to_compose = fheight;
			int lines_to_compose = (slines_to_compose<flines_to_compose)?slines_to_compose:flines_to_compose;
#pragma omp parallel for
			for (int composition_line = 0; composition_line < lines_to_compose; composition_line++) {
				uint32_t *sline = (uint32_t*)(sdata + sy*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					RGBPixel pix = (*ud)->GetPixel(fx, fy);
					unsigned char a = 0xff - ((sline[sx] & 0xff000000) >> 24);
					pix.r = ((sline[sx] & 0x00ff0000) >> 16) + a*pix.r/255;
					pix.g = ((sline[sx] & 0x0000ff00) >> 8) + a*pix.g/255;
					pix.b = (sline[sx] & 0x000000ff) + a*pix.b/255;
					(*ud)->SetPixel(fx, fy, pix);
				}
				fy++, sy++;
			}
		}

		else if (sformat == CAIRO_FORMAT_RGB24) {
			// Assume opaque alpha for all pixels
			int fy = ypos; // frame y
			int sy = 0; // source y
			if (fy < 0) fy = 0, sy = -ypos;
			int slines_to_compose = sheight-sy, flines_to_compose = fheight;
			int lines_to_compose = (slines_to_compose<flines_to_compose)?slines_to_compose:flines_to_compose;
#pragma omp parallel for
			for (int composition_line = 0; composition_line < lines_to_compose; composition_line++) {
				uint32_t *sline = (uint32_t*)(sdata + sy*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					RGBPixel pix(
						(sline[sx] & 0x00ff0000) >> 16,
						(sline[sx] & 0x0000ff00) >> 8,
						sline[sx] & 0x000000ff);
					(*ud)->SetPixel(fx, fy, pix);
				}
			}
		}

		else if (sformat == CAIRO_FORMAT_A8) {
			// This one is problematic - it doesn't contain any colour information
			// Two primary choices would be to fill with either black or white,
			// but neither would be an optimum solution.
			// A third option would be to take a fourth argument to this function,
			// specifying the colour to be used.
			lua_pushliteral(L, "8 bpp alpha images are not (yet) supported for overlay operations");
			lua_error(L);
			return 0;
		}

		else if (sformat == CAIRO_FORMAT_A1) {
			lua_pushliteral(L, "1 bpp alpha images are not supported for overlay operations");
			lua_error(L);
			return 0;
		}

		else {
			lua_pushliteral(L, "Unknown source image format for overlay operation");
			lua_error(L);
			return 0;
		}

		return 0;
	}

};


// Now, escape from the template madness that is OverLuaVideoFrame.
// OverLuaFrameAggregate is the class that will be used for most passing around
// in the C++ code. It nicely hides all templatyness away into various implementations.
// This could probably have been designed better. Shame on me.

class OverLuaFrameAggregate {
public:
	virtual RGBPixel GetPixel(unsigned x, unsigned y) = 0;
	virtual void SetPixel(unsigned x, unsigned y, const RGBPixel &val) = 0;
	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;
	virtual void CreateLuaObject(lua_State *L) = 0;
};

template <ptrdiff_t Rpos, ptrdiff_t Gpos, ptrdiff_t Bpos, ptrdiff_t PixelWidth>
class OverLuaFrameAggregateImpl : public OverLuaFrameAggregate {
public:
	typedef OverLuaVideoFrame<Rpos, Gpos, Bpos, PixelWidth> VideoFrameType;

private:
	VideoFrameType *frame;
	bool ownframe;

public:
	OverLuaFrameAggregateImpl(unsigned _width, unsigned _height, ptrdiff_t _stride, unsigned char *_data)
	{
		frame = new VideoFrameType(_width, _height, _stride, _data);
		ownframe = true;
	}

	OverLuaFrameAggregateImpl(VideoFrameType *_frame)
	{
		frame = _frame;
		ownframe = false;
	}

	RGBPixel GetPixel(unsigned x, unsigned y)
	{
		return frame->GetPixel(x, y);
	}

	void SetPixel(unsigned x, unsigned y, const RGBPixel &val)
	{
		frame->SetPixel(x, y, val);
	}

	unsigned GetWidth()
	{
		return frame->width;
	}

	unsigned GetHeight()
	{
		return frame->height;
	}

	void CreateLuaObject(lua_State *L)
	{
		frame->CreateLuaObject(L, this);
	}

};

// All common, sensible formats
typedef OverLuaFrameAggregateImpl<2, 1, 0, 3> OverLuaVideoFrameBGR;
typedef OverLuaFrameAggregateImpl<2, 1, 0, 4> OverLuaVideoFrameBGRX;
typedef OverLuaFrameAggregateImpl<2, 1, 0, 4> OverLuaVideoFrameBGRA;
typedef OverLuaFrameAggregateImpl<0, 1, 2, 3> OverLuaVideoFrameRGB;
typedef OverLuaFrameAggregateImpl<0, 1, 2, 4> OverLuaVideoFrameRGBX;
typedef OverLuaFrameAggregateImpl<0, 1, 2, 4> OverLuaVideoFrameRGBA;
typedef OverLuaFrameAggregateImpl<1, 2, 3, 4> OverLuaVideoFrameXRGB;
typedef OverLuaFrameAggregateImpl<1, 2, 3, 4> OverLuaVideoFrameARGB;
typedef OverLuaFrameAggregateImpl<3, 2, 1, 4> OverLuaVideoFrameXBGR;
typedef OverLuaFrameAggregateImpl<3, 2, 1, 4> OverLuaVideoFrameABGR;

#endif
