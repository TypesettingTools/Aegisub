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

/// @file ass_override.h
/// @see ass_override.cpp
/// @ingroup subs_storage
///




///////////
// Headers
#ifndef AGI_PRE
#include <list>
#include <vector>
#endif

#include "variable_data.h"


//////////////
// Prototypes
class AssDialogueBlockOverride;



/// DOCME
enum ASS_ParameterClass {

	/// DOCME
	PARCLASS_NORMAL,

	/// DOCME
	PARCLASS_ABSOLUTE_SIZE,

	/// DOCME
	PARCLASS_ABSOLUTE_POS_X,

	/// DOCME
	PARCLASS_ABSOLUTE_POS_Y,

	/// DOCME
	PARCLASS_RELATIVE_SIZE_X,

	/// DOCME
	PARCLASS_RELATIVE_SIZE_Y,

	/// DOCME
	PARCLASS_RELATIVE_TIME_START,

	/// DOCME
	PARCLASS_RELATIVE_TIME_END,

	/// DOCME
	PARCLASS_KARAOKE,

	/// DOCME
	PARCLASS_DRAWING
};


/// DOCME
/// @class AssOverrideParameter
/// @brief DOCME
///
/// DOCME
class AssOverrideParameter : public VariableData {
public:

	/// DOCME
	ASS_ParameterClass classification;

	/// DOCME
	bool ommited;

	AssOverrideParameter();
	~AssOverrideParameter();

	void operator= (const AssOverrideParameter &param);
	void CopyFrom (const AssOverrideParameter &param);
};



/// DOCME
/// @class AssOverrideTag
/// @brief DOCME
///
/// DOCME
class AssOverrideTag {
private:

	/// DOCME
	bool valid;

public:

	/// DOCME
	wxString Name;

	/// DOCME
	std::vector <AssOverrideParameter*> Params;

	AssOverrideTag();
	~AssOverrideTag();

	bool IsValid();
	void ParseParameters(const wxString &text);
	void Clear();
	void SetText(const wxString &text);
	wxString ToString();
};



/// DOCME
enum ASS_ParameterOptional {

	/// DOCME
	NOT_OPTIONAL = 0xFF,

	/// DOCME
	OPTIONAL_0 = 0x00,

	/// DOCME
	OPTIONAL_1 = 0x01,

	/// DOCME
	OPTIONAL_2 = 0x02,

	/// DOCME
	OPTIONAL_3 = 0x04,

	/// DOCME
	OPTIONAL_4 = 0x08,

	/// DOCME
	OPTIONAL_5 = 0x10,

	/// DOCME
	OPTIONAL_6 = 0x20,

	/// DOCME
	OPTIONAL_7 = 0x40
};


/// DOCME
/// @class AssOverrideParamProto
/// @brief DOCME
///
/// DOCME
class AssOverrideParamProto {
public:

	/// DOCME
	int optional;

	/// DOCME
	VariableDataType type;

	/// DOCME
	AssOverrideParameter defaultValue;

	/// DOCME
	ASS_ParameterClass classification;

	AssOverrideParamProto (VariableDataType _type,int opt=NOT_OPTIONAL,ASS_ParameterClass classi=PARCLASS_NORMAL);
	~AssOverrideParamProto();
};


/// DOCME
/// @class AssOverrideTagProto
/// @brief DOCME
///
/// DOCME
class AssOverrideTagProto {
public:

	/// DOCME
	wxString name;

	/// DOCME
	std::vector<AssOverrideParamProto> params;

	/// DOCME
	static std::list<AssOverrideTagProto> proto;

	/// DOCME
	static bool loaded;
	static void LoadProtos();

	AssOverrideTagProto();
	~AssOverrideTagProto();
};


