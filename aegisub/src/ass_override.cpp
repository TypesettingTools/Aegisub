// Copyright (c) 2005, Rodrigo Braz Monteiro
// Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
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
#include "utils.h"

AssOverrideParameter::AssOverrideParameter()
: classification(PARCLASS_NORMAL)
, omitted(false)
{
}

AssOverrideParameter::AssOverrideParameter(const AssOverrideParameter &param)
: VariableData(param)
, classification(param.classification)
, omitted(param.omitted)
{
}

void AssOverrideParameter::operator=(const AssOverrideParameter &param) {
	DeleteValue();
	new(this) AssOverrideParameter(param);
}

// From ass_dialogue.h
AssDialogueBlockOverride::~AssDialogueBlockOverride() {
	delete_clear(Tags);
}

void AssDialogueBlockOverride::ParseTags() {
	delete_clear(Tags);

	wxStringTokenizer tkn(text, L"\\", wxTOKEN_STRTOK);
	wxString curTag;
	if (text.StartsWith(L"\\")) curTag = L"\\";

	while (tkn.HasMoreTokens()) {
		curTag += tkn.GetNextToken();

		// Check for parenthesis matching for \t
		while (curTag.Freq(L'(') > curTag.Freq(L')') && tkn.HasMoreTokens()) {
			curTag << L"\\" << tkn.GetNextToken();
		}

		Tags.push_back(new AssOverrideTag(curTag));

		curTag = L"\\";
	}
}
void AssDialogueBlockOverride::AddTag(wxString const& tag) {
	Tags.push_back(new AssOverrideTag(tag));
}

wxString AssDialogueBlockOverride::GetText() {
	text.clear();
	for (std::vector<AssOverrideTag*>::iterator cur=Tags.begin();cur!=Tags.end();cur++) {
		text += **cur;
	}
	return text;
}

void AssDialogueBlockOverride::ProcessParameters(AssDialogueBlockOverride::ProcessParametersCallback callback,void *userData) {
	for (std::vector<AssOverrideTag*>::iterator cur=Tags.begin();cur!=Tags.end();cur++) {
		AssOverrideTag *curTag = *cur;

		// Find parameters
		for (unsigned n = 0; n < curTag->Params.size(); n++) {
			AssOverrideParameter *curPar = curTag->Params[n];

			if (curPar->GetType() != VARDATA_NONE && !curPar->omitted) {
				(*callback)(curTag->Name,n,curPar,userData);

				// Go recursive if it's a block parameter
				if (curPar->GetType() == VARDATA_BLOCK) {
					curPar->Get<AssDialogueBlockOverride*>()->ProcessParameters(callback,userData);
				}
			}
		}
	}
}

AssOverrideParamProto::AssOverrideParamProto(VariableDataType type,int opt,ASS_ParameterClass classi)
: type(type)
, optional(opt)
, classification(classi)
{
}

void AssOverrideTagProto::AddParam(VariableDataType type, ASS_ParameterClass classi, int opt) {
	params.push_back(AssOverrideParamProto(type, opt, classi));
}
void AssOverrideTagProto::Set(wxString newName, VariableDataType type, ASS_ParameterClass classi, int opt) {
	name = newName;
	params.push_back(AssOverrideParamProto(type, opt, classi));
}

