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
#include "../lua51/src/lauxlib.h"
#include "cairo_wrap.h"
#include <stddef.h>
#include <memory.h>
#include <stdint.h>
#include <omp.h>


// Forward
class BaseImageAggregate;


// Supported pixel formats
namespace PixelFormat {
	// A constant value with a fake assignment operator
	template <class T, T v>
	struct NopAssigningConstant {
		operator T() { return v; }
		void operator = (const T &n) { }
	};
	typedef NopAssigningConstant<uint8_t,255> ROA; // "read only alpha"

	// 24 bit formats
	struct RGB {
		uint8_t r, g, b;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		RGB() : r(0), g(0), b(0) { }
		template <class PixFmt> RGB(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};
	struct BGR {
		uint8_t b, g, r;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		BGR() : r(0), g(0), b(0) { }
		template <class PixFmt> BGR(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};

	// 32 bit alpha-less formats
	struct RGBX {
		uint8_t r, g, b, x;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		RGBX() : r(0), g(0), b(0) { }
		template <class PixFmt> RGBX(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};
	struct BGRX {
		uint8_t b, g, r, x;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		BGRX() : r(0), g(0), b(0) { }
		template <class PixFmt> BGRX(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};
	struct XRGB {
		uint8_t x, r, g, b;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		XRGB() : r(0), g(0), b(0) { }
		template <class PixFmt> XRGB(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};
	struct XBGR {
		uint8_t x, b, g, r;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline ROA A() const { return ROA(); }
		XBGR() : r(0), g(0), b(0) { }
		template <class PixFmt> XBGR(const PixFmt &src) { r = src.R(); g = src.G(); b = src.B(); }
	};

	// 32 bit with alpha
	struct RGBA {
		uint8_t r, g, b, a;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline uint8_t &A() { return a; } inline uint8_t A() const { return a; }
		RGBA() : r(0), g(0), b(0), a(0) { }
		template <class PixFmt> RGBA(const PixFmt &src) { a = src.A(); r = src.R(); g = src.G(); b = src.B(); }
	};
	struct BGRA {
		uint8_t b, g, r, a;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline uint8_t &A() { return a; } inline uint8_t A() const { return a; }
		BGRA() : r(0), g(0), b(0), a(0) { }
		template <class PixFmt> BGRA(const PixFmt &src) { a = src.A(); r = src.R(); g = src.G(); b = src.B(); }
	};
	struct ARGB {
		uint8_t a, r, g, b;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline uint8_t &A() { return a; } inline uint8_t A() const { return a; }
		ARGB() : r(0), g(0), b(0), a(0) { }
		template <class PixFmt> ARGB(const PixFmt &src) { a = src.A(); r = src.R(); g = src.G(); b = src.B(); }
	};
	struct ABGR {
		uint8_t a, b, g, r;
		inline uint8_t &R() { return r; } inline uint8_t R() const { return r; }
		inline uint8_t &G() { return g; } inline uint8_t G() const { return g; }
		inline uint8_t &B() { return b; } inline uint8_t B() const { return b; }
		inline uint8_t &A() { return a; } inline uint8_t A() const { return a; }
		ABGR() : r(0), g(0), b(0), a(0) { }
		template <class PixFmt> ABGR(const PixFmt &src) { a = src.A(); r = src.R(); g = src.G(); b = src.B(); }
	};
};


// Support any interleaved RGB format with 8 bit per channel
// You usually don't want to instance this class directly,
// look at OverLuaFrameAggregate defined below
template<class PixFmt>
class BaseImage {
public:
	typedef BaseImage<PixFmt> MyType;

	// video properties
	int width;
	int height;
	ptrdiff_t stride;
	unsigned char *data;

	// Access a pixel value
	inline const PixFmt &Pixel(int x, int y) const
	{
		return *((PixFmt*)(data + y*stride) + x)
	}

	inline PixFmt &Pixel(int x, int y)
	{
		return *((PixFmt*)(data + y*stride) + x);
	}

	BaseImage(unsigned _width, unsigned _height, ptrdiff_t _stride, unsigned char *_data)
		: width(_width), height(_height), stride(_stride), data(_data)
	{
		owndata = false;
		// nothing further to init
	}

	BaseImage(const MyType &src, bool copydata = false)
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

	~BaseImage()
	{
		if (owndata)
			delete[] data;
	}

	// should never be called more than once on the same C++ object
	void CreateLuaObject(lua_State *L, BaseImageAggregate *aggregate = 0)
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
			lua_setfield(L, -2, "image");
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
				int x = n % (*ud)->width;
				int y = n / (*ud)->width;
				if (x < 0 || y < 0 || x >= (*ud)->width || y >= (*ud)->height) return 0;

				// read first three entries from table
				PixFmt color;
				lua_pushnil(L);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.R() = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 1);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.G() = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 1);
				if (!lua_next(L, 3)) goto badtable;
				if (!lua_isnumber(L, -1)) goto badtable;
				color.B() = (unsigned char)lua_tointeger(L, -1);
				lua_pop(L, 2);

				(*ud)->Pixel(x, y) = color;
				return 0;

			} else {
badtable:
				lua_pushliteral(L, "Value set into image pixel must be a table with at least 3 entries");
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
				lua_pushfstring(L, "Undefined field name in image: %s", fieldname);
				lua_error(L);
			}

			return 1;

		}

		lua_pushfstring(L, "Unhandled field type indexing image: %s", lua_typename(L, lua_type(L, 2)));
		lua_error(L);
		return 0;
	}

