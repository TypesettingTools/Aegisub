// Copyright (c) 2007, Rodrigo Braz Monteiro
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file gl_wrap.h
/// @see gl_wrap.cpp
/// @ingroup video_output
///


#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>

/// DOCME
typedef GLuint GLhandleARB;
#endif

#ifndef AGI_PRE
#include <wx/thread.h>
#include <wx/colour.h>
#endif


/// DOCME
/// @class OpenGLWrapper
/// @brief DOCME
///
/// DOCME
class OpenGLWrapper {
private:

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	float r1,g1,b1,a1;

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	float r2,g2,b2,a2;

	/// DOCME
	int lw;

public:
	OpenGLWrapper();


	/// DOCME
	static wxMutex glMutex;

	void SetLineColour(wxColour col,float alpha=1.0f,int width=1);
	void SetFillColour(wxColour col,float alpha=1.0f);
	void SetModeLine() const;
	void SetModeFill() const;
	void DrawLine(float x1,float y1,float x2,float y2) const;
	void DrawDashedLine(float x1,float y1,float x2,float y2,float dashLen) const;
	void DrawEllipse(float x,float y,float radiusX,float radiusY) const;

	/// @brief DOCME
	/// @param x      
	/// @param y      
	/// @param radius 
	///
	void DrawCircle(float x,float y,float radius) const { DrawEllipse(x,y,radius,radius); }
	void DrawRectangle(float x1,float y1,float x2,float y2) const;
	void DrawRing(float x,float y,float r1,float r2,float ar=1.0f,float arcStart=0.0f,float arcEnd=0.0f) const;
	void DrawTriangle(float x1,float y1,float x2,float y2,float x3,float y3) const;

	static bool IsExtensionSupported(const char *ext);
};


