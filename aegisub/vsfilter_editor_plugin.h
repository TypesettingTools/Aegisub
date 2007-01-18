/* 
 *	Copyright (C) 2007 Niels Martin Hansen
 *	http://www.aegisub.net/
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */


// This file provides a renderer-interface suited for applications
// that need to interactively update the subtitles and re-render
// the frame often, with modified subs.


// Don't include windows.h if this is an MFC project (it won't compile)
#ifndef __AFX_H__
#include <windows.h>
#endif

#ifndef PLUGIN_INTERFACE
#define _i_defined_plugin_interface
#define PLUGIN_INTERFACE extern "C" __declspec(dllimport)
#endif


struct EditorPluginRenderer;


// Create a new renderer and return an opaque handle to it
// Returns NULL on fail
PLUGIN_INTERFACE EditorPluginRenderer *renderer_new();
// Free a renderer object
// Does not fail
PLUGIN_INTERFACE void renderer_free(EditorPluginRenderer *renderer);
// Set renderer resolution and clear all styles+dialogue data
// renderer and script_res are mandatory
// If screen_res is NULL, it's assumed to be the same as script_res
// If video_rect is NULL, it's assumed to have origin in (0,0) and same size as screen_res
PLUGIN_INTERFACE void renderer_set_resolution(EditorPluginRenderer *renderer, const SIZE *script_res, const SIZE *screen_res, const RECT *video_rect);
// Clears script and reinstates script resolution
PLUGIN_INTERFACE void renderer_clear(EditorPluginRenderer *renderer);
// Set wrap style
// Both arguments mandatory
PLUGIN_INTERFACE void renderer_set_wrap_style(EditorPluginRenderer *renderer, int wrap_style);
// Add a style definition
// All arguments mandatory
PLUGIN_INTERFACE void renderer_add_style(EditorPluginRenderer *renderer, const wchar_t *name, const wchar_t *fontname, double fontsize, COLORREF colors[4], BYTE alpha[4],
					 int bold, int italic, int underline, int strikeout, double scalex, double scaley, double spacing, double angle,
					 int borderstyle, double outline, double shadow, int alignment, const RECT *margins, int encoding, int relativeto);
// Add a dialogue line
// All arguments mandatory
PLUGIN_INTERFACE void renderer_add_dialogue(EditorPluginRenderer *renderer, int layer, int start, int end, const wchar_t *style, const wchar_t *name,
					    const RECT *margins, const wchar_t *effect, const wchar_t *text);
// Render a frame of subtitles laid over existing video
// time is the timestamp in milliseconds
// frame is a pointer to the 24 bpp RGB data to render over, assumed to have the screen_res dimensions, stride equal to width and top to bottom scanline ordering
PLUGIN_INTERFACE void renderer_render_overlay(EditorPluginRenderer *renderer, unsigned int time, BYTE *frame);
// Render a frame to an RGBA buffer
// time as above
// frame is a pointer to a buffer to contain the 32 bpp RGBA bitmap rendered; same assumptions as above
PLUGIN_INTERFACE void renderer_render_alpha(EditorPluginRenderer *renderer, unsigned int time, BYTE *frame);


#ifdef _i_defined_plugin_interface
#undef PLUGIN_INTERFACE
#undef _i_defined_plugin_interface
#endif

