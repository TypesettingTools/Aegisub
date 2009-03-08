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


#pragma once


///////////
// Headers
#include <list>
#include <vector>
#include "variable_data.h"


//////////////
// Prototypes
class AssDialogueBlockOverride;


// Enum of parameter classification
// This is used for things like checking which tags VFR needs to update
enum ASS_ParameterClass {
	PARCLASS_NORMAL,
	PARCLASS_ABSOLUTE_SIZE,
	PARCLASS_ABSOLUTE_POS_X,
	PARCLASS_ABSOLUTE_POS_Y,
	PARCLASS_RELATIVE_SIZE_X,
	PARCLASS_RELATIVE_SIZE_Y,
	PARCLASS_RELATIVE_TIME_START,
	PARCLASS_RELATIVE_TIME_END,
	//PARCLASS_RELATIVE_TIME_START_CENTI,
	//PARCLASS_RELATIVE_TIME_END_CENTI,
	PARCLASS_KARAOKE,
	PARCLASS_DRAWING
};

// Actual class
class AssOverrideParameter : public VariableData {
public:
	ASS_ParameterClass classification;
	bool ommited;

	AssOverrideParameter();
	~AssOverrideParameter();

	void operator= (const AssOverrideParameter &param);
	void CopyFrom (const AssOverrideParameter &param);
};


///////////////////////////
// Class for override tags
// -----------------------
//
// GetText() returns the text representation of the tag
//
class AssOverrideTag {
private:
	bool valid;

public:
	wxString Name;
	std::vector <AssOverrideParameter*> Params;

	AssOverrideTag();
	~AssOverrideTag();

	bool IsValid();
	void ParseParameters(const wxString &text);
	void Clear();
	void SetText(const wxString &text);
	wxString ToString();
};


///////////////////////////
// Override tags prototype
enum ASS_ParameterOptional {
	NOT_OPTIONAL = 0xFF,
	OPTIONAL_0 = 0x00,
	OPTIONAL_1 = 0x01,
	OPTIONAL_2 = 0x02,
	OPTIONAL_3 = 0x04,
	OPTIONAL_4 = 0x08,
	OPTIONAL_5 = 0x10,
	OPTIONAL_6 = 0x20,
	OPTIONAL_7 = 0x40
};

class AssOverrideParamProto {
public:
	int optional;
	VariableDataType type;
	AssOverrideParameter defaultValue;
	ASS_ParameterClass classification;

	AssOverrideParamProto (VariableDataType _type,int opt=NOT_OPTIONAL,ASS_ParameterClass classi=PARCLASS_NORMAL);
	~AssOverrideParamProto();
};

class AssOverrideTagProto {
public:
	wxString name;
	std::vector<AssOverrideParamProto> params;
	static std::list<AssOverrideTagProto> proto;
	static bool loaded;
	static void LoadProtos();

	AssOverrideTagProto();
	~AssOverrideTagProto();
};
