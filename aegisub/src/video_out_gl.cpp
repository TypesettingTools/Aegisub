// Copyright (c) 2010, Thomas Goyne
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

/// @file video_out_gl.cpp
/// @brief OpenGL based video renderer
/// @ingroup video
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/log.h>
#include <algorithm>
#include <utility>
#endif

using std::min;
using std::max;

// These must be included before local headers.
#ifdef __APPLE__
#include <OpenGL/GL.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "video_out_gl.h"
#include "utils.h"
#include "video_frame.h"

#define CHECK_INIT_ERROR(cmd) cmd; if (GLenum err = glGetError()) throw VideoOutInitException(#cmd, err)
#define CHECK_ERROR(cmd) cmd; if (GLenum err = glGetError()) throw VideoOutRenderException(#cmd, err)

/// @brief Structure tracking all precomputable information about a subtexture
struct VideoOutGL::TextureInfo {
	GLuint textureID;
	int dataOffset;
	int sourceH;
	int sourceW;
	TextureInfo()
		: textureID(0)
		, dataOffset(0)
		, sourceH(0)
		, sourceW(0)
	{ }
};

/// @brief Test if a texture can be created
/// @param width The width of the texture
/// @param height The height of the texture
/// @param format The texture's format
/// @return Whether the texture could be created.
static bool TestTexture(int width, int height, GLint format) {
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
	while (glGetError()) { } // Silently swallow all errors as we don't care why it failed if it did

	wxLogDebug(L"VideoOutGL::TestTexture: %dx%d\n", width, height);
	return format != 0;
}

VideoOutGL::VideoOutGL()
:	maxTextureSize(0),
	supportsRectangularTextures(false),
	internalFormat(0),
	frameWidth(0),
	frameHeight(0),
	frameFormat(0),
	frameFlipped(false),
	textureIdList(),
	textureList(),
	textureCount(0),
	textureRows(0),
	textureCols(0)
{ }

/// @brief Runtime detection of required OpenGL capabilities
void VideoOutGL::DetectOpenGLCapabilities() {
	if (maxTextureSize != 0) return;

	// Test for supported internalformats
	if (TestTexture(64, 64, GL_RGBA8)) internalFormat = GL_RGBA8;
	else if (TestTexture(64, 64, GL_RGBA)) internalFormat = GL_RGBA;
	else throw VideoOutInitException("Could not create a 64x64 RGB texture in any format.");

	// Test for the maximum supported texture size
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	while (maxTextureSize > 64 && !TestTexture(maxTextureSize, maxTextureSize, internalFormat)) maxTextureSize >>= 1;
	wxLogDebug(L"VideoOutGL::DetectOpenGLCapabilities: Maximum texture size is %dx%d\n", maxTextureSize, maxTextureSize);

	// Test for rectangular texture support
	supportsRectangularTextures = TestTexture(maxTextureSize, maxTextureSize >> 1, internalFormat);
}

