// Copyright (c) 2005, Rodrigo Braz Monteiro
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

/// @file ass_dialogue.cpp
/// @brief Class for dialogue lines in subtitles
/// @ingroup subs_storage

#include "config.h"

#ifndef AGI_PRE
#include <fstream>
#include <list>
#include <vector>

#include <wx/regex.h>
#include <wx/tokenzr.h>
#endif

#include "ass_dialogue.h"
#include "ass_override.h"
#include "utils.h"

AssDialogue::AssDialogue()
: AssEntry(wxString(), "[Events]")
, Comment(false)
, Layer(0)
, Start(0)
, End(5000)
, Style("Default")
{
	for (int i=0;i<4;i++) Margin[i] = 0;
}

AssDialogue::AssDialogue(AssDialogue const& that)
: AssEntry(wxString(), that.group)
, Comment(that.Comment)
, Layer(that.Layer)
, Start(that.Start)
, End(that.End)
, Style(that.Style)
, Actor(that.Actor)
, Effect(that.Effect)
, Text(that.Text)
{
	for (int i=0;i<4;i++) Margin[i] = that.Margin[i];
}

/// @brief DOCME
/// @param _data   
/// @param version 
AssDialogue::AssDialogue(wxString _data,int version)
: AssEntry(wxString(), "[Events]")
, Comment(false)
, Layer(0)
, Start(0)
, End(5000)
, Style("Default")
{
	bool valid = false;
	// Try parsing in different ways
	int count = 0;
	while (!valid && count < 3) {
		valid = Parse(_data,version);
		count++;
		version++;
		if (version > 2) version = 0;
	}

	// Not valid
	if (!valid)
		throw "Failed parsing line.";
}

AssDialogue::~AssDialogue () {
	delete_clear(Blocks);
}

void AssDialogue::ClearBlocks() {
	delete_clear(Blocks);
}

bool AssDialogue::Parse(wxString rawData, int version) {
	size_t pos = 0;
	wxString temp;

	// Get type
	if (rawData.StartsWith("Dialogue:")) {
		Comment = false;
		pos = 10;
	}
	else if (rawData.StartsWith("Comment:")) {
		Comment = true;
		pos = 9;
	}
	else return false;

	wxStringTokenizer tkn(rawData.Mid(pos),",",wxTOKEN_RET_EMPTY_ALL);
	if (!tkn.HasMoreTokens()) return false;

	// Get first token and see if it has "Marked=" in it
	temp = tkn.GetNextToken().Trim(false).Trim(true);
	if (temp.Lower().StartsWith("marked=")) version = 0;
	else if (version == 0) version = 1;

	// Get layer number
	if (version == 0) Layer = 0;
	else {
		long templ;
		temp.ToLong(&templ);
		Layer = templ;
	}

	// Get start time
	if (!tkn.HasMoreTokens()) return false;
	Start.ParseASS(tkn.GetNextToken());

	// Get end time
	if (!tkn.HasMoreTokens()) return false;
	End.ParseASS(tkn.GetNextToken());

	// Get style
	if (!tkn.HasMoreTokens()) return false;
	Style = tkn.GetNextToken();
	Style.Trim(true);
	Style.Trim(false);

	// Get actor
	if (!tkn.HasMoreTokens()) return false;
	Actor = tkn.GetNextToken();
	Actor.Trim(true);
	Actor.Trim(false);

	// Get left margin
	if (!tkn.HasMoreTokens()) return false;
	SetMarginString(tkn.GetNextToken().Trim(false).Trim(true),0);

	// Get right margin
	if (!tkn.HasMoreTokens()) return false;
	SetMarginString(tkn.GetNextToken().Trim(false).Trim(true),1);

	// Get top margin
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken().Trim(false).Trim(true);
	SetMarginString(temp,2);
	if (version == 1) SetMarginString(temp,3);

	// Get bottom margin
	bool rollBack = false;
	if (version == 2) {
		if (!tkn.HasMoreTokens()) return false;
		wxString oldTemp = temp;
		temp = tkn.GetNextToken().Trim(false).Trim(true);
		if (!temp.IsNumber()) {
			version = 1;
			rollBack = true;
		}
	}

	// Get effect
	if (!rollBack) {
		if (!tkn.HasMoreTokens()) return false;
		temp = tkn.GetNextToken();
	}
	Effect = temp;
	Effect.Trim(true);
	Effect.Trim(false);

	// Get text
	Text = rawData.Mid(pos+tkn.GetPosition());

	return true;
}

