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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include "subtitle_format_ttxt.h"
#include "ass_time.h"
#include "ass_file.h"
#include "options.h"


///////////////////
// Get format name
wxString TTXTSubtitleFormat::GetName() {
	return _T("MPEG-4 Streaming Text");
}


//////////////////////
// Get read wildcards
wxArrayString TTXTSubtitleFormat::GetReadWildcards() {
	wxArrayString formats;
	formats.Add(_T("ttxt"));
	return formats;
}


///////////////////////
// Get write wildcards
wxArrayString TTXTSubtitleFormat::GetWriteWildcards() {
	return GetReadWildcards();
	//return wxArrayString();
}


////////////////////
// Can read a file?
bool TTXTSubtitleFormat::CanReadFile(wxString filename) {
	return (filename.Right(5).Lower() == _T(".ttxt"));
}


/////////////////////
// Can write a file?
bool TTXTSubtitleFormat::CanWriteFile(wxString filename) {
	//return false;
	return (filename.Right(5).Lower() == _T(".ttxt"));
}


///////////////
// Read a file
void TTXTSubtitleFormat::ReadFile(wxString filename,wxString forceEncoding) {
	// Load default
	LoadDefault(false);

	// Load XML document
	wxXmlDocument doc;
	if (!doc.Load(filename)) throw _T("Failed loading TTXT XML file.");

	// Check root node name
	if (doc.GetRoot()->GetName() != _T("TextStream")) throw _T("Invalid TTXT file.");

	// Check version
	wxString verStr = doc.GetRoot()->GetPropVal(_T("version"),_T(""));
	version = -1;
	if (verStr == _T("1.0")) version = 0;
	else if (verStr == _T("1.1")) version = 1;
	else throw wxString(_T("Unknown TTXT version: ") + verStr);

	// Get children
	diag = NULL;
	wxXmlNode *child = doc.GetRoot()->GetChildren();
	int lines = 0;
	while (child) {
		// Line
		if (child->GetName() == _T("TextSample")) {
			if (ProcessLine(child)) lines++;
		}

		// Header
		else if (child->GetName() == _T("TextStreamHeader")) {
			ProcessHeader(child);
		}

		// Proceed to next child
		child = child->GetNext();
	}

	// No lines?
	if (lines == 0) {
		AssDialogue *line = new AssDialogue();
		line->group = _T("[Events]");
		line->Style = _T("Default");
		line->StartMS = 0;
		line->Start.SetMS(0);
		line->End.SetMS(5000);
		Line->push_back(line);
	}
}


///////////////////////////
// Process a dialogue line
bool TTXTSubtitleFormat::ProcessLine(wxXmlNode *node) {
	// Get time
	wxString sampleTime = node->GetPropVal(_T("sampleTime"),_T("00:00:00.000"));
	AssTime time;
	time.ParseASS(sampleTime);

	// Set end time of last line
	if (diag) diag->End = time;
	diag = NULL;

	// Get text
	wxString text;
	if (version == 0) text = node->GetPropVal(_T("text"),_T(""));
	else text = node->GetNodeContent();

	// Create line
	if (!text.IsEmpty()) {
		// Create dialogue
		diag = new AssDialogue();
		diag->Start = time;
		diag->End.SetMS(36000000-10);
		diag->group = _T("[Events]");
		diag->Style = _T("Default");
		diag->Comment = false;
		diag->StartMS = diag->Start.GetMS();

		// Process text for 1.0
		if (version == 0) {
			wxString finalText;
			finalText.Alloc(text.Length());
			bool in = false;
			bool first = true;
			for (size_t i=0;i<text.Length();i++) {
				if (text[i] == _T('\'')) {
					if (!in && !first) finalText += _T("\\N");
					first = false;
					in = !in;
				}
				else if (in) finalText += text[i];
			}
			diag->Text = finalText;
		}

		// Process text for 1.1
		else {
			text.Replace(_T("\r"),_T(""));
			text.Replace(_T("\n"),_T("\\N"));
			diag->Text = text;
		}

		// Insert dialogue
		Line->push_back(diag);
		return true;
	}

	else return false;
}


//////////////////////
// Process the header
void TTXTSubtitleFormat::ProcessHeader(wxXmlNode *node) {
	// TODO
}


