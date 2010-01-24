// Copyright (c) 2009, Thomas Goyne
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
#endif

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

// Windows only has headers for OpenGL 1.1 and GL_CLAMP_TO_EDGE is 1.2
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#define CHECK_INIT_ERROR(cmd) cmd; if (GLenum err = glGetError()) throw VideoOutInitException(_T(#cmd), err)
#define CHECK_ERROR(cmd) cmd; if (GLenum err = glGetError()) throw VideoOutRenderException(_T(#cmd), err)

/// @brief Structure tracking all precomputable information about a subtexture
struct VideoOutGL::TextureInfo {
	/// The OpenGL texture id this is for
	GLuint textureID;
	/// The byte offset into the frame's data block
	int dataOffset;
	int sourceH;
	int sourceW;

	float destX1;
	float destY1;
	float destX2;
	float destY2;

	float texTop;
	float texBottom;
	float texLeft;
	float texRight;

	TextureInfo()
		: textureID(0)
		, dataOffset(0)
		, sourceH(0)
		, sourceW(0)
		, destX1(0)
		, destY1(0)
		, destX2(0)
		, destY2(0)
		, texTop(0)
		, texBottom(1.0f)
		, texLeft(0)
		, texRight(1.0f)
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

	wxLogDebug("VideoOutGL::TestTexture: %dx%d\n", width, height);
	return format != 0;
}

VideoOutGL::VideoOutGL()
:	maxTextureSize(0),
	supportsRectangularTextures(false),
	supportsGlClampToEdge(false),
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
	else throw VideoOutInitException(L"Could not create a 64x64 RGB texture in any format.");

	// Test for the maximum supported texture size
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	while (maxTextureSize > 64 && !TestTexture(maxTextureSize, maxTextureSize, internalFormat)) maxTextureSize >>= 1;
	wxLogDebug("VideoOutGL::DetectOpenGLCapabilities: Maximum texture size is %dx%d\n", maxTextureSize, maxTextureSize);

	// Test for rectangular texture support
	supportsRectangularTextures = TestTexture(maxTextureSize, maxTextureSize >> 1, internalFormat);

	// Test GL_CLAMP_TO_EDGE support
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if (glGetError()) {
		supportsGlClampToEdge = false;
		wxLogDebug("VideoOutGL::DetectOpenGLCapabilities: Using GL_CLAMP\n");
	}
	else {
		supportsGlClampToEdge = true;
		wxLogDebug("VideoOutGL::DetectOpenGLCapabilities: Using GL_CLAMP_TO_EDGE\n");
	}
}

/// @brief If needed, create the grid of textures for displaying frames of the given format
/// @param width The frame's width
/// @param height The frame's height
/// @param format The frame's format
/// @param bpp The frame's bytes per pixel
void VideoOutGL::InitTextures(int width, int height, GLenum format, int bpp, bool flipped) {
	// Do nothing if the frame size and format are unchanged
	if (width == frameWidth && height == frameHeight && format == frameFormat && flipped == frameFlipped) return;
	frameWidth   = width;
	frameHeight  = height;
	frameFormat  = format;
	frameFlipped = flipped;
	wxLogDebug("VideoOutGL::InitTextures: Video size: %dx%d\n", width, height);

	DetectOpenGLCapabilities();

	// Clean up old textures
	if (textureIdList.size() > 0) {
		CHECK_INIT_ERROR(glDeleteTextures(textureIdList.size(), &textureIdList[0]));
		textureIdList.clear();
		textureList.clear();
	}

	// Create the textures
	textureRows  = (int)ceil(double(height) / maxTextureSize);
	textureCols  = (int)ceil(double(width) / maxTextureSize);
	textureIdList.resize(textureRows * textureCols);
	textureList.resize(textureRows * textureCols);
	CHECK_INIT_ERROR(glGenTextures(textureIdList.size(), &textureIdList[0]));

	// Calculate the position information for each texture
	int sourceY = 0;
	float destY = -1.0f;
	for (int i = 0; i < textureRows; i++) {
		int sourceX = 0;
		float destX = -1.0f;

		int sourceH = maxTextureSize;
		int textureH = maxTextureSize;
		// If the last row doesn't need a full texture, shrink it to the smallest one possible
		if (i == textureRows - 1 && height % maxTextureSize > 0) {
			sourceH = height % maxTextureSize;
			textureH = SmallestPowerOf2(sourceH);
		}

		for (int j = 0; j < textureCols; j++) {
			TextureInfo& ti = textureList[i * textureCols + j];

			// Copy the current position information into the struct
			ti.destX1 = destX;
			ti.destY1 = destY;
			ti.sourceH = sourceH;
			ti.textureID = textureIdList[i * textureCols + j];

			ti.sourceW = maxTextureSize;
			int textureW = maxTextureSize;
			// If the last column doesn't need a full texture, shrink it to the smallest one possible
			if (j == textureCols - 1 && width % maxTextureSize > 0) {
				ti.sourceW = width % maxTextureSize;
				textureW = SmallestPowerOf2(ti.sourceW);
			}

			int w = textureW;
			int h = textureH;
			if (!supportsRectangularTextures) w = h = MAX(w, h);

			CreateTexture(w, h, ti, format);

			if (!supportsGlClampToEdge) {
				// Stretch the texture a half pixel in each direction to eliminate the border
				ti.texLeft = 1.0f / (2 * w);
				ti.texTop = 1.0f / (2 * h);
			}

			ti.destX2 = ti.destX1 + w * 2.0f / width;
			ti.destY2 = ti.destY1 + h * 2.0f / height;

			ti.texRight = 1.0f - ti.texLeft;
			if (flipped) {
				ti.texBottom = 1.0f - ti.texTop;
				ti.dataOffset = sourceY * width * bpp + sourceX * bpp;
			}
			else {
				ti.texBottom = ti.texTop - float(h - ti.sourceH) / h;
				ti.texTop = 1.0f - ti.texTop - float(h - ti.sourceH) / h;

				ti.dataOffset = (height - sourceY - ti.sourceH) * width * bpp + sourceX * bpp;
			}

			destX = ti.destX2;
			sourceX += ti.sourceW;
		}
		destY += sourceH * 2.0f / height;
		sourceY += sourceH;
	}
}

