// Copyright (c) 2025, Aegisub Project
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

/// @file subtitle_format_cinecanvas.h
/// @see subtitle_format_cinecanvas.cpp
/// @ingroup subtitle_io
/// @brief CineCanvas/Interop DCP XML subtitle format

#pragma once

#include "subtitle_format.h"

class AssDialogue;
class AssStyle;
namespace agi { struct Color; }
class wxXmlNode;

/// @class CineCanvasSubtitleFormat
/// @brief Reads and writes CineCanvas-style XML subtitles for Digital Cinema Packages
class CineCanvasSubtitleFormat final : public SubtitleFormat {
	/// Convert AssFile to CineCanvas-compatible format
	void ConvertToCineCanvas(AssFile &file) const;

	/// Write XML document header and metadata
	/// @param root Root XML node
	/// @param src Source subtitle file
	void WriteHeader(wxXmlNode *root, const AssFile *src) const;

	/// Write a single subtitle entry
	/// @param fontNode Parent font node
	/// @param line Dialogue line to write
	/// @param spotNumber Sequential subtitle number
	/// @param fps Frame rate for timing conversion
	void WriteSubtitle(wxXmlNode *fontNode, const AssDialogue *line,
	                   int spotNumber, const agi::vfr::Framerate &fps) const;

	/// Convert ASS color to CineCanvas RRGGBBAA hex format
	/// @param color ASS color object
	/// @param alpha Alpha value (0-255)
	/// @return 8-character hex string (RRGGBBAA)
	std::string ConvertColorToRGBA(const agi::Color &color, uint8_t alpha = 255) const;

	/// Generate RFC 4122 compliant UUID
	/// @return UUID string in urn:uuid:xxx format
	std::string GenerateUUID() const;

	/// Parse ASS style and set Font node attributes
	/// @param style ASS style to convert
	/// @param fontNode XML Font node to populate
	void ParseFontAttributes(const AssStyle *style, wxXmlNode *fontNode) const;

	/// Parse dialogue line positioning and set Text node attributes
	/// @param line Dialogue line with positioning info
	/// @param textNode XML Text node to populate
	void ParseTextPosition(const AssDialogue *line, wxXmlNode *textNode) const;

public:
	CineCanvasSubtitleFormat();

	std::vector<std::string> GetReadWildcards() const override;
	std::vector<std::string> GetWriteWildcards() const override;

	void ReadFile(AssFile *target, agi::fs::path const& filename,
	              agi::vfr::Framerate const& fps, const char *encoding) const override;
	void WriteFile(const AssFile *src, agi::fs::path const& filename,
	               agi::vfr::Framerate const& fps, const char *encoding="") const override;

	bool CanSave(const AssFile *file) const override;
};
