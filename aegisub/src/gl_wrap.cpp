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

/// @file gl_wrap.cpp
/// @brief Convenience functions for drawing various geometric primitives on an OpenGL surface
/// @ingroup video_output
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/msgdlg.h>
#endif

#include "gl_wrap.h"
#include "options.h"

#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include "gl/glext.h"
#endif


//////////////////////////
// Extension get function
#ifdef __WIN32__

/// @brief DOCME
/// @param str 
/// @return 
///
void* glGetProc(const char *str) { return wglGetProcAddress(str); }
#else

/// DOCME
#define glGetProc(a) glXGetProcAddress((const GLubyte *)(a))
#endif




/// @brief Constructor 
///
OpenGLWrapper::OpenGLWrapper() {
	r1 = g1 = b1 = a1 = 1.0f;
	r2 = g2 = b2 = a2 = 1.0f;
	lw = 1;
}



/// @brief Draw line 
/// @param x1 
/// @param y1 
/// @param x2 
/// @param y2 
///
void OpenGLWrapper::DrawLine(float x1,float y1,float x2,float y2) const {
	SetModeLine();
	glBegin(GL_LINES);
		glVertex2f(x1,y1);
		glVertex2f(x2,y2);
	glEnd();
}



/// @brief Draw line 
/// @param x1   
/// @param y1   
/// @param x2   
/// @param y2   
/// @param step 
///
void OpenGLWrapper::DrawDashedLine(float x1,float y1,float x2,float y2,float step) const {
	float dist = sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
	int steps = (int)((dist-20)/step);
	double stepx = double(x2-x1)/steps;
	double stepy = double(y2-y1)/steps;
	for (int i=0;i<steps;i++) {
		if (i % 2 == 0) DrawLine(x1+int(i*stepx),y1+int(i*stepy),x1+int((i+1)*stepx),y1+int((i+1)*stepy));
	}
}



/// @brief Draw circle 
/// @param x       
/// @param y       
/// @param radiusX 
/// @param radiusY 
///
void OpenGLWrapper::DrawEllipse(float x,float y,float radiusX,float radiusY) const {
	DrawRing(x,y,radiusY,radiusY,radiusX/radiusY);
}



/// @brief Draw rectangle 
/// @param x1 
/// @param y1 
/// @param x2 
/// @param y2 
///
void OpenGLWrapper::DrawRectangle(float x1,float y1,float x2,float y2) const {
	// Fill
	if (a2 != 0.0) {
		SetModeFill();
		glBegin(GL_QUADS);
			glVertex2f(x1,y1);
			glVertex2f(x2,y1);
			glVertex2f(x2,y2);
			glVertex2f(x1,y2);
		glEnd();
	}

	// Outline
	if (a1 != 0.0) {
		SetModeLine();
		glBegin(GL_LINE_LOOP);
			glVertex2f(x1,y1);
			glVertex2f(x2,y1);
			glVertex2f(x2,y2);
			glVertex2f(x1,y2);
		glEnd();
	}
}



/// @brief Draw triangle 
/// @param x1 
/// @param y1 
/// @param x2 
/// @param y2 
/// @param x3 
/// @param y3 
///
void OpenGLWrapper::DrawTriangle(float x1,float y1,float x2,float y2,float x3,float y3) const {
	// Fill
	if (a2 != 0.0) {
		SetModeFill();
		glBegin(GL_TRIANGLES);
			glVertex2f(x1,y1);
			glVertex2f(x2,y2);
			glVertex2f(x3,y3);
		glEnd();
	}

	// Outline
	if (a1 != 0.0) {
		SetModeLine();
		glBegin(GL_LINE_LOOP);
			glVertex2f(x1,y1);
			glVertex2f(x2,y2);
			glVertex2f(x3,y3);
		glEnd();
	}
}