////////////////
// Write a file
void TTXTSubtitleFormat::WriteFile(wxString filename,wxString encoding) {
	// Convert to TTXT
	CreateCopy();
	ConvertToTTXT();

	// Create XML structure
	wxXmlDocument doc;
	wxXmlNode *root = new wxXmlNode(NULL,wxXML_ELEMENT_NODE,_T("TextStream"));
	root->AddProperty(_T("version"),_T("1.1"));
	doc.SetRoot(root);

	// Create header
	WriteHeader(root);

	// Create lines
	int i=1;
	using std::list;
	prev = NULL;
	for (list<AssEntry*>::iterator cur=Line->begin();cur!=Line->end();cur++) {
		AssDialogue *current = AssEntry::GetAsDialogue(*cur);
		if (current && !current->Comment) {
			WriteLine(root,current);
			i++;
		}
		else throw _T("Unexpected line type in TTXT file");
	}

	// Save XML
	//prevNode->SetNext(NULL);
	doc.Save(filename);

	// Clear
	ClearCopy();
}


////////////////
// Write header
void TTXTSubtitleFormat::WriteHeader(wxXmlNode *root) {
	// Write stream header
	wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("TextStreamHeader"));
	node->AddProperty(_T("width"),_T("400"));
	node->AddProperty(_T("height"),_T("60"));
	node->AddProperty(_T("layer"),_T("0"));
	node->AddProperty(_T("translation_x"),_T("0"));
	node->AddProperty(_T("translation_y"),_T("0"));
	root->AddChild(node);
	root = node;

	// Write sample description
	node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("TextSampleDescription"));
	node->AddProperty(_T("horizontalJustification"),_T("center"));
	node->AddProperty(_T("verticalJustification"),_T("bottom"));
	node->AddProperty(_T("backColor"),_T("0 0 0 0"));
	node->AddProperty(_T("verticalText"),_T("no"));
	node->AddProperty(_T("fillTextRegion"),_T("no"));
	node->AddProperty(_T("continuousKaraoke"),_T("no"));
	node->AddProperty(_T("scroll"),_T("None"));
	root->AddChild(node);
	root = node;

	// Write font table
	node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("FontTable"));
	wxXmlNode *subNode = new wxXmlNode(wxXML_ELEMENT_NODE,_T("FontTableEntry"));
	subNode->AddProperty(_T("fontName"),_T("Sans"));
	subNode->AddProperty(_T("fontID"),_T("1"));
	node->AddChild(subNode);
	root->AddChild(node);
	
	// Write text box
	node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("TextBox"));
	node->AddProperty(_T("top"),_T("0"));
	node->AddProperty(_T("left"),_T("0"));
	node->AddProperty(_T("bottom"),_T("60"));
	node->AddProperty(_T("right"),_T("400"));
	root->AddChild(node);

	// Write style
	node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("Style"));
	node->AddProperty(_T("styles"),_T("Normal"));
	node->AddProperty(_T("fontID"),_T("1"));
	node->AddProperty(_T("fontSize"),_T("18"));
	node->AddProperty(_T("color"),_T("ff ff ff ff"));
	root->AddChild(node);
}


//////////////
// Write line
void TTXTSubtitleFormat::WriteLine(wxXmlNode *root, AssDialogue *line) {
	// If it doesn't start at the end of previous, add blank
	wxXmlNode *node,*subNode;
	if (prev && prev->End != line->Start) {
		node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("TextSample"));
		node->AddProperty(_T("sampleTime"),_T("0") + prev->End.GetASSFormated(true));
		node->AddProperty(_T("xml:space"),_T("preserve"));
		subNode = new wxXmlNode(wxXML_TEXT_NODE,_T(""),_T(""));
		node->AddChild(subNode);
		root->AddChild(node);
	}

	// Generate and insert node
	node = new wxXmlNode(wxXML_ELEMENT_NODE,_T("TextSample"));
	node->AddProperty(_T("sampleTime"),_T("0") + line->Start.GetASSFormated(true));
	node->AddProperty(_T("xml:space"),_T("preserve"));
	subNode = new wxXmlNode(wxXML_TEXT_NODE,_T(""),line->Text);
	node->AddChild(subNode);
	root->AddChild(node);

	// Set as previous
	prev = line;
}


//////////////////////////////
// Converts whole file to TTXT
void TTXTSubtitleFormat::ConvertToTTXT () {
	// Convert
	SortLines();
	StripComments();
	RecombineOverlaps();
	MergeIdentical();
	ConvertTags(1,_T("\r\n"));

	// Find last line
	AssTime lastTime;
	for (std::list<AssEntry*>::reverse_iterator cur=Line->rbegin();cur!=Line->rend();cur++) {
		AssDialogue *prev = AssEntry::GetAsDialogue(*cur);
		if (prev) {
			lastTime = prev->End;
			break;
		}
	}

	// Insert blank line at the end
	AssDialogue *diag = new AssDialogue();
	diag->Start = lastTime;
	diag->End.SetMS(lastTime.GetMS()+Options.AsInt(_T("Timing Default Duration")));
	diag->group = _T("[Events]");
	diag->Style = _T("Default");
	diag->Comment = false;
	diag->StartMS = diag->Start.GetMS();
	Line->push_back(diag);
}
