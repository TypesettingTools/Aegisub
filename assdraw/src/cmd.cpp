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
// Name:        cmd.cpp
// Purpose:     ASSDraw drawing command classes
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#include "cmd.hpp" // the header for this file
#include <wx/log.h>
// ----------------------------------------------------------------------------
// DrawCmd_M
// ----------------------------------------------------------------------------

// constructor
DrawCmd_M::DrawCmd_M ( int x, int y, PointSystem *ps, DrawCmd *prev ) 
     : DrawCmd ( x, y, ps, prev ) 
{
     type = M;
}

// to ASS drawing command
wxString DrawCmd_M::ToString()
{
     return wxString::Format(_T("m %d %d"), m_point->x(), m_point->y());
}



// ----------------------------------------------------------------------------
// DrawCmd_L
// ----------------------------------------------------------------------------

// constructor
DrawCmd_L::DrawCmd_L ( int x, int y, PointSystem *ps, DrawCmd *prev ) 
     : DrawCmd ( x, y, ps, prev ) 
{
     type = L;
}

// to ASS drawing command
wxString DrawCmd_L::ToString()
{
     return wxString::Format(_T("l %d %d"), m_point->x(), m_point->y());
}



// ----------------------------------------------------------------------------
// DrawCmd_B
// ----------------------------------------------------------------------------

// constructor
DrawCmd_B::DrawCmd_B 
( int x, int y, int x1, int y1, int x2, int y2, PointSystem *ps, DrawCmd *prev ) 
    : DrawCmd ( x, y, ps, prev ) 
{
     type = B;
     controlpoints.push_back( new Point(x1, y1, ps, CP, this, 1) );
     controlpoints.push_back( new Point(x2, y2, ps, CP, this, 2) );
     initialized = true;
     C1Cont = false;
}

// constructor
DrawCmd_B::DrawCmd_B ( int x, int y, PointSystem *ps, DrawCmd *prev ) 
    : DrawCmd ( x, y, ps, prev ) 
{
     type = B;
     initialized = false;
     C1Cont = false;
}

// initialize; generate control points
void DrawCmd_B::Init ( unsigned n )
{
     // Ignore if this is already initted
     if (initialized) return;

     wxPoint wx0 = prev->m_point->ToWxPoint();
     wxPoint wx1 = m_point->ToWxPoint();
     int xdiff = (wx1.x - wx0.x) / 3;     
     int ydiff = (wx1.y - wx0.y) / 3;     
     int xg, yg;

     // first control
     m_point->pointsys->FromWxPoint( wx0.x + xdiff, wx0.y + ydiff, xg, yg );
     controlpoints.push_back( new Point( xg, yg, m_point->pointsys, CP, this, 1 ) );

     // second control
     m_point->pointsys->FromWxPoint( wx1.x - xdiff, wx1.y - ydiff, xg, yg );
     controlpoints.push_back( new Point( xg, yg, m_point->pointsys, CP, this, 2 ) );

     initialized = true;
     
}

// to ASS drawing command
wxString DrawCmd_B::ToString()
{
	if (initialized) {
		PointList::iterator iterate = controlpoints.begin();
		Point* c1 = (*iterate++);
		Point* c2 = (*iterate);
		return wxString::Format(_T("b %d %d %d %d %d %d"), c1->x(), c1->y(), c2->x(), c2->y(), m_point->x(), m_point->y());
	}
	else
		return wxString::Format(_T("b ? ? ? ? %d %d"), m_point->x(), m_point->y());
}


// ----------------------------------------------------------------------------
// DrawCmd_S
// ----------------------------------------------------------------------------

// constructor
DrawCmd_S::DrawCmd_S
	( int x, int y, PointSystem *ps, DrawCmd *prev )
    : DrawCmd ( x, y, ps, prev )
{
	type = S;
	initialized = false;
	closed = false;
}

// constructor
DrawCmd_S::DrawCmd_S
	( int x, int y, std::vector< int > vals, PointSystem *ps, DrawCmd *prev )
    : DrawCmd ( x, y, ps, prev )
{
	type = S;
	std::vector< int >::iterator it = vals.begin();
	unsigned n = 0;
	while (it != vals.end())
	{
		int ix = *it; it++;
		int iy = *it; it++;
		n++;
		//::wxLogMessage(_T("%d %d\n"), ix, iy);
		controlpoints.push_back( new Point( ix, iy, ps, CP, this, n ) );
	}

	initialized = true;
	closed = false;
}

// initialize; generate control points
void DrawCmd_S::Init(unsigned n)
{
     // Ignore if this is already initted
     if (initialized) return;

     wxPoint wx0 = prev->m_point->ToWxPoint();
     wxPoint wx1 = m_point->ToWxPoint();
     int xdiff = (wx1.x - wx0.x) / 3;
     int ydiff = (wx1.y - wx0.y) / 3;
     int xg, yg;

     // first control
     m_point->pointsys->FromWxPoint( wx0.x + xdiff, wx0.y + ydiff, xg, yg );
     controlpoints.push_back( new Point( xg, yg, m_point->pointsys, CP, this, 1 ) );

     // second control
     m_point->pointsys->FromWxPoint( wx1.x - xdiff, wx1.y - ydiff, xg, yg );
     controlpoints.push_back( new Point( xg, yg, m_point->pointsys, CP, this, 2 ) );

     initialized = true;

}

// to ASS drawing command
wxString DrawCmd_S::ToString()
{
	PointList::iterator iterate = controlpoints.begin();
	wxString assout = _T("s");
	for (; iterate != controlpoints.end(); iterate++)
	{
		if (initialized)
			assout = wxString::Format(_T("%s %d %d"), assout.c_str(), (*iterate)->x(), (*iterate)->y()); 
		else
			assout = wxString::Format(_T("%s ? ?"), assout.c_str());
	}
	assout = wxString::Format(_T("%s %d %d"), assout.c_str(), m_point->x(), m_point->y());
	if (closed) assout = wxString::Format(_T("%s c"), assout.c_str());
	return assout;
}
