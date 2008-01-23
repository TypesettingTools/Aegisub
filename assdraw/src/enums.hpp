/*
* Copyright (c) 2007, ai-chan
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the ASSDraw3 Team nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY AI-CHAN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL AI-CHAN BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

// enum for IDs of menus
enum {
	MENU_DUMMY = 200,
	MENU_CLEAR,
	MENU_PREVIEW,
	MENU_TRANSFORM,
	MENU_LIBRARY,
	MENU_HELP,
	MENU_RESETPERSPECTIVE,
	MENU_SETTINGS,
	MENU_UNDO,
	MENU_REDO,
	MENU_PASTE,
	MENU_BGIMG_LOAD,
	MENU_BGIMG_ALPHA,
	MENU_BGIMG_REMOVE,
	MENU_RECENTER,
	MENU_TBAR,
	MENU_REPOS_TOPLEFT,
	MENU_REPOS_TOPRIGHT,
	MENU_REPOS_CENTER,
	MENU_REPOS_BOTLEFT,
	MENU_REPOS_BOTRIGHT,
	MENU_BGIMG_RECENTER,
	MENU_REPOS_BGTOPLEFT,
	MENU_REPOS_BGTOPRIGHT,
	MENU_REPOS_BGCENTER,
	MENU_REPOS_BGBOTLEFT,
	MENU_REPOS_BGBOTRIGHT,
	MENU_DRC_LNTOBEZ,
	MENU_DRC_C1CONTBEZ,
	MENU_DRC_BEZTOLN,
	MENU_DRC_MOVE00,
	MENU_TB_ALL,
	MENU_TB_NONE,
	MENU_TB_DOCK,
	MENU_TB_UNDOCK,
	MENU_TB_DRAW,
	MENU_TB_MODE,
	MENU_TB_BGIMG
};

// enum for modes (i.e. create m, b, l etc or normal mode)
// also use as tools IDs
enum MODE
{
    MODE_ARR = 100,
    MODE_M = 101,
    MODE_N = 102,
    MODE_L = 103,
    MODE_B = 104,
    MODE_S = 105,
    MODE_P = 106,
    MODE_C = 107,
    MODE_DEL = 108,
	MODE_SCALEROTATE = 109,
	MODE_NUT_BILINEAR = 110
};

// enum for IDs of other tools on the toolbar
enum {
     TB_CLEAR = 111,
     TB_EDITSRC = 112,
     TB_PREVIEW = 113,
     TB_TRANSFORM = 114,
     TB_HELP = 115,
     TB_ZOOMSLIDER = 116,
     TB_BGALPHA_SLIDER = 117
};

enum DRAGMODETOOL
{
    DRAG_DWG = 120,
    DRAG_BGIMG = 121,
    DRAG_BOTH = 122
};

struct DRAGMODE
{
	bool drawing;
	bool bgimg;
	DRAGMODE() { drawing = true, bgimg = false; }
	DRAGMODE(bool d, bool b) { drawing = d, bgimg = b; }
};
