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

/// @file ass_override.cpp
/// @brief Parse and modify ASSA style overrides
/// @ingroup subs_storage
///

#include "config.h"

#ifndef AGI_PRE
#include <wx/log.h>
#include <wx/tokenzr.h>
#endif

#include <libaegisub/log.h>

#include "ass_dialogue.h"
#include "ass_override.h"

/// @brief Constructor
AssOverrideParameter::AssOverrideParameter () {
	classification = PARCLASS_NORMAL;
	ommited = false;
}

/// @brief Destructor 
AssOverrideParameter::~AssOverrideParameter () {
	DeleteValue();
}

/// @brief Copy 
/// @param param 
///
void AssOverrideParameter::CopyFrom (const AssOverrideParameter &param) {
	switch(param.GetType()) {
		case VARDATA_INT: SetInt(param.AsInt()); break;
		case VARDATA_FLOAT: SetFloat(param.AsFloat()); break;
		case VARDATA_TEXT: SetText(param.AsText()); break;
		case VARDATA_BOOL: SetBool(param.AsBool()); break;
		case VARDATA_COLOUR: SetColour(param.AsColour()); break;
		case VARDATA_BLOCK: SetBlock(param.AsBlock()); break;
		default: DeleteValue();
	}
	classification = param.classification;
	ommited = param.ommited;
}

/// @brief DOCME
/// @param param 
///
void AssOverrideParameter::operator= (const AssOverrideParameter &param) {
	CopyFrom(param);
}

/// @brief Constructor
AssDialogueBlockOverride::AssDialogueBlockOverride () {
}

/// @brief Destructor 
AssDialogueBlockOverride::~AssDialogueBlockOverride () {
	for (size_t i=0;i<Tags.size();i++) {
		delete Tags[i];
	}
	Tags.clear();
}

/// @brief Read tags 
///
void AssDialogueBlockOverride::ParseTags () {
	// Clear current vector
	for (size_t i=0;i<Tags.size();i++) {
		delete Tags[i];
	}
	Tags.clear();

	// Fix parenthesis matching
	while (text.Freq(_T('(')) > text.Freq(_T(')'))) {
		text += _T(")");
	}

	// Initialize tokenizer
	wxStringTokenizer tkn(text,_T("\\"),wxTOKEN_RET_EMPTY_ALL);
	wxString curTag;
	if (text.StartsWith(_T("\\"))) curTag = _T("\\");

	while (tkn.HasMoreTokens()) {
		//curTag will always start with a backslash after first loop - see end of loop
		curTag += tkn.GetNextToken();
		if (curTag == _T("\\")) continue;

		// Check for parenthesis matching
		while (curTag.Freq(_T('(')) > curTag.Freq(_T(')'))) {
			if (!tkn.HasMoreTokens()) {
				wxLogWarning(_T("Unmatched parenthesis! Line contents: ") + parent->Text);
				break;
			}
			curTag << _T("\\") << tkn.GetNextToken();
		}

		AssOverrideTag *newTag = new AssOverrideTag;
		newTag->SetText(curTag);
		Tags.push_back(newTag);

		curTag = _T("\\");
	}
}
void AssDialogueBlockOverride::AddTag(wxString const& tag) {
	AssOverrideTag *newTag = new AssOverrideTag;
	newTag->SetText(tag);
	Tags.push_back(newTag);
}

/// @brief Get Text representation 
/// @return 
///
wxString AssDialogueBlockOverride::GetText () {
	text = _T("");
	for (std::vector<AssOverrideTag*>::iterator cur=Tags.begin();cur!=Tags.end();cur++) {
		text += (*cur)->ToString();
	}
	return text;
}

void AssDialogueBlockOverride::ProcessParameters(AssDialogueBlockOverride::ProcessParametersCallback callback,void *userData) {
	for (std::vector<AssOverrideTag*>::iterator cur=Tags.begin();cur!=Tags.end();cur++) {
		int n = 0;
		AssOverrideTag *curTag = *cur;

		// Find parameters
		for (std::vector<AssOverrideParameter*>::iterator curParam=curTag->Params.begin();curParam!=curTag->Params.end();curParam++) {
			AssOverrideParameter *curPar = *curParam;

			if (curPar->GetType() != VARDATA_NONE && curPar->ommited == false) {
				// Do callback
				(*callback)(curTag->Name,n,curPar,userData);

				// Go recursive if it's a block parameter
				if (curPar->GetType() == VARDATA_BLOCK) {
					curPar->AsBlock()->ProcessParameters(callback,userData);
				}
			}

			n++;
		}
	}
}

/// @brief Constructor
/// @param _type  
/// @param opt    
/// @param classi 
///
AssOverrideParamProto::AssOverrideParamProto (VariableDataType _type,int opt,ASS_ParameterClass classi) {
	type = _type;
	optional = opt;
	classification = classi;
}

