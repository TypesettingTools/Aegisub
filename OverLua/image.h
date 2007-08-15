/*
 * OverLua RGB(A) image interface
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
#include <math.h>


// Forward
class BaseImageAggregate;


// Supported pixel formats
namespace PixelFormat {
	// A constant value with a fake assignment operator, taking up no space
	template <class T, T v>
	struct NopAssigningConstant {
		operator T() const { return v; }
		void operator = (const T &n) { }
	};
	typedef NopAssigningConstant<uint8_t,255> ROA; // "read only alpha"
	typedef NopAssigningConstant<uint8_t,0> ROC; // "read only colour"

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
		template <class PixFmt> RGBA(const PixFmt &src) { a = src.a; r = src.r; g = src.g; b = src.b; }
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

	// Alpha only
	struct A8 {
		uint8_t a;
		inline ROC R() const { return ROC(); }
		inline ROC G() const { return ROC(); }
		inline ROC B() const { return ROC(); }
		inline uint8_t &A() { return a; } inline uint8_t A() const { return a; }
		A8() : a(0) { }
		template <class PixFmt> A8(const PixFmt &src) { a = src.A(); }
	};

	// cairo types
	// These are only true for little endian architectures
	// If OverLua is ever to run on big endian archs some conditional compiling should be used here
	typedef BGRX cairo_rgb24;
	typedef BGRA cairo_argb32;
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

				(*ud)->Pixel(x, y) = color;
				return 0;

			} else {
badtable:
				lua_pushliteral(L, "Value set into image pixel must be a table with 3 entries");
				lua_error(L);
				return 0;
			}
		}

		lua_pushliteral(L, "Undefined field in image");
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
			lua_pushliteral(L, "Unable to create cairo surface from image");
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
			PixelFormat::cairo_rgb24 *opix = (PixelFormat::cairo_rgb24*)(surfdata + y*surfstride);
			for (int x = 0; x < width; x++) {
				*opix++ = *ipix++;
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
			lua_pushliteral(L, "Argument to overlay_cairo_surface is not a cairo surface");
			lua_error(L);
			return 0;
		}
		int xpos = luaL_checkint(L, 2);
		int ypos = luaL_checkint(L, 3);

		// More argument checks
		cairo_surface_t *surf = surfwrap->GetSurface();
		if (cairo_surface_get_type(surf) != CAIRO_SURFACE_TYPE_IMAGE) {
			lua_pushliteral(L, "Argument to overlay_cairo_surface is not a cairo image surface");
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
				PixelFormat::cairo_argb32 *sline = (PixelFormat::cairo_argb32*)(sdata + composition_line*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					PixFmt &pix = (*ud)->Pixel(fx, fy+composition_line);
					unsigned char a = 0xff - sline[sx].a;
					pix.r = sline[sx].r + a*pix.r/255;
					pix.g = sline[sx].g + a*pix.g/255;
					pix.b = sline[sx].b + a*pix.b/255;
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
				PixelFormat::cairo_rgb24 *sline = (PixelFormat::cairo_rgb24*)(sdata + composition_line*sstride);
				int fx = xpos;
				int sx = 0;
				if (fx < 0) fx = 0, sx = -xpos;
				for ( ; sx < swidth && fx < fwidth; fx++, sx++) {
					(*ud)->Pixel(fx, fy+composition_line) = sline[sx];
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


// Now something so we can pretend all images have the same pixel format
// and can pass around pointers to objects of one fixed base class.
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


// Access pixels with various edge conditions
namespace EdgeCondition {
	template<class PixFmt>
	inline PixFmt &blackness(BaseImage<PixFmt> &img, int x, int y)
	{
		if (x < 0 || y < 0 || x >= img.width || x >= img.height) {
			return PixFmt(); // all construct with all zeroes
		} else {
			return img.Pixel(x,y);
		}
	}

	template<class PixFmt>
	inline PixFmt &closest(BaseImage<PixFmt> &img, int x, int y)
	{
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x >= img.width) x = img.width-1;
		if (y >= img.height) y = img.height - 1;
		return img.Pixel(x,y);
	}

	template<class PixFmt>
	inline PixFmt &repeat(BaseImage<PixFmt> &img, int x, int y)
	{
		while (x < 0) x += img.width;
		while (y < 0) y += img.height;
		while (x >= img.width) x -= img.width;
		while (y >= img.height) y -= img.height;
		return img.Pixel(x,y);
	}

	template<class PixFmt>
	inline PixFmt &mirror(BaseImage<PixFmt> &img, int x, int y)
	{
		while (x < 0) x += img.width*2;
		while (y < 0) y += img.height*2;
		while (x >= img.width*2) x -= img.width*2;
		while (y >= img.height*2) y -= img.height*2;
		if (x >= img.width) x = img.width - x;
		if (y >= img.height) y = img.height - y;
		return img.Pixel(x,y);
	}
}


// FIXME: this is completely broken, the compiler won't accept it
// when instantiated with one of the EdgeCondition functions for EdgeCond
template<class PixFmt, class EdgeCond>
inline PixFmt GetPixelBilinear(BaseImage<PixFmt> &img, double x, double y)
{
	PixFmt res;
	double xpct = x - floor(x), ypct = y - floor(y);

	const PixFmt &src11 = EdgeCond(img, (int)x, (int)y);
	const PixFmt &src12 = EdgeCond(img, (int)x, 1+(int)y);
	const PixFmt &src21 = EdgeCond(img, 1+(int)x, (int)y);
	const PixFmt &src22 = EdgeCond(img, 1+(int)x, 1+(int)y);

	res.R() += (1-xpct) * (1-ypct) * src11.R() + (1-xpct) * ypct * src12.R() + xpct * (1-ypct) * src21.R() + xpct * ypct * src22.R();
	res.G() += (1-xpct) * (1-ypct) * src11.G() + (1-xpct) * ypct * src12.G() + xpct * (1-ypct) * src21.G() + xpct * ypct * src22.G();
	res.B() += (1-xpct) * (1-ypct) * src11.B() + (1-xpct) * ypct * src12.B() + xpct * (1-ypct) * src21.B() + xpct * ypct * src22.B();
	res.A() += (1-xpct) * (1-ypct) * src11.A() + (1-xpct) * ypct * src12.A() + xpct * (1-ypct) * src21.A() + xpct * ypct * src22.A();

	return res;
}


#endif
