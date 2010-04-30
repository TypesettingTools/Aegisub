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


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/dcmemory.h>
#include <wx/image.h>
#include <algorithm>
#endif

#include "gl_text.h"
#include "utils.h"

OpenGLText::OpenGLText() {
	r = g = b = a = 1.0f;
}

OpenGLText::~OpenGLText() {
	Reset();
}

/// @brief Reset 
void OpenGLText::Reset() {
	textures.clear();
	glyphs.clear();
}

/// @brief Get instance 
/// @return 
OpenGLText& OpenGLText::GetInstance() {
	static OpenGLText instance;
	return instance;
}

/// @brief Set font 
/// @param face    
/// @param size    
/// @param bold    
/// @param italics 
/// @return 
///
void OpenGLText::DoSetFont(wxString face,int size,bool bold,bool italics) {
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
	Reset();
}

/// @brief Set colour 
/// @param col   
/// @param alpha 
///
void OpenGLText::DoSetColour(wxColour col,float alpha) {
	r = col.Red() / 255.0f;
	g = col.Green() / 255.0f;
	b = col.Blue() / 255.0f;
	a = alpha;
}

/// @brief Print 
/// @param text 
/// @param x    
/// @param y    
void OpenGLText::DoPrint(const wxString &text,int x,int y) {
	// Set OpenGL
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

/// @brief Draw a string at (x,y)
void OpenGLText::DrawString(const wxString &text,int x,int y) {
	// Variables
	size_t len = text.Length();
	OpenGLTextGlyph glyph;
	lineHeight = 0;
	int dx=x,dy=y;

	// Draw string
	for (size_t i=0;i<len;i++) {
		// Get current character
		int curChar = text[i];

		// Handle carriage returns
		if (curChar == '\n') {
			dx = x;
			dy += lineHeight;
		}

		// Handle normal glyphs
		else {
			glyph = GetGlyph(curChar);
			glyph.Draw(dx,dy);
			dx += glyph.w;
			if (glyph.h > lineHeight) lineHeight = glyph.h;
		}
	}
}

/// @brief Calculate text extent 
/// @param text Text to get the extents of
/// @param w    [out] Width
/// @param h    [out] Height
void OpenGLText::DoGetExtent(const wxString &text,int &w,int &h) {
	size_t len = text.Length();
	OpenGLTextGlyph glyph;
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
			glyph = GetGlyph(curChar);
			dx += glyph.w;
			if (glyph.h > lineHeight) lineHeight = glyph.h;
		}
	}

	// Return results
	if (dx > w) w = dx;
	h = dy+lineHeight;
}

/// @brief Get a glyph 
/// @param i 
/// @return 
OpenGLTextGlyph OpenGLText::GetGlyph(int i) {
	glyphMap::iterator res = glyphs.find(i);

	// Found
	if (res != glyphs.end()) return res->second;

	// Not found, create it
	return CreateGlyph(i);
}

/// @brief Create a glyph 
/// @param n 
OpenGLTextGlyph OpenGLText::CreateGlyph(int n) {
	// Create glyph
	OpenGLTextGlyph glyph;
	glyph.value = n;
	glyph.GetMetrics();

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
		textures.push_back(new OpenGLTextTexture(glyph.w,glyph.h));
		textures.back()->TryToInsert(glyph);
	}

	// Set glyph and return it
	glyphs[n] = glyph;
	return glyph;
}

/// @brief Texture constructor 
/// @param w 
/// @param h 
OpenGLTextTexture::OpenGLTextTexture(int w,int h) {
	using std::max;
	// Properties
	x = y = nextY = 0;
	width = max(SmallestPowerOf2(w), 64);
	height = max(SmallestPowerOf2(h), 64);
	width = height = max(width, height);
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
}

OpenGLTextTexture::~OpenGLTextTexture() {
	if (tex) {
		glDeleteTextures(1,&tex);
		tex = 0;
	}
}

/// @brief Can fit a glyph in it? 
/// @param glyph 
/// @return 
bool OpenGLTextTexture::TryToInsert(OpenGLTextGlyph &glyph) {
	// Get size
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
	// Glyph data
	wxString str = wxChar(glyph.value);
	int w = glyph.w;
	int h = glyph.h;

	// Fill glyph structure
	glyph.x1 = float(x)/width;
	glyph.y1 = float(y)/height;
	glyph.x2 = float(x+w)/width;
	glyph.y2 = float(y+h)/height;
	glyph.tex = tex;

	// Create bitmap and bind it to a DC
	wxBitmap bmp(((w+1)/2)*2,((h+1)/2)*2,24);
	wxMemoryDC dc(bmp);

	// Draw text and convert to image
	dc.SetBackground(wxBrush(wxColour(0,0,0)));
	dc.Clear();
	dc.SetFont(OpenGLText::GetFont());
	dc.SetTextForeground(wxColour(255,255,255));
	dc.DrawText(str,0,0);
	wxImage img = bmp.ConvertToImage();

	// Convert to alpha
	int imgw = img.GetWidth();
	int imgh = img.GetHeight();
	size_t len = imgw*imgh;
	const unsigned char *src = img.GetData();
	const unsigned char *read = src;
	unsigned char *alpha = new unsigned char[len*2];
	unsigned char *write = alpha;
	for (size_t i=0;i<len;i++) {
		*write++ = 255;
		*write++ = *read++;
		read += 2;
	}

	// Upload image to video memory
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D,0,x,y,imgw,imgh,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,alpha);
	delete[] alpha;
	if (glGetError()) throw _T("Internal OpenGL text renderer error: Error uploading glyph data to video memory.");
}

/// @brief Draw a glyph at (x,y)
/// @param x 
/// @param y 
void OpenGLTextGlyph::Draw(int x,int y) {
	// Store matrix and translate
	glPushMatrix();
	glTranslatef((float)x,(float)y,0.0f);

	// Set texture
	glBindTexture(GL_TEXTURE_2D, tex);

	// Draw quad
	glBegin(GL_QUADS);
		// Top-left
		glTexCoord2f(x1,y1);
		glVertex2f(0,0);
		// Bottom-left
		glTexCoord2f(x1,y2);
		glVertex2f(0,h);
		// Bottom-right
		glTexCoord2f(x2,y2);
		glVertex2f(w,h);
		// Top-right
		glTexCoord2f(x2,y1);
		glVertex2f(w,0);
	glEnd();

	// Restore matrix
	glPopMatrix();
}

OpenGLTextGlyph::~OpenGLTextGlyph() {
	if (tempBmp) delete tempBmp;
	tempBmp = NULL;
}

/// DOCME
wxBitmap *OpenGLTextGlyph::tempBmp = NULL;

/// @brief DOCME
void OpenGLTextGlyph::GetMetrics() {
	// Glyph data
	wxCoord desc,lead;
	wxString str = wxChar(value);

	// Create bitmap, if needed
	if (!tempBmp) tempBmp = new wxBitmap(16,16,24);

	// Get text extents
	wxMemoryDC dc(*tempBmp);
	dc.SetFont(OpenGLText::GetFont());
	dc.GetTextExtent(str,&w,&h,&desc,&lead);
}