/// @brief Draw ring (annulus) 
/// @param x        
/// @param y        
/// @param r1       
/// @param r2       
/// @param ar       
/// @param arcStart 
/// @param arcEnd   
///
void OpenGLWrapper::DrawRing(float x,float y,float r1,float r2,float ar,float arcStart,float arcEnd) const {
	// Make r1 bigger
	if (r2 > r1) {
		float temp = r1;
		r1 = r2;
		r2 = temp;
	}

	// Arc range
	bool hasEnds = arcStart != arcEnd;
	float pi = 3.1415926535897932384626433832795f;
	arcEnd *= pi / 180.f;
	arcStart *= pi / 180.f;
	if (arcEnd <= arcStart) arcEnd += 2.0f*pi;
	float range = arcEnd - arcStart;

	// Math
	int steps = int((r1 + r1*ar) * range / (2.0f*pi))*4;
	if (steps < 12) steps = 12;
	//float end = arcEnd;
	float step = range/steps;
	float curAngle = arcStart;

	// Fill
	if (a2 != 0.0) {
		SetModeFill();

		// Annulus
		if (r1 != r2) {
			glBegin(GL_QUADS);
			for (int i=0;i<steps;i++) {
				glVertex2f(x+sin(curAngle)*r1*ar,y+cos(curAngle)*r1);
				glVertex2f(x+sin(curAngle)*r2*ar,y+cos(curAngle)*r2);
				curAngle += step;
				glVertex2f(x+sin(curAngle)*r2*ar,y+cos(curAngle)*r2);
				glVertex2f(x+sin(curAngle)*r1*ar,y+cos(curAngle)*r1);
			}
			glEnd();
		}

		// Circle
		else {
			glBegin(GL_POLYGON);
			for (int i=0;i<steps;i++) {
				glVertex2f(x+sin(curAngle)*r1,y+cos(curAngle)*r1);
				curAngle += step;
			}
			glEnd();
		}

		// Reset angle
		curAngle = arcStart;
	}

	// Outlines
	if (a1 != 0.0) {
		// Outer
		steps++;
		SetModeLine();
		glBegin(GL_LINE_STRIP);
		for (int i=0;i<steps;i++) {
			glVertex2f(x+sin(curAngle)*r1,y+cos(curAngle)*r1);
			curAngle += step;
		}
		glEnd();

		// Inner
		if (r1 != r2) {
			curAngle = arcStart;
			glBegin(GL_LINE_STRIP);
			for (int i=0;i<steps;i++) {
				glVertex2f(x+sin(curAngle)*r2,y+cos(curAngle)*r2);
				curAngle += step;
			}
			glEnd();

			// End caps
			if (hasEnds) {
				glBegin(GL_LINES);
					glVertex2f(x+sin(arcStart)*r1,y+cos(arcStart)*r1);
					glVertex2f(x+sin(arcStart)*r2,y+cos(arcStart)*r2);
					glVertex2f(x+sin(arcEnd)*r1,y+cos(arcEnd)*r1);
					glVertex2f(x+sin(arcEnd)*r2,y+cos(arcEnd)*r2);
				glEnd();
			}
		}
	}
}



/// @brief Set line colour 
/// @param col   
/// @param alpha 
/// @param width 
///
void OpenGLWrapper::SetLineColour(wxColour col,float alpha,int width) {
	r1 = col.Red()/255.0f;
	g1 = col.Green()/255.0f;
	b1 = col.Blue()/255.0f;
	a1 = alpha;
	lw = width;
}



/// @brief Set fill colour 
/// @param col   
/// @param alpha 
///
void OpenGLWrapper::SetFillColour(wxColour col,float alpha) {
	r2 = col.Red()/255.0f;
	g2 = col.Green()/255.0f;
	b2 = col.Blue()/255.0f;
	a2 = alpha;
}



/// @brief Line 
///
void OpenGLWrapper::SetModeLine() const {
	glColor4f(r1,g1,b1,a1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(lw);
	glEnable(GL_LINE_SMOOTH);
}



/// @brief Fill 
///
void OpenGLWrapper::SetModeFill() const {
	glColor4f(r2,g2,b2,a2);
	if (a2 == 1.0f) glDisable(GL_BLEND);
	else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
}



/// @brief Is extension supported? 
/// @param ext 
/// @return 
///
bool OpenGLWrapper::IsExtensionSupported(const char *ext) {
    char *extList = (char*) glGetString(GL_EXTENSIONS);
	if (!extList) return false;
    return strstr(extList, ext) != NULL;
}




/// DOCME
wxMutex OpenGLWrapper::glMutex;


