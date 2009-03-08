/*
 * Lua interface for the cairo graphics library
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

#include "cairo_wrap.h"
#include "../lua51/src/lauxlib.h"

#include <stdint.h>

#ifdef WIN32
#include <windows.h>
#endif


// Macros to help implementing callables

#define CALLABLE_IMPL(C,N) int C ## ::lua_ ## N (lua_State *L)
#define CALLABLE_NOTIMPL(C,N) CALLABLE_IMPL(C,N) { lua_pushliteral(L, "Method " # N " in class " # C " is not implemented"); lua_error(L); return 0; }
#define CALLABLE_REG(name) AddCallable(lua_ ## name, # name)
#define CALLABLE_REG2(func,name) AddCallable(lua_ ## func, # name)


// Maps from strings to various cairo enum types

static const char *status_names_list[] = {
	"success",
	"no_memory",
	"invalid_restore", "invalid_pop_group", "no_current_point",
	"invalid_matrix", "invalid_status", "null_pointer",
	"invalid_string", "invalid_path_data",
	"read_error", "write_error",
	"surface_type_mismatch", "pattern_type_mismatch",
	"invalid_content", "invalid_format", "invalid_visual",
	"file_not_found",
	"invalid_dash", "invalid_dsc_comment", "invalid_index",
	"clip_not_representable",
	0 };
static const char *fill_rule_list[] = {"winding", "even_odd", 0};
static const char *line_cap_list[] = {"butt", "round", "square", 0};
static const char *line_join_list[] = {"miter", "round", "bevel", 0};
static const char *antialias_types_list[] = {"default", "none", "gray", "subpixel", 0};
static const char *subpixel_order_list[] ={"default", "rgb", "bgr", "vrgb", "vbgr", 0};
static const char *hint_style_list[] = {"default", "none", "slight", "medium", "full", 0};
static const char *hint_metrics_list[] = {"default", "on", "off", 0};
static const char *content_types_list[] = {"c", "a", "ca", 0};
static const char *operators_list[] = {
	"clear",
	"source", "over", "in", "out", "atop",
	"dest", "dest_over", "dest_in", "dest_out", "dest_atop",
	"xor", "add", "saturate",
	0 };
static const char *font_slant_list[] = {"", "italic", "oblique", 0};
static const char *font_weight_list[] = {"", "bold", 0};
static const char *image_formats_list[] = {"argb32", "rgb24", "a8", "a1", "rgb16_565", 0};
static const char *pattern_extend_list[] = {"none", "repeat", "reflect", "pad", 0};
static const char *pattern_filter_list[] = {"fast", "good", "best", "nearest", "bilinear", "gaussian", 0};
static const char *pattern_type_list[] = {"solid", "surface", "linear", "radial", 0};


// Misc. helper functions

static void font_extents_to_lua(lua_State *L, cairo_font_extents_t &extents)
{
	lua_newtable(L);
	lua_pushnumber(L, extents.ascent);
	lua_setfield(L, -2, "ascent");
	lua_pushnumber(L, extents.descent);
	lua_setfield(L, -2, "descent");
	lua_pushnumber(L, extents.height);
	lua_setfield(L, -2, "height");
	lua_pushnumber(L, extents.max_x_advance);
	lua_setfield(L, -2, "max_x_advance");
	lua_pushnumber(L, extents.max_y_advance);
	lua_setfield(L, -2, "max_y_advance");
}

static void text_extents_to_lua(lua_State *L, cairo_text_extents_t &extents)
{
	lua_newtable(L);
	lua_pushnumber(L, extents.x_bearing);
	lua_setfield(L, -2, "x_bearing");
	lua_pushnumber(L, extents.y_bearing);
	lua_setfield(L, -2, "y_bearing");
	lua_pushnumber(L, extents.width);
	lua_setfield(L, -2, "width");
	lua_pushnumber(L, extents.height);
	lua_setfield(L, -2, "height");
	lua_pushnumber(L, extents.x_advance);
	lua_setfield(L, -2, "x_advance");
	lua_pushnumber(L, extents.y_advance);
	lua_setfield(L, -2, "y_advance");
}


// Context (cairo_t)

CALLABLE_IMPL(LuaCairoContext, reference)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoContext *newctx = new LuaCairoContext(L, ctx->context);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, status)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_status(ctx->context);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, save)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_save(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, restore)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_restore(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_target)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_t *surf = cairo_get_target(ctx->context);
	new LuaCairoSurface(L, surf);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, push_group)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_push_group(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, push_group_with_content)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_content_t ct = (cairo_content_t)luaL_checkoption(L, 1, "ca", content_types_list);
	cairo_push_group_with_content(ctx->context, ct);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, pop_group)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_pattern_t *pat = cairo_pop_group(ctx->context);
	LuaCairoPattern *patobj = new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, pop_group_to_source)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_pop_group_to_source(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_group_target)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_t *surf = cairo_get_group_target(ctx->context);
	new LuaCairoSurface(L, surf);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_source_rgb)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	cairo_set_source_rgb(ctx->context, r, g, b);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_source_rgba)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double r = luaL_checknumber(L, 1);
	double g = luaL_checknumber(L, 2);
	double b = luaL_checknumber(L, 3);
	double a = luaL_checknumber(L, 4);
	cairo_set_source_rgba(ctx->context, r, g, b, a);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_source)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoPattern *pat = LuaCairoPattern::GetObjPointer(L, 1);
	cairo_set_source(ctx->context, pat->GetPattern());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_source_surface)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoSurface *surf = LuaCairoSurface::GetObjPointer(L, 1);
	double surfx = luaL_checknumber(L, 2);
	double surfy = luaL_checknumber(L, 3);
	cairo_set_source_surface(ctx->context, surf->GetSurface(), surfx, surfy);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_source)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_pattern_t *pat = cairo_get_source(ctx->context);
	// The pattern is owned by cairo here, so creating the Lua object will create the needed reference from Lua
	new LuaCairoPattern(L, pat);
	// Meaning that calling pattern_destroy here would be wrong
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_antialias)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_antialias_t aa = (cairo_antialias_t)luaL_checkoption(L, 1, NULL, antialias_types_list);
	cairo_set_antialias(ctx->context, aa);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_antialias)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_antialias_t aa = cairo_get_antialias(ctx->context);
	lua_pushstring(L, antialias_types_list[aa]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_dash)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	
	if (lua_isnoneornil(L, 1)) {
		// left out parameters, disable dashing
		cairo_set_dash(ctx->context, NULL, 0, 0);
		return 0;

	} else if (lua_istable(L, 1)) {
		// dashing pattern in table, complex case
		double offset = luaL_optnumber(L, 2, 0);
		size_t num_dashes = lua_objlen(L, 1);
		double *dashes = new double[num_dashes];
		size_t i = 0;
		lua_pushnil(L);
		while (lua_next(L, 1)) {
			if (lua_isnumber(L, -1)) {
				dashes[i] = lua_tonumber(L, -1);
			} else {
				dashes[i] = 0;
			}
			lua_pop(L, 1);
		}
		cairo_set_dash(ctx->context, dashes, (int)num_dashes, offset);
		delete[] dashes;
		return 0;

	} else if (lua_isnumber(L, 1)) {
		// single dash, alternating on/off
		double dash = luaL_checknumber(L, 1);
		double offset = luaL_optnumber(L, 2, 0);
		if (dash > 0) {
			cairo_set_dash(ctx->context, &dash, 1, offset);
		} else {
			// ok number was invalid, disable dashing instead of putting context into error state
			cairo_set_dash(ctx->context, NULL, 0, 0);
		}
		return 0;

	} else {
		luaL_error(L, "First argument to set_dash must be nil, table or number, was %s", luaL_typename(L, 1));
		return 0;
	}
}

CALLABLE_IMPL(LuaCairoContext, get_dash_count)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	int dash_count = cairo_get_dash_count(ctx->context);
	lua_pushinteger(L, dash_count);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, get_dash)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	int dash_count = cairo_get_dash_count(ctx->context);

	if (dash_count <= 0) {
		lua_newtable(L);
		lua_pushnumber(L, 0);
		return 2;
	}

	double *dashes = new double[dash_count];
	double offset;
	cairo_get_dash(ctx->context, dashes, &offset);
	lua_newtable(L);
	for (int i = 0; i < dash_count; i++) {
		lua_pushnumber(L, dashes[i]);
		lua_rawseti(L, -2, i+1);
	}
	delete[] dashes;
	lua_pushnumber(L, offset);
	return 2;
}

CALLABLE_IMPL(LuaCairoContext, set_fill_rule)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_fill_rule_t fr = (cairo_fill_rule_t)luaL_checkoption(L, 1, NULL, fill_rule_list);
	cairo_set_fill_rule(ctx->context, fr);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_fill_rule)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_fill_rule_t fr = cairo_get_fill_rule(ctx->context);
	lua_pushstring(L, fill_rule_list[fr]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_line_cap)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_line_cap_t lc = (cairo_line_cap_t)luaL_checkoption(L, 1, NULL, line_cap_list);
	cairo_set_line_cap(ctx->context, lc);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_line_cap)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_line_cap_t lc = cairo_get_line_cap(ctx->context);
	lua_pushstring(L, line_cap_list[lc]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_line_join)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_line_join_t lj = (cairo_line_join_t)luaL_checkoption(L, 1, NULL, line_join_list);
	cairo_set_line_join(ctx->context, lj);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_line_join)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_line_join_t lj = cairo_get_line_join(ctx->context);
	lua_pushstring(L, line_join_list[lj]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_line_width)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double w = luaL_checknumber(L, 1);
	cairo_set_line_width(ctx->context, w);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_line_width)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double w = cairo_get_line_width(ctx->context);
	lua_pushnumber(L, w);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_miter_limit)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double l = luaL_checknumber(L, 1);
	cairo_set_miter_limit(ctx->context, l);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_miter_limit)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double l = cairo_get_miter_limit(ctx->context);
	lua_pushnumber(L, l);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_operator)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_operator_t op = (cairo_operator_t)luaL_checkoption(L, 1, NULL, operators_list);
	cairo_set_operator(ctx->context, op);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_operator)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_operator_t op = cairo_get_operator(ctx->context);
	lua_pushstring(L, operators_list[op]);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_tolerance)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double tol = luaL_checknumber(L, 1);
	cairo_set_tolerance(ctx->context, tol);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_tolerance)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double tol = cairo_get_tolerance(ctx->context);
	lua_pushnumber(L, tol);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, clip)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_clip(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, clip_preserve)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_clip_preserve(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, clip_extents)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1, y1, x2, y2;
	cairo_clip_extents(ctx->context, &x1, &y1, &x2, &y2);
	lua_pushnumber(L, x1);
	lua_pushnumber(L, y1);
	lua_pushnumber(L, x2);
	lua_pushnumber(L, y2);
	return 4;
}

CALLABLE_IMPL(LuaCairoContext, reset_clip)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_reset_clip(ctx->context);
	return 0;
}

CALLABLE_NOTIMPL(LuaCairoContext, copy_clip_rectangle_list)

CALLABLE_IMPL(LuaCairoContext, fill)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_fill(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, fill_preserve)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_fill_preserve(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, fill_extents)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1, y1, x2, y2;
	cairo_fill_extents(ctx->context, &x1, &y1, &x2, &y2);
	lua_pushnumber(L, x1);
	lua_pushnumber(L, y1);
	lua_pushnumber(L, x2);
	lua_pushnumber(L, y2);
	return 4;
}

CALLABLE_IMPL(LuaCairoContext, in_fill)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	lua_pushboolean(L, cairo_in_fill(ctx->context, x, y));
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, mask)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoPattern *pat = LuaCairoPattern::GetObjPointer(L, 1);
	cairo_mask(ctx->context, pat->GetPattern());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, mask_surface)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoSurface *surf = LuaCairoSurface::GetObjPointer(L, 1);
	double surfx = luaL_checknumber(L, 2);
	double surfy = luaL_checknumber(L, 3);
	cairo_mask_surface(ctx->context, surf->GetSurface(), surfx, surfy);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, paint)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_paint(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, paint_with_alpha)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double a = luaL_checknumber(L, 1);
	cairo_paint_with_alpha(ctx->context, a);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, stroke)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_stroke(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, stroke_preserve)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_stroke_preserve(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, stroke_extents)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1, y1, x2, y2;
	cairo_stroke_extents(ctx->context, &x1, &y1, &x2, &y2);
	lua_pushnumber(L, x1);
	lua_pushnumber(L, y1);
	lua_pushnumber(L, x2);
	lua_pushnumber(L, y2);
	return 4;
}

CALLABLE_IMPL(LuaCairoContext, in_stroke)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	lua_pushboolean(L, cairo_in_stroke(ctx->context, x, y));
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, copy_page)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_copy_page(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, show_page)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_show_page(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, copy_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_path_t *path = cairo_copy_path(ctx->context);
	new LuaCairoPath(L, path);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, copy_path_flat)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_path_t *path = cairo_copy_path_flat(ctx->context);
	new LuaCairoPath(L, path);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, append_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoPath *path = LuaCairoPath::GetObjPointer(L, 1);
	cairo_append_path(ctx->context, path->GetPath());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_current_point)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x, y;
	cairo_get_current_point(ctx->context, &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

CALLABLE_IMPL(LuaCairoContext, new_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_new_path(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, new_sub_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_new_sub_path(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, close_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_close_path(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, arc)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double xc = luaL_checknumber(L, 1);
	double yc = luaL_checknumber(L, 2);
	double radius = luaL_checknumber(L, 3);
	double angle1 = luaL_checknumber(L, 4);
	double angle2 = luaL_checknumber(L, 5);
	cairo_arc(ctx->context, xc, yc, radius, angle1, angle2);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, arc_negative)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double xc = luaL_checknumber(L, 1);
	double yc = luaL_checknumber(L, 2);
	double radius = luaL_checknumber(L, 3);
	double angle1 = luaL_checknumber(L, 4);
	double angle2 = luaL_checknumber(L, 5);
	cairo_arc_negative(ctx->context, xc, yc, radius, angle1, angle2);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, curve_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double x2 = luaL_checknumber(L, 3);
	double y2 = luaL_checknumber(L, 4);
	double x3 = luaL_checknumber(L, 5);
	double y3 = luaL_checknumber(L, 6);
	cairo_curve_to(ctx->context, x1, y1, x2, y2, x3, y3);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, line_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_line_to(ctx->context, x, y);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, move_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_move_to(ctx->context, x, y);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, rectangle)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double x2 = luaL_checknumber(L, 3);
	double y2 = luaL_checknumber(L, 4);
	cairo_rectangle(ctx->context, x1, y1, x2, y2);
	return 0;
}

CALLABLE_NOTIMPL(LuaCairoContext, glyph_path)

CALLABLE_IMPL(LuaCairoContext, text_path)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	const char *utf8 = luaL_checkstring(L, 1);
	cairo_text_path(ctx->context, utf8);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, rel_curve_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double x2 = luaL_checknumber(L, 3);
	double y2 = luaL_checknumber(L, 4);
	double x3 = luaL_checknumber(L, 5);
	double y3 = luaL_checknumber(L, 6);
	cairo_rel_curve_to(ctx->context, x1, y1, x2, y2, x3, y3);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, rel_line_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_rel_line_to(ctx->context, x, y);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, rel_move_to)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_rel_move_to(ctx->context, x, y);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, translate)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double tx = luaL_checknumber(L, 1);
	double ty = luaL_checknumber(L, 2);
	cairo_translate(ctx->context, tx, ty);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, scale)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double sx = luaL_checknumber(L, 1);
	double sy = luaL_checknumber(L, 2);
	cairo_scale(ctx->context, sx, sy);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, rotate)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double rads = luaL_checknumber(L, 1);
	cairo_rotate(ctx->context, rads);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, transform)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = LuaCairoMatrix::GetObjPointer(L, 1);
	cairo_transform(ctx->context, mat->GetMatrix());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_matrix)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = LuaCairoMatrix::GetObjPointer(L, 1);
	cairo_set_matrix(ctx->context, mat->GetMatrix());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_matrix)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_matrix_t mat;
	cairo_get_matrix(ctx->context, &mat);
	new LuaCairoMatrix(L, &mat);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, identity_matrix)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_identity_matrix(ctx->context);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, user_to_device)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_user_to_device(ctx->context, &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

CALLABLE_IMPL(LuaCairoContext, user_to_device_distance)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_user_to_device_distance(ctx->context, &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

CALLABLE_IMPL(LuaCairoContext, device_to_user)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_device_to_user(ctx->context,& x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

CALLABLE_IMPL(LuaCairoContext, device_to_user_distance)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_device_to_user_distance(ctx->context, &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;

}

CALLABLE_IMPL(LuaCairoContext, select_font_face)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	const char *family = luaL_checkstring(L, 1);
	cairo_font_slant_t slant = (cairo_font_slant_t)luaL_checkoption(L, 2, "", font_slant_list);
	cairo_font_weight_t weight = (cairo_font_weight_t)luaL_checkoption(L, 3, "", font_weight_list);
	cairo_select_font_face(ctx->context, family, slant, weight);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_font_size)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	double sz = luaL_checknumber(L, 1);
	cairo_set_font_size(ctx->context, sz);
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, set_font_matrix)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = LuaCairoMatrix::GetObjPointer(L, 1);
	cairo_set_font_matrix(ctx->context, mat->GetMatrix());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_font_matrix)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = new LuaCairoMatrix(L);
	cairo_get_font_matrix(ctx->context, mat->GetMatrix());
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_font_options)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *fo = LuaCairoFontOptions::GetObjPointer(L, 1);
	cairo_set_font_options(ctx->context, fo->GetFontOptions());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_font_options)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *fo = new LuaCairoFontOptions(L);
	cairo_get_font_options(ctx->context, fo->GetFontOptions());
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_font_face)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontFace *face = LuaCairoFontFace::GetObjPointer(L, 1);
	cairo_set_font_face(ctx->context, face->GetFontFace());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_font_face)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_font_face_t *face = cairo_get_font_face(ctx->context);
	new LuaCairoFontFace(L, face);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, set_scaled_font)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoScaledFont *sf = LuaCairoScaledFont::GetObjPointer(L, 1);
	cairo_set_scaled_font(ctx->context, sf->GetScaledFont());
	return 0;
}

CALLABLE_IMPL(LuaCairoContext, get_scaled_font)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_scaled_font_t *sf = cairo_get_scaled_font(ctx->context);
	new LuaCairoScaledFont(L, sf);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, show_text)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	const char *utf8 = luaL_checkstring(L, 1);
	cairo_show_text(ctx->context, utf8);
	return 0;
}

CALLABLE_NOTIMPL(LuaCairoContext, show_glyphs)

CALLABLE_IMPL(LuaCairoContext, font_extents)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	cairo_font_extents_t extents;
	cairo_font_extents(ctx->context, &extents);
	font_extents_to_lua(L, extents);
	return 1;
}

CALLABLE_IMPL(LuaCairoContext, text_extents)
{
	LuaCairoContext *ctx = GetObjPointer(L, lua_upvalueindex(1));
	const char *utf8 = luaL_checkstring(L, 1);
	cairo_text_extents_t extents;
	cairo_text_extents(ctx->context, utf8, &extents);
	text_extents_to_lua(L, extents);
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoContext, glyph_extents)

const char *LuaCairoContext::GetTypeName()
{
	return "context";
}

LuaCairoContext::LuaCairoContext(lua_State *L, cairo_t *_context) :
	LuaCairoBase(L)
{
	CALLABLE_REG(reference);
	CALLABLE_REG(status);
	CALLABLE_REG(save);
	CALLABLE_REG(restore);
	CALLABLE_REG(get_target);
	CALLABLE_REG(push_group);
	CALLABLE_REG(push_group_with_content);
	CALLABLE_REG(pop_group);
	CALLABLE_REG(pop_group_to_source);
	CALLABLE_REG(get_group_target);
	CALLABLE_REG(set_source_rgb);
	CALLABLE_REG(set_source_rgba);
	CALLABLE_REG(set_source);
	CALLABLE_REG(set_source_surface);
	CALLABLE_REG(get_source);
	CALLABLE_REG(set_antialias);
	CALLABLE_REG(get_antialias);
	CALLABLE_REG(set_dash);
	CALLABLE_REG(get_dash_count);
	CALLABLE_REG(get_dash);
	CALLABLE_REG(set_fill_rule);
	CALLABLE_REG(get_fill_rule);
	CALLABLE_REG(set_line_cap);
	CALLABLE_REG(get_line_cap);
	CALLABLE_REG(set_line_join);
	CALLABLE_REG(get_line_join);
	CALLABLE_REG(set_line_width);
	CALLABLE_REG(get_line_width);
	CALLABLE_REG(set_miter_limit);
	CALLABLE_REG(get_miter_limit);
	CALLABLE_REG(set_operator);
	CALLABLE_REG(get_operator);
	CALLABLE_REG(set_tolerance);
	CALLABLE_REG(get_tolerance);
	CALLABLE_REG(clip);
	CALLABLE_REG(clip_preserve);
	CALLABLE_REG(clip_extents);
	CALLABLE_REG(reset_clip);
	CALLABLE_REG(copy_clip_rectangle_list);
	CALLABLE_REG(fill);
	CALLABLE_REG(fill_preserve);
	CALLABLE_REG(fill_extents);
	CALLABLE_REG(in_fill);
	CALLABLE_REG(mask);
	CALLABLE_REG(mask_surface);
	CALLABLE_REG(paint);
	CALLABLE_REG(paint_with_alpha);
	CALLABLE_REG(stroke);
	CALLABLE_REG(stroke_preserve);
	CALLABLE_REG(stroke_extents);
	CALLABLE_REG(in_stroke);
	CALLABLE_REG(copy_page);
	CALLABLE_REG(show_page);
	CALLABLE_REG(copy_path);
	CALLABLE_REG(copy_path_flat);
	CALLABLE_REG(append_path);
	CALLABLE_REG(get_current_point);
	CALLABLE_REG(new_path);
	CALLABLE_REG(new_sub_path);
	CALLABLE_REG(close_path);
	CALLABLE_REG(arc);
	CALLABLE_REG(arc_negative);
	CALLABLE_REG(curve_to);
	CALLABLE_REG(line_to);
	CALLABLE_REG(move_to);
	CALLABLE_REG(rectangle);
	CALLABLE_REG(glyph_path);
	CALLABLE_REG(text_path);
	CALLABLE_REG(rel_curve_to);
	CALLABLE_REG(rel_line_to);
	CALLABLE_REG(rel_move_to);
	CALLABLE_REG(translate);
	CALLABLE_REG(scale);
	CALLABLE_REG(rotate);
	CALLABLE_REG(transform);
	CALLABLE_REG(set_matrix);
	CALLABLE_REG(get_matrix);
	CALLABLE_REG(identity_matrix);
	CALLABLE_REG(user_to_device);
	CALLABLE_REG(user_to_device_distance);
	CALLABLE_REG(device_to_user);
	CALLABLE_REG(device_to_user_distance);
	CALLABLE_REG(select_font_face);
	CALLABLE_REG(set_font_size);
	CALLABLE_REG(set_font_matrix);
	CALLABLE_REG(get_font_matrix);
	CALLABLE_REG(set_font_options);
	CALLABLE_REG(get_font_options);
	CALLABLE_REG(set_font_face);
	CALLABLE_REG(get_font_face);
	CALLABLE_REG(set_scaled_font);
	CALLABLE_REG(get_scaled_font);
	CALLABLE_REG(show_text);
	CALLABLE_REG(show_glyphs);
	CALLABLE_REG(font_extents);
	CALLABLE_REG(text_extents);
	CALLABLE_REG(glyph_extents);

	cairo_reference(_context);
	context = _context;
}

LuaCairoContext::~LuaCairoContext()
{
	cairo_destroy(context);
}


// Surface (cairo_surface_t)

CALLABLE_IMPL(LuaCairoSurface, create_similar)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_content_t ct = (cairo_content_t)luaL_checkoption(L, 1, NULL, content_types_list);
	int width = luaL_checkint(L, 2);
	int height = luaL_checkint(L, 3);
	cairo_surface_t *nsurf = cairo_surface_create_similar(surf->surface, ct, width, height);
	new LuaCairoSurface(L, nsurf);
	cairo_surface_destroy(nsurf);
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoSurface, reference)

CALLABLE_IMPL(LuaCairoSurface, status)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_surface_status(surf->surface);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_IMPL(LuaCairoSurface, create_context)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));

	cairo_t *context = cairo_create(surf->surface);

	if (!context || cairo_status(context) != CAIRO_STATUS_SUCCESS) {
		lua_pushliteral(L, "Failed creating cairo context");
		lua_error(L);
		return 0;
	}

	LuaCairoContext *ctxojb = new LuaCairoContext(L, context);
	cairo_destroy(context);
	return 1;
}

CALLABLE_IMPL(LuaCairoSurface, finish)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_finish(surf->surface);
	return 0;
}

CALLABLE_IMPL(LuaCairoSurface, flush)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_flush(surf->surface);
	return 0;
}

CALLABLE_IMPL(LuaCairoSurface, get_font_options)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *fo = new LuaCairoFontOptions(L);
	cairo_surface_get_font_options(surf->surface, fo->GetFontOptions());
	return 1;
}

CALLABLE_IMPL(LuaCairoSurface, get_content)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_content_t ct = cairo_surface_get_content(surf->surface);
	lua_pushstring(L, content_types_list[ct]);
	return 1;
}

CALLABLE_IMPL(LuaCairoSurface, mark_dirty)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_mark_dirty(surf->surface);
	return 0;
}

CALLABLE_IMPL(LuaCairoSurface, mark_dirty_rectangle)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	int width = luaL_checkint(L, 3);
	int height = luaL_checkint(L, 4);
	cairo_surface_mark_dirty_rectangle(surf->surface, x, y, width, height);
	return 0;
}

CALLABLE_IMPL(LuaCairoSurface, set_device_offset)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	double xofs = luaL_checknumber(L, 1);
	double yofs = luaL_checknumber(L, 2);
	cairo_surface_set_device_offset(surf->surface, xofs, yofs);
	return 0;
}

CALLABLE_IMPL(LuaCairoSurface, get_device_offset)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	double xofs, yofs;
	cairo_surface_get_device_offset(surf->surface, &xofs, &yofs);
	lua_pushnumber(L, xofs);
	lua_pushnumber(L, yofs);
	return 2;
}

CALLABLE_IMPL(LuaCairoSurface, set_fallback_resolution)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	double xppi = luaL_checknumber(L, 1);
	double yppi = luaL_checknumber(L, 2);
	cairo_surface_set_device_offset(surf->surface, xppi, yppi);
	return 0;
}

CALLABLE_NOTIMPL(LuaCairoSurface, get_type)

CALLABLE_IMPL(LuaCairoSurface, image_get_format)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	if (cairo_surface_get_type(surf->GetSurface()) != CAIRO_SURFACE_TYPE_IMAGE) {
		lua_pushliteral(L, "Surface is not an image surface");
		lua_error(L);
		return 0;
	}

	switch (cairo_image_surface_get_format(surf->GetSurface())) {
		case CAIRO_FORMAT_ARGB32:
			lua_pushliteral(L, "argb32");
			return 1;
		case CAIRO_FORMAT_RGB24:
			lua_pushliteral(L, "rgb24");
			return 1;
		case CAIRO_FORMAT_A8:
			lua_pushliteral(L, "a8");
			return 1;
		case CAIRO_FORMAT_A1:
			lua_pushliteral(L, "a1");
			return 1;
		default:
			lua_pushnil(L);
			return 1;
	}
}

CALLABLE_IMPL(LuaCairoSurface, image_get_width)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	if (cairo_surface_get_type(surf->GetSurface()) != CAIRO_SURFACE_TYPE_IMAGE) {
		lua_pushliteral(L, "Surface is not an image surface");
		lua_error(L);
		return 0;
	}

	lua_pushinteger(L, cairo_image_surface_get_width(surf->GetSurface()));
	return 1;
}

CALLABLE_IMPL(LuaCairoSurface, image_get_height)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	if (cairo_surface_get_type(surf->GetSurface()) != CAIRO_SURFACE_TYPE_IMAGE) {
		lua_pushliteral(L, "Surface is not an image surface");
		lua_error(L);
		return 0;
	}

	lua_pushinteger(L, cairo_image_surface_get_height(surf->GetSurface()));
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoSurface, image_set_pixel)

CALLABLE_IMPL(LuaCairoSurface, image_get_pixel)
{
	LuaCairoSurface *surf = GetObjPointer(L, lua_upvalueindex(1));
	int x = luaL_checkint(L, 1);
	int y = luaL_checkint(L, 2);
	if (cairo_surface_get_type(surf->surface) != CAIRO_SURFACE_TYPE_IMAGE) {
		lua_pushliteral(L, "Surface is not an image surface");
		lua_error(L);
		return 0;
	}

	// Assume the surface is already flush()ed
	int width = cairo_image_surface_get_width(surf->surface);
	int height = cairo_image_surface_get_height(surf->surface);
	int stride = cairo_image_surface_get_stride(surf->surface);
	cairo_format_t format = cairo_image_surface_get_format(surf->surface);
	unsigned char *data = cairo_image_surface_get_data(surf->surface);

	switch (format) {
		case CAIRO_FORMAT_ARGB32: {
			uint32_t pixel = *(uint32_t*)(data + y*stride + x * 4);
			lua_pushinteger(L, (pixel & 0xff000000)>>24); // alpha
			lua_pushinteger(L, (pixel & 0x00ff0000)>>16); // red
			lua_pushinteger(L, (pixel & 0x0000ff00)>>8); // green
			lua_pushinteger(L, pixel & 0x000000ff); // blue
			return 4; }

		case CAIRO_FORMAT_RGB24: {
			uint32_t pixel = *(uint32_t*)(data + y*stride + x * 4);
			lua_pushinteger(L, (pixel & 0x00ff0000)>>16); // red
			lua_pushinteger(L, (pixel & 0x0000ff00)>>8); // green
			lua_pushinteger(L, pixel & 0x000000ff); // blue
			return 3; }

		case CAIRO_FORMAT_A8:
			lua_pushinteger(L, data[y*stride+x]);
			return 1;

		default:
			lua_pushliteral(L, "Unhandled pixel format in image surface get pixel");
			lua_error(L);
			return 0;
	}
}

CALLABLE_IMPL(LuaCairoSurface, image_surface_create)
{
	// Swap arguments a bit and make a default value for the format
	int width = luaL_checkint(L, 1);
	int height = luaL_checkint(L, 2);
	cairo_format_t format = (cairo_format_t)luaL_checkoption(L, 3, "argb32", image_formats_list);

	// Create surface from parameters
	cairo_surface_t *surf = cairo_image_surface_create(format, width, height);

	// Check that the surface was successfully created
	if (!surf || cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS)
		return 0;

	// Create wrapping Lua object
	LuaCairoSurface *surfobj = new LuaCairoSurface(L, surf);
	// Lua object takes its own reference to surface, so release ours
	cairo_surface_destroy(surf);
	// Return surface object
	return 1;
}

const char *LuaCairoSurface::GetTypeName()
{
	return "surface";
}

LuaCairoSurface::LuaCairoSurface(lua_State *L, cairo_surface_t *_surface) :
	LuaCairoBase(L)
{
	cairo_surface_reference(_surface);
	surface = _surface;

	cairo_surface_type_t st = cairo_surface_get_type(surface);

	CALLABLE_REG(create_similar);
	CALLABLE_REG(reference);
	CALLABLE_REG(status);
	CALLABLE_REG(create_context);
	CALLABLE_REG(finish);
	CALLABLE_REG(flush);
	CALLABLE_REG(get_font_options);
	CALLABLE_REG(get_content);
	CALLABLE_REG(mark_dirty);
	CALLABLE_REG(mark_dirty_rectangle);
	CALLABLE_REG(set_device_offset);
	CALLABLE_REG(get_device_offset);
	CALLABLE_REG(set_fallback_resolution);
	CALLABLE_REG(get_type);

	if (st == CAIRO_SURFACE_TYPE_IMAGE) {
		CALLABLE_REG2(image_get_format, get_format);
		CALLABLE_REG2(image_get_width, get_width);
		CALLABLE_REG2(image_get_height, get_height);
		CALLABLE_REG2(image_set_pixel, set_pixel);
		CALLABLE_REG2(image_get_pixel, get_pixel);
	}
}

LuaCairoSurface::~LuaCairoSurface()
{
	cairo_surface_destroy(surface);
}


// Font face (cairo_font_face_t)

CALLABLE_NOTIMPL(LuaCairoFontFace, create_scaled_font)
CALLABLE_NOTIMPL(LuaCairoFontFace, reference)

CALLABLE_IMPL(LuaCairoFontFace, status)
{
	LuaCairoFontFace *face = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_font_face_status(face->font_face);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoFontFace, get_type)

const char *LuaCairoFontFace::GetTypeName()
{
	return "font_face";
}

LuaCairoFontFace::LuaCairoFontFace(lua_State *L, cairo_font_face_t *_font_face) :
	LuaCairoBase(L)
{
	CALLABLE_REG(create_scaled_font);
	CALLABLE_REG(reference);
	CALLABLE_REG(status);
	CALLABLE_REG(get_type);

	cairo_font_face_reference(_font_face);
	font_face = _font_face;
}

LuaCairoFontFace::~LuaCairoFontFace()
{
	cairo_font_face_destroy(font_face);
}


// Scaled font (cairo_scaled_font_t)

CALLABLE_NOTIMPL(LuaCairoScaledFont, reference)

CALLABLE_IMPL(LuaCairoScaledFont, status)
{
	LuaCairoScaledFont*sf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_scaled_font_status(sf->scaled_font);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_IMPL(LuaCairoScaledFont, extents)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_font_extents_t extents;
	cairo_scaled_font_extents(scf->scaled_font, &extents);
	font_extents_to_lua(L, extents);
	return 1;
}

CALLABLE_IMPL(LuaCairoScaledFont, text_extents)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	const char *utf8 = luaL_checkstring(L, 1);
	cairo_text_extents_t extents;
	cairo_scaled_font_text_extents(scf->scaled_font, utf8, &extents);
	text_extents_to_lua(L, extents);
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoScaledFont, glyph_extents)

CALLABLE_IMPL(LuaCairoScaledFont, get_font_face)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	cairo_font_face_t *face = cairo_scaled_font_get_font_face(scf->scaled_font);
	new LuaCairoFontFace(L, face);
	return 1;
}

CALLABLE_IMPL(LuaCairoScaledFont, get_font_options)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *opt = new LuaCairoFontOptions(L);
	cairo_scaled_font_get_font_options(scf->scaled_font, opt->GetFontOptions());
	return 1;
}

CALLABLE_IMPL(LuaCairoScaledFont, get_font_matrix)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = new LuaCairoMatrix(L);
	cairo_scaled_font_get_font_matrix(scf->scaled_font, mat->GetMatrix());
	return 1;
}

CALLABLE_IMPL(LuaCairoScaledFont, get_ctm)
{
	LuaCairoScaledFont *scf = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = new LuaCairoMatrix(L);
	cairo_scaled_font_get_ctm(scf->scaled_font, mat->GetMatrix());
	return 1;
}

CALLABLE_NOTIMPL(LuaCairoScaledFont, get_type)

const char *LuaCairoScaledFont::GetTypeName()
{
	return "scaled_font";
}

LuaCairoScaledFont::LuaCairoScaledFont(lua_State *L, cairo_scaled_font_t *_scaled_font) :
	LuaCairoBase(L)
{
	CALLABLE_REG(reference);
	CALLABLE_REG(status);
	CALLABLE_REG(extents);
	CALLABLE_REG(text_extents);
	CALLABLE_REG(glyph_extents);
	CALLABLE_REG(get_font_face);
	CALLABLE_REG(get_font_options);
	CALLABLE_REG(get_font_matrix);
	CALLABLE_REG(get_ctm);
	CALLABLE_REG(get_type);

	cairo_scaled_font_reference(_scaled_font);
	scaled_font = _scaled_font;
}

LuaCairoScaledFont::~LuaCairoScaledFont()
{
	cairo_scaled_font_destroy(scaled_font);
}


// Font options (cairo_font_options_t)

CALLABLE_IMPL(LuaCairoFontOptions, copy)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_font_options_t *nfo = cairo_font_options_copy(fo->font_options);
	new LuaCairoFontOptions(L, nfo);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, status)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_font_options_status(fo->font_options);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, merge)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *other = GetObjPointer(L, 1);
	cairo_font_options_merge(fo->font_options, other->font_options);
	return 0;
}

CALLABLE_IMPL(LuaCairoFontOptions, hash)
{
	char hash[sizeof(unsigned long)*2+1]; // hex + null
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	unsigned long lhash = cairo_font_options_hash(fo->font_options);
	sprintf(hash, "%0*x", sizeof(unsigned long)*2, lhash);
	lua_pushstring(L, hash);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, equal)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoFontOptions *other = GetObjPointer(L, 1);
	cairo_bool_t eq = cairo_font_options_equal(fo->font_options, other->font_options);
	lua_pushboolean(L, eq);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, set_antialias)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_antialias_t aa = (cairo_antialias_t)luaL_checkoption(L, 1, NULL, antialias_types_list);
	cairo_font_options_set_antialias(fo->font_options, aa);
	return 0;
}

CALLABLE_IMPL(LuaCairoFontOptions, get_antialias)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_antialias_t aa = cairo_font_options_get_antialias(fo->font_options);
	lua_pushstring(L, antialias_types_list[aa]);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, set_subpixel_order)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_subpixel_order_t spo = (cairo_subpixel_order_t)luaL_checkoption(L, 1, NULL, subpixel_order_list);
	cairo_font_options_set_subpixel_order(fo->font_options, spo);
	return 0;
}

CALLABLE_IMPL(LuaCairoFontOptions, get_subpixel_order)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_subpixel_order_t spo = cairo_font_options_get_subpixel_order(fo->font_options);
	lua_pushstring(L, subpixel_order_list[spo]);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, set_hint_style)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_hint_style_t hs = (cairo_hint_style_t)luaL_checkoption(L, 1, NULL, hint_style_list);
	cairo_font_options_set_hint_style(fo->font_options, hs);
	return 0;
}

CALLABLE_IMPL(LuaCairoFontOptions, get_hint_style)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_hint_style_t hs = cairo_font_options_get_hint_style(fo->font_options);
	lua_pushstring(L, hint_style_list[hs]);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, set_hint_metrics)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_hint_metrics_t hm = (cairo_hint_metrics_t)luaL_checkoption(L, 1, NULL, hint_metrics_list);
	cairo_font_options_set_hint_metrics(fo->font_options, hm);
	return 0;
}

CALLABLE_IMPL(LuaCairoFontOptions, get_hint_metrics)
{
	LuaCairoFontOptions *fo = GetObjPointer(L, lua_upvalueindex(1));
	cairo_hint_metrics_t hm = cairo_font_options_get_hint_metrics(fo->font_options);
	lua_pushstring(L, hint_metrics_list[hm]);
	return 1;
}

CALLABLE_IMPL(LuaCairoFontOptions, create)
{
	new LuaCairoFontOptions(L);
	return 1;
}

void LuaCairoFontOptions::RegFontOptionsCallables()
{
	CALLABLE_REG(copy);
	CALLABLE_REG(status);
	CALLABLE_REG(merge);
	CALLABLE_REG(hash);
	CALLABLE_REG(equal);
	CALLABLE_REG(set_antialias);
	CALLABLE_REG(get_antialias);
	CALLABLE_REG(set_subpixel_order);
	CALLABLE_REG(get_subpixel_order);
	CALLABLE_REG(set_hint_style);
	CALLABLE_REG(get_hint_style);
	CALLABLE_REG(set_hint_metrics);
	CALLABLE_REG(get_hint_metrics);
}

const char *LuaCairoFontOptions::GetTypeName()
{
	return "font_options";
}

LuaCairoFontOptions::LuaCairoFontOptions(lua_State *L) :
	LuaCairoBase(L)
{
	RegFontOptionsCallables();

	owned = true;
	font_options = cairo_font_options_create();
}

LuaCairoFontOptions::LuaCairoFontOptions(lua_State *L, cairo_font_options_t *_font_options) :
	LuaCairoBase(L)
{
	RegFontOptionsCallables();

	owned = false;
	font_options = _font_options;
}

LuaCairoFontOptions::~LuaCairoFontOptions()
{
	if (owned)
		cairo_font_options_destroy(font_options);
}


// Matrix (cairo_matrix_t)

CALLABLE_IMPL(LuaCairoMatrix, init)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double xx = luaL_checknumber(L, 1);
	double yx = luaL_checknumber(L, 2);
	double xy = luaL_checknumber(L, 3);
	double yy = luaL_checknumber(L, 4);
	double x0 = luaL_checknumber(L, 5);
	double y0 = luaL_checknumber(L, 6);
	cairo_matrix_init(&mat->matrix, xx, yx, xy, yy, x0, y0);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, init_identity)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	cairo_matrix_init_identity(&mat->matrix);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, init_translate)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double tx = luaL_checknumber(L, 1);
	double ty = luaL_checknumber(L, 2);
	cairo_matrix_init_translate(&mat->matrix, tx, ty);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, init_scale)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double sx = luaL_checknumber(L, 1);
	double sy = luaL_checknumber(L, 2);
	cairo_matrix_init_scale(&mat->matrix, sx, sy);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, init_rotate)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double rads = luaL_checknumber(L, 1);
	cairo_matrix_init_rotate(&mat->matrix, rads);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, translate)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double tx = luaL_checknumber(L, 1);
	double ty = luaL_checknumber(L, 2);
	cairo_matrix_translate(&mat->matrix, tx, ty);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, scale)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double sx = luaL_checknumber(L, 1);
	double sy = luaL_checknumber(L, 2);
	cairo_matrix_scale(&mat->matrix, sx, sy);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, rotate)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double rads = luaL_checknumber(L, 1);
	cairo_matrix_rotate(&mat->matrix, rads);
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, invert)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t res = cairo_matrix_invert(&mat->matrix);
	if (res == CAIRO_STATUS_SUCCESS)
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);
	return 1;
}

CALLABLE_IMPL(LuaCairoMatrix, multiply)
{
	LuaCairoMatrix *mat1 = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat2 = (LuaCairoMatrix*)GetObjPointer(L, 1);
	cairo_matrix_t res;
	cairo_matrix_multiply(&res, &mat1->matrix, &mat2->matrix);
	new LuaCairoMatrix(L, &res);
	return 1;
}

CALLABLE_IMPL(LuaCairoMatrix, transform_distance)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double dx = luaL_checknumber(L, 1);
	double dy = luaL_checknumber(L, 2);
	cairo_matrix_transform_distance(&mat->matrix, &dx, &dy);
	lua_pushnumber(L, dx);
	lua_pushnumber(L, dy);
	return 2;
}

CALLABLE_IMPL(LuaCairoMatrix, transform_point)
{
	LuaCairoMatrix *mat = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	cairo_matrix_transform_distance(&mat->matrix, &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

CALLABLE_IMPL(LuaCairoMatrix, op_add)
{
	if (lua_isuserdata(L, 1) && lua_isuserdata(L, 2)) {
		// matrix + matrix (pointwise)
		LuaCairoMatrix *mat1 = (LuaCairoMatrix*)GetObjPointer(L, 1);
		LuaCairoMatrix *mat2 = (LuaCairoMatrix*)GetObjPointer(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &a = mat1->matrix, &b = mat2->matrix, &res = matR->matrix;
		res.xx = a.xx + b.xx; res.yx = a.yx + b.yx;
		res.xy = a.xy + b.xy; res.yy = a.yy + b.yy;
		res.x0 = a.x0 + b.x0; res.y0 = a.y0 + b.y0;
		return 1;

	} else if (lua_isuserdata(L, 1) && lua_isnumber(L, 2)) {
		// matrix + number
		LuaCairoMatrix *matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
		double num = luaL_checknumber(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
		R.xx = num + I.xx; R.yx = num + I.yx;
		R.xy = num + I.xy; R.yy = num + I.yy;
		R.x0 = num + I.x0; R.y0 = num + I.y0;
		return 1;

	} else {
		lua_pushliteral(L, "Unsupported addition operation on matrix");
		lua_error(L);
		return 0;
	}
}

CALLABLE_IMPL(LuaCairoMatrix, op_sub)
{
	if (lua_isuserdata(L, 1) && lua_isuserdata(L, 2)) {
		// matrix - matrix (pointwise)
		LuaCairoMatrix *mat1 = (LuaCairoMatrix*)GetObjPointer(L, 1);
		LuaCairoMatrix *mat2 = (LuaCairoMatrix*)GetObjPointer(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &a = mat1->matrix, &b = mat2->matrix, &res = matR->matrix;
		res.xx = a.xx - b.xx; res.yx = a.yx - b.yx;
		res.xy = a.xy - b.xy; res.yy = a.yy - b.yy;
		res.x0 = a.x0 - b.x0; res.y0 = a.y0 - b.y0;
		return 1;

	} else if (lua_isuserdata(L, 1) && lua_isnumber(L, 2)) {
		// matrix - number
		LuaCairoMatrix *matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
		double num = luaL_checknumber(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
		R.xx = I.xx - num; R.yx = I.yx - num;
		R.xy = I.xy - num; R.yy = I.yy - num;
		R.x0 = I.x0 - num; R.y0 = I.y0 - num;
		return 1;

	} else {
		lua_pushliteral(L, "Unsupported subtraction operation on matrix");
		lua_error(L);
		return 0;
	}
}

CALLABLE_IMPL(LuaCairoMatrix, op_mul)
{
	if (lua_isuserdata(L, 1) && lua_isuserdata(L, 2)) {
		// matrix * matrix (matrix multiplication)
		LuaCairoMatrix *mat1 = (LuaCairoMatrix*)GetObjPointer(L, 1);
		LuaCairoMatrix *mat2 = (LuaCairoMatrix*)GetObjPointer(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_multiply(&matR->matrix, &mat1->matrix, &mat2->matrix);
		return 1;

	} else if ((lua_isuserdata(L, 1) && lua_isnumber(L, 2)) || (lua_isnumber(L, 1) && lua_isuserdata(L, 2))) {
		// matrix * number or number * matrix
		LuaCairoMatrix *matI;
		double num;
		if (lua_isuserdata(L, 1)) {
			matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
			num = luaL_checknumber(L, 2);
		} else {
			num = luaL_checknumber(L, 1);
			matI = (LuaCairoMatrix*)GetObjPointer(L, 2);
		}
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
		R.xx = num * I.xx; R.yx = num * I.yx;
		R.xy = num * I.xy; R.yy = num * I.yy;
		R.x0 = num * I.x0; R.y0 = num * I.y0;
		return 1;

	} else {
		lua_pushliteral(L, "Unsupported multiplication operation on matrix");
		lua_error(L);
		return 0;
	}
}

CALLABLE_IMPL(LuaCairoMatrix, op_div)
{
	if (lua_isuserdata(L, 1) && lua_isnumber(L, 2)) {
		// matrix / number = matrix * (1/number)
		LuaCairoMatrix *matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
		double num = luaL_checknumber(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L);
		cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
		R.xx = I.xx / num; R.yx = I.yx / num;
		R.xy = I.xy / num; R.yy = I.yy / num;
		R.x0 = I.x0 / num; R.y0 = I.y0 / num;
		return 1;

	} else if (lua_isnumber(L, 1) && lua_isuserdata(L, 2)) {
		// number / matrix = number * inv(matrix)
		LuaCairoMatrix *matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
		double num = luaL_checknumber(L, 2);
		LuaCairoMatrix *matR = new LuaCairoMatrix(L, &matI->matrix);
		cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
		if (!cairo_matrix_invert(&R)) {
			// inversion failed, result is nil
			lua_pushnil(L);
			return 1;
		}
		R.xx = R.xx * num; R.yx = R.yx * num;
		R.xy = R.xy * num; R.yy = R.yy * num;
		R.x0 = R.x0 * num; R.y0 = R.y0 * num;
		return 1;

	} else {
		lua_pushliteral(L, "Unsupported division operation on matrix");
		lua_error(L);
		return 0;
	}
}

CALLABLE_IMPL(LuaCairoMatrix, op_unm)
{
	LuaCairoMatrix *matI = (LuaCairoMatrix*)GetObjPointer(L, 1);
	LuaCairoMatrix *matR = new LuaCairoMatrix(L);
	cairo_matrix_t &I = matI->matrix, &R = matR->matrix;
	R.xx = -I.xx; R.yx = -I.yx;
	R.xy = -I.xy; R.yy = -I.yy;
	R.x0 = -I.x0; R.y0 = -I.y0;
	return 0;
}

CALLABLE_IMPL(LuaCairoMatrix, op_eq)
{
	LuaCairoMatrix *mat1 = (LuaCairoMatrix*)GetObjPointer(L, 1);
	LuaCairoMatrix *mat2 = (LuaCairoMatrix*)GetObjPointer(L, 2);
	cairo_matrix_t &a = mat1->matrix, &b = mat2->matrix;
	bool res =
		a.xx==b.xx && a.yx==b.yx &&
		a.xy==b.xy && a.yy==b.yy &&
		a.x0==b.x0 && a.y0==b.y0;
	lua_pushboolean(L, (int)res);
	return 1;
}

CALLABLE_IMPL(LuaCairoMatrix, copy)
{
	LuaCairoMatrix *org = (LuaCairoMatrix*)GetObjPointer(L, lua_upvalueindex(1));
	new LuaCairoMatrix(L, &org->matrix);
	return 1;
}

CALLABLE_IMPL(LuaCairoMatrix, create)
{
	new LuaCairoMatrix(L);
	return 1;
}

void LuaCairoMatrix::RegMatrixCallables(lua_State *L)
{
	CALLABLE_REG(init);
	CALLABLE_REG(init_identity);
	CALLABLE_REG(init_translate);
	CALLABLE_REG(init_scale);
	CALLABLE_REG(init_rotate);
	CALLABLE_REG(translate);
	CALLABLE_REG(scale);
	CALLABLE_REG(rotate);
	CALLABLE_REG(invert);
	CALLABLE_REG(multiply);
	CALLABLE_REG(transform_distance);
	CALLABLE_REG(transform_point);
	CALLABLE_REG(copy);
}

int LuaCairoMatrix::internal_lua_index(lua_State *L)
{
	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *field = lua_tostring(L, 2);
		if (strcmp(field, "xx") == 0) {
			lua_pushnumber(L, matrix.xx);
			return 1;
		} else if (strcmp(field, "yx") == 0) {
			lua_pushnumber(L, matrix.yx);
			return 1;
		} else if (strcmp(field, "xy") == 0) {
			lua_pushnumber(L, matrix.xy);
			return 1;
		} else if (strcmp(field, "yy") == 0) {
			lua_pushnumber(L, matrix.yy);
			return 1;
		} else if (strcmp(field, "x0") == 0) {
			lua_pushnumber(L, matrix.x0);
			return 1;
		} else if (strcmp(field, "y0") == 0) {
			lua_pushnumber(L, matrix.y0);
			return 1;
		}
	}

	return LuaCairoBase::internal_lua_index(L);
}

int LuaCairoMatrix::internal_lua_newindex(lua_State *L)
{
	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *field = lua_tostring(L, 2);
		if (strcmp(field, "xx") == 0) {
			matrix.xx = luaL_checknumber(L, 3);
			return 0;
		} else if (strcmp(field, "yx") == 0) {
			matrix.yx = luaL_checknumber(L, 3);
			return 0;
		} else if (strcmp(field, "xy") == 0) {
			matrix.xy = luaL_checknumber(L, 3);
			return 0;
		} else if (strcmp(field, "yy") == 0) {
			matrix.yy = luaL_checknumber(L, 3);
			return 0;
		} else if (strcmp(field, "x0") == 0) {
			matrix.x0 = luaL_checknumber(L, 3);
			return 0;
		} else if (strcmp(field, "y0") == 0) {
			matrix.y0 = luaL_checknumber(L, 3);
			return 0;
		}
	}

	return LuaCairoBase::internal_lua_newindex(L);
}

const char *LuaCairoMatrix::GetTypeName()
{
	return "matrix";
}

void LuaCairoMatrix::CreateMetaTable(lua_State *L)
{
	LuaCairoBase::CreateMetaTable(L);

	lua_pushcclosure(L, lua_op_add, 0);
	lua_setfield(L, -2, "__add");
	lua_pushcclosure(L, lua_op_sub, 0);
	lua_setfield(L, -2, "__sub");
	lua_pushcclosure(L, lua_op_mul, 0);
	lua_setfield(L, -2, "__mul");
	lua_pushcclosure(L, lua_op_div, 0);
	lua_setfield(L, -2, "__div");
	lua_pushcclosure(L, lua_op_unm, 0);
	lua_setfield(L, -2, "__unm");
	lua_pushcclosure(L, lua_op_eq, 0);
	lua_setfield(L, -2, "__eq");
}

LuaCairoMatrix::LuaCairoMatrix(lua_State *L) :
	LuaCairoBase(L)
{
	RegMatrixCallables(L);

	cairo_matrix_init_identity(&matrix);
}

LuaCairoMatrix::LuaCairoMatrix(lua_State *L, const cairo_matrix_t *_matrix) :
	LuaCairoBase(L)
{
	RegMatrixCallables(L);

	memcpy(&matrix, _matrix, sizeof(matrix));
}

LuaCairoMatrix::~LuaCairoMatrix()
{
	// Automatic memory management here - nothing to free
}

cairo_matrix_t *LuaCairoMatrix::GetMatrix()
{
	return &matrix;
}


// Path (cairo_path_t)

static void path_element_to_lua(cairo_path_data_t *path, lua_State *L)
{
	lua_newtable(L);
	switch (path[0].header.type) {
		case CAIRO_PATH_MOVE_TO:
			lua_pushliteral(L, "move_to");
			lua_setfield(L, -2, "type");
			lua_pushnumber(L, path[1].point.x);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, path[1].point.y);
			lua_setfield(L, -2, "y");
			break;

		case CAIRO_PATH_LINE_TO:
			lua_pushliteral(L, "line_to");
			lua_setfield(L, -2, "type");
			lua_pushnumber(L, path[1].point.x);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, path[1].point.y);
			lua_setfield(L, -2, "y");
			break;

		case CAIRO_PATH_CURVE_TO:
			lua_pushliteral(L, "curve_to");
			lua_setfield(L, -2, "type");
			lua_pushnumber(L, path[1].point.x);
			lua_setfield(L, -2, "x0");
			lua_pushnumber(L, path[1].point.y);
			lua_setfield(L, -2, "y0");
			lua_pushnumber(L, path[2].point.x);
			lua_setfield(L, -2, "x1");
			lua_pushnumber(L, path[2].point.y);
			lua_setfield(L, -2, "y1");
			lua_pushnumber(L, path[3].point.x);
			lua_setfield(L, -2, "x2");
			lua_pushnumber(L, path[3].point.y);
			lua_setfield(L, -2, "y2");
			break;

		case CAIRO_PATH_CLOSE_PATH:
			lua_pushliteral(L, "close");
			lua_setfield(L, -2, "type");
			break;

		default:
			lua_pushliteral(L, "unknown");
			lua_setfield(L, -2, "type");
			break;
	}
}

static void read_lua_path_element(lua_State *L, cairo_path_data_t *path)
{
	lua_getfield(L, -1, "type");
	if (!lua_isstring(L, -1)) {
		luaL_error(L, "Invalid or missing 'type' field in path element table");
		return;
	}
	const char *type = lua_tostring(L, -1);

	if (strcmp(type, "move_to") == 0) {
		path[0].header.length = 2;
		path[0].header.type = CAIRO_PATH_MOVE_TO;
		lua_getfield(L, -2, "x");
		lua_getfield(L, -3, "y");
		if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
			luaL_error(L, "Invalid 'x' or 'y' field in path element table with type 'move_to'");
			return;
		}
		path[1].point.x = lua_tonumber(L, -2);
		path[1].point.y = lua_tonumber(L, -1);
		lua_pop(L, 3);
		return;

	} else if (strcmp(type, "line_to") == 0) {
		path[0].header.length = 2;
		path[0].header.type = CAIRO_PATH_LINE_TO;
		lua_getfield(L, -2, "x");
		lua_getfield(L, -3, "y");
		if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
			luaL_error(L, "Invalid 'x' or 'y' field in path element table with type 'line_to'");
			return;
		}
		path[1].point.x = lua_tonumber(L, -2);
		path[1].point.y = lua_tonumber(L, -1);
		lua_pop(L, 3);
		return;

	} else if (strcmp(type, "curve_to") == 0) {
		path[0].header.length = 4;
		path[0].header.type = CAIRO_PATH_CURVE_TO;
		lua_getfield(L, -2, "x0");
		lua_getfield(L, -3, "y0");
		if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
			luaL_error(L, "Invalid 'x0' or 'y0' field in path element table with type 'line_to'");
			return;
		}
		path[1].point.x = lua_tonumber(L, -2);
		path[1].point.y = lua_tonumber(L, -1);
		lua_getfield(L, -4, "x1");
		lua_getfield(L, -5, "y1");
		if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
			luaL_error(L, "Invalid 'x1' or 'y1' field in path element table with type 'line_to'");
			return;
		}
		path[2].point.x = lua_tonumber(L, -2);
		path[2].point.y = lua_tonumber(L, -1);
		lua_getfield(L, -2, "x2");
		lua_getfield(L, -3, "y2");
		if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
			luaL_error(L, "Invalid 'x2' or 'y2' field in path element table with type 'line_to'");
			return;
		}
		path[3].point.x = lua_tonumber(L, -2);
		path[3].point.y = lua_tonumber(L, -1);
		lua_pop(L, 7);
		return;

	} else if (strcmp(type, "close") == 0) {
		path[0].header.length = 1;
		path[0].header.type = CAIRO_PATH_CLOSE_PATH;
		lua_pop(L, 1);
		return;

	} else {
		luaL_error(L, "Invalid 'type' field in path element table, '%s'", type);
	}
}

CALLABLE_NOTIMPL(LuaCairoPath, clear)
CALLABLE_NOTIMPL(LuaCairoPath, move_to)
CALLABLE_NOTIMPL(LuaCairoPath, line_to)
CALLABLE_NOTIMPL(LuaCairoPath, curve_to)
CALLABLE_NOTIMPL(LuaCairoPath, close)

CALLABLE_IMPL(LuaCairoPath, map)
{
	LuaCairoPath *path = GetObjPointer(L, lua_upvalueindex(1));
	if (!lua_isfunction(L, 1)) {
		luaL_error(L, "First argument to path.map_coords must be a function, is %s", luaL_typename(L, 1));
		return 0;
	}

	// Function should be p->p
	cairo_path_t *p = path->path;
	if (!p->num_data || !p->data) return 0;

	// Prepare a new path for building
	path->path = (cairo_path_t*)malloc(sizeof(cairo_path_t));
	cairo_path_t *np = path->path;
	np->num_data = 0;
	np->status = CAIRO_STATUS_SUCCESS;
	np->data = 0;

	cairo_path_data_t *pd = p->data;
	int outi = 0;
	for (int i = 0; i < p->num_data; ) {
		lua_pushvalue(L, 1);
		path_element_to_lua(pd, L);
		lua_call(L, 1, 1);

		path->EnsureSpaceFor(4); // dumb but simple, ensures there's always enough space for even the longest segments
		read_lua_path_element(L, np->data+outi);
		np->num_data += np->data[outi].header.length;
		outi += np->data[outi].header.length;
		
		i += pd->header.length;
		pd += pd->header.length;
	}

	if (path->cairo_owns_memory) {
		cairo_path_destroy(p);
		path->cairo_owns_memory = false;
	} else {
		free(p->data);
		free(p);
	}

	// Now just r should be left on top of stack
	return 1;}

CALLABLE_IMPL(LuaCairoPath, map_coords)
{
	LuaCairoPath *path = GetObjPointer(L, lua_upvalueindex(1));
	if (!lua_isfunction(L, 1)) {
		luaL_error(L, "First argument to path.map_coords must be a function, is %s", luaL_typename(L, 1));
		return 0;
	}

	// Function should be (x,y)->(x,y)
	cairo_path_t *p = path->path;
	if (!p->num_data || !p->data) return 0;

	int length_to_go = 0;
	cairo_path_data_t *pd = p->data;
	for (int i = 0; i < p->num_data; i++, pd++) {
		if (length_to_go > 0) {
			lua_pushvalue(L, 1);
			lua_pushnumber(L, pd->point.x);
			lua_pushnumber(L, pd->point.y);
			lua_call(L, 2, 2);
			if (!lua_isnumber(L, -1) || !lua_isnumber(L, -2)) {
				luaL_error(L, "The function given to path.map_coords must return two numbers");
				return 0;
			}
			pd->point.x = lua_tonumber(L, -2);
			pd->point.y = lua_tonumber(L, -1);
			lua_pop(L, 2);
			length_to_go--;

		} else {
			length_to_go = pd->header.length-1;
		}
	}

	return 0;
}

CALLABLE_IMPL(LuaCairoPath, fold)
{
	LuaCairoPath *path = GetObjPointer(L, lua_upvalueindex(1));
	if (!lua_isfunction(L, 1)) {
		luaL_error(L, "First argument to path.map_coords must be a function, is %s", luaL_typename(L, 1));
		return 0;
	}
	luaL_checkany(L, 2);

	// Function should be (r,p)->r
	cairo_path_t *p = path->path;
	if (!p->num_data || !p->data) return 0;

	cairo_path_data_t *pd = p->data;
	lua_pushvalue(L, 2); // initial 'r' for function
	for (int i = 0; i < p->num_data; ) {
		lua_pushvalue(L, 1);
		lua_pushvalue(L, -2); // dig up 'r'
		lua_remove(L, -3); // remove dug up 'r'
		path_element_to_lua(pd, L);
		lua_call(L, 2, 1);
		// leave result 'r' on stack for next iteration or final return
		i += pd->header.length;
		pd += pd->header.length;
	}

	// Now just r should be left on top of stack
	return 1;
}

CALLABLE_IMPL(LuaCairoPath, fold_coords)
{
	LuaCairoPath *path = GetObjPointer(L, lua_upvalueindex(1));
	if (!lua_isfunction(L, 1)) {
		luaL_error(L, "First argument to path.map_coords must be a function, is %s", luaL_typename(L, 1));
		return 0;
	}
	luaL_checkany(L, 2);

	// Function should be (r,x,y)->r
	cairo_path_t *p = path->path;
	if (!p->num_data || !p->data) return 0;

	int length_to_go = 0;
	cairo_path_data_t *pd = p->data;
	lua_pushvalue(L, 2); // initial 'r' for function
	for (int i = 0; i < p->num_data; i++, pd++) {
		if (length_to_go > 0) {
			lua_pushvalue(L, 1);
			lua_pushvalue(L, -2); // dig up 'r'
			lua_remove(L, -3); // remove dug up 'r'
			lua_pushnumber(L, pd->point.x);
			lua_pushnumber(L, pd->point.y);
			lua_call(L, 3, 1);
			// leave result 'r' on stack for next iteration or final return
			length_to_go--;

		} else {
			length_to_go = pd->header.length - 1;
		}
	}

	// Now just r should be left on top of stack
	return 1;
}

void LuaCairoPath::EnsurePathOwned()
{
	if (cairo_owns_memory) {
		cairo_path_t *np = (cairo_path_t*)malloc(sizeof(cairo_path_t));
		np->status = path->status;
		np->num_data = path->num_data;
		np->data = (cairo_path_data_t*)malloc(path->num_data*sizeof(cairo_path_data_t));
		memcpy(np->data, path->data, np->num_data*sizeof(cairo_path_data_t));
		cairo_path_destroy(path);
		path = np;
		cairo_owns_memory = false;
	}
}

void LuaCairoPath::EnsureSpaceFor(size_t n)
{
	EnsurePathOwned();

	if (path_elements_allocated - path->num_data < n) {
		path_elements_allocated = path->num_data*2 + n;
		path->data = (cairo_path_data_t*)realloc(path->data, path_elements_allocated*sizeof(cairo_path_data_t));
	}
}

void LuaCairoPath::RegPathCallables(lua_State *L)
{
	CALLABLE_REG(clear);
	CALLABLE_REG(move_to);
	CALLABLE_REG(line_to);
	CALLABLE_REG(curve_to);
	CALLABLE_REG(close);
	CALLABLE_REG(map);
	CALLABLE_REG(map_coords);
	CALLABLE_REG(fold);
	CALLABLE_REG(fold_coords);
}

int LuaCairoPath::internal_lua_index(lua_State *L)
{
	return LuaCairoBase::internal_lua_index(L);
}

const char *LuaCairoPath::GetTypeName()
{
	return "path";
}

LuaCairoPath::LuaCairoPath(lua_State *L) :
	LuaCairoBase(L)
{
	RegPathCallables(L);

	cairo_owns_memory = false;
	path = (cairo_path_t*)malloc(sizeof(cairo_path_t));
	path->status = CAIRO_STATUS_SUCCESS;
	path->num_data = 0;
	path->data = 0;
	EnsureSpaceFor(8);
}

LuaCairoPath::LuaCairoPath(lua_State *L, cairo_path_t *_path) :
	LuaCairoBase(L)
{
	RegPathCallables(L);

	cairo_owns_memory = true;
	path = _path;
}

LuaCairoPath::~LuaCairoPath()
{
	if (cairo_owns_memory)
		cairo_path_destroy(path);
	else {
		free(path->data);
		free(path);
	}
}


// Pattern (cairo_pattern_t)

CALLABLE_IMPL(LuaCairoPattern, add_color_stop_rgb)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	double offset = luaL_checknumber(L, 1);
	double red = luaL_checknumber(L, 2);
	double green = luaL_checknumber(L, 3);
	double blue = luaL_checknumber(L, 4);
	cairo_pattern_add_color_stop_rgb(pat->pattern, offset, red, green, blue);
	return 0;
}

CALLABLE_IMPL(LuaCairoPattern, add_color_stop_rgba)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	double offset = luaL_checknumber(L, 1);
	double red = luaL_checknumber(L, 2);
	double green = luaL_checknumber(L, 3);
	double blue = luaL_checknumber(L, 4);
	double alpha = luaL_checknumber(L, 5);
	cairo_pattern_add_color_stop_rgba(pat->pattern, offset, red, green, blue, alpha);
	return 0;
}

CALLABLE_IMPL(LuaCairoPattern, get_color_stop_count)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	int count;
	cairo_status_t res = cairo_pattern_get_color_stop_count(pat->pattern, &count);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	lua_pushinteger(L, count);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, get_color_stop_rgba)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	int index = luaL_checkint(L, 1);
	double offset, red, green, blue, alpha;
	cairo_status_t res = cairo_pattern_get_color_stop_rgba(pat->pattern, index, &offset, &red, &green, &blue, &alpha);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	lua_pushnumber(L, offset);
	lua_pushnumber(L, red);
	lua_pushnumber(L, green);
	lua_pushnumber(L, blue);
	lua_pushnumber(L, alpha);
	return 5;
}

CALLABLE_IMPL(LuaCairoPattern, get_rgba)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	double red, green, blue, alpha;
	cairo_status_t res = cairo_pattern_get_rgba(pat->pattern, &red, &green, &blue, &alpha);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	lua_pushnumber(L, red);
	lua_pushnumber(L, green);
	lua_pushnumber(L, blue);
	lua_pushnumber(L, alpha);
	return 4;
}

CALLABLE_IMPL(LuaCairoPattern, get_surface)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_surface_t *surf;
	cairo_status_t res = cairo_pattern_get_surface(pat->pattern, &surf);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	new LuaCairoSurface(L, surf);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, get_linear_points)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	double x0, y0, x1, y1;
	cairo_status_t res = cairo_pattern_get_linear_points(pat->pattern, &x0, &y0, &x1, &y1);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	lua_pushnumber(L, x0);
	lua_pushnumber(L, y0);
	lua_pushnumber(L, x1);
	lua_pushnumber(L, y1);
	return 4;
}

CALLABLE_IMPL(LuaCairoPattern, get_radial_circles)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	double x0, y0, r0, x1, y1, r1;
	cairo_status_t res = cairo_pattern_get_radial_circles(pat->pattern, &x0, &y0, &r0, &x1, &y1, &r1);
	if (res != CAIRO_STATUS_SUCCESS)
		return 0;
	lua_pushnumber(L, x0);
	lua_pushnumber(L, y0);
	lua_pushnumber(L, r0);
	lua_pushnumber(L, x1);
	lua_pushnumber(L, y1);
	lua_pushnumber(L, r1);
	return 6;
}

CALLABLE_NOTIMPL(LuaCairoPattern, reference)

CALLABLE_IMPL(LuaCairoPattern, status)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_status_t st = cairo_pattern_status(pat->pattern);
	lua_pushstring(L, status_names_list[st]);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, set_extend)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_extend_t ext = (cairo_extend_t)luaL_checkoption(L, 1, NULL, pattern_extend_list);
	cairo_pattern_set_extend(pat->pattern, ext);
	return 0;
}

CALLABLE_IMPL(LuaCairoPattern, get_extend)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_extend_t ext = cairo_pattern_get_extend(pat->pattern);
	lua_pushstring(L, pattern_extend_list[ext]);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, set_filter)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_filter_t filt = (cairo_filter_t)luaL_checkoption(L, 1, NULL, pattern_filter_list);
	cairo_pattern_set_filter(pat->pattern, filt);
	return 0;
}

CALLABLE_IMPL(LuaCairoPattern, get_filter)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_filter_t filt = cairo_pattern_get_filter(pat->pattern);
	lua_pushstring(L, pattern_filter_list[filt]);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, set_matrix)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = LuaCairoMatrix::GetObjPointer(L, 1);
	cairo_pattern_set_matrix(pat->pattern, mat->GetMatrix());
	return 0;
}

CALLABLE_IMPL(LuaCairoPattern, get_matrix)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	LuaCairoMatrix *mat = new LuaCairoMatrix(L);
	cairo_pattern_get_matrix(pat->pattern, mat->GetMatrix());
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, get_type)
{
	LuaCairoPattern *pat = GetObjPointer(L, lua_upvalueindex(1));
	cairo_pattern_type_t type = cairo_pattern_get_type(pat->pattern);
	lua_pushstring(L, pattern_type_list[type]);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, create_rgb)
{
	double red = luaL_checknumber(L, 1);
	double green = luaL_checknumber(L, 2);
	double blue = luaL_checknumber(L, 3);
	cairo_pattern_t *pat = cairo_pattern_create_rgb(red, green, blue);
	new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, create_rgba)
{
	double red = luaL_checknumber(L, 1);
	double green = luaL_checknumber(L, 2);
	double blue = luaL_checknumber(L, 3);
	double alpha = luaL_checknumber(L, 4);
	cairo_pattern_t *pat = cairo_pattern_create_rgba(red, green, blue, alpha);
	new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, create_for_surface)
{
	LuaCairoSurface *surf = LuaCairoSurface::GetObjPointer(L, 1);
	cairo_pattern_t *pat = cairo_pattern_create_for_surface(surf->GetSurface());
	new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, create_linear)
{
	double x0 = luaL_checknumber(L, 1);
	double y0 = luaL_checknumber(L, 2);
	double x1 = luaL_checknumber(L, 3);
	double y1 = luaL_checknumber(L, 4);
	cairo_pattern_t *pat = cairo_pattern_create_linear(x0, y0, x1, y1);
	new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

CALLABLE_IMPL(LuaCairoPattern, create_radial)
{
	double cx0 = luaL_checknumber(L, 1);
	double cy0 = luaL_checknumber(L, 2);
	double radius0 = luaL_checknumber(L, 3);
	double cx1 = luaL_checknumber(L, 4);
	double cy1 = luaL_checknumber(L, 5);
	double radius1 = luaL_checknumber(L, 6);
	cairo_pattern_t *pat = cairo_pattern_create_radial(cx0, cy0, radius0, cx1, cy1, radius1);
	new LuaCairoPattern(L, pat);
	cairo_pattern_destroy(pat);
	return 1;
}

const char *LuaCairoPattern::GetTypeName()
{
	return "pattern";
}

LuaCairoPattern::LuaCairoPattern(lua_State *L, cairo_pattern_t *_pattern) :
	LuaCairoBase(L)
{
	CALLABLE_REG(add_color_stop_rgb);
	CALLABLE_REG(add_color_stop_rgba);
	CALLABLE_REG(get_color_stop_count);
	CALLABLE_REG(get_color_stop_rgba);
	CALLABLE_REG(get_rgba);
	CALLABLE_REG(get_surface);
	CALLABLE_REG(get_linear_points);
	CALLABLE_REG(get_radial_circles);
	CALLABLE_REG(reference);
	CALLABLE_REG(status);
	CALLABLE_REG(set_extend);
	CALLABLE_REG(get_extend);
	CALLABLE_REG(set_filter);
	CALLABLE_REG(get_filter);
	CALLABLE_REG(set_matrix);
	CALLABLE_REG(get_matrix);
	CALLABLE_REG(get_type);

	cairo_pattern_reference(_pattern);
	pattern = _pattern;
}

LuaCairoPattern::~LuaCairoPattern()
{
	cairo_pattern_destroy(pattern);
}


// Lua registration

static const luaL_Reg cairolib[] = {
	{"image_surface_create", LuaCairoSurface::lua_image_surface_create},
	{"font_options_create", LuaCairoFontOptions::lua_create},
	{"matrix_create", LuaCairoMatrix::lua_create},
	{"pattern_create_rgb", LuaCairoPattern::lua_create_rgb},
	{"pattern_create_rgba", LuaCairoPattern::lua_create_rgba},
	{"pattern_create_for_surface", LuaCairoPattern::lua_create_for_surface},
	{"pattern_create_linear", LuaCairoPattern::lua_create_linear},
	{"pattern_create_radial", LuaCairoPattern::lua_create_radial},
	{NULL,NULL}
};

int luaopen_cairo(lua_State *L)
{
	luaL_register(L, "cairo", cairolib);
	return 0;
}
