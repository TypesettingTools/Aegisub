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

/// @file subtitle_format_ttxt.h
/// @see subtitle_format_ttxt.cpp
/// @ingroup subtitle_io
///

#include "subtitle_format.h"

class AssDialogue;
class wxXmlNode;

class TTXTSubtitleFormat final : public SubtitleFormat {
	AssDialogue *ProcessLine(wxXmlNode *node, AssDialogue *prev, int version) const;
	void ProcessHeader(wxXmlNode *node) const;

	void WriteHeader(wxXmlNode *root) const;
	void WriteLine(wxXmlNode *root, const AssDialogue *prev, const AssDialogue *line) const;

	void ConvertToTTXT(AssFile &file) const;

public:
	TTXTSubtitleFormat();
	std::vector<std::string> GetReadWildcards() const override;
	std::vector<std::string> GetWriteWildcards() const override;

	void ReadFile(AssFile *target, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& forceEncoding) const override;
	void WriteFile(const AssFile *src, agi::fs::path const& filename, agi::vfr::Framerate const& fps, std::string const& encoding) const override;
};
