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

/// @file subtitle_format_cinecanvas.cpp
/// @brief Reading/writing CineCanvas-style XML subtitles for Digital Cinema Packages
/// @ingroup subtitle_io

#include "subtitle_format_cinecanvas.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "options.h"

#include <libaegisub/ass/time.h>
#include <libaegisub/color.h>

#include <wx/xml/xml.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

DEFINE_EXCEPTION(CineCanvasParseError, SubtitleFormatParseError);

CineCanvasSubtitleFormat::CineCanvasSubtitleFormat()
: SubtitleFormat("CineCanvas XML")
{
}

std::vector<std::string> CineCanvasSubtitleFormat::GetReadWildcards() const {
	return {"xml"};
}

std::vector<std::string> CineCanvasSubtitleFormat::GetWriteWildcards() const {
	return {"xml"};
}

bool CineCanvasSubtitleFormat::CanSave(const AssFile *file) const {
	// CineCanvas format supports basic subtitle functionality
	// More validation will be added in future phases
	return true;
}

void CineCanvasSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename,
                                        agi::vfr::Framerate const& fps, const char *encoding) const {
	// Import functionality will be implemented in Phase 4.3
	// For now, throw an exception to indicate it's not yet supported
	throw CineCanvasParseError("CineCanvas import not yet implemented");
}

void CineCanvasSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename,
                                         agi::vfr::Framerate const& fps, const char *encoding) const {
	// Convert to CineCanvas-compatible format
	AssFile copy(*src);
	ConvertToCineCanvas(copy);

	// Create XML structure
	wxXmlDocument doc;
	wxXmlNode *root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "DCSubtitle");
	root->AddAttribute("Version", "1.0");
	doc.SetRoot(root);

	// Write header (metadata and font definitions)
	WriteHeader(root, src);

	// Create Font container node
	// For now, use a default font configuration
	wxXmlNode *fontNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Font");
	fontNode->AddAttribute("Id", "Font1");
	fontNode->AddAttribute("Size", "42");
	fontNode->AddAttribute("Weight", "normal");
	fontNode->AddAttribute("Color", "FFFFFFFF");
	fontNode->AddAttribute("Effect", "border");
	fontNode->AddAttribute("EffectColor", "FF000000");
	root->AddChild(fontNode);

	// Write subtitle entries
	int spotNumber = 1;
	for (auto const& line : copy.Events) {
		if (!line.Comment) {
			WriteSubtitle(fontNode, &line, spotNumber++, fps);
		}
	}

	// Save XML to file
	doc.Save(filename.wstring());
}

void CineCanvasSubtitleFormat::ConvertToCineCanvas(AssFile &file) const {
	// Prepare file for CineCanvas export
	file.Sort();
	StripComments(file);
	RecombineOverlaps(file);
	MergeIdentical(file);
	StripTags(file);
	ConvertNewlines(file, "\\N", false);
}

void CineCanvasSubtitleFormat::WriteHeader(wxXmlNode *root, const AssFile *src) const {
	// SubtitleID with UUID
	wxXmlNode *subIdNode = new wxXmlNode(wxXML_ELEMENT_NODE, "SubtitleID");
	root->AddChild(subIdNode);
	subIdNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", to_wx(GenerateUUID())));

	// MovieTitle
	wxXmlNode *movieTitleNode = new wxXmlNode(wxXML_ELEMENT_NODE, "MovieTitle");
	root->AddChild(movieTitleNode);
	movieTitleNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", "Untitled"));

	// ReelNumber
	wxXmlNode *reelNode = new wxXmlNode(wxXML_ELEMENT_NODE, "ReelNumber");
	root->AddChild(reelNode);
	reelNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", "1"));

	// Language
	wxXmlNode *langNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Language");
	root->AddChild(langNode);
	langNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", "en"));

	// LoadFont (placeholder - will be enhanced in later phases)
	wxXmlNode *loadFontNode = new wxXmlNode(wxXML_ELEMENT_NODE, "LoadFont");
	loadFontNode->AddAttribute("Id", "Font1");
	loadFontNode->AddAttribute("URI", "");
	root->AddChild(loadFontNode);
}

void CineCanvasSubtitleFormat::WriteSubtitle(wxXmlNode *fontNode, const AssDialogue *line,
                                             int spotNumber, const agi::vfr::Framerate &fps) const {
	// Create Subtitle element
	wxXmlNode *subtitleNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Subtitle");
	subtitleNode->AddAttribute("SpotNumber", wxString::Format("%d", spotNumber));

	// Convert timing to HH:MM:SS:mmm format
	std::string timeIn = line->Start.GetAssFormatted(true);
	std::string timeOut = line->End.GetAssFormatted(true);

	// Convert from 0:00:00.00 to 00:00:00:000 format
	if (timeIn[0] != '0' || timeIn[1] != '0') {
		timeIn = "00" + timeIn.substr(1);
	}
	if (timeOut[0] != '0' || timeOut[1] != '0') {
		timeOut = "00" + timeOut.substr(1);
	}

	// Replace . with : for milliseconds and ensure 3 digits
	size_t dotPosIn = timeIn.find_last_of('.');
	if (dotPosIn != std::string::npos) {
		timeIn[dotPosIn] = ':';
		timeIn += "0"; // Convert centiseconds to milliseconds
	}

	size_t dotPosOut = timeOut.find_last_of('.');
	if (dotPosOut != std::string::npos) {
		timeOut[dotPosOut] = ':';
		timeOut += "0"; // Convert centiseconds to milliseconds
	}

	subtitleNode->AddAttribute("TimeIn", to_wx(timeIn));
	subtitleNode->AddAttribute("TimeOut", to_wx(timeOut));
	subtitleNode->AddAttribute("FadeUpTime", "20");
	subtitleNode->AddAttribute("FadeDownTime", "20");

	fontNode->AddChild(subtitleNode);

	// Create Text element
	wxXmlNode *textNode = new wxXmlNode(wxXML_ELEMENT_NODE, "Text");
	textNode->AddAttribute("VAlign", "bottom");
	textNode->AddAttribute("VPosition", "10.0");
	subtitleNode->AddChild(textNode);

	// Add text content
	textNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", to_wx(line->Text)));
}

std::string CineCanvasSubtitleFormat::ConvertColorToRGBA(const agi::Color &color, uint8_t alpha) const {
	// Convert ASS BGR color to CineCanvas RRGGBBAA format
	return (boost::format("%02X%02X%02X%02X")
		% static_cast<int>(color.r)
		% static_cast<int>(color.g)
		% static_cast<int>(color.b)
		% static_cast<int>(alpha)).str();
}

std::string CineCanvasSubtitleFormat::GenerateUUID() const {
	// Generate a simple UUID for SubtitleID
	// Full UUID implementation will use Boost.UUID in later phases
	// For now, use a placeholder format
	return "urn:uuid:00000000-0000-0000-0000-000000000000";
}

void CineCanvasSubtitleFormat::ParseFontAttributes(const AssStyle *style, wxXmlNode *fontNode) const {
	// Font attribute parsing will be implemented in later phases
	// This is a placeholder for the basic structure
}

void CineCanvasSubtitleFormat::ParseTextPosition(const AssDialogue *line, wxXmlNode *textNode) const {
	// Position parsing will be implemented in later phases
	// This is a placeholder for the basic structure
}
