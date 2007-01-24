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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


////////////
// Includes
#include <fstream>
#include <wx/tokenzr.h>
#if USE_FEXTRACKER == 1
#include "../FexTrackerSource/FexTracker.h"
#include "../FexTrackerSource/FexMovement.h"
#endif
#include "setup.h"
#include "ass_dialogue.h"
#include "ass_override.h"
#include "vfr.h"
#include "utils.h"


////////////////////// AssDialogue //////////////////////
// Constructs AssDialogue
AssDialogue::AssDialogue() {
#if USE_FEXTRACKER == 1
	Tracker = 0;
	Movement = 0;
#endif

	group = _T("[Events]");

	Valid = true;
	Start.SetMS(0);
	End.SetMS(5000);
	StartMS = 0;
	Layer = 0;
	for (int i=0;i<4;i++) Margin[i] = 0;
	Text = _T("");
	Style = _T("Default");
	Actor = _T("");
	Effect = _T("");
	Comment = false;

	UpdateData();
}


AssDialogue::AssDialogue(wxString _data,int version) {
#if USE_FEXTRACKER == 1
	Tracker = 0;
	Movement = 0;
#endif

	// Set group
	group = _T("[Events]");

	// Try parsing in different ways
	int count = 0;
	Valid = false;
	while (!Valid && count < 3) {
		Valid = Parse(_data,version);
		count++;
		version++;
		if (version > 2) version = 0;
	}

	// Not valid
	if (!Valid) {
		throw _T("Failed parsing line.");
	}

	// update
	UpdateData();
}


//////////////
// Destructor
AssDialogue::~AssDialogue () {
	Clear();
}


/////////
// Clear
void AssDialogue::Clear () {
	ClearBlocks();
#if USE_FEXTRACKER == 1
	if( Tracker )
	{
		delete Tracker;
		Tracker = 0;
	}
	if( Movement )
	{
		DeleteMovement( Movement );
		Movement = 0;
	}
#endif
}


////////////////
// Clear blocks
void AssDialogue::ClearBlocks() {
	using std::vector;
	for (vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		delete *cur;
	}
	Blocks.clear();
}


//////////////////
// Parse ASS Data
bool AssDialogue::Parse(wxString rawData, int version) {
	size_t pos = 0;
	wxString temp;

	// Get type
	if (rawData.substr(pos,9) == _T("Dialogue:")) {
		Comment = false;
		pos = 10;
	}
	else if (rawData.substr(pos,8) == _T("Comment:")) {
		Comment = true;
		pos = 9;
	}
	else return false;
	wxStringTokenizer tkn(rawData.Mid(pos),_T(","),wxTOKEN_RET_EMPTY_ALL);

	// Get first token and see if it has "Marked=" in it
	if (!tkn.HasMoreTokens()) return false;
	temp = tkn.GetNextToken().Trim(false).Trim(true);
	if (temp.Lower().Left(7) == _T("marked=")) version = 0;
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
	StartMS = Start.GetMS();

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

#if USE_FEXTRACKER == 1
	if( Effect.BeforeFirst(':')==_T("FexMovement") )
	{
		if( Movement ) DeleteMovement( Movement );
		Movement = CreateMovement();
		LoadMovement( Movement, Effect.AfterFirst(':').c_str() );
	}
#endif

	// Get text
	Text = tkn.GetNextToken();
	while (tkn.HasMoreTokens()) {
		Text += _T(",");
		Text += tkn.GetNextToken();
	}
	return true;
}


/////////////
// Make data
wxString AssDialogue::MakeData() {
	// Prepare
	wxString final = _T("");

	// Write all final
	if (Comment) final += _T("Comment: ");
	else final += _T("Dialogue: ");

	final += wxString::Format(_T("%01i"),Layer);
	final += _T(",");

	final += Start.GetASSFormated() + _T(",");
	final += End.GetASSFormated() + _T(",");

	Style.Replace(_T(","),_T(";"));
	Actor.Replace(_T(","),_T(";"));
	final += Style + _T(",");
	final += Actor + _T(",");

	final += GetMarginString(0);
	final += _T(",");
	final += GetMarginString(1);
	final += _T(",");
	final += GetMarginString(2);
	final += _T(",");

	Effect.Replace(_T(","),_T(";"));
	final += Effect + _T(",");
	final += Text;
	return final;
}


//////////////////////////////////
// Update AssDialogue's data line
void AssDialogue::UpdateData () {
}


//////////////////
// Get entry data
const wxString AssDialogue::GetEntryData() {
	return MakeData();
}


