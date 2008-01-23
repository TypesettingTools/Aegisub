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
// Name:        cmd.hpp
// Purpose:     header file for ASSDraw drawing command classes
// Author:      ai-chan
// Created:     08/26/06
// Copyright:   (c) ai-chan
// Licence:     3-clause BSD
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "engine.hpp" // include the engine header for DrawCmd
#include <vector> // ok, we use vector too

// this header file declares the following classes
class DrawCmd_M;
class DrawCmd_L;
class DrawCmd_B;

// The M command
class DrawCmd_M: public DrawCmd
{
public:
	// Constructor
	DrawCmd_M ( int x, int y, PointSystem *ps, DrawCmd *prev );
	
	// to ASS drawing command
	wxString ToString();

};

// The L command
class DrawCmd_L: public DrawCmd
{
public:
	// Constructor
	DrawCmd_L ( int x, int y, PointSystem *ps, DrawCmd *prev );
	
	// to ASS drawing command
	wxString ToString();
	
};

// The B command
class DrawCmd_B: public DrawCmd
{
public:
	// Constructor
	DrawCmd_B ( int x, int y, int x1, int y1, int x2, int y2,  PointSystem *ps, DrawCmd *prev );
	
	// Special constructor where only m_point is defined
	// Need to call Init() to generate the controls
	DrawCmd_B ( int x, int y, PointSystem *ps, DrawCmd *prev );
	
	// Init this B command; generate controlpoints
	void Init ( unsigned n = 0 );
	
	// to ASS drawing command
	wxString ToString();
	
	//special
	bool C1Cont;
	
};

// The S command
class DrawCmd_S: public DrawCmd
{
public:
	// Constructor
	DrawCmd_S ( int x, int y, PointSystem *ps, DrawCmd *prev );

	// Constructor (with points info)
	DrawCmd_S ( int x, int y, std::vector< int > vals, PointSystem *ps, DrawCmd *prev );

	// Init this S command; generate controlpoints
	void Init ( unsigned n = 0 );

	// to ASS drawing command
	wxString ToString();

	// special
	bool closed;
};

