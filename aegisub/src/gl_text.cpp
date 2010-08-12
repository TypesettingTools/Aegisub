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

/// @file gl_text.cpp
/// @brief Create and render text using OpenGL
/// @ingroup video_output
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

#include <algorithm>
#endif

#include "gl_text.h"
#include "utils.h"

/// @class OpenGLTextGlyph
/// @brief Struct storing the information needed to draw a glyph
struct OpenGLTextGlyph {
	wxString str; ///< String containing the glyph(s) this is for
	int tex;      ///< OpenGL texture to draw for this glyph
	float x1;     ///< Left x coordinate of this glyph in the containing texture
	float x2;     ///< Right x coordinate of this glyph in the containing texture
	float y1;     ///< Left y coordinate of this glyph in the containing texture
	float y2;     ///< Right y coordinate of this glyph in the containing texture
	int w;        ///< Width of the glyph in pixels
	int h;        ///< Height of the glyph in pixels
	wxFont font;  ///< Font used for this glyph

	OpenGLTextGlyph(int value, wxFont const& font);
	void Draw(int x,int y) const;
};


/// @class OpenGLTextTexture
/// @brief OpenGL texture which stores one or more glyphs as sprites
class OpenGLTextTexture {
	int x;      ///< Next x coordinate at which a glyph can be inserted
	int y;      ///< Next y coordinate at which a glyph can be inserted
	int nextY;  ///< Y coordinate of the next line; tracked due to that lines
	            ///< are only as tall as needed to fit the glyphs in them
	int width;  ///< Width of the texture
	int height; ///< Height of the texture
	GLuint tex; ///< The texture

	/// Insert the glyph into this texture at the current coordinates
	void Insert(OpenGLTextGlyph &glyph);

public:
	/// @brief Try to insert a glyph into this texture
	/// @param[in][out] glyph Texture to insert
	/// @return Was the texture successfully added?
	bool TryToInsert(OpenGLTextGlyph &glyph);

	OpenGLTextTexture(OpenGLTextGlyph &glyph);
	~OpenGLTextTexture();
};


OpenGLText::OpenGLText()
: r(1.f)
, g(1.f)
, b(1.f)
, a(1.f)
, lineHeight(0)
, fontSize(0)
, fontBold(false)
, fontItalics(false)
{
}

OpenGLText::~OpenGLText() {
}

void OpenGLText::SetFont(wxString face,int size,bool bold,bool italics) {
	// No change required
	if (size == fontSize && face == fontFace && bold == fontBold && italics == fontItalics) return;

	// Set font
	fontFace = face;
	fontSize = size;
	fontBold = bold;
	fontItalics = italics;
	font.SetFaceName(fontFace);
	font.SetPointSize(size);
	font.SetWeight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);

	// Delete all old data
	textures.clear();
	glyphs.clear();
}

void OpenGLText::SetColour(wxColour col,float alpha) {
	r = col.Red()   / 255.f;
	g = col.Green() / 255.f;
	b = col.Blue()  / 255.f;
	a = alpha;
}

void OpenGLText::Print(const wxString &text,int x,int y) {
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	// Draw border
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	DrawString(text,x-1,y);
	DrawString(text,x+1,y);
	DrawString(text,x,y-1);
	DrawString(text,x,y+1);

	// Draw primary string
	glColor4f(r,g,b,a);
	DrawString(text,x,y);

	// Disable blend
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void OpenGLText::DrawString(const wxString &text,int x,int y) {
	size_t len = text.Length();
	lineHeight = 0;
	int dx=x,dy=y;

	for (size_t i=0;i<len;i++) {
		int curChar = text[i];

		// Handle carriage returns
		if (curChar == '\n') {
			dx = x;
			dy += lineHeight;
		}

		// Handle normal glyphs
		else {
			OpenGLTextGlyph const& glyph = GetGlyph(curChar);
			glyph.Draw(dx,dy);
			dx += glyph.w;
			if (glyph.h > lineHeight) lineHeight = glyph.h;
		}
	}
}

void OpenGLText::GetExtent(wxString const& text, int &w, int &h) {
	size_t len = text.Length();
	lineHeight = 0;
	int dx=0,dy=0;
	w = 0;
	h = 0;

	// Simulate drawing of string
	for (size_t i=0;i<len;i++) {
		// Get current character
		int curChar = text[i];

		// Handle carriage returns
		if (curChar == '\n') {
			if (dx > w) w = dx;
			dx = 0;
			dy += lineHeight;
			lineHeight = 0;
		}

		// Handle normal glyphs
		else {
			OpenGLTextGlyph const& glyph = GetGlyph(curChar);
			dx += glyph.w;
			if (glyph.h > lineHeight) lineHeight = glyph.h;
		}
	}

	// Return results
	if (dx > w) w = dx;
	h = dy+lineHeight;
}

OpenGLTextGlyph const& OpenGLText::GetGlyph(int i) {
	glyphMap::iterator res = glyphs.find(i);

	if (res != glyphs.end()) return res->second;
	return CreateGlyph(i);
}

OpenGLTextGlyph const& OpenGLText::CreateGlyph(int n) {
	OpenGLTextGlyph &glyph = glyphs.insert(std::make_pair(n, OpenGLTextGlyph(n, font))).first->second;

	// Insert into some texture
	bool ok = false;
	for (unsigned int i=0;i<textures.size();i++) {
		if (textures[i]->TryToInsert(glyph)) {
			ok = true;
			break;
		}
	}

	// No texture could fit it, create a new one
	if (!ok) {
		textures.push_back(std::tr1::shared_ptr<OpenGLTextTexture>(new OpenGLTextTexture(glyph)));
	}

	return glyph;
}

/// @brief Texture constructor 
/// @param w 
/// @param h 
OpenGLTextTexture::OpenGLTextTexture(OpenGLTextGlyph &glyph) {
	x = y = nextY = 0;
	width = std::max(SmallestPowerOf2(glyph.w), 64);
	height = std::max(SmallestPowerOf2(glyph.h), 64);
	width = height = std::max(width, height);
	tex = 0;

	// Generate and bind
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// Texture parameters
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	// Allocate texture
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_ALPHA,GL_UNSIGNED_BYTE,NULL);
	if (glGetError()) throw _T("Internal OpenGL text renderer error: Could not allocate Text Texture");

	TryToInsert(glyph);
}