//////////////////
// Set entry data
void AssDialogue::SetEntryData(wxString newData) {
}


///////////////////////////////
// Get SSA version of Dialogue
wxString AssDialogue::GetSSAText () {
	// Prepare
	wxString work = _T("");

	// Write all work
	if (Comment) work += _T("Comment: ");
	else work += _T("Dialogue: ");

	work += _T("Marked=0,");

	work += Start.GetASSFormated() + _T(",");
	work += End.GetASSFormated() + _T(",");

	Style.Replace(_T(","),_T(";"));
	Actor.Replace(_T(","),_T(";"));
	work += Style + _T(",");
	work += Actor + _T(",");

	work += GetMarginString(0);
	work += _T(",");
	work += GetMarginString(1);
	work += _T(",");
	work += GetMarginString(2);
	work += _T(",");

	Effect.Replace(_T(","),_T(";"));
	work += Effect + _T(",");
	work += Text;

	return work;
}


//////////////////
// Parse SRT tags
// --------------
// Yea, I convert to ASS tags, then parse that. So sue me.
void AssDialogue::ParseSRTTags () {
	// Search and replace
	size_t total = 0;
	total += Text.Replace(_T("<i>"),_T("{\\i1}"));
	total += Text.Replace(_T("</i>"),_T("{\\i0}"));
	total += Text.Replace(_T("<b>"),_T("{\\b1}"));
	total += Text.Replace(_T("</b>"),_T("{\\b0}"));
	total += Text.Replace(_T("<u>"),_T("{\\u1}"));
	total += Text.Replace(_T("</u>"),_T("{\\u0}"));
	total += Text.Replace(_T("<s>"),_T("{\\s1}"));
	total += Text.Replace(_T("</s>"),_T("{\\s0}"));

	// Process <font> tag
	wxString work = Text;
	work.UpperCase();
	size_t pos_open = 0;
	size_t pos_close = 0;
	size_t pos = 0;
	size_t end = 0;
	size_t start = 0;
	bool isOpen;

	// Iterate
	pos_open = work.find(_T("<FONT"),0);
	pos_close = work.find(_T("</FONT"),0);
	while (pos_open != wxString::npos || pos_close != wxString::npos) {
		// Determine if it's an open or close tag
		if (pos_open < pos_close) {
			start = pos_open;
			isOpen = true;
		}
		else {
			start = pos_close;
			isOpen = false;
		}
		end = work.find(_T(">"),start)+1;
		//if (end == wxString::npos) continue;

		// Open tag
		if (isOpen) {
			wxString replaced = _T("");

			// Color tag
			if ((pos = work.find(_T("COLOR=\""),start)) != wxString::npos) {
				if (pos < end) {
					pos += 7;
					size_t end_tag = Text.find(_T("\""),pos);
					if (end_tag != wxString::npos) {
						if (end_tag-pos == 7) {
							replaced += _T("{\\c&H");
							replaced += work.substr(pos+5,2);
							replaced += work.substr(pos+3,2);
							replaced += work.substr(pos+1,2);
							replaced += _T("&}");
							total++;
						}
					}
				}
			}

			// Face tag
			if ((pos = work.find(_T("FACE=\""),start)) != wxString::npos) {
				if (pos < end) {
					pos += 6;
					size_t end_tag = work.find(_T("\""),pos);
					if (end_tag != wxString::npos) {
						replaced += _T("{\\fn");
						replaced += work.substr(pos,end_tag-pos);
						replaced += _T("}");
						total++;
					}
				}
			}

			// Size tag
			if ((pos = work.find(_T("SIZE=\""),start)) != wxString::npos) {
				if (pos < end) {
					pos += 6;
					size_t end_tag = Text.find(_T("\""),pos);
					if (end_tag != wxString::npos) {
						replaced += _T("{\\fs");
						replaced += work.substr(pos,end_tag-pos);
						replaced += _T("}");
						total++;
					}
				}
			}

			// Replace whole tag
			//Text = Text.substr(0,start) + replaced + Text.substr(end);
			Text = Text.substr(0, start);
			Text << replaced << Text.substr(end);
			total++;
		}

		// Close tag
		else {
			// Find if it's italic, bold, underline, and strikeout
			wxString prev = Text.Left(start);
			bool isItalic=false,isBold=false,isUnder=false,isStrike=false;
			if (CountMatches(prev,_T("{\\i1}")) > CountMatches(prev,_T("{\\i0}"))) isItalic = true;
			if (CountMatches(prev,_T("{\\b1}")) > CountMatches(prev,_T("{\\b0}"))) isBold = true;
			if (CountMatches(prev,_T("{\\u1}")) > CountMatches(prev,_T("{\\u0}"))) isUnder = true;
			if (CountMatches(prev,_T("{\\s1}")) > CountMatches(prev,_T("{\\s0}"))) isStrike = true;

			// Generate new tag, by reseting and then restoring flags
			wxString replaced = _T("{\\r");
			if (isItalic) replaced += _T("\\i1");
			if (isBold) replaced += _T("\\b1");
			if (isUnder) replaced += _T("\\u1");
			if (isStrike) replaced += _T("\\s1");
			replaced += _T("}");

			// Replace
			//Text = Text.substr(0,start) + replaced + Text.substr(end);
			Text = Text.substr(0, start);
			Text << replaced << Text.substr(end);
			total++;
		}

		// Get next
		work = Text;
		work.UpperCase();
		pos_open = work.find(_T("<FONT"),0);
		pos_close = work.find(_T("</FONT"),0);
	}

	// Remove double tagging
	Text.Replace(_T("}{"),_T(""));

	// Update all stuff
	//if (total > 0) UpdateText();
	UpdateData();
}


