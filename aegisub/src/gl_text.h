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

#ifndef AGI_PRE
#include <map>
#include <tr1/memory>
#include <vector>

#include <wx/colour.h>
#include <wx/font.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

struct OpenGLTextGlyph;
class OpenGLTextTexture;

/// DOCME
typedef std::map<int,OpenGLTextGlyph> glyphMap;

/// DOCME
/// @class OpenGLText
/// @brief DOCME
///
/// DOCME
class OpenGLText {
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
	std::vector<std::tr1::shared_ptr<OpenGLTextTexture> > textures;

	OpenGLText(OpenGLText const&);
	OpenGLText& operator=(OpenGLText const&);

	/// @brief Get the glyph for the character chr, creating it if necessary
	/// @param chr Character to get the glyph of
	/// @return The appropriate OpenGLTextGlyph
	OpenGLTextGlyph const& GetGlyph(int chr);
	/// @brief Create a new glyph
	OpenGLTextGlyph const& CreateGlyph(int chr);

	void DrawString(const wxString &text,int x,int y);
public:
	/// @brief Get the currently active font
	wxFont GetFont() const { return font; }

	/// @brief Set the currently active font
	/// @param face    Name of the desired font
	/// @param size    Size in points of the desired font
	/// @param bold    Should the font be bold?
	/// @param italics Should the font be italic?
	void SetFont(wxString face,int size,bool bold,bool italics);
	/// @brief Set the text color
	/// @param col   Color
	/// @param alpha Alpha value from 0.f-1.f
	void SetColour(wxColour col,float alpha);
	/// @brief Print a string on screen
	/// @param text String to print
	/// @param x    x coordinate
	/// @param y    y coordinate
	void Print(const wxString &text,int x,int y);
	/// @brief Get the extents of a string printed with the current font in pixels
	/// @param text String to get extends of
	/// @param[out] w    Width
	/// @param[out] h    Height
	void GetExtent(const wxString &text,int &w,int &h);

	OpenGLText();
	~OpenGLText();
};
