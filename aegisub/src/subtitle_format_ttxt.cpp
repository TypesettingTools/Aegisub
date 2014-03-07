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

/// @file subtitle_format_ttxt.cpp
/// @brief Reading/writing MPEG-4 Timed Text subtitles in TTXT XML format
/// @ingroup subtitle_io
///

#include "config.h"

#include "subtitle_format_ttxt.h"

#include <wx/xml/xml.h>

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_time.h"
#include "compat.h"
#include "options.h"

#include <boost/range/adaptor/reversed.hpp>

DEFINE_SIMPLE_EXCEPTION(TTXTParseError, SubtitleFormatParseError, "subtitle_io/parse/ttxt")

TTXTSubtitleFormat::TTXTSubtitleFormat()
: SubtitleFormat("MPEG-4 Streaming Text")
{
}

std::vector<std::string> TTXTSubtitleFormat::GetReadWildcards() const {
	std::vector<std::string> formats;
	formats.push_back("ttxt");
	return formats;
}

std::vector<std::string> TTXTSubtitleFormat::GetWriteWildcards() const {
	return GetReadWildcards();
}

void TTXTSubtitleFormat::ReadFile(AssFile *target, agi::fs::path const& filename, std::string const& encoding) const {
	target->LoadDefault(false);

	// Load XML document
	wxXmlDocument doc;
	if (!doc.Load(filename.wstring())) throw TTXTParseError("Failed loading TTXT XML file.", nullptr);

	// Check root node name
	if (doc.GetRoot()->GetName() != "TextStream") throw TTXTParseError("Invalid TTXT file.", nullptr);

	// Check version
	wxString verStr = doc.GetRoot()->GetAttribute("version", "");
	int version = -1;
	if (verStr == "1.0")
		version = 0;
	else if (verStr == "1.1")
		version = 1;
	else
		throw TTXTParseError("Unknown TTXT version: " + from_wx(verStr), nullptr);

	// Get children
	AssDialogue *diag = nullptr;
	int lines = 0;
	for (wxXmlNode *child = doc.GetRoot()->GetChildren(); child; child = child->GetNext()) {
		// Line
		if (child->GetName() == "TextSample") {
			if ((diag = ProcessLine(child, diag, version))) {
				lines++;
				target->Events.push_back(*diag);
			}
		}
		// Header
		else if (child->GetName() == "TextStreamHeader") {
			ProcessHeader(child);
		}
	}

	// No lines?
	if (lines == 0)
		target->Events.push_back(*new AssDialogue);
}

AssDialogue *TTXTSubtitleFormat::ProcessLine(wxXmlNode *node, AssDialogue *prev, int version) const {
	// Get time
	wxString sampleTime = node->GetAttribute("sampleTime", "00:00:00.000");
	AssTime time(from_wx(sampleTime));

	// Set end time of last line
	if (prev)
		prev->End = time;

	// Get text
	wxString text;
	if (version == 0)
		text = node->GetAttribute("text", "");
	else
		text = node->GetNodeContent();

	// Create line
	if (text.empty()) return nullptr;

	// Create dialogue
	auto diag = new AssDialogue;
	diag->Start = time;
	diag->End = 36000000-10;

	// Process text for 1.0
	if (version == 0) {
		wxString finalText;
		finalText.reserve(text.size());
		bool in = false;
		bool first = true;
		for (auto chr : text) {
			if (chr == '\'') {
				if (!in && !first) finalText += "\\N";
				first = false;
				in = !in;
			}
			else if (in) finalText += chr;
		}
		diag->Text = from_wx(finalText);
	}

	// Process text for 1.1
	else {
		text.Replace("\r", "");
		text.Replace("\n", "\\N");
		diag->Text = from_wx(text);
	}

	return diag;
}

void TTXTSubtitleFormat::ProcessHeader(wxXmlNode *node) const {
	// TODO
}

