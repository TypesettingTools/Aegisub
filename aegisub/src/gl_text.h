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

/// @file gl_text.h
/// @see gl_text.cpp
/// @ingroup video_output
///

#pragma once

///////////
// Headers

#ifndef AGI_PRE
#include <map>
#include <vector>

#include <wx/bitmap.h>
#include <wx/font.h>
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#else
#include <GL/gl.h>
#endif


/// DOCME
/// @class OpenGLTextGlyph
/// @brief DOCME
///
/// DOCME
class OpenGLTextGlyph {
private:

	/// DOCME
	static wxBitmap *tempBmp;

public:

	/// DOCME
	int value;

	/// DOCME
	int tex;

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	float x1,y1,x2,y2;

	/// DOCME

	/// DOCME
	int w,h;

	void GetMetrics();
	void Draw(int x,int y);

	~OpenGLTextGlyph();
};


/// DOCME
typedef std::map<int,OpenGLTextGlyph> glyphMap;



/// DOCME
/// @class OpenGLTextTexture
/// @brief DOCME
///
/// DOCME
class OpenGLTextTexture {
private:

	/// DOCME

	/// DOCME

	/// DOCME
	int x,y,nextY;

	/// DOCME

	/// DOCME
	int width,height;

	void Insert(OpenGLTextGlyph &glyph);

public:

	/// DOCME
	GLuint tex;

	bool TryToInsert(OpenGLTextGlyph &glyph);

	OpenGLTextTexture(int w,int h);
	~OpenGLTextTexture();
};



/// DOCME
/// @class OpenGLText
/// @brief DOCME
///
/// DOCME
class OpenGLText {
private:

	/// DOCME

	/// DOCME

	/// DOCME

	/// DOCME
	float r,g,b,a;

	/// DOCME
	int lineHeight;

	/// DOCME
	int fontSize;

	/// DOCME
	bool fontBold;

	/// DOCME
	bool fontItalics;

	/// DOCME
	wxString fontFace;

	/// DOCME
	wxFont font;


	/// DOCME
	glyphMap glyphs;

	/// DOCME
	std::vector <OpenGLTextTexture*> textures;

	OpenGLText();
	~OpenGLText();
	OpenGLText(OpenGLText const&);
	OpenGLText& operator=(OpenGLText const&);

	OpenGLTextGlyph GetGlyph(int i);
	OpenGLTextGlyph CreateGlyph(int i);
	void Reset();

	static OpenGLText& GetInstance();
	void DoSetFont(wxString face,int size,bool bold,bool italics);
	void DoSetColour(wxColour col,float alpha);
	void DoPrint(wxString text,int x,int y);
	void DrawString(wxString text,int x,int y);
	void DoGetExtent(wxString text,int &w,int &h);

public:

	/// @brief DOCME
	/// @return 
	///
	static wxFont GetFont() { return GetInstance().font; }

	/// @brief DOCME
	/// @param face    
	/// @param size    
	/// @param bold    
	/// @param italics 
	///
	static void SetFont(wxString face=_T("Verdana"),int size=10,bool bold=true,bool italics=false) { GetInstance().DoSetFont(face,size,bold,italics); }

	/// @brief DOCME
	/// @param col   
	/// @param alpha 
	///
	static void SetColour(wxColour col,float alpha=1.0f) { GetInstance().DoSetColour(col,alpha); }

	/// @brief DOCME
	/// @param text 
	/// @param x    
	/// @param y    
	///
	static void Print(wxString text,int x,int y) { GetInstance().DoPrint(text,x,y); }

	/// @brief DOCME
	/// @param text 
	/// @param w    
	/// @param h    
	///
	static void GetExtent(wxString text,int &w,int &h) { GetInstance().DoGetExtent(text,w,h); }
};