	static int lua_getpixel(lua_State *L)
	{
		// first arg = object
		// second arg = x
		// third arg = y
		MyType **ud = (MyType**)lua_touserdata(L, 1);

		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);

		if (x < 0 || y < 0 || x >= (*ud)->width || y >= (*ud)->height) {
			lua_pushinteger(L, 0);
			lua_pushinteger(L, 0);
			lua_pushinteger(L, 0);
		} else {
			const PixFmt &p = (*ud)->Pixel(x, y);
			lua_pushinteger(L, p.r);
			lua_pushinteger(L, p.g);
			lua_pushinteger(L, p.b);
		}
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
			lua_pushliteral(L, "Unable to create cairo surface from video frame");
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
			PixFmt *ipix = (PixFmt*)((*ud)->data + y*((*ud)->stride));
			PixelFormat::XBGR *opix = (PixelFormat::XBGR*)(surfdata + y*surfstride);
			for (int x = 0; x < width; x++) {
				// Hoping this will get optimised at least a bit by the compiler
				*opix++ = PixelFormat::XBGR(*ipix++);
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
		LuaCairoSurface *surfwrap = LuaCairoSurface::GetObjPointer(L, 1);
		if (!surfwrap) {
			lua_pushliteral(L, "Argument to frame.overlay_cairo_surface is not a cairo surface");
			lua_error(L);
			return 0;
		}
		int xpos = luaL_checkint(L, 2);
		int ypos = luaL_checkint(L, 3);

		// More argument checks
		cairo_surface_t *surf = surfwrap->GetSurface();
		if (cairo_surface_get_type(surf) != CAIRO_SURFACE_TYPE_IMAGE) {
			lua_pushliteral(L, "Argument to frame.overlay_cairo_surface is not a cairo image surface");
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
			if (fy < 0) fy = 0, sdata -= ypos, sheight -= ypos;
			int slines_to_compose = sheight, flines_to_compose = fheight;
			int lines_to_compose = (slines_to_compose<flines_to_compose)?slines_to_compose:flines_to_compose;
#pragma omp parallel for
			for (int composition_line = 0; composition_line < lines_to_compose; composition_line++) {
				PixelFormat::ARGB *sline = (PixelFormat::ARGB*)(sdata + composition_line*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					PixFmt &pix = (*ud)->Pixel(fx, fy+composition_line);
					unsigned char a = 0xff - sline[sx].A();
					pix.R() = sline[sx].R() + a*pix.R()/255;
					pix.G() = sline[sx].G() + a*pix.G()/255;
					pix.B() = sline[sx].B() + a*pix.B()/255;
				}
			}
		}

		else if (sformat == CAIRO_FORMAT_RGB24) {
			// Assume opaque alpha for all pixels
			int fy = ypos; // frame y
			if (fy < 0) fy = 0, sdata -= ypos, sheight -= ypos;
			int slines_to_compose = sheight, flines_to_compose = fheight;
			int lines_to_compose = (slines_to_compose<flines_to_compose)?slines_to_compose:flines_to_compose;
#pragma omp parallel for
			for (int composition_line = 0; composition_line < lines_to_compose; composition_line++) {
				PixelFormat::XRGB *sline = (PixelFormat::XRGB*)(sdata + composition_line*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					sline[sx] = PixelFormat::XRGB((*ud)->Pixel(fx, fy+composition_line));
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

class BaseImageAggregate {
public:
	virtual PixelFormat::ARGB GetPixel(int x, int y) = 0;
	virtual void SetPixel(int x, int y, const PixelFormat::ARGB &val) = 0;
	virtual unsigned GetWidth() = 0;
	virtual unsigned GetHeight() = 0;
	virtual void CreateLuaObject(lua_State *L) = 0;
};

template <class PixFmt>
class BaseImageAggregateImpl : public BaseImageAggregate {
public:
	typedef BaseImage<PixFmt> ImageType;

private:
	ImageType *frame;
	bool ownframe;

public:
	BaseImageAggregateImpl(unsigned _width, unsigned _height, ptrdiff_t _stride, unsigned char *_data)
	{
		frame = new ImageType(_width, _height, _stride, _data);
		ownframe = true;
	}

	BaseImageAggregateImpl(ImageType *_frame)
	{
		frame = _frame;
		ownframe = false;
	}

	PixelFormat::ARGB GetPixel(int x, int y)
	{
		return PixelFormat::ARGB(frame->Pixel(x, y));
	}

	void SetPixel(int x, int y, const PixelFormat::ARGB &val)
	{
		frame->Pixel(x, y) = PixFmt(val);
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
typedef BaseImageAggregateImpl<PixelFormat::RGB> ImageRGB;
typedef BaseImageAggregateImpl<PixelFormat::BGR> ImageBGR;
typedef BaseImageAggregateImpl<PixelFormat::RGBX> ImageRGBX;
typedef BaseImageAggregateImpl<PixelFormat::BGRX> ImageBGRX;
typedef BaseImageAggregateImpl<PixelFormat::XRGB> ImageXRGB;
typedef BaseImageAggregateImpl<PixelFormat::XBGR> ImageXBGR;
typedef BaseImageAggregateImpl<PixelFormat::RGBA> ImageRGBA;
typedef BaseImageAggregateImpl<PixelFormat::BGRA> ImageBGRA;
typedef BaseImageAggregateImpl<PixelFormat::ARGB> ImageARGB;
typedef BaseImageAggregateImpl<PixelFormat::ABGR> ImageABGR;

#endif
