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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//

#pragma once

///////////
// Headers
#include <wx/wxprec.h>
#include <wx/bitmap.h>
#include <wx/font.h>
#ifdef __APPLE__
#include <OpenGL/GL.h>
#else
#include <GL/gl.h>
#endif
#include <map>
#include <vector>


/////////////////////
// Glyph information
class OpenGLTextGlyph {
private:
	static wxBitmap *tempBmp;

public:
	int value;
	int tex;
	float x1,y1,x2,y2;
	int w,h;

	void GetMetrics();
	void Draw(int x,int y);

	~OpenGLTextGlyph();
};

typedef std::map<int,OpenGLTextGlyph> glyphMap;


///////////////
// Texture map
class OpenGLTextTexture {
private:
	int x,y,nextY;
	int width,height;

	void Insert(OpenGLTextGlyph &glyph);

public:
	GLuint tex;

	bool TryToInsert(OpenGLTextGlyph &glyph);

	OpenGLTextTexture(int w,int h);
	~OpenGLTextTexture();
};


/////////////////////////////
// OpenGL Text Drawing class
class OpenGLText {
private:
	float r,g,b,a;
	int lineHeight;
	int fontSize;
	bool fontBold;
	bool fontItalics;
	wxString fontFace;
	wxFont font;

	glyphMap glyphs;
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
	static wxFont GetFont() { return GetInstance().font; }
	static void SetFont(wxString face=_T("Verdana"),int size=10,bool bold=true,bool italics=false) { GetInstance().DoSetFont(face,size,bold,italics); }
	static void SetColour(wxColour col,float alpha=1.0f) { GetInstance().DoSetColour(col,alpha); }
	static void Print(wxString text,int x,int y) { GetInstance().DoPrint(text,x,y); }
	static void GetExtent(wxString text,int &w,int &h) { GetInstance().DoGetExtent(text,w,h); }
};