void TTXTSubtitleFormat::WriteFile(const AssFile *src, agi::fs::path const& filename, std::string const& encoding) const {
	// Convert to TTXT
	AssFile copy(*src);
	ConvertToTTXT(copy);

	// Create XML structure
	wxXmlDocument doc;
	wxXmlNode *root = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "TextStream");
	root->AddAttribute("version", "1.1");
	doc.SetRoot(root);

	// Create header
	WriteHeader(root);

	// Create lines
	const AssDialogue *prev = nullptr;
	for (auto const& current : copy.Events) {
		WriteLine(root, prev, &current);
		prev = &current;
	}

	// Save XML
	doc.Save(filename.wstring());
}

void TTXTSubtitleFormat::WriteHeader(wxXmlNode *root) const {
	// Write stream header
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "TextStreamHeader");
	node->AddAttribute("width", "400");
	node->AddAttribute("height", "60");
	node->AddAttribute("layer", "0");
	node->AddAttribute("translation_x", "0");
	node->AddAttribute("translation_y", "0");
	root->AddChild(node);
	root = node;

	// Write sample description
	node = new wxXmlNode(wxXML_ELEMENT_NODE, "TextSampleDescription");
	node->AddAttribute("horizontalJustification", "center");
	node->AddAttribute("verticalJustification", "bottom");
	node->AddAttribute("backColor", "0 0 0 0");
	node->AddAttribute("verticalText", "no");
	node->AddAttribute("fillTextRegion", "no");
	node->AddAttribute("continuousKaraoke", "no");
	node->AddAttribute("scroll", "None");
	root->AddChild(node);
	root = node;

	// Write font table

	node = new wxXmlNode(wxXML_ELEMENT_NODE, "FontTable");
	root->AddChild(node);

	wxXmlNode *subNode = new wxXmlNode(wxXML_ELEMENT_NODE, "FontTableEntry");
	subNode->AddAttribute("fontName", "Sans");
	subNode->AddAttribute("fontID", "1");
	node->AddChild(subNode);

	// Write text box
	node = new wxXmlNode(wxXML_ELEMENT_NODE, "TextBox");
	node->AddAttribute("top", "0");
	node->AddAttribute("left", "0");
	node->AddAttribute("bottom", "60");
	node->AddAttribute("right", "400");
	root->AddChild(node);

	// Write style
	node = new wxXmlNode(wxXML_ELEMENT_NODE, "Style");
	node->AddAttribute("styles", "Normal");
	node->AddAttribute("fontID", "1");
	node->AddAttribute("fontSize", "18");
	node->AddAttribute("color", "ff ff ff ff");
	root->AddChild(node);
}

void TTXTSubtitleFormat::WriteLine(wxXmlNode *root, const AssDialogue *prev, const AssDialogue *line) const {
	// If it doesn't start at the end of previous, add blank
	if (prev && prev->End != line->Start) {
		wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "TextSample");
		node->AddAttribute("sampleTime", to_wx("0" + prev->End.GetAssFormated(true)));
		node->AddAttribute("xml:space", "preserve");
		root->AddChild(node);
		node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", ""));
	}

	// Generate and insert node
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "TextSample");
	node->AddAttribute("sampleTime", to_wx("0" + line->Start.GetAssFormated(true)));
	node->AddAttribute("xml:space", "preserve");
	root->AddChild(node);
	node->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", to_wx(line->Text)));
}

void TTXTSubtitleFormat::ConvertToTTXT(AssFile &file) const {
	file.Sort();
	StripComments(file);
	RecombineOverlaps(file);
	MergeIdentical(file);
	StripTags(file);
	ConvertNewlines(file, "\r\n");

	// Find last line
	AssTime lastTime;
	for (auto const& line : file.Events | boost::adaptors::reversed) {
		lastTime = line.End;
		break;
	}

	// Insert blank line at the end
	auto diag = new AssDialogue;
	diag->Start = lastTime;
	diag->End = lastTime+OPT_GET("Timing/Default Duration")->GetInt();
	file.Events.push_back(*diag);
}
