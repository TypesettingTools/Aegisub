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

namespace {
	/// @brief Structure tracking all precomputable information about a subtexture
	struct TextureInfo {
		/// The OpenGL texture id this is for
		GLuint textureID;
		/// The byte offset into the frame's data block
		int dataOffset;
		int sourceH;
		int sourceW;

		int textureH;
		int textureW;

		float destH;
		float destW;
		float destX;
		float destY;

		float texTop;
		float texBottom;
		float texLeft;
		float texRight;
	};
	/// @brief Test if a texture can be created
	/// @param width The width of the texture
	/// @param height The height of the texture
	/// @param format The texture's format
	/// @return Whether the texture could be created.
	bool TestTexture(int width, int height, GLint format) {
		GLuint texture;
		glGenTextures(1, &texture);
		glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
		glDeleteTextures(1, &texture);
		while (glGetError()) { } // Silently swallow all errors as we don't care why it failed if it did

		wxLogDebug("VideoOutGL::TestTexture: %dx%d\n", width, height);
		return format != 0;
	}
}

VideoOutGL::VideoOutGL()
:	maxTextureSize(0),
	supportsRectangularTextures(false),
	supportsGlClampToEdge(false),
	internalFormat(0),
	frameWidth(0),
	frameHeight(0),
	frameFormat(0),
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
	else throw VideoOutUnsupportedException(L"Could not create a 64x64 RGB texture in any format.");

	// Test for the maximum supported texture size
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	while (maxTextureSize > 64 && !TestTexture(maxTextureSize, maxTextureSize, internalFormat)) maxTextureSize >>= 1;
	wxLogDebug("VideoOutGL::DetectOpenGLCapabilities: Maximum texture size is %dx%d\n", maxTextureSize, maxTextureSize);

	// Test for rectangular texture support
	supportsRectangularTextures = TestTexture(maxTextureSize, maxTextureSize >> 1, internalFormat);

	// Test GL_CLAMP_TO_EDGE support
	GLuint texture;
	glGenTextures(1, &texture);
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
void VideoOutGL::InitTextures(int width, int height, GLenum format, int bpp) {
	// Do nothing if the frame size and format are unchanged
	if (width == frameWidth && height == frameHeight && format == frameFormat) return;
	wxLogDebug("VideoOutGL::InitTextures: Video size: %dx%d\n", width, height);

	DetectOpenGLCapabilities();

	// Clean up old textures
	if (textureIdList.size() > 0) {
		glDeleteTextures(textureIdList.size(), &textureIdList[0]);
		if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glDeleteTextures", err);
		textureIdList.clear();
		textureList.clear();
	}

	// Create the textures
	textureRows  = (int)ceil(double(height) / maxTextureSize);
	textureCols  = (int)ceil(double(width) / maxTextureSize);
	textureIdList.resize(textureRows * textureCols);
	textureList.resize(textureRows * textureCols);
	glGenTextures(textureIdList.size(), &textureIdList[0]);
	if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glGenTextures", err);

	// Calculate the position information for each texture
	int sourceY = 0;
	float destY = 0.0f;
	for (int i = 0; i < textureRows; i++) {
		int sourceX = 0;
		float destX = 0.0f;

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
			ti.destX = destX;
			ti.destY = destY;
			ti.sourceH = sourceH;

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

			if (supportsGlClampToEdge) {
				ti.texLeft = 0.0f;
				ti.texTop = 0.0f;
			}
			else {
				// Stretch the texture a half pixel in each direction to eliminate the border
				ti.texLeft = 1.0f / (2 * w);
				ti.texTop = 1.0f / (2 * h);
			}

			// Calculate what percent of the texture is actually used
			ti.texRight = float(ti.sourceW) / w - ti.texLeft;
			ti.texBottom = float(ti.sourceH) / h - ti.texTop;

			// destW/H is the percent of the output which this texture covers
			ti.destW = float(w) / width;
			ti.destH = float(h) / height;

			ti.textureID = textureIdList[i * textureCols + j];
			ti.dataOffset = sourceY * width * bpp + sourceX * bpp;

			// Actually create the texture and set the scaling mode
			glBindTexture(GL_TEXTURE_2D, ti.textureID);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glBindTexture", err);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, NULL);
			wxLogDebug("VideoOutGL::InitTextures: Using texture size: %dx%d\n", w, h);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexImage2D", err);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexParameteri(GL_TEXTURE_MIN_FILTER)", err);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexParameteri(GL_TEXTURE_MAG_FILTER)", err);

			GLint mode = supportsGlClampToEdge ? GL_CLAMP_TO_EDGE : GL_CLAMP;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexParameteri(GL_TEXTURE_WRAP_S)", err);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
			if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexParameteri(GL_TEXTURE_WRAP_T)", err);

			destX += ti.destW;
			sourceX += ti.sourceW;
		}
		destY += float(sourceH) / height;
		sourceY += sourceH;
	}

	// Store the information needed to know when the grid must be recreated
	frameWidth  = width;
	frameHeight = height;
	frameFormat = format;
}

void VideoOutGL::DisplayFrame(AegiVideoFrame frame, int sw, int sh) {
	if (frame.h == 0 || frame.w == 0) return;

	glEnable(GL_TEXTURE_2D);
	if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glEnable(GL_TEXTURE_2d)", err);

	GLuint format = frame.invertChannels ? GL_BGRA_EXT : GL_RGBA;
	InitTextures(frame.w, frame.h, format, frame.GetBpp(0));

	// Set the row length, needed to be able to upload partial rows
	glPixelStorei(GL_UNPACK_ROW_LENGTH, frame.w);
	if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glPixelStorei(GL_UNPACK_ROW_LENGTH, FrameWidth)", err);

	for (unsigned i = 0; i < textureList.size(); i++) {
		TextureInfo& ti = textureList[i];

		float destX = ti.destX * sw;
		float destW = ti.destW * sw;
		float destY = ti.destY * sh;
		float destH = ti.destH * sh;

		glBindTexture(GL_TEXTURE_2D, ti.textureID);
		if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glBindTexture", err);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ti.sourceW, ti.sourceH, format, GL_UNSIGNED_BYTE, frame.data[0] + ti.dataOffset);
		if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glTexSubImage2D", err);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glColor4f", err);

		float top, bottom;
		if (frame.flipped) {
			top = ti.texBottom;
			bottom = ti.texTop;
		}
		else {
			top = ti.texTop;
			bottom = ti.texBottom;
		}

		glBegin(GL_QUADS);
			glTexCoord2f(ti.texLeft,  top);     glVertex2f(destX, destY);
			glTexCoord2f(ti.texRight, top);     glVertex2f(destX + destW, destY);
			glTexCoord2f(ti.texRight, bottom);  glVertex2f(destX + destW, destY + destH);
			glTexCoord2f(ti.texLeft,  bottom);  glVertex2f(destX, destY + destH);
		glEnd();
		if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"GL_QUADS", err);
	}

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glPixelStorei(GL_UNPACK_ROW_LENGTH, default)", err);

	glDisable(GL_TEXTURE_2D);
	if (GLenum err = glGetError()) throw VideoOutOpenGLException(L"glDisable(GL_TEXTURE_2d)", err);
}

VideoOutGL::~VideoOutGL() {
	if (textureIdList.size() > 0) {
		glDeleteTextures(textureIdList.size(), &textureIdList[0]);
	}
}
