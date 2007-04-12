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
#include <GL/gl.h>
#include <map>
#include <vector>


/////////////////////
// Glyph information
class OpenGLTextGlyph {
public:
	int tex;
	float x,y,w,h;
};

typedef std::map<int,OpenGLTextGlyph> glyphMap;


///////////////
// Texture map
class OpenGLTextTexture {
public:
	GLuint tex;

	bool CanFit(int w,int h);
	void Insert(OpenGLTextGlyph glyph);

	OpenGLTextTexture();
	~OpenGLTextTexture();
};


/////////////////////////////
// OpenGL Text Drawing class
class OpenGLText {
private:
	float r,g,b,a;
	static OpenGLText* instance;

	glyphMap glyphs;
	std::vector <OpenGLTextTexture*> textures;

	OpenGLText();
	~OpenGLText();

	OpenGLTextGlyph GetGlyph(int i);
	OpenGLTextGlyph CreateGlyph(int i);

	static OpenGLText* GetInstance();
	void DoSetFont(wxString face,int size);
	void DoSetColour(wxColour col,float alpha);
	void DoPrint(wxString text,int x,int y);

public:
	static void SetFont(wxString face,int size) { GetInstance()->DoSetFont(face,size); }
	static void SetColour(wxColour col,float alpha=1.0f) { GetInstance()->DoSetColour(col,alpha); }
	static void Print(wxString text,int x,int y) { GetInstance()->DoPrint(text,x,y); }
};