wxString AssDialogue::GetData(bool ssa) const {
	wxString s = Style;
	wxString a = Actor;
	wxString e = Effect;
	s.Replace(",",";");
	a.Replace(",",";");
	e.Replace(",",";");

	wxString str = wxString::Format(
		"%s: %s,%s,%s,%s,%s,%d,%d,%d,%s,%s",
		Comment ? "Comment" : "Dialogue",
		ssa ? "Marked=0" : wxString::Format("%01d", Layer),
		Start.GetASSFormated(),
		End.GetASSFormated(),
		s, a,
		Margin[0], Margin[1], Margin[2],
		e,
		Text);

	// Make sure that final has no line breaks
	str.Replace("\n", "");
	str.Replace("\r", "");

	return str;
}

const wxString AssDialogue::GetEntryData() const {
	return GetData(false);
}

wxString AssDialogue::GetSSAText () const {
	return GetData(true);
}

void AssDialogue::ParseASSTags() {
	ClearBlocks();

	// Empty line, make an empty block
	if (Text.empty()) {
		Blocks.push_back(new AssDialogueBlockPlain);
		return;
	}

	int drawingLevel = 0;

	for (size_t len = Text.size(), cur = 0; cur < len; ) {
		// Overrides block
		if (Text[cur] == '{') {
			++cur;
			// Get contents of block
			wxString work;
			size_t end = Text.find("}", cur);
			if (end == wxString::npos) {
				work = Text.substr(cur);
				cur = len;
			}
			else {
				work = Text.substr(cur, end - cur);
				cur = end + 1;
			}
			
			if (work.size() && work.Find("\\") == wxNOT_FOUND) {
				//We've found an override block with no backslashes
				//We're going to assume it's a comment and not consider it an override block
				//Currently we'll treat this as a plain text block, but feel free to create a new class
				Blocks.push_back(new AssDialogueBlockPlain("{" + work + "}"));
			}
			else {
				// Create block
				AssDialogueBlockOverride *block = new AssDialogueBlockOverride(work);
				block->ParseTags();
				Blocks.push_back(block);

				// Look for \p in block
				std::vector<AssOverrideTag*>::iterator curTag;
				for (curTag = block->Tags.begin();curTag != block->Tags.end();curTag++) {
					if ((*curTag)->Name == "\\p") {
						drawingLevel = (*curTag)->Params[0]->Get<int>(0);
					}
				}
			}
		}
		// Plain-text/drawing block
		else {
			wxString work;
			size_t end = Text.find("{",cur);
			if (end == wxString::npos) {
				work = Text.substr(cur);
				cur = len;
			}
			else {
				work = Text.substr(cur, end - cur);
				cur = end;
			}

			// Plain-text
			if (drawingLevel == 0) {
				Blocks.push_back(new AssDialogueBlockPlain(work));
			}
			// Drawing
			else {
				AssDialogueBlockDrawing *block = new AssDialogueBlockDrawing(work);
				block->Scale = drawingLevel;
				Blocks.push_back(block);
			}
		}
	}
}

void AssDialogue::StripTags () {
	Text = GetStrippedText();
}

