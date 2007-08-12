/*
 * Lua interface for the Cairo graphics library
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

#ifndef CAIRO_WRAP_H
#define CAIRO_WRAP_H

#include "../lua51/src/lua.h"
#include <cairo.h>
#include <string>
#include <map>


int luaopen_cairo(lua_State *L);


template <class ChildClass>
class LuaCairoBase {
private:
	// Default handlers for metatable stuff
	static int lua_index(lua_State *L)
	{
		LuaCairoBase **obj = (LuaCairoBase**)lua_touserdata(L, 1);
		return (*obj)->internal_lua_index(L);
	}

	static int lua_newindex(lua_State *L)
	{
		LuaCairoBase **obj = (LuaCairoBase**)lua_touserdata(L, 1);
		return (*obj)->internal_lua_newindex(L);
	}

	static int lua_callobj(lua_State *L)
	{
		LuaCairoBase **obj = (LuaCairoBase**)lua_touserdata(L, 1);
		return (*obj)->internal_lua_callobj(L);
	}

	static int lua_gc(lua_State *L)
	{
		LuaCairoBase **obj = (LuaCairoBase**)lua_touserdata(L, 1);
		delete *obj;
		return 0;
	}

	// Hidden constructors
	LuaCairoBase() { }
	LuaCairoBase(const LuaCairoBase &obj) { }

	// List of callables
	typedef std::map<std::string, lua_CFunction> CallableMap;
	CallableMap callables;

	// "Magic" value - used to test that pointers are really valid
	lua_CFunction magic;

protected:
	virtual int internal_lua_index(lua_State *L)
	{
		if (lua_type(L, 2) == LUA_TSTRING) {
			const char *field = lua_tostring(L, 2);

			CallableMap::iterator func = callables.find(field);
			if (func != callables.end()) {
				lua_pushvalue(L, 1);
				lua_pushcclosure(L, func->second, 1);
				return 1;
			}

			if (strcmp("_type", lua_tostring(L, 2))) {
				lua_pushstring(L, GetTypeName());
				return 1;
			}
		}

		return 0;
	}
	virtual int internal_lua_newindex(lua_State *L)
	{
		lua_pushfstring(L, "Cairo object of type '%s' can not have field set", GetTypeName());
		lua_error(L);
		return 0;
	}
	virtual int internal_lua_callobj(lua_State *L)
	{
		lua_pushfstring(L, "Cairo objct of type '%s' can not be called", GetTypeName());
		lua_error(L);
		return 0;
	}

	virtual const char *GetTypeName() { return "_base"; }

	virtual void CreateMetaTable(lua_State *L)
	{
		lua_newtable(L);
		lua_pushcclosure(L, lua_index, 0);
		lua_setfield(L, -2, "__index");
		lua_pushcclosure(L, lua_newindex, 0);
		lua_setfield(L, -2, "__newindex");
		lua_pushcclosure(L, lua_callobj, 0);
		lua_setfield(L, -2, "__call");
		lua_pushcclosure(L, lua_gc, 0);
		lua_setfield(L, -2, "__gc");
	}

	void AddCallable(lua_CFunction func, const char *name)
	{
		callables[name] = func;
	}

	// Primary constructor for use with inherited stuff
	LuaCairoBase(lua_State *L)
	{
		LuaCairoBase **ud = (LuaCairoBase**)lua_newuserdata(L, sizeof(ChildClass*));
		*ud = this;
		CreateMetaTable(L);
		lua_setmetatable(L, -2);
		magic = luaopen_cairo;
	}

public:
	virtual ~LuaCairoBase() { }

	// Helper: Get the object pointer from a callback
	static ChildClass *GetObjPointer(lua_State *L, int index)
	{
		if (!lua_isuserdata(L, index)) {
			lua_pushliteral(L, "Passed non-userdata where one expected");
			lua_error(L);
		}
		ChildClass *obj = CheckPointer(*(void**)lua_touserdata(L, index));
		return obj;
	}

	// Check whether a pointer plausibly points to an object of this class
	static ChildClass *CheckPointer(void *ptr)
	{
		ChildClass *testptr = (ChildClass*)ptr;
		if (testptr->magic == luaopen_cairo)
			return testptr;
		else
			return 0;
	}
};


#define CALLABLE(name) static int lua_ ## name (lua_State *L)


class LuaCairoContext : public LuaCairoBase<LuaCairoContext> {
private:
	cairo_t *context;

	CALLABLE(reference);
	CALLABLE(status);

	CALLABLE(save);
	CALLABLE(restore);

	CALLABLE(get_target);

	CALLABLE(push_group);
	CALLABLE(push_group_with_content);
	CALLABLE(pop_group);
	CALLABLE(pop_group_to_source);
	CALLABLE(get_group_target);

	CALLABLE(set_source_rgb);
	CALLABLE(set_source_rgba);
	CALLABLE(set_source);
	CALLABLE(set_source_surface);
	CALLABLE(get_source);

	CALLABLE(set_antialias);
	CALLABLE(get_antialias);

	CALLABLE(set_dash);
	CALLABLE(get_dash_count);
	CALLABLE(get_dash);

	CALLABLE(set_fill_rule);
	CALLABLE(get_fill_rule);

	CALLABLE(set_line_cap);
	CALLABLE(get_line_cap);

	CALLABLE(set_line_join);
	CALLABLE(get_line_join);

	CALLABLE(set_line_width);
	CALLABLE(get_line_width);

	CALLABLE(set_miter_limit);
	CALLABLE(get_miter_limit);

	CALLABLE(set_operator);
	CALLABLE(get_operator);

	CALLABLE(set_tolerance);
	CALLABLE(get_tolerance);

	CALLABLE(clip);
	CALLABLE(clip_preserve);
	CALLABLE(clip_extents);
	CALLABLE(reset_clip);

	// rectangle_list_destroy is not needed,
	// copy_clip_rectangle_list will convert the rect list into a pure Lua structure
	CALLABLE(copy_clip_rectangle_list);

	CALLABLE(fill);
	CALLABLE(fill_preserve);
	CALLABLE(fill_extents);
	CALLABLE(in_fill);

	CALLABLE(mask);
	CALLABLE(mask_surface);

	CALLABLE(paint);
	CALLABLE(paint_with_alpha);

	CALLABLE(stroke);
	CALLABLE(stroke_preserve);
	CALLABLE(stroke_extents);
	CALLABLE(in_stroke);

	CALLABLE(copy_page);
	CALLABLE(show_page);

	// Path operations

	CALLABLE(copy_path);
	CALLABLE(copy_path_flat);

	CALLABLE(append_path);

	CALLABLE(get_current_point);

	CALLABLE(new_path);
	CALLABLE(new_sub_path);
	CALLABLE(close_path);

	CALLABLE(arc);
	CALLABLE(arc_negative);
	CALLABLE(curve_to);
	CALLABLE(line_to);
	CALLABLE(move_to);
	CALLABLE(rectangle);
	CALLABLE(glyph_path);
	CALLABLE(text_path);
	CALLABLE(rel_curve_to);
	CALLABLE(rel_line_to);
	CALLABLE(rel_move_to);

	// Transformations

	CALLABLE(translate);
	CALLABLE(scale);
	CALLABLE(rotate);

	CALLABLE(transform);
	CALLABLE(set_matrix);
	CALLABLE(get_matrix);
	CALLABLE(identity_matrix);

	CALLABLE(user_to_device);
	CALLABLE(user_to_device_distance);
	CALLABLE(device_to_user);
	CALLABLE(device_to_user_distance);

	// Text/font operations

	CALLABLE(select_font_face);

	CALLABLE(set_font_size);
	CALLABLE(set_font_matrix);
	CALLABLE(get_font_matrix);

	CALLABLE(set_font_options);
	CALLABLE(get_font_options);

	CALLABLE(set_font_face);
	CALLABLE(get_font_face);

	CALLABLE(set_scaled_font);
	CALLABLE(get_scaled_font);

	CALLABLE(show_text);
	CALLABLE(show_glyphs);

	CALLABLE(font_extents);
	CALLABLE(text_extents);
	CALLABLE(glyph_extents);

protected:
	const char *GetTypeName();

public:
	// Create another reference for a context
	LuaCairoContext(lua_State *L, cairo_t *_context);
	// Destructor
	virtual ~LuaCairoContext();
};


class LuaCairoSurface : public LuaCairoBase<LuaCairoSurface> {
private:
	CALLABLE(create_similar);
	CALLABLE(reference);
	CALLABLE(status);

	// Create Cairo context for this surface
	// This deviates from the regular Cairo API
	CALLABLE(create_context);

	CALLABLE(finish);
	CALLABLE(flush);

	CALLABLE(get_font_options);

	CALLABLE(get_content);

	CALLABLE(mark_dirty);
	CALLABLE(mark_dirty_rectangle);

	CALLABLE(set_device_offset);
	CALLABLE(get_device_offset);

	CALLABLE(set_fallback_resolution);

	CALLABLE(get_type);

	// Image surface functions

	CALLABLE(image_get_format);
	CALLABLE(image_get_width);
	CALLABLE(image_get_height);

	// These replace the get_data and get_stride functions
	CALLABLE(image_set_pixel);
	CALLABLE(image_get_pixel);

protected:
	// Protected because inheriting classes might want it too
	cairo_surface_t *surface;

	const char *GetTypeName();

	// For child classes that will set surface themselves
	LuaCairoSurface(lua_State *L);

public:
	// Create another reference for a surface
	LuaCairoSurface(lua_State *L, cairo_surface_t *_surface);
	// Destructor
	virtual ~LuaCairoSurface();

	cairo_surface_t *GetSurface() { return surface; }

	// Creation functions - these aren't in image surface objects but in a global table
	CALLABLE(image_surface_create);
};


class LuaCairoFontFace : public LuaCairoBase<LuaCairoFontFace> {
private:
	cairo_font_face_t *font_face;

	CALLABLE(create_scaled_font);

	CALLABLE(reference);
	CALLABLE(status);
	CALLABLE(get_type);

protected:
	const char *GetTypeName();

public:
	// Create another reference for a font face
	LuaCairoFontFace(lua_State *L, cairo_font_face_t *_font_face);
	// Destructor
	virtual ~LuaCairoFontFace();

	cairo_font_face_t *GetFontFace() { return font_face; }
};


class LuaCairoScaledFont : public LuaCairoBase<LuaCairoScaledFont> {
private:
	cairo_scaled_font_t *scaled_font;

	CALLABLE(reference);
	CALLABLE(status);

	CALLABLE(extents);
	CALLABLE(text_extents);
	CALLABLE(glyph_extents);

	CALLABLE(get_font_face);
	CALLABLE(get_font_options);
	CALLABLE(get_font_matrix);
	CALLABLE(get_ctm);

	CALLABLE(get_type);

protected:
	const char *GetTypeName();

public:
	// Create another reference for a scaled font
	LuaCairoScaledFont(lua_State *L, cairo_scaled_font_t *_scaled_font);
	// Destructor
	virtual ~LuaCairoScaledFont();

	cairo_scaled_font_t *GetScaledFont() { return scaled_font; }
};


class LuaCairoFontOptions : public LuaCairoBase<LuaCairoFontOptions> {
private:
	cairo_font_options_t *font_options;
	bool owned;

	CALLABLE(copy);
	CALLABLE(status);

	CALLABLE(merge);
	CALLABLE(hash);
	CALLABLE(equal);

	CALLABLE(set_antialias);
	CALLABLE(get_antialias);

	CALLABLE(set_subpixel_order);
	CALLABLE(get_subpixel_order);

	CALLABLE(set_hint_style);
	CALLABLE(get_hint_style);

	CALLABLE(set_hint_metrics);
	CALLABLE(get_hint_metrics);

	void RegFontOptionsCallables();

protected:
	const char *GetTypeName();

public:
	// Create a new font options object - will be owned
	LuaCairoFontOptions(lua_State *L);
	// Wrap an existing font options object - will not be owned
	LuaCairoFontOptions(lua_State *L, cairo_font_options_t *_font_options);
	// Destructor - only destroy font_options if owned
	virtual ~LuaCairoFontOptions();

	cairo_font_options_t *GetFontOptions() { return font_options; }

	// Creation function - global
	CALLABLE(create);
};


class LuaCairoMatrix : public LuaCairoBase<LuaCairoMatrix> {
private:
	cairo_matrix_t matrix;

	CALLABLE(init);
	CALLABLE(init_identity);
	CALLABLE(init_translate);
	CALLABLE(init_scale);
	CALLABLE(init_rotate);

	CALLABLE(translate);
	CALLABLE(scale);
	CALLABLE(rotate);

	// Matrix inversion
	CALLABLE(invert);
	// Matrix multiplication
	CALLABLE(multiply);

	CALLABLE(transform_distance);
	CALLABLE(transform_point);

	// Pointwise arithmetic on matrices - not part of Cairo API
	CALLABLE(op_add);
	CALLABLE(op_sub);
	CALLABLE(op_mul);
	CALLABLE(op_div);
	CALLABLE(op_unm);
	// Equality operator
	CALLABLE(op_eq);

	// Not in Cairo API
	CALLABLE(copy);

	void RegMatrixCallables(lua_State *L);

protected:
	virtual int internal_lua_index(lua_State *L);
	virtual int internal_lua_newindex(lua_State *L);
	const char *GetTypeName();

	// Extend the meta table with various operators
	void CreateMetaTable(lua_State *L);

public:
	// Create new matrix, inited to identity matrix
	LuaCairoMatrix(lua_State *L);
	// Duplicate exixting matrix
	LuaCairoMatrix(lua_State *L, const cairo_matrix_t *_matrix);
	// Destructor
	virtual ~LuaCairoMatrix();

	cairo_matrix_t *GetMatrix();

	// Creation function - global
	CALLABLE(create);
};


class LuaCairoPath : public LuaCairoBase<LuaCairoPath> {
private:
	cairo_path_t *path;

	// Specifies whether the memory for the cairo_path_t object is owned by the Cairo library.
	// If Cairo owns it we cannot add/remove elements from the path.
	bool cairo_owns_memory;

	// Number of path elemts we have allocated memory for. Undefined if cairo_owns_memory.
	// This is different from path->length because that specifies the number of elements in use.
	size_t path_elements_allocated;

	// Management
	void EnsurePathOwned(); // ensure that we own the path memory
	void EnsureSpaceFor(size_t n); // ensure we own the path memory and there's space to add at least n more elements

	// TODO: figure out what methods are needed
	// Something to iterate over the parts at least
	// Support for creating/modifying paths?

	CALLABLE(clear);
	CALLABLE(move_to);
	CALLABLE(line_to);
	CALLABLE(curve_to);
	CALLABLE(close);

	// Functional programming support
	CALLABLE(map); // transform each path segment with a function
	CALLABLE(map_coords); // transform each coordinate pair with a function
	CALLABLE(fold); // fold path segments into a single result value
	CALLABLE(fold_coords); // fold coordinate pairs into a single result value

	void RegPathCallables(lua_State *L);

protected:
	virtual int internal_lua_index(lua_State *L);
	const char *GetTypeName();

public:
	// Create object with new path - we will own the memory
	LuaCairoPath(lua_State *L);
	// Create object based on path - does not copy path, and lets Cairo own the memory
	LuaCairoPath(lua_State *L, cairo_path_t *_path);
	// Destructor
	virtual ~LuaCairoPath();

	cairo_path_t *GetPath() { return path; }

	// Modifying the path
	void ClearPath();
	void MoveTo(double x, double y);
	void LineTo(double x, double y);
	void CurveTo(double x0, double y0, double x1, double y1, double x2, double y2);
	void ClosePath();
};


class LuaCairoPattern : public LuaCairoBase<LuaCairoPattern> {
private:
	cairo_pattern_t *pattern;

	CALLABLE(add_color_stop_rgb);
	CALLABLE(add_color_stop_rgba);
	CALLABLE(get_color_stop_count);
	CALLABLE(get_color_stop_rgba);

	CALLABLE(get_rgba);

	CALLABLE(get_surface);

	CALLABLE(get_linear_points);
	CALLABLE(get_radial_circles);

	CALLABLE(reference);
	CALLABLE(status);

	CALLABLE(set_extend);
	CALLABLE(get_extend);

	CALLABLE(set_filter);
	CALLABLE(get_filter);

	CALLABLE(set_matrix);
	CALLABLE(get_matrix);

	CALLABLE(get_type);

protected:
	const char *GetTypeName();

public:
	// Create another reference for a pattern object
	LuaCairoPattern(lua_State *L, cairo_pattern_t *_pattern);
	// Destructor
	virtual ~LuaCairoPattern();

	cairo_pattern_t *GetPattern() { return pattern; }

	// Creation functions - these aren't in pattern objects but in a global table
	CALLABLE(create_rgb);
	CALLABLE(create_rgba);
	CALLABLE(create_for_surface);
	CALLABLE(create_linear);
	CALLABLE(create_radial);
};


#undef CALLABLE

#endif
