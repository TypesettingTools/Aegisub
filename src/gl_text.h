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

#include <boost/container/map.hpp>

#include <string>
#include <vector>

#include <wx/font.h>

namespace {
struct OpenGLTextGlyph;
class OpenGLTextTexture;
}

namespace agi { struct Color; }

typedef boost::container::map<int, OpenGLTextGlyph> glyphMap;

class OpenGLText {
	float r = 1.f, g = 1.f, b = 1.f, a = 1.f;

	int fontSize = 0;
	bool fontBold = false;
	bool fontItalics = false;
	std::string fontFace;
	wxFont font;

	glyphMap glyphs;

	std::vector<OpenGLTextTexture> textures;

	OpenGLText(OpenGLText const&) = delete;
	OpenGLText& operator=(OpenGLText const&) = delete;

	/// @brief Get the glyph for the character chr, creating it if necessary
	/// @param chr Character to get the glyph of
	/// @return The appropriate OpenGLTextGlyph
	OpenGLTextGlyph const& GetGlyph(int chr);
	/// @brief Create a new glyph
	OpenGLTextGlyph const& CreateGlyph(int chr);

	void DrawString(const std::string &text,int x,int y);
public:
	/// @brief Get the currently active font
	wxFont GetFont() const { return font; }

	/// @brief Set the currently active font
	/// @param face    Name of the desired font
	/// @param size    Size in points of the desired font
	/// @param bold    Should the font be bold?
	/// @param italics Should the font be italic?
	void SetFont(std::string const& face, int size, bool bold, bool italics);
	/// @brief Set the currently active font
	/// @param font    The desired font
	void SetFont(const wxFont& newFont);
	/// @brief Set the text color
	/// @param col   Color
	void SetColour(agi::Color col);
	/// @brief Print a string on screen
	/// @param text String to print
	/// @param x    x coordinate
	/// @param y    y coordinate
	void Print(const std::string &text,int x,int y);
	/// @brief Get the extents of a string printed with the current font in pixels
	/// @param text String to get extends of
	/// @param[out] w    Width
	/// @param[out] h    Height
	void GetExtent(const std::string &text,int &w,int &h);

	OpenGLText();
	~OpenGLText();
};