/// @brief Destructor 
AssOverrideParamProto::~AssOverrideParamProto() {
}

/// DOCME
std::vector<AssOverrideTagProto> AssOverrideTagProto::proto;

/// DOCME
bool AssOverrideTagProto::loaded = false;

/// @brief Constructor 
AssOverrideTagProto::AssOverrideTagProto() {
}

/// @brief Destructor 
AssOverrideTagProto::~AssOverrideTagProto() {
}

/// @brief Load prototypes 
/// @return 
///
void AssOverrideTagProto::LoadProtos () {
	if (loaded) return;
	loaded = true;

	proto.resize(56);
	int i = 0;

	// Longer tag names must appear before shorter tag names

	// \alpha
	proto[i].name = _T("\\alpha");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT));

	// \bord<depth>
	i++;
	proto[i].name = _T("\\bord");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \xbord<depth>
	i++;
	proto[i].name = _T("\\xbord");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \ybord<depth>
	i++;
	proto[i].name = _T("\\ybord");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \shad<depth>
	i++;
	proto[i].name = _T("\\shad");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \xshad<depth>
	i++;
	proto[i].name = _T("\\xshad");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \yshad<depth>
	i++;
	proto[i].name = _T("\\yshad");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \fade(<a1>,<a2>,<a3>,<t1>,<t2>,<t3>,<t4>)
	i++;
	proto[i].name = _T("\\fade");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));

	// \move(<x1>,<y1>,<x2>,<y2>[,<t1>,<t2>])
	i++;
	proto[i].name = _T("\\move");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_6,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_6,PARCLASS_RELATIVE_TIME_START));

	// If these are rearranged, keep rect clip and vector clip adjacent in this order
	// \clip(<x1>,<y1>,<x2>,<y2>)
	i++;
	proto[i].name = _T("\\clip");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \clip([<scale>,]<some drawings>)
	i++;
	proto[i].name = _T("\\clip");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_2,PARCLASS_NORMAL));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_DRAWING));

	// \iclip(<x1>,<y1>,<x2>,<y2>)
	i++;
	proto[i].name = _T("\\iclip");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \iclip([<scale>,]<some drawings>)
	i++;
	proto[i].name = _T("\\iclip");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_2,PARCLASS_NORMAL));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_DRAWING));

	// \fscx<percent>
	i++;
	proto[i].name = _T("\\fscx");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_RELATIVE_SIZE_X));

	// \fscy<percent>
	i++;
	proto[i].name = _T("\\fscy");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_RELATIVE_SIZE_Y));

	// \pos(<x>,<y>)
	i++;
	proto[i].name = _T("\\pos");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \org(<x>,<y>)
	i++;
	proto[i].name = _T("\\org");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \pbo<y>
	i++;
	proto[i].name = _T("\\pbo");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \fad(<t1>,<t2>)
	i++;
	proto[i].name = _T("\\fad");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_END));

	// \fsp<pixels>
	i++;
	proto[i].name = _T("\\fsp");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \frx<degrees>
	i++;
	proto[i].name = _T("\\frx");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fry<degrees>
	i++;
	proto[i].name = _T("\\fry");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));
	
	// \frz<degrees>
	i++;
	proto[i].name = _T("\\frz");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fr<degrees>
	i++;
	proto[i].name = _T("\\fr");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fax<factor>
	i++;
	proto[i].name = _T("\\fax");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fay<factor>
	i++;
	proto[i].name = _T("\\fay");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \1c&H<bbggrr>&
	i++;
	proto[i].name = _T("\\1c");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \2c&H<bbggrr>&
	i++;
	proto[i].name = _T("\\2c");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \3c&H<bbggrr>&
	i++;
	proto[i].name = _T("\\3c");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \4c&H<bbggrr>&
	i++;
	proto[i].name = _T("\\4c");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \1a&H<aa>&
	i++;
	proto[i].name = _T("\\1a");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \2a&H<aa>&
	i++;
	proto[i].name = _T("\\2a");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \3a&H<aa>&
	i++;
	proto[i].name = _T("\\3a");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \4a&H<aa>&
	i++;
	proto[i].name = _T("\\4a");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fe<charset>
	i++;
	proto[i].name = _T("\\fe");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \ko<duration>
	i++;
	proto[i].name = _T("\\ko");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \kf<duration>
	i++;
	proto[i].name = _T("\\kf");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \be<strength>
	i++;
	proto[i].name = _T("\\be");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \blur<strength>
	i++;
	proto[i].name = _T("\\blur");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fn<name>
	i++;
	proto[i].name = _T("\\fn");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs+<size>
	i++;
	proto[i].name = _T("\\fs+");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs-<size>
	i++;
	proto[i].name = _T("\\fs-");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs<size>
	i++;
	proto[i].name = _T("\\fs");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \an<alignment>
	i++;
	proto[i].name = _T("\\an");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \c&H<bbggrr>&
	i++;
	proto[i].name = _T("\\c");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \b<0/1/weight>
	i++;
	proto[i].name = _T("\\b");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_1,PARCLASS_NORMAL));
	proto[i].params.back().defaultValue.SetBool(false);

	// \i<0/1>
	i++;
	proto[i].name = _T("\\i");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto[i].params.back().defaultValue.SetBool(false);

	// \u<0/1>
	i++;
	proto[i].name = _T("\\u");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto[i].params.back().defaultValue.SetBool(false);

	// \s<0/1>
	i++;
	proto[i].name = _T("\\s");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto[i].params.back().defaultValue.SetBool(false);

	// \a<alignment>
	i++;
	proto[i].name = _T("\\a");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \k<duration>
	i++;
	proto[i].name = _T("\\k");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \K<duration>
	i++;
	proto[i].name = _T("\\K");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \q<0-3>
	i++;
	proto[i].name = _T("\\q");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \p<n>
	i++;
	proto[i].name = _T("\\p");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \r[<name>]
	i++;
	proto[i].name = _T("\\r");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_TEXT,OPTIONAL_1,PARCLASS_NORMAL));

	// \t([<t1>,<t2>,][<accel>,]<style modifiers>)
	i++;
	proto[i].name = _T("\\t");
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_3 | OPTIONAL_4,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_3 | OPTIONAL_4,PARCLASS_RELATIVE_TIME_START));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_FLOAT,OPTIONAL_2 | OPTIONAL_4,PARCLASS_NORMAL));
	proto[i].params.push_back(AssOverrideParamProto(VARDATA_BLOCK,NOT_OPTIONAL,PARCLASS_NORMAL));
}