/// @brief If needed, create the grid of textures for displaying frames of the given format
/// @param width The frame's width
/// @param height The frame's height
/// @param format The frame's format
/// @param bpp The frame's bytes per pixel
void VideoOutGL::InitTextures(int width, int height, GLenum format, int bpp, bool flipped) {
	using namespace std;

	frameFlipped = flipped;
	// Do nothing if the frame size and format are unchanged
	if (width == frameWidth && height == frameHeight && format == frameFormat) return;
	frameWidth  = width;
	frameHeight = height;
	frameFormat = format;
	wxLogDebug(L"VideoOutGL::InitTextures: Video size: %dx%d\n", width, height);

	DetectOpenGLCapabilities();

	// Clean up old textures
	if (textureIdList.size() > 0) {
		CHECK_INIT_ERROR(glDeleteTextures(textureIdList.size(), &textureIdList[0]));
		textureIdList.clear();
		textureList.clear();
	}

	// Create the textures
	int textureArea = maxTextureSize - 2;
	textureRows  = (int)ceil(double(height) / textureArea);
	textureCols  = (int)ceil(double(width) / textureArea);
	textureCount = textureRows * textureCols;
	textureIdList.resize(textureCount);
	textureList.resize(textureCount);
	CHECK_INIT_ERROR(glGenTextures(textureIdList.size(), &textureIdList[0]));
	vector<pair<int, int> > textureSizes;
	textureSizes.reserve(textureCount);

	/* Unfortunately, we can't simply use one of the two standard ways to do
	 * tiled textures to work around texture size limits in OpenGL, due to our
	 * need to support Microsoft's OpenGL emulation for RDP/VPC/video card
	 * drivers that don't support OpenGL (such as the ones which Windows
	 * Update pushes for ATI cards in Windows 7). GL_CLAMP_TO_EDGE requires
	 * OpenGL 1.2, but the emulation only supports 1.1. GL_CLAMP + borders has
	 * correct results, but takes several seconds to render each frame. As a
	 * result, the code below essentially manually reimplements borders, by
	 * just not using the edge when mapping the texture onto a quad. The one
	 * exception to this is the texture edges which are also frame edges, as
	 * there does not appear to be a trivial way to mirror the edges, and the
	 * nontrivial ways are more complex that is worth to avoid a single row of
	 * slightly discolored pixels along the edges at zooms over 100%.
	 *
	 * Given a 64x64 maximum texture size:
	 *     Quads touching the top of the frame are 63 pixels tall
	 *     Quads touching the bottom of the frame are up to 63 pixels tall
	 *     All other quads are 62 pixels tall
	 *     Quads not on the top skip the first row of the texture
	 *     Quads not on the bottom skip the last row of the texture
	 *     Width behaves in the same way with respect to left/right edges
	 */

	// Set up the display list
	dl = glGenLists(1);
	glNewList(dl, GL_COMPILE);

	CHECK_ERROR(glClearColor(0,0,0,0));
	CHECK_ERROR(glClearStencil(0));
	CHECK_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));

	CHECK_ERROR(glShadeModel(GL_FLAT));
	CHECK_ERROR(glDisable(GL_BLEND));

	// Switch to video coordinates
	CHECK_ERROR(glMatrixMode(GL_PROJECTION));
	CHECK_ERROR(glLoadIdentity());
	CHECK_ERROR(glPushMatrix());
	if (frameFlipped) {
		CHECK_ERROR(glOrtho(0.0f, frameWidth, 0.0f, frameHeight, -1000.0f, 1000.0f));
	}
	else {
		CHECK_ERROR(glOrtho(0.0f, frameWidth, frameHeight, 0.0f, -1000.0f, 1000.0f));
	}

	CHECK_ERROR(glEnable(GL_TEXTURE_2D));

	// Calculate the position information for each texture
	int lastRow = textureRows - 1;
	int lastCol = textureCols - 1;
	for (int row = 0; row < textureRows; ++row) {
		for (int col = 0; col < textureCols; ++col) {
			TextureInfo& ti = textureList[row * textureCols + col];

			// Width and height of the area read from the frame data
			int sourceX = col * textureArea;
			int sourceY = row * textureArea;
			ti.sourceW  = min(frameWidth  - sourceX, maxTextureSize);
			ti.sourceH  = min(frameHeight - sourceY, maxTextureSize);

			// Used instead of GL_PACK_SKIP_ROWS/GL_PACK_SKIP_PIXELS due to
			// performance issues with the emulation
			ti.dataOffset = sourceY * frameWidth * bpp + sourceX * bpp;

			int textureHeight = SmallestPowerOf2(ti.sourceH);
			int textureWidth  = SmallestPowerOf2(ti.sourceW);
			if (!supportsRectangularTextures) {
				textureWidth = textureHeight = max(textureWidth, textureHeight);
			}

			// Location where this texture is placed
			// X2/Y2 will be offscreen unless the video frame happens to
			// exactly use all of the texture
			float x1 = sourceX + (col != 0);
			float y1 = sourceY + (row != 0);
			float x2 = sourceX + textureWidth - (col != lastCol);
			float y2 = sourceY + textureHeight - (row != lastRow);

			// Portion of the texture actually used
			float top    = row == 0 ? 0 : 1.0f / textureHeight;
			float left   = col == 0 ? 0 : 1.0f / textureWidth;
			float bottom = row == lastRow ? 1.0f : 1.0f - 1.0f / textureHeight;
			float right  = col == lastCol ? 1.0f : 1.0f - 1.0f / textureWidth;

			// Store the stuff needed later
			ti.textureID = textureIdList[row * textureCols + col];
			textureSizes.push_back(make_pair(textureWidth, textureHeight));

			CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, ti.textureID));
			CHECK_ERROR(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));

			// Place the texture
			glBegin(GL_QUADS);
				glTexCoord2f(left,  top);     glVertex2f(x1, y1);
				glTexCoord2f(right, top);     glVertex2f(x2, y1);
				glTexCoord2f(right, bottom);  glVertex2f(x2, y2);
				glTexCoord2f(left,  bottom);  glVertex2f(x1, y2);
			glEnd();
			if (GLenum err = glGetError()) throw VideoOutRenderException("GL_QUADS", err);
		}
	}
	CHECK_ERROR(glDisable(GL_TEXTURE_2D));
	CHECK_ERROR(glPopMatrix());

	glEndList();

	// Create the textures outside of the display list as there's no need to
	// remake them on every frame
	for (int i = 0; i < textureCount; ++i) {
		CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, textureIdList[i]));
		CHECK_INIT_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, textureSizes[i].first, textureSizes[i].second, 0, format, GL_UNSIGNED_BYTE, NULL));
		wxLogDebug(L"VideoOutGL::InitTextures: Using texture size: %dx%d\n", textureSizes[i].first, textureSizes[i].second);
		CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
		CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
	}
}

void VideoOutGL::UploadFrameData(const AegiVideoFrame& frame) {
	if (frame.h == 0 || frame.w == 0) return;

	GLuint format = frame.invertChannels ? GL_BGRA_EXT : GL_RGBA;
	InitTextures(frame.w, frame.h, format, frame.GetBpp(0), frame.flipped);

	// Set the row length, needed to be able to upload partial rows
	CHECK_ERROR(glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.w));

	for (unsigned i = 0; i < textureList.size(); i++) {
		TextureInfo& ti = textureList[i];

		CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, ti.textureID));
		CHECK_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ti.sourceW,
			ti.sourceH, format, GL_UNSIGNED_BYTE, frame.data[0] + ti.dataOffset));
	}

	CHECK_ERROR(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
}

void VideoOutGL::Render(int dx1, int dy1, int dx2, int dy2) {
	CHECK_ERROR(glViewport(dx1, dy1, dx2, dy2));
	glCallList(dl);
	CHECK_ERROR(glMatrixMode(GL_MODELVIEW));
	CHECK_ERROR(glLoadIdentity());

}

VideoOutGL::~VideoOutGL() {
	if (textureIdList.size() > 0) {
		glDeleteTextures(textureIdList.size(), &textureIdList[0]);
		glDeleteLists(dl, 1);
	}
}