static std::vector<AssOverrideTagProto> proto;
static void load_protos() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	proto.resize(56);
	int i = 0;

	// Longer tag names must appear before shorter tag names

	proto[0].Set(L"\\alpha", VARDATA_TEXT); // \alpha
	proto[++i].Set(L"\\bord", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \bord<depth>
	proto[++i].Set(L"\\xbord", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \xbord<depth>
	proto[++i].Set(L"\\ybord", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \ybord<depth>
	proto[++i].Set(L"\\shad", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \shad<depth>
	proto[++i].Set(L"\\xshad", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \xshad<depth>
	proto[++i].Set(L"\\yshad", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \yshad<depth>
	
	// \fade(<a1>,<a2>,<a3>,<t1>,<t2>,<t3>,<t4>)
	i++;
	proto[i].name = L"\\fade";
	proto[i].AddParam(VARDATA_INT);
	proto[i].AddParam(VARDATA_INT);
	proto[i].AddParam(VARDATA_INT);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);

	// \move(<x1>,<y1>,<x2>,<y2>[,<t1>,<t2>])
	i++;
	proto[i].name = L"\\move";
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_Y);
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_Y);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);

	// If these are rearranged, keep rect clip and vector clip adjacent in this order
	// \clip(<x1>,<y1>,<x2>,<y2>)
	i++;
	proto[i].name = L"\\clip";
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y);

	// \clip([<scale>,]<some drawings>)
	i++;
	proto[i].name = L"\\clip";
	proto[i].AddParam(VARDATA_INT,PARCLASS_NORMAL,OPTIONAL_2);
	proto[i].AddParam(VARDATA_TEXT,PARCLASS_DRAWING);

	// \iclip(<x1>,<y1>,<x2>,<y2>)
	i++;
	proto[i].name = L"\\iclip";
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y);

	// \iclip([<scale>,]<some drawings>)
	i++;
	proto[i].name = L"\\iclip";
	proto[i].AddParam(VARDATA_INT,PARCLASS_NORMAL,OPTIONAL_2);
	proto[i].AddParam(VARDATA_TEXT,PARCLASS_DRAWING);

	proto[++i].Set(L"\\fscx", VARDATA_FLOAT,PARCLASS_RELATIVE_SIZE_X); // \fscx<percent>
	proto[++i].Set(L"\\fscy", VARDATA_FLOAT,PARCLASS_RELATIVE_SIZE_Y); // \fscy<percent>
	// \pos(<x>,<y>)
	i++;
	proto[i].name = L"\\pos";
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_ABSOLUTE_POS_Y);

	// \org(<x>,<y>)
	i++;
	proto[i].name = L"\\org";
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_X);
	proto[i].AddParam(VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y);

	proto[++i].Set(L"\\pbo", VARDATA_INT,PARCLASS_ABSOLUTE_POS_Y); // \pbo<y>
	// \fad(<t1>,<t2>)
	i++;
	proto[i].name = L"\\fad";
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_END);

	proto[++i].Set(L"\\fsp", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \fsp<pixels>
	proto[++i].Set(L"\\frx", VARDATA_FLOAT); // \frx<degrees>
	proto[++i].Set(L"\\fry", VARDATA_FLOAT); // \fry<degrees>
	proto[++i].Set(L"\\frz", VARDATA_FLOAT); // \frz<degrees>
	proto[++i].Set(L"\\fr", VARDATA_FLOAT); // \fr<degrees>
	proto[++i].Set(L"\\fax", VARDATA_FLOAT); // \fax<factor>
	proto[++i].Set(L"\\fay", VARDATA_FLOAT); // \fay<factor>
	proto[++i].Set(L"\\1c", VARDATA_TEXT); // \1c&H<bbggrr>&
	proto[++i].Set(L"\\2c", VARDATA_TEXT); // \2c&H<bbggrr>&
	proto[++i].Set(L"\\3c", VARDATA_TEXT); // \3c&H<bbggrr>&
	proto[++i].Set(L"\\4c", VARDATA_TEXT); // \4c&H<bbggrr>&
	proto[++i].Set(L"\\1a", VARDATA_TEXT); // \1a&H<aa>&
	proto[++i].Set(L"\\2a", VARDATA_TEXT); // \2a&H<aa>&
	proto[++i].Set(L"\\3a", VARDATA_TEXT); // \3a&H<aa>&
	proto[++i].Set(L"\\4a", VARDATA_TEXT); // \4a&H<aa>&
	proto[++i].Set(L"\\fe", VARDATA_TEXT); // \fe<charset>
	proto[++i].Set(L"\\ko", VARDATA_INT,PARCLASS_KARAOKE); // \ko<duration>
	proto[++i].Set(L"\\kf", VARDATA_INT,PARCLASS_KARAOKE); // \kf<duration>
	proto[++i].Set(L"\\be", VARDATA_INT); // \be<strength>
	proto[++i].Set(L"\\blur", VARDATA_FLOAT); // \blur<strength>
	proto[++i].Set(L"\\fn", VARDATA_TEXT); // \fn<name>
	proto[++i].Set(L"\\fs+", VARDATA_FLOAT); // \fs+<size>
	proto[++i].Set(L"\\fs-", VARDATA_FLOAT); // \fs-<size>
	proto[++i].Set(L"\\fs", VARDATA_FLOAT,PARCLASS_ABSOLUTE_SIZE); // \fs<size>
	proto[++i].Set(L"\\an", VARDATA_INT); // \an<alignment>
	proto[++i].Set(L"\\c", VARDATA_TEXT); // \c&H<bbggrr>&
	proto[++i].Set(L"\\b", VARDATA_INT); // \b<0/1/weight>
	proto[++i].Set(L"\\i", VARDATA_BOOL); // \i<0/1>
	proto[++i].Set(L"\\u", VARDATA_BOOL); // \u<0/1>
	proto[++i].Set(L"\\s", VARDATA_BOOL); // \s<0/1>
	proto[++i].Set(L"\\a", VARDATA_INT); // \a<alignment>
	proto[++i].Set(L"\\k", VARDATA_INT,PARCLASS_KARAOKE); // \k<duration>
	proto[++i].Set(L"\\K", VARDATA_INT,PARCLASS_KARAOKE); // \K<duration>
	proto[++i].Set(L"\\q", VARDATA_INT); // \q<0-3>
	proto[++i].Set(L"\\p", VARDATA_INT); // \p<n>
	proto[++i].Set(L"\\r", VARDATA_TEXT); // \r[<name>]

	// \t([<t1>,<t2>,][<accel>,]<style modifiers>)
	i++;
	proto[i].name = L"\\t";
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START,OPTIONAL_3 | OPTIONAL_4);
	proto[i].AddParam(VARDATA_INT,PARCLASS_RELATIVE_TIME_START,OPTIONAL_3 | OPTIONAL_4);
	proto[i].AddParam(VARDATA_FLOAT,PARCLASS_NORMAL,OPTIONAL_2 | OPTIONAL_4);
	proto[i].AddParam(VARDATA_BLOCK);
}

