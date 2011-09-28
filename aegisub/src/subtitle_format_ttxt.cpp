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

/// @file subtitle_format_ttxt.cpp
/// @brief Reading/writing MPEG-4 Timed Text subtitles in TTXT XML format
/// @ingroup subtitle_io
///


///////////
// Headers
#include "config.h"

#include "ass_file.h"
#include "ass_time.h"
#include "compat.h"
#include "main.h"
#include "subtitle_format_ttxt.h"


/// @brief Get format name 
/// @return 
///
wxString TTXTSubtitleFormat::GetName() {
	return "MPEG-4 Streaming Text";
}



/// @brief Get read wildcards 
/// @return 
///
wxArrayString TTXTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add("ttxt");
	return formats;
}



/// @brief Get write wildcards 
/// @return 
///
wxArrayString TTXTSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
	//return wxArrayString();
}



/// @brief Can read a file? 
/// @param filename 
/// @return 
///
bool TTXTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(5).Lower() == ".ttxt");
}



/// @brief Can write a file? 
/// @param filename 
/// @return 
///
bool TTXTSubtitleFormat::CanWriteFile(wxString filename) {
	//return false;
	return (filename.Right(5).Lower() == ".ttxt");
}



/// @brief Read a file 
/// @param filename      
/// @param forceEncoding 
///
void TTXTSubtitleFormat::ReadFile(wxString filename,wxString forceEncoding) {
	// Load default
	LoadDefault(false);

	// Load XML document
	wxXmlDocument doc;
	if (!doc.Load(filename)) throw "Failed loading TTXT XML file.";

	// Check root node name
	if (doc.GetRoot()->GetName() != "TextStream") throw "Invalid TTXT file.";

	// Check version
	wxString verStr = doc.GetRoot()->GetAttribute("version","");
	version = -1;
	if (verStr == "1.0") version = 0;
	else if (verStr == "1.1") version = 1;
	else throw wxString("Unknown TTXT version: " + verStr);

	// Get children
	diag = NULL;
	wxXmlNode *child = doc.GetRoot()->GetChildren();
	int lines = 0;
	while (child) {
		// Line
		if (child->GetName() == "TextSample") {
			if (ProcessLine(child)) lines++;
		}

		// Header
		else if (child->GetName() == "TextStreamHeader") {
			ProcessHeader(child);
		}

		// Proceed to next child
		child = child->GetNext();
	}

	// No lines?
	if (lines == 0) {
		AssDialogue *line = new AssDialogue();
		line->group = "[Events]";
		line->Style = "Default";
		line->Start.SetMS(0);
		line->End.SetMS(5000);
		Line->push_back(line);
	}
}



/// @brief Process a dialogue line 
/// @param node 
/// @return 
///
bool TTXTSubtitleFormat::ProcessLine(wxXmlNode *node) {
	// Get time
	wxString sampleTime = node->GetAttribute("sampleTime","00:00:00.000");
	AssTime time;
	time.ParseASS(sampleTime);

	// Set end time of last line
	if (diag) diag->End = time;
	diag = NULL;

	// Get text
	wxString text;
	if (version == 0) text = node->GetAttribute("text","");
	else text = node->GetNodeContent();

	// Create line
	if (!text.IsEmpty()) {
		// Create dialogue
		diag = new AssDialogue();
		diag->Start.SetMS(time.GetMS());
		diag->End.SetMS(36000000-10);
		diag->group = "[Events]";
		diag->Style = "Default";
		diag->Comment = false;

		// Process text for 1.0
		if (version == 0) {
			wxString finalText;
			finalText.Alloc(text.Length());
			bool in = false;
			bool first = true;
			for (size_t i=0;i<text.Length();i++) {
				if (text[i] == '\'') {
					if (!in && !first) finalText += "\\N";
					first = false;
					in = !in;
				}
				else if (in) finalText += text[i];
			}
			diag->Text = finalText;
		}

		// Process text for 1.1
		else {
			text.Replace("\r","");
			text.Replace("\n","\\N");
			diag->Text = text;
		}

		// Insert dialogue
		Line->push_back(diag);
		return true;
	}

	else return false;
}



/// @brief Process the header 
/// @param node 
///
void TTXTSubtitleFormat::ProcessHeader(wxXmlNode *node) {
	// TODO
}



