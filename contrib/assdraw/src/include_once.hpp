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
///////////////////////////////////////////////////////////////////////////////
// Name:        include_once.hpp
// Purpose:     A header file that is included exactly once for the whole code
//              (supposed to be included by assdraw.cpp only)
//              Put must-define-exactly-once codes here
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

// tooltips
const wxString TIPS_CLEAR = wxT("Clear the canvas and create a new drawing");
const wxString TIPS_EDITSRC = wxT("Edit the source");
const wxString TIPS_PREVIEW = wxT("Draw the shapes without enlarged points and control points");
const wxString TIPS_TRANSFORM = wxT("Transform the drawing using matrix transformation");
const wxString TIPS_LIBRARY = wxT("Shapes library");
const wxString TIPS_HELP = wxT("Help! Help!");
const wxString TIPS_PASTE = wxT("Depending on what's in the clipboard, import as drawing commands or background");
const wxString TIPS_UNDO = wxT("Undo last action");
const wxString TIPS_REDO = wxT("Redo last undo");
const wxString TIPS_ARR = wxT("Drag mode");
const wxString TIPS_M = wxT("Draw M mode (Close current shape and move the virtual pen to a new point)");
const wxString TIPS_N = wxT("Draw N mode (Same as M but doesn't close the shape)");
const wxString TIPS_L = wxT("Draw L mode (Straight line)");
const wxString TIPS_B = wxT("Draw B mode (Cubic Bezier curve)");
const wxString TIPS_S = wxT("Draw S mode (Spline)");
const wxString TIPS_P = wxT("Draw P mode (Extends a spline with another point)");
const wxString TIPS_C = wxT("Draw C mode (Close the last spline)");
const wxString TIPS_DEL = wxT("Delete mode");
const wxString TIPS_NUTB = wxT("Bilinear transformation mode: Drag the vertices to distort the shape; Dragging an edge moves two adjacent vertices together");
const wxString TIPS_SCALEROTATE = wxT("Scale/Rotate mode: Drag a vertex or an edge to rescale the shape; Right-drag to rotate");
const wxString TIPS_DWG = wxT("Right-dragging pans drawing, mousewheel zooms in/out drawing");
const wxString TIPS_BGIMG = wxT("Right-dragging pans background, mousewheel zooms in/out background");
const wxString TIPS_BOTH = wxT("Right-dragging pans drawing & background, mousewheel zooms in/out drawing & background");

const wxString TBNAME_DRAW = wxT("Canvas");
const wxString TBNAME_MODE = wxT("Drawing mode");
const wxString TBNAME_BGIMG = wxT("Background");
const wxString TBNAME_ZOOM = wxT("Zoom");

wxString ASSDrawTransformDlg::combo_templatesStrings[] = {
	_("<Select>"),
	_("Move 5 units down"),
	_("Move 5 units right"),
	_("Rotate 90\370 clockwise at (1, 2)"),
	_("Rotate 90\370 counterclockwise at (-1, 2)"),
	_("Rotate 180\370 at (0, 0)"),
	_("Flip horizontally at X = 4"),
	_("Flip vertically at Y = 3"),
	_("Scale up horizontally by a factor of 2"),
	_("Scale up vertically by a factor of 3")
};

int ASSDrawTransformDlg::combo_templatesCount = 10;

EightDouble ASSDrawTransformDlg::combo_templatesValues[] = {
	{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, //<Select>
	{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 5.0f }, //5 units down
	{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 5.0f, 0.0f }, //5 units left
	{ 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 2.0f, 1.0f, 2.0f }, //90 CW (1,2)
	{ 0.0f, 1.0f, -1.0f, 0.0f, -1.0f, 2.0f, -1.0f, 2.0f }, //90 CCW, (-1,2)
	{ -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, //180 (0,0)
	{ -1.0f, 0.0f, 0.0f, 1.0f, 4.0f, 0.0f, 4.0f, 0.0f }, //Flip X = 4
	{ 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 3.0f, 0.0f, 3.0f }, //Flip Y = 3
	{ 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }, //Scale X * 2
	{ 1.0f, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f } //Scale Y * 3
};