//////////////////
// Parse ASS tags
void AssDialogue::ParseASSTags () {
	// Clear blocks
	ClearBlocks();

	// Is drawing?
	int drawingLevel = 0;

	// Loop through
	const size_t len = Text.size();
	size_t cur = 0;
	size_t end = 0;
	while (cur < len) {
		// Overrides block
		if (Text[cur] == '{') {
			// Get contents of block
			wxString work;
			end = Text.find(_T("}"),cur);
			if (end == wxString::npos) {
				work = Text.substr(cur);
				end = len;
			}
			else work = Text.substr(cur,end-cur);
			work = Text.substr(cur+1,end-cur-1);

			// Create block
			AssDialogueBlockOverride *block = new AssDialogueBlockOverride;
			block->parent = this;
			block->text = work;
			block->ParseTags();
			Blocks.push_back(block);

			// Look for \p in block
			std::vector<AssOverrideTag*>::iterator curTag;
			for (curTag = block->Tags.begin();curTag != block->Tags.end();curTag++) {
				AssOverrideTag *tag = *curTag;
				if (tag->Name == _T("\\p")) {
					drawingLevel = tag->Params.at(0)->AsInt();
				}
			}

			// Increase
			cur = end+1;
		}

		// Plain-text/drawing block
		else {
			wxString work;
			end = Text.find(_T("{"),cur);
			if (end == wxString::npos) {
				work = Text.substr(cur);
				end = len;
			}
			else work = Text.substr(cur,end-cur);

			// Plain-text
			if (drawingLevel == 0) {
				AssDialogueBlockPlain *block = new AssDialogueBlockPlain;
				block->text = work;
				Blocks.push_back(block);
			}

			// Drawing
			else {
				AssDialogueBlockDrawing *block = new AssDialogueBlockDrawing;
				block->text = work;
				block->Scale = drawingLevel;
				Blocks.push_back(block);
			}

			cur = end;
		}
	}

	// Empty line, make an empty block
	if (len == 0) {
		AssDialogueBlockPlain *block = new AssDialogueBlockPlain;
		block->text = _T("");
		Blocks.push_back(block);
	}
}


//////////////
// Strip tags
void AssDialogue::StripTags () {
	using std::list;
	using std::vector;
	ParseASSTags();
	vector<AssDialogueBlock*>::iterator next;
	for (vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur=next) {
		next = cur;
		next++;
		// FIXME: doesn't this crash when there's too many override blocks in one line?
		if ((*cur)->type == BLOCK_OVERRIDE) {
			delete *cur;
			Blocks.erase(cur);
		}
	}
	UpdateText();
	UpdateData();
	ClearBlocks();
}


////////////////////////
// Strip a specific tag
void AssDialogue::StripTag (wxString tagName) {
	// Parse
	using std::list;
	using std::vector;
	ParseASSTags();
	wxString final;

	// Look for blocks
	for (vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->type == BLOCK_OVERRIDE) {
			AssDialogueBlockOverride *over = AssDialogueBlock::GetAsOverride(*cur);
			wxString temp;
			for (size_t i=0;i<over->Tags.size();i++) {
				if (over->Tags[i]->Name != tagName) temp += over->Tags[i]->ToString();
			}

			// Insert
			if (!temp.IsEmpty()) final += _T("{") + temp + _T("}");
		}
		else final += (*cur)->GetText();
	}

	// Update
	ClearBlocks();
	Text = final;
	UpdateData();
}


