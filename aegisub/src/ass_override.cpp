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


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/log.h>
#include <wx/tokenzr.h>
#endif

#include "ass_dialogue.h"
#include "ass_override.h"


/// @brief Constructor  AssOverrideParameter //////////////////////
///
AssOverrideParameter::AssOverrideParameter () {
	classification = PARCLASS_NORMAL;
	ommited = false;
}



/// @brief Destructor 
///
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



/// @brief Constructor  AssDialogueBlockOverride //////////////////////
///
AssDialogueBlockOverride::AssDialogueBlockOverride () {
}



/// @brief Destructor 
///
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
	AssOverrideTag *curTag;
	AssOverrideParameter *curPar;

	// Find tags
	for (std::vector<AssOverrideTag*>::iterator cur=Tags.begin();cur!=Tags.end();cur++) {
		int n = 0;
		curTag = *cur;

		// Find parameters
		for (std::vector<AssOverrideParameter*>::iterator curParam=curTag->Params.begin();curParam!=curTag->Params.end();curParam++) {
            curPar = *curParam;

			if (curPar->GetType() != VARDATA_NONE && curPar->ommited == false) {
				// Do callback
				(*callback)(curTag->Name,n,curPar,userData);

				// Go recursive if it's a block parameter
				//if (curPar->classification == VARDATA_BLOCK) {
				if (curPar->GetType() == VARDATA_BLOCK) {
					curPar->AsBlock()->ProcessParameters(callback,userData);
				}
			}

			n++;
		}
	}
}



/// @brief Constructor  AssOverrideParamProto //////////////////////////
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
///
AssOverrideParamProto::~AssOverrideParamProto() {
}



/// DOCME
std::list<AssOverrideTagProto> AssOverrideTagProto::proto;

/// DOCME
bool AssOverrideTagProto::loaded = false;



/// @brief Constructor 
///
AssOverrideTagProto::AssOverrideTagProto() {
}



/// @brief Destructor 
///
AssOverrideTagProto::~AssOverrideTagProto() {
}



/// @brief Load prototypes 
/// @return 
///
void AssOverrideTagProto::LoadProtos () {
	if (loaded) return;
	loaded = true;

	// \alpha
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\alpha");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT));

	// \bord<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\bord");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \xbord<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\xbord");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \ybord<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\ybord");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \shad<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\shad");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \xshad<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\xshad");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \yshad<depth>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\yshad");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \fade(<a1>,<a2>,<a3>,<t1>,<t2>,<t3>,<t4>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fade");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));

	// \move(<x1>,<y1>,<x2>,<y2>[,<t1>,<t2>])
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\move");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_6,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_6,PARCLASS_RELATIVE_TIME_START));

	// \clip(<x1>,<y1>,<x2>,<y2>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\clip");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \clip([<scale>,]<some drawings>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\clip");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_2,PARCLASS_NORMAL));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_DRAWING));

	// \iclip(<x1>,<y1>,<x2>,<y2>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\iclip");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \iclip([<scale>,]<some drawings>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\iclip");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_2,PARCLASS_NORMAL));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_DRAWING));

	// \fscx<percent>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fscx");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_RELATIVE_SIZE_X));

	// \fscy<percent>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fscy");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_RELATIVE_SIZE_Y));

	// \pos(<x>,<y>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\pos");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \org(<x>,<y>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\org");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_X));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \pbo<y>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\pbo");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_POS_Y));

	// \fad(<t1>,<t2>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fad");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_RELATIVE_TIME_END));

	// \fsp<pixels>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fsp");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));

	// \frx<degrees>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\frx");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fry<degrees>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fry");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));
	
	// \frz<degrees>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\frz");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fr<degrees>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fr");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fax<factor>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fax");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fay<factor>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fay");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \1c&H<bbggrr>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\1c");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \2c&H<bbggrr>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\2c");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \3c&H<bbggrr>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\3c");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \4c&H<bbggrr>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\4c");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \1a&H<aa>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\1a");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \2a&H<aa>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\2a");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \3a&H<aa>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\3a");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \4a&H<aa>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\4a");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fe<charset>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fe");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \ko<duration>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\ko");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \kf<duration>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\kf");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \be<strength>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\be");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \blur<strength>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\blur");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fn<name>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fn");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs+<size>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fs+");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs-<size>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fs-");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \fs<size>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\fs");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,NOT_OPTIONAL,PARCLASS_ABSOLUTE_SIZE));


	// \an<alignment>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\an");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \c&H<bbggrr>&
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\c");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \b<0/1/weight>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\b");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_1,PARCLASS_NORMAL));
	proto.back().params.back().defaultValue.SetBool(false);

	// \i<0/1>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\i");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto.back().params.back().defaultValue.SetBool(false);

	// \u<0/1>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\u");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto.back().params.back().defaultValue.SetBool(false);

	// \s<0/1>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\s");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_BOOL,OPTIONAL_1,PARCLASS_NORMAL));
	proto.back().params.back().defaultValue.SetBool(false);

	// \a<alignment>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\a");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \k<duration>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\k");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \K<duration>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\K");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_KARAOKE));

	// \q<0-3>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\q");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \p<n>
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\p");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,NOT_OPTIONAL,PARCLASS_NORMAL));

	// \r[<name>]
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\r");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_TEXT,OPTIONAL_1,PARCLASS_NORMAL));

	// \t([<t1>,<t2>,][<accel>,]<style modifiers>)
	proto.push_back(AssOverrideTagProto());
	proto.back().name = _T("\\t");
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_3 | OPTIONAL_4,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_INT,OPTIONAL_3 | OPTIONAL_4,PARCLASS_RELATIVE_TIME_START));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_FLOAT,OPTIONAL_2 | OPTIONAL_4,PARCLASS_NORMAL));
	proto.back().params.push_back(AssOverrideParamProto(VARDATA_BLOCK,NOT_OPTIONAL,PARCLASS_NORMAL));
}



