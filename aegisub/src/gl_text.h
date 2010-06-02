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
#include <vector>
#include "boost/shared_ptr.hpp"

#include <wx/font.h>
#endif

#ifdef __APPLE__
#include <OpenGL/GL.h>
#else
#include <GL/gl.h>
#endif

class OpenGLTextGlyph;
class OpenGLTextTexture;

/// DOCME
typedef std::map<int,OpenGLTextGlyph> glyphMap;

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
	std::vector<boost::shared_ptr<OpenGLTextTexture> > textures;

	OpenGLText();
	~OpenGLText();
	OpenGLText(OpenGLText const&);
	OpenGLText& operator=(OpenGLText const&);

	/// @brief Get the glyph for the character chr, creating it if necessary
	/// @param chr Character to get the glyph of
	/// @return The appropriate OpenGLTextGlyph
	OpenGLTextGlyph const& GetGlyph(int chr);
	/// @brief Create a new glyph
	OpenGLTextGlyph const& CreateGlyph(int chr);

	/// @brief Get the singleton OpenGLText instance
	static OpenGLText& GetInstance();
	/// @brief Set the currently active font
	/// @param face    Name of the desired font
	/// @param size    Size in points of the desired font
	/// @param bold    Should the font be bold?
	/// @param italics Should the font be italic?
	void DoSetFont(wxString face,int size,bool bold,bool italics);
	/// @brief Set the text color
	/// @param col   Color
	/// @param alpha Alpha value from 0.f-1.f
	void DoSetColour(wxColour col,float alpha);
	/// @brief Print a string onscreen
	/// @param text String to print
	/// @param x    x coordinate
	/// @param y    y coordinate
	void DoPrint(const wxString &text,int x,int y);
	void DrawString(const wxString &text,int x,int y);
	/// @brief Get the extents of a string printed with the current font in pixels
	/// @param text String to get extends of
	/// @param[out] w    Width
	/// @param[out] h    Height
	void DoGetExtent(const wxString &text,int &w,int &h);

public:
	/// @brief Get the currently active font
	static wxFont GetFont() { return GetInstance().font; }

	/// @brief Set the currently active font
	/// @param face    Name of the desired font
	/// @param size    Size in points of the desired font
	/// @param bold    Should the font be bold?
	/// @param italics Should the font be italic?
	static void SetFont(wxString face=_T("Verdana"),int size=10,bool bold=true,bool italics=false) { GetInstance().DoSetFont(face,size,bold,italics); }

	/// @brief Set the text color
	/// @param col   Color
	/// @param alpha Alpha value from 0.f-1.f
	static void SetColour(wxColour col,float alpha=1.0f) { GetInstance().DoSetColour(col,alpha); }

	/// @brief Print a string onscreen
	/// @param text String to print
	/// @param x    x coordinate
	/// @param y    y coordinate
	static void Print(const wxString &text,int x,int y) { GetInstance().DoPrint(text,x,y); }

	/// @brief Get the extents of a string printed with the current font in pixels
	/// @param text   String to get extends of
	/// @param[out] w Width
	/// @param[out] h Height
	static void GetExtent(const wxString &text,int &w,int &h) { GetInstance().DoGetExtent(text,w,h); }
};