/// @brief Constructor
AssOverrideTag::AssOverrideTag () {
	valid = false;
}

/// @brief Destructor
AssOverrideTag::~AssOverrideTag () {
	Clear();
}

/// @brief Clear
void AssOverrideTag::Clear() {
	for (std::vector<AssOverrideParameter*>::iterator cur=Params.begin();cur!=Params.end();cur++) {
		delete *cur;
	}
	Params.clear();
	Params.reserve(6);
	valid = false;
}

/// @brief Parses text and sets tag 
/// @param text 
void AssOverrideTag::SetText (const wxString &text) {
	// Determine name
	for (AssOverrideTagProto::iterator cur=AssOverrideTagProto::proto.begin();cur!=AssOverrideTagProto::proto.end();cur++) {
		if (text.StartsWith(cur->name)) {
			Name = cur->name;
			ParseParameters(text.Mid(Name.length()), cur);
			valid = true;
			return;
		}
	}
	// Junk tag
	Name = text;
	valid = false;
}

/// @brief Checks if it is valid 
/// @return 
///
bool AssOverrideTag::IsValid() {
	return valid;
}

/// @brief Parses the parameters for the ass override tag
/// @param text All text between the name and the next \ or the end of the override block
///
void AssOverrideTag::ParseParameters(const wxString &text, AssOverrideTagProto::iterator proto) {
	Clear();

	// Tokenize text, attempting to find all parameters
	wxArrayString paramList;
	paramList.reserve(6);
	wxString work;

	{
		if (text.IsEmpty() || text[0] != _T('(')) {
			// There's just one (or none at all) parameter (because there's no parantheses)
			// This means text is all our parameters
			wxString param(text);
			paramList.Add(param.Trim(true).Trim(false));
			// Only using goto here to avoid yet another nested block (keeps the code cleaner!)
			goto end_tokenizing;
		}

		// Ok, so there are parantheses used here, so there may be more than one parameter
		// Enter fullscale parsing!
		size_t i = 0, textlen = text.Length();
		size_t start = 0;
		int parDepth = 1;
		while (i < textlen && parDepth > 0) {
			// Just skip until next ',' or ')', whichever comes first
			// (Next ')' is achieved when parDepth == 0)
			start = ++i;
			while (i < textlen && parDepth > 0) {
				wxChar c = text[i];
				// parDepth 1 is where we start, and the tag-level we're interested in parsing on
				if (c == L',' && parDepth == 1) break;
				if (c == _T('(')) parDepth++;
				else if (c == _T(')')) {
					parDepth--;
					if (parDepth < 0) {
						wxLogWarning(_T("Unmatched parenthesis near '%s'!\nTag-parsing incomplete."), text.SubString(i, 10).c_str());
						goto end_tokenizing;
					}
					else if (parDepth == 0) {
						// We just ate the paranthesis ending this parameter block
						// Make sure it doesn't get included in the parameter text
						break;
					}
				}
				i++;
			}
			// i now points to the first character not member of this parameter
			work = text.SubString(start, i-1);
			work.Trim(true).Trim(false);
			paramList.Add(work);
			//LOG_D("subtitle/ass/override" << "Got parameter: <<  work.c_str();
		}

		if (i+1 < textlen) {
			// There's some additional garbage after the parantheses
			// Just add it in for completeness
			paramList.Add(text.Mid(i+1));
		}
	}
	// This label is only gone to from inside the previous block, if the tokenizing needs to end early
end_tokenizing:

	int curPar = 0;
	size_t totalPars = paramList.GetCount();

	// Get optional parameters flag
	ASS_ParameterOptional parsFlag = OPTIONAL_0;
	switch (totalPars) {
		case 1: parsFlag = OPTIONAL_1; break;
		case 2: parsFlag = OPTIONAL_2; break;
		case 3: parsFlag = OPTIONAL_3; break;
		case 4: parsFlag = OPTIONAL_4; break;
		case 5: parsFlag = OPTIONAL_5; break;
		case 6: parsFlag = OPTIONAL_6; break;
		case 7: parsFlag = OPTIONAL_7; break;
	}

	// vector (i)clip is the second clip prototype in the list
	if ((Name == _T("\\clip") || Name == _T("\\iclip")) && totalPars != 4) {
		++proto;
	}
	
	// Get parameters
	size_t n=0;
	wxString curtok = _T("");
	if (curPar < (signed)totalPars) {
		curtok = paramList[curPar];
		curPar++;
	}

	// For each parameter
	while (n < proto->params.size()) {
		AssOverrideParamProto *curproto = &proto->params[n];
		bool isDefault = false;
		n++;

		// Create parameter
		AssOverrideParameter *newparam = new AssOverrideParameter;
		newparam->classification = curproto->classification;
		Params.push_back(newparam);

		// Check if it's optional and not set (set to default)
		if (!(curproto->optional & parsFlag)) {
			if (curproto->defaultValue.GetType() != VARDATA_NONE) {
				isDefault = true;
				newparam->CopyFrom(curproto->defaultValue);
			}
			newparam->ommited = true;
			// This parameter doesn't really count against the number of parsed parameters,
			// since it's left out. Don't count it.
			curPar--;
		}

		if (isDefault == false && curtok.length() > 0) {
			wxChar firstChar = curtok[0];
			bool auto4 = (firstChar == _T('!') || firstChar == _T('$') || firstChar == _T('%')) && curproto->type != VARDATA_BLOCK;
			if (auto4) {
				newparam->SetText(curtok);
			}
			else {
			// Determine parameter type and set value
				switch (curproto->type) {
					case VARDATA_INT: {
						long temp = 0;
						curtok.ToLong(&temp);
						newparam->SetInt(temp);
						break;
					}
					case VARDATA_FLOAT: {
						double temp = 0.0;
						curtok.ToDouble(&temp);
						newparam->SetFloat(temp);
						break;
					}
					case VARDATA_TEXT:
						newparam->SetText(curtok);
						break;
					case VARDATA_BOOL: {
						long temp = false;
						curtok.ToLong(&temp);
						newparam->SetBool(temp != 0);
						break;
					}
					case VARDATA_BLOCK: {
						AssDialogueBlockOverride *temp = new AssDialogueBlockOverride;
						temp->text = curtok;
						temp->ParseTags();
						newparam->SetBlock(temp);
						break;
					}
					default:
						break;
				}
			}

			// Get next actual parameter
			if (curPar < (signed)totalPars) {
				// Unless this parameter was omitted (in which case the token shouldn't be eaten)
				if (!newparam->ommited) {
					curtok = paramList[curPar];
				}
				curPar++;
			}
			else curtok = _T("");
		}
	}
}

/// @brief Get string 
wxString AssOverrideTag::ToString() {
	// Start with name
	wxString result = Name;

	// Determine if it needs parentheses
	bool parenthesis = false;
	if (Name == _T("\\t") ||
	    Name == _T("\\pos") ||
	    Name == _T("\\fad") ||
	    Name == _T("\\org") ||
	    Name == _T("\\clip") ||
	    Name == _T("\\iclip") ||
	    Name == _T("\\move") ||
	    Name == _T("\\fade")) parenthesis = true;
	if (parenthesis) result += _T("(");

	// Add parameters
	int n = 0;
	for (std::vector<AssOverrideParameter*>::iterator cur=Params.begin();cur!=Params.end();cur++) {
		if ((*cur)->GetType() != VARDATA_NONE && (*cur)->ommited == false) {
			result += (*cur)->AsText();
			result += _T(",");
			n++;
		}
	}
	if (n > 0) result = result.Left(result.Length()-1);

	// Finish
	if (parenthesis) result += _T(")");
	return result;
}