/// @brief Constructor  AssOverrideTag //////////////////////////
///
AssOverrideTag::AssOverrideTag () {
	valid = false;
}



/// @brief Destructor 
///
AssOverrideTag::~AssOverrideTag () {
	Clear();
}



/// @brief Clear 
///
void AssOverrideTag::Clear() {
	for (std::vector<AssOverrideParameter*>::iterator cur=Params.begin();cur!=Params.end();cur++) {
		delete *cur;
	}
	Params.clear();
	valid = false;
}



/// @brief Parses text and sets tag 
/// @param text 
///
void AssOverrideTag::SetText (const wxString &text) {
	// Determine name
	Name = _T("");
	AssOverrideTagProto *proto;
	for (std::list<AssOverrideTagProto>::iterator cur=AssOverrideTagProto::proto.begin();cur!=AssOverrideTagProto::proto.end();cur++) {
		proto = &(*cur);
		if (text.Left(proto->name.length()) == proto->name) {
			Name = proto->name;
			break;
		}
	}

	// Set tag name
	if (!Name.empty()) {
		ParseParameters(text.Mid(Name.length()));
		valid = true;
	}

	// Junk tag
	else {
		Name = text;
		valid = false;
	}
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
void AssOverrideTag::ParseParameters(const wxString &text) {
	// Clear first
	Clear();

	// Tokenize text, attempting to find all parameters
	wxArrayString paramList;
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
				if (text[i] == _T('(')) parDepth++;
				if (text[i] == _T(')')) parDepth--;
				// parDepth 1 is where we start, and the tag-level we're interested in parsing on
				if (text[i] == _T(',') && parDepth == 1) break;
				if (parDepth < 0) {
					wxLogWarning(_T("Unmatched parenthesis near '%s'!\nTag-parsing incomplete."), text.SubString(i, 10).c_str());
					goto end_tokenizing;
				}
				if (parDepth == 0) {
					// We just ate the paranthesis ending this parameter block
					// Make sure it doesn't get included in the parameter text
					break;
				}
				i++;
			}
			// i now points to the first character not member of this parameter
			work = text.SubString(start, i-1);
			work.Trim(true).Trim(false);
			paramList.Add(work);
			//wxLogDebug(_T("Got parameter: %s"), work.c_str());
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

	// Find prototype
	bool clipOnce = true, iclipOnce = true;
	AssOverrideTagProto *proto = NULL;
	for (std::list<AssOverrideTagProto>::iterator cur=AssOverrideTagProto::proto.begin();cur!=AssOverrideTagProto::proto.end();cur++) {
		if (Name == (*cur).name) {
			if (Name == _T("\\clip") && totalPars != 4 && clipOnce) {
				clipOnce = false;
				continue;
			}
			if (Name == _T("\\iclip") && totalPars != 4 && iclipOnce) {
				iclipOnce = false;
				continue;
			}
			proto = &(*cur);
			break;
		}
	}
	if (proto == NULL) {
		throw _T("Couldn't find tag prototype while parsing.");
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

		// Add to list
		newparam->classification = curproto->classification;
		Params.push_back(newparam);
	}
}



/// @brief Get string 
///
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


