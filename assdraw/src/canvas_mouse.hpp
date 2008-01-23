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

#include "_common.hpp"
#include "enums.hpp"
#include "engine.hpp"

#include <wx/event.h>

struct MouseOnCanvasData
{
	MODE mode;
	wxMouseEvent event;
	enum { NONE, LEFT, RIGHT, BOTH } button;

    Point* mousedownAt_point;
	Point* pointedAt_point;
	Point* dblclicked_point_right;

	wxPoint mouse_point;
    wxPoint* dragAnchor_left;
	wxPoint* lastDrag_left;
    wxPoint* dragAnchor_right;
	wxPoint* lastDrag_right;
};

class ASSDrawCanvas;

class ASSDrawMouseOnCanvasEvent : public wxNotifyEvent
{
public:
	ASSDrawMouseOnCanvasEvent(const ASSDrawCanvas* canvas);
	
	wxEvent* Clone();
	
	void SetData(MouseOnCanvasData *data);
	
	MouseOnCanvasData* GetData();
	
private:
	const ASSDrawCanvas* _canvas;
	MouseOnCanvasData* _data;

};

DECLARE_EVENT_TYPE( wxEVT_MOUSEONCANVAS, -1 )

typedef void (wxEvtHandler::*wxMouseOnCanvasEventFunction)(ASSDrawMouseOnCanvasEvent&);

#define EVT_MOUSEONCANVAS(fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_MOUSEONCANVAS, -1, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( wxMouseOnCanvasEventFunction, & fn ), (wxObject *) NULL ),

class ASSDrawMouseOnCanvasHandler
{
	
	
};