OpenGLTextTexture::~OpenGLTextTexture() {
	if (tex) glDeleteTextures(1, &tex);
}

/// @brief Can fit a glyph in it? 
/// @param glyph 
/// @return 
bool OpenGLTextTexture::TryToInsert(OpenGLTextGlyph &glyph) {
	int w = glyph.w;
	int h = glyph.h;
	if (w > width) return false;
	if (y+h > height) return false;

	// Can fit in this row?
	if (x + w < width) {
		Insert(glyph);
		x += w;
		if (y+h > nextY) nextY = y+h;
		return true;
	}

	// Can fit the next row?
	else {
		if (nextY+h > height) return false;
		x = 0;
		y = nextY;
		nextY = y+h;
		Insert(glyph);
		return true;
	}
}

/// @brief Insert 
/// @param glyph 
void OpenGLTextTexture::Insert(OpenGLTextGlyph &glyph) {
	int w = glyph.w;
	int h = glyph.h;

	// Fill glyph structure
	glyph.x1 = float(x)/width;
	glyph.y1 = float(y)/height;
	glyph.x2 = float(x+w)/width;
	glyph.y2 = float(y+h)/height;
	glyph.tex = tex;

	// Create bitmap and bind it to a DC
	wxBitmap bmp(w + (w & 1), h + (h & 1),24);
	wxMemoryDC dc(bmp);

	// Draw text and convert to image
	dc.SetBackground(wxBrush(wxColour(0,0,0)));
	dc.Clear();
	dc.SetFont(glyph.font);
	dc.SetTextForeground(wxColour(255,255,255));
	dc.DrawText(glyph.str,0,0);
	wxImage img = bmp.ConvertToImage();

	// Convert to alpha
	int imgw = img.GetWidth();
	int imgh = img.GetHeight();
	size_t len = imgw*imgh;
	const unsigned char *src = img.GetData();
	const unsigned char *read = src;
	std::vector<unsigned char> alpha(len * 2, 255);
	unsigned char *write = &alpha[1];
	for (size_t i=0;i<len;i++) {
		*write = *read;
		write += 2;
		read += 3;
	}

	// Upload image to video memory
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D,0,x,y,imgw,imgh,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,&alpha[0]);
	if (glGetError()) throw _T("Internal OpenGL text renderer error: Error uploading glyph data to video memory.");
}

/// Draw a glyph at (x,y)
void OpenGLTextGlyph::Draw(int x,int y) const {
	// Store matrix and translate
	glPushMatrix();
	glTranslatef((float)x,(float)y,0.0f);

	glBindTexture(GL_TEXTURE_2D, tex);

	glBegin(GL_QUADS);
		glTexCoord2f(x1,y1); glVertex2f(0,0); // Top-left
		glTexCoord2f(x1,y2); glVertex2f(0,h); // Bottom-left
		glTexCoord2f(x2,y2); glVertex2f(w,h); // Bottom-right
		glTexCoord2f(x2,y1); glVertex2f(w,0); // Top-right
	glEnd();

	glPopMatrix();
}

/// @brief DOCME
OpenGLTextGlyph::OpenGLTextGlyph(int value, wxFont const& font)
: str(wxChar(value))
, font(font)
{
	wxCoord desc,lead;

	wxBitmap tempBmp(32, 32, 24);
	wxMemoryDC dc(tempBmp);
	dc.SetFont(font);
	dc.GetTextExtent(str,&w,&h,&desc,&lead);
}