///////////////////////
// Convert tags to SRT
// -------------------
// TODO: Improve this code
//
void AssDialogue::ConvertTagsToSRT () {
	// Setup
	using std::list;
	using std::vector;
	AssDialogueBlockOverride* curBlock;
	AssDialogueBlockPlain *curPlain;
	AssOverrideTag* curTag;
	wxString final = _T("");
	bool isItalic=false,isBold=false,isUnder=false,isStrike=false;
	bool temp;

	// Iterate through blocks
	ParseASSTags();
	for (size_t i=0;i<Blocks.size();i++) {
		curBlock = AssDialogueBlock::GetAsOverride(Blocks.at(i));
		if (curBlock) {
			// Iterate through overrides
			for (size_t j=0;j<curBlock->Tags.size();j++) {
				curTag = curBlock->Tags.at(j);
				if (curTag->IsValid()) {
					// Italics
					if (curTag->Name == _T("\\i")) {
						temp = curTag->Params.at(0)->AsBool();
						if (temp && !isItalic) {
							isItalic = true;
							final += _T("<i>");
						}
						if (!temp && isItalic) {
							isItalic = false;
							final += _T("</i>");
						}
					}

					// Underline
					if (curTag->Name == _T("\\u")) {
						temp = curTag->Params.at(0)->AsBool();
						if (temp && !isUnder) {
							isUnder = true;
							final += _T("<u>");
						}
						if (!temp && isUnder) {
							isUnder = false;
							final += _T("</u>");
						}
					}

					// Strikeout
					if (curTag->Name == _T("\\s")) {
						temp = curTag->Params.at(0)->AsBool();
						if (temp && !isStrike) {
							isStrike = true;
							final += _T("<s>");
						}
						if (!temp && isStrike) {
							isStrike = false;
							final += _T("</s>");
						}
					}

					// Bold
					if (curTag->Name == _T("\\b")) {
						temp = curTag->Params.at(0)->AsBool();
						if (temp && !isBold) {
							isBold = true;
							final += _T("<b>");
						}
						if (!temp && isBold) {
							isBold = false;
							final += _T("</b>");
						}
					}
				}
			}
		}

		// Plain text
		else {
			curPlain = AssDialogueBlock::GetAsPlain(Blocks.at(i));
			if (curPlain) {
				final += curPlain->GetText();
			}
		}
	}

	Text = final;
	UpdateData();
	ClearBlocks();
}


//////////////////////////
// Updates text from tags
void AssDialogue::UpdateText () {
	using std::vector;
	Text = _T("");
	for (vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->type == BLOCK_OVERRIDE) {
			Text += _T("{");
			Text += (*cur)->GetText();
			Text += _T("}");
		}
		else Text += (*cur)->GetText();
	}
}


/////////////////////////////
// Sets margin from a string
void AssDialogue::SetMarginString(const wxString origvalue,int which) {
	// Make it numeric
	wxString strvalue = origvalue;
	if (!strvalue.IsNumber()) {
		strvalue = _T("");
		for (size_t i=0;i<origvalue.Length();i++) {
			if (origvalue.Mid(i,1).IsNumber()) {
				strvalue += origvalue.Mid(i,1);
			}
		}
	}

	// Get value
	long value;
	strvalue.ToLong(&value);

	// Cap it
	if (value < 0) value = 0;
	if (value > 9999) value = 9999;

	// Assign
	if (which < 0 || which >= 4) throw _T("Invalid Margin");
	Margin[which] = value;
}


//////////////////////////
// Gets string for margin
wxString AssDialogue::GetMarginString(int which,bool pad) {
	if (which < 0 || which >= 4) throw _T("Invalid margin");
	int value = Margin[which];
	if (pad) return wxString::Format(_T("%04i"),value);
	else return wxString::Format(_T("%i"),value);
}


///////////////////////////////////
// Process parameters via callback
void AssDialogue::ProcessParameters(void (*callback)(wxString tagName,int par_n,AssOverrideParameter *param,void *userData),void *userData) {
	// Apply for all override blocks
	AssDialogueBlockOverride *curBlock;
	//ParseASSTags();
	for (std::vector<AssDialogueBlock*>::iterator cur=Blocks.begin();cur!=Blocks.end();cur++) {
		if ((*cur)->type == BLOCK_OVERRIDE) {
			curBlock = static_cast<AssDialogueBlockOverride*> (*cur);
			curBlock->ProcessParameters(callback,userData);
		}
	}
	//ClearBlocks();
}