void VideoOutGL::CreateTexture(int w, int h, const TextureInfo& ti, GLenum format) {
	CHECK_INIT_ERROR(glBindTexture(GL_TEXTURE_2D, ti.textureID));
	CHECK_INIT_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, NULL));
	wxLogDebug("VideoOutGL::InitTextures: Using texture size: %dx%d\n", w, h);
	CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLint mode = supportsGlClampToEdge ? GL_CLAMP_TO_EDGE : GL_CLAMP;
	CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode));
	CHECK_INIT_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode));
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
void VideoOutGL::SetViewport(int x, int y, int width, int height) {
	CHECK_ERROR(glViewport(x, y, width, height));
}

void VideoOutGL::Render(int sw, int sh) {
	// Clear the frame buffer
	CHECK_ERROR(glClearColor(0,0,0,0));
	CHECK_ERROR(glClearStencil(0));
	CHECK_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));


	CHECK_ERROR(glShadeModel(GL_FLAT));
	CHECK_ERROR(glDisable(GL_BLEND));

	CHECK_ERROR(glMatrixMode(GL_PROJECTION));
	CHECK_ERROR(glLoadIdentity());

	// Render the current frame
	CHECK_ERROR(glEnable(GL_TEXTURE_2D));

	for (unsigned i = 0; i < textureList.size(); i++) {
		TextureInfo& ti = textureList[i];

		CHECK_ERROR(glBindTexture(GL_TEXTURE_2D, ti.textureID));
		CHECK_ERROR(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));

		glBegin(GL_QUADS);
			glTexCoord2f(ti.texLeft,  ti.texTop);     glVertex2f(ti.destX1, ti.destY1);
			glTexCoord2f(ti.texRight, ti.texTop);     glVertex2f(ti.destX2, ti.destY1);
			glTexCoord2f(ti.texRight, ti.texBottom);  glVertex2f(ti.destX2, ti.destY2);
			glTexCoord2f(ti.texLeft,  ti.texBottom);  glVertex2f(ti.destX1, ti.destY2);
		glEnd();
		if (GLenum err = glGetError()) throw VideoOutRenderException(L"GL_QUADS", err);
	}
	CHECK_ERROR(glDisable(GL_TEXTURE_2D));

	CHECK_ERROR(glOrtho(0.0f, sw, sh, 0.0f, -1000.0f, 1000.0f));
	CHECK_ERROR(glMatrixMode(GL_MODELVIEW));
	CHECK_ERROR(glLoadIdentity());
}

VideoOutGL::~VideoOutGL() {
	if (textureIdList.size() > 0) {
		glDeleteTextures(textureIdList.size(), &textureIdList[0]);
	}
}