AssOverrideTag::AssOverrideTag() : valid(false) { }
AssOverrideTag::AssOverrideTag(wxString text) {
	SetText(text);
}

AssOverrideTag::~AssOverrideTag () {
	delete_clear(Params);
}

void AssOverrideTag::Clear() {
	delete_clear(Params);
	Params.reserve(6);
	valid = false;
}

void AssOverrideTag::SetText(const wxString &text) {
	load_protos();
	for (AssOverrideTagProto::iterator cur=proto.begin();cur!=proto.end();cur++) {
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

bool AssOverrideTag::IsValid() {
	return valid;
}

std::vector<wxString> tokenize(const wxString &text) {
	std::vector<wxString> paramList;
	paramList.reserve(6);

	if (text.empty()) {
		return paramList;
	}
	if (text[0] != L'(') {
		// There's just one parameter (because there's no parentheses)
		// This means text is all our parameters
		wxString param(text);
		paramList.push_back(param.Trim(true).Trim(false));
		return paramList;
	}

	// Ok, so there are parentheses used here, so there may be more than one parameter
	// Enter fullscale parsing!
	size_t i = 0, textlen = text.size();
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
					wxLogWarning(L"Unmatched parenthesis near '%s'!\nTag-parsing incomplete.", text.SubString(i, 10).c_str());
					return paramList;
				}
				else if (parDepth == 0) {
					// We just ate the parenthesis ending this parameter block
					// Make sure it doesn't get included in the parameter text
					break;
				}
			}
			i++;
		}
		// i now points to the first character not member of this parameter
		paramList.push_back(text.SubString(start, i-1).Trim(true).Trim(false));
	}

	if (i+1 < textlen) {
		// There's some additional garbage after the parentheses
		// Just add it in for completeness
		paramList.push_back(text.Mid(i+1));
	}
	return paramList;
}

void AssOverrideTag::ParseParameters(const wxString &text, AssOverrideTagProto::iterator proto) {
	Clear();

	// Tokenize text, attempting to find all parameters
	std::vector<wxString> paramList = tokenize(text);
	size_t totalPars = paramList.size();

	int parsFlag = 1 << (totalPars - 1); // Get optional parameters flag
	// vector (i)clip is the second clip prototype in the list
	if ((Name == L"\\clip" || Name == L"\\iclip") && totalPars != 4) {
		++proto;
	}

	unsigned curPar = 0;
	for (size_t n = 0; n < proto->params.size(); n++) {
		AssOverrideParamProto *curproto = &proto->params[n];

		// Create parameter
		AssOverrideParameter *newparam = new AssOverrideParameter;
		newparam->classification = curproto->classification;
		Params.push_back(newparam);

		// Check if it's optional and not present
		if (!(curproto->optional & parsFlag) || curPar >= totalPars) {
			newparam->omitted = true;
			continue;
		}

		wxString curtok = paramList[curPar++];

		if (curtok.empty()) {
			curPar++;
			continue;
		}

		wxChar firstChar = curtok[0];
		bool auto4 = (firstChar == _T('!') || firstChar == _T('$') || firstChar == _T('%')) && curproto->type != VARDATA_BLOCK;
		if (auto4) {
			newparam->Set(curtok);
		}
		else {
			switch (curproto->type) {
				case VARDATA_INT: {
					long temp;
					curtok.ToLong(&temp);
					newparam->Set<int>(temp);
					break;
				}
				case VARDATA_FLOAT: {
					double temp;
					curtok.ToDouble(&temp);
					newparam->Set(temp);
					break;
				}
				case VARDATA_TEXT:
					newparam->Set(curtok);
					break;
				case VARDATA_BOOL: {
					long temp;
					curtok.ToLong(&temp);
					newparam->Set<bool>(temp != 0);
					break;
				}
				case VARDATA_BLOCK: {
					AssDialogueBlockOverride *temp = new AssDialogueBlockOverride;
					temp->text = curtok;
					temp->ParseTags();
					newparam->Set(temp);
					break;
				}
				default:
					break;
			}
		}
	}
}

AssOverrideTag::operator wxString() {
	wxString result = Name;

	// Determine if it needs parentheses
	bool parentheses =
		Name == L"\\t" ||
		Name == L"\\pos" ||
		Name == L"\\fad" ||
		Name == L"\\org" ||
		Name == L"\\clip" ||
		Name == L"\\iclip" ||
		Name == L"\\move" ||
		Name == L"\\fade";
	if (parentheses) result += L"(";

	// Add parameters
	bool any = false;
	for (std::vector<AssOverrideParameter*>::iterator cur=Params.begin();cur!=Params.end();cur++) {
		if ((*cur)->GetType() != VARDATA_NONE && !(*cur)->omitted) {
			result += (*cur)->Get<wxString>();
			result += L",";
			any = true;
		}
	}
	if (any) result = result.Left(result.Length()-1);

	if (parentheses) result += L")";
	return result;
}