///////////////////////////////
// Checks if two lines collide
bool AssDialogue::CollidesWith(AssDialogue *target) {
	if (!target) return false;
	int a = Start.GetMS();
	int b = End.GetMS();
	int c = target->Start.GetMS();
	int d = target->End.GetMS();
	return ((a < c) ? (c < b) : (a < d));
}


////////////////////////
// Return just the text without any overrides
wxString AssDialogue::GetStrippedText() {
	wxString justtext = wxString(_T(""));
	bool inCode = false;
	
	for (size_t charindex = 0; charindex != Text.Len(); charindex++) {
		if (Text[charindex] == '{') inCode = true;
		else if (Text[charindex] == '}') inCode = false;
		else if (!inCode) justtext = justtext + Text[charindex];
	}
	return justtext;
}
/////////
// Clone
AssEntry *AssDialogue::Clone() {
	// Create clone
	AssDialogue *final = new AssDialogue();

	// Copy data
	final->group = group;
	final->StartMS = StartMS;
	final->Valid = Valid;
	final->Actor = Actor;
	final->Comment = Comment;
	final->Effect = Effect;
	final->End = End;
	final->Layer = Layer;
	for (int i=0;i<4;i++) final->Margin[i] = Margin[i];
	final->Start = Start;
	final->StartMS = final->StartMS;
	final->Style = Style;
	final->Text = Text;
	final->SetEntryData(GetEntryData());

	// Return
	return final;
}


////////////////////// AssDialogueBlock //////////////////////
///////////////
// Constructor
AssDialogueBlock::AssDialogueBlock () {
	type = BLOCK_BASE;
}


//////////////
// Destructor
AssDialogueBlock::~AssDialogueBlock () {
}


////////////////////////////
// Returns as a plain block
// ----------------------
// If it isn't a plain block, returns NULL
AssDialogueBlockPlain *AssDialogueBlock::GetAsPlain(AssDialogueBlock *base) {
	if (!base) return NULL;
	if (base->type == BLOCK_PLAIN) {
		return static_cast<AssDialogueBlockPlain*> (base);
	}
	return NULL;
}


////////////////////////////////
// Returns as an override block
// ----------------------------
// If it isn't an override block, returns NULL
AssDialogueBlockOverride *AssDialogueBlock::GetAsOverride(AssDialogueBlock *base) {
	if (!base) return NULL;
	if (base->type == BLOCK_OVERRIDE) {
		return static_cast<AssDialogueBlockOverride*> (base);
	}
	return NULL;
}


//////////////////////////////
// Returns as a drawing block
// ----------------------------
// If it isn't an drawing block, returns NULL
AssDialogueBlockDrawing *AssDialogueBlock::GetAsDrawing(AssDialogueBlock *base) {
	if (!base) return NULL;
	if (base->type == BLOCK_DRAWING) {
		return static_cast<AssDialogueBlockDrawing*> (base);
	}
	return NULL;
}


////////////////////// AssDialogueBlockPlain //////////////////////
///////////////
// Constructor
AssDialogueBlockPlain::AssDialogueBlockPlain () {
	type = BLOCK_PLAIN;
}


///////////////////
// Return the text
wxString AssDialogueBlockPlain::GetText() {
	return text;
}


////////////////////// AssDialogueBlockDrawing //////////////////////
///////////////
// Constructor
AssDialogueBlockDrawing::AssDialogueBlockDrawing () {
	type = BLOCK_DRAWING;
}


///////////////////
// Return the text
wxString AssDialogueBlockDrawing::GetText() {
	return text;
}


////////////////////////
// Multiply coordinates
void AssDialogueBlockDrawing::MultiplyCoords(double x,double y) {
	// HACK: Implement a proper parser ffs!!
	wxStringTokenizer tkn(GetText(),_T(" "),wxTOKEN_DEFAULT);
	wxString cur;
	wxString final;
	bool isX = true;
	long temp;

	// Process tokens
	while (tkn.HasMoreTokens()) {
		cur = tkn.GetNextToken().Lower();

		// Number, process it
		if (cur.IsNumber()) {
			// Multiply it
			cur.ToLong(&temp);
			if (isX) temp = (long int)(temp*x + 0.5);
			else temp = (long int)(temp*y + 0.5);

			// Write back to list
			final += wxString::Format(_T("%i "),temp);

			// Toggle X/Y
			isX = !isX;
		}

		// Text
		else {
			if (cur == _T("m") || cur == _T("n") || cur == _T("l") || cur == _T("b") || cur == _T("s") || cur == _T("p") || cur == _T("c")) isX = true;
			final += cur + _T(" ");
		}
	}

	// Write back final
	final = final.Left(final.Length()-1);
	text = final;
}