void AssDialogue::StripTag (wxString tagName) {
	using std::list;
	using std::vector;
	ParseASSTags();
	wxString final;

	// Look for blocks
	for (vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->GetType() == BLOCK_OVERRIDE) {
			AssDialogueBlockOverride *over = dynamic_cast<AssDialogueBlockOverride*>(*cur);
			wxString temp;
			for (size_t i=0;i<over->Tags.size();i++) {
				if (over->Tags[i]->Name != tagName) temp += *over->Tags[i];
			}

			// Insert
			if (!temp.IsEmpty()) final += "{" + temp + "}";
		}
		else final += (*cur)->GetText();
	}

	ClearBlocks();
	Text = final;
}

void AssDialogue::UpdateText () {
	if (Blocks.empty()) return;
	Text.clear();
	for (std::vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->GetType() == BLOCK_OVERRIDE) {
			Text += "{";
			Text += (*cur)->GetText();
			Text += "}";
		}
		else Text += (*cur)->GetText();
	}
}

void AssDialogue::SetMarginString(const wxString origvalue,int which) {
	if (which < 0 || which >= 4) throw Aegisub::InvalidMarginIdError();

	// Make it numeric
	wxString strvalue = origvalue;
	if (!strvalue.IsNumber()) {
		strvalue.clear();
		for (size_t i=0;i<origvalue.Length();i++) {
			if (origvalue.Mid(i,1).IsNumber()) {
				strvalue += origvalue.Mid(i,1);
			}
		}
	}

	// Get value
	long value = 0;
	strvalue.ToLong(&value);
	Margin[which] = mid<int>(0, value, 9999);
}

wxString AssDialogue::GetMarginString(int which,bool pad) const {
	if (which < 0 || which >= 4) throw Aegisub::InvalidMarginIdError();
	int value = Margin[which];
	if (pad) return wxString::Format("%04i",value);
	else return wxString::Format("%i",value);
}

void AssDialogue::ProcessParameters(AssDialogueBlockOverride::ProcessParametersCallback callback,void *userData) {
	// Apply for all override blocks
	AssDialogueBlockOverride *curBlock;
	//ParseASSTags();
	for (std::vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->GetType() == BLOCK_OVERRIDE) {
			curBlock = static_cast<AssDialogueBlockOverride*> (*cur);
			curBlock->ProcessParameters(callback,userData);
		}
	}
	//ClearBlocks();
}

bool AssDialogue::CollidesWith(AssDialogue *target) {
	if (!target) return false;
	return ((Start < target->Start) ? (target->Start < End) : (Start < target->End));
}

wxString AssDialogue::GetStrippedText() const {
	static wxRegEx reg("\\{[^\\{]*\\}",wxRE_ADVANCED);
	wxString txt(Text);
	reg.Replace(&txt,"");
	return txt;
}

AssEntry *AssDialogue::Clone() const {
	return new AssDialogue(*this);
}

void AssDialogueBlockDrawing::TransformCoords(int mx,int my,double x,double y) {
	// HACK: Implement a proper parser ffs!!
	// Could use Spline but it'd be slower and this seems to work fine
	wxStringTokenizer tkn(GetText()," ",wxTOKEN_DEFAULT);
	wxString cur;
	wxString final;
	bool isX = true;
	long temp;

	// Process tokens
	while (tkn.HasMoreTokens()) {
		cur = tkn.GetNextToken().Lower();

		// Number, process it
		if (cur.IsNumber()) {
			// Transform it
			cur.ToLong(&temp);
			if (isX) temp = (long int)((temp+mx)*x + 0.5);
			else temp = (long int)((temp+my)*y + 0.5);

			// Write back to list
			final += wxString::Format("%i ",temp);

			// Toggle X/Y
			isX = !isX;
		}

		// Text
		else {
			if (cur == "m" || cur == "n" || cur == "l" || cur == "b" || cur == "s" || cur == "p" || cur == "c") isX = true;
			final += cur + " ";
		}
	}

	// Write back final
	final = final.Left(final.Length()-1);
	text = final;
}