/// @brief Write a file 
/// @param filename 
/// @param encoding 
///
void TTXTSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
	// Convert to TTXT
	CreateCopy();
	ConvertToTTXT();

	// Create XML structure
	wxXmlDocument doc;
	wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,"TextStream");
	root->AddAttribute("version","1.1");
	doc.SetRoot(root);

	// Create header
	WriteHeader(root);

	// Create lines
	int i=1;
	using std::list;
	prev = NULL;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = dynamic_cast<AssDialogue*>(*cur);
		if (current && !current->Comment) {
			WriteLine(root,current);
			i++;
		}
		else throw "Unexpected line type in TTXT file";
	}

	// Save XML
	//prevNode->SetNext(NULL);
	doc.Save(filename);

	// Clear
	ClearCopy();
}



/// @brief Write header 
/// @param root 
///
void TTXTSubtitleFormat::WriteHeader(wxXmlNode *root) {
	// Write stream header
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE,"TextStreamHeader");
	node->AddAttribute("width","400");
	node->AddAttribute("height","60");
	node->AddAttribute("layer","0");
	node->AddAttribute("translation_x","0");
	node->AddAttribute("translation_y","0");
	root->AddChild(node);
	root = node;

	// Write sample description
	node = new wxXmlNode(wxXML_ELEMENT_NODE,"TextSampleDescription");
	node->AddAttribute("horizontalJustification","center");
	node->AddAttribute("verticalJustification","bottom");
	node->AddAttribute("backColor","0 0 0 0");
	node->AddAttribute("verticalText","no");
	node->AddAttribute("fillTextRegion","no");
	node->AddAttribute("continuousKaraoke","no");
	node->AddAttribute("scroll","None");
	root->AddChild(node);
	root = node;

	// Write font table
	node = new wxXmlNode(wxXML_ELEMENT_NODE,"FontTable");
	wxXmlNode *subNode = new wxXmlNode(wxXML_ELEMENT_NODE,"FontTableEntry");
	subNode->AddAttribute("fontName","Sans");
	subNode->AddAttribute("fontID","1");
	node->AddChild(subNode);
	root->AddChild(node);
	
	// Write text box
	node = new wxXmlNode(wxXML_ELEMENT_NODE,"TextBox");
	node->AddAttribute("top","0");
	node->AddAttribute("left","0");
	node->AddAttribute("bottom","60");
	node->AddAttribute("right","400");
	root->AddChild(node);

	// Write style
	node = new wxXmlNode(wxXML_ELEMENT_NODE,"Style");
	node->AddAttribute("styles","Normal");
	node->AddAttribute("fontID","1");
	node->AddAttribute("fontSize","18");
	node->AddAttribute("color","ff ff ff ff");
	root->AddChild(node);
}



/// @brief Write line 
/// @param root 
/// @param line 
///
void TTXTSubtitleFormat::WriteLine(wxXmlNode *root, AssDialogue *line) {
	// If it doesn't start at the end of previous, add blank
	wxXmlNode *node,*subNode;
	if (prev && prev->End != line->Start) {
		node = new wxXmlNode(wxXML_ELEMENT_NODE,"TextSample");
		node->AddAttribute("sampleTime","0" + prev->End.GetASSFormated(true));
		node->AddAttribute("xml:space","preserve");
		subNode = new wxXmlNode(wxXML_TEXT_NODE,"","");
		node->AddChild(subNode);
		root->AddChild(node);
	}

	// Generate and insert node
	node = new wxXmlNode(wxXML_ELEMENT_NODE,"TextSample");
	node->AddAttribute("sampleTime","0" + line->Start.GetASSFormated(true));
	node->AddAttribute("xml:space","preserve");
	subNode = new wxXmlNode(wxXML_TEXT_NODE,"",line->Text);
	node->AddChild(subNode);
	root->AddChild(node);

	// Set as previous
	prev = line;
}



/// @brief Converts whole file to TTXT 
///
void TTXTSubtitleFormat::ConvertToTTXT () {
	// Convert
	SortLines();
	StripComments();
	RecombineOverlaps();
	MergeIdentical();
	ConvertTags(1,"\r\n");

	// Find last line
	AssTime lastTime;
	for (std::list<AssEntry*>::reverse_iterator cur=Line->rbegin();cur!=Line->rend();cur++) {
		AssDialogue *prev = dynamic_cast<AssDialogue*>(*cur);
		if (prev) {
			lastTime = prev->End;
			break;
		}
	}

	// Insert blank line at the end
	AssDialogue *diag = new AssDialogue();
	diag->Start.SetMS(lastTime.GetMS());
	diag->End.SetMS(lastTime.GetMS()+OPT_GET("Timing/Default Duration")->GetInt());
	diag->group = "[Events]";
	diag->Style = "Default";
	diag->Comment = false;
	Line->push_back(diag);
}


