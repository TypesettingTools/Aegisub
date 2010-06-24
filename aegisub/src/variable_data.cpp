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

/// @file variable_data.cpp
/// @brief A variant-type implementation
/// @ingroup utility subs_storage

////////////
// Includes
#include "config.h"

#include "ass_dialogue.h"
#include "ass_style.h"
#include "utils.h"
#include "variable_data.h"

/// @brief Constructor 
VariableData::VariableData() {
	type = VARDATA_NONE;
	value = NULL;
}

/// @brief Destructor 
VariableData::~VariableData() {
	DeleteValue ();
}

/// @brief Deletes the stored value 
/// @return 
void VariableData::DeleteValue () {
	if (!value) return;
	if (type == VARDATA_NONE) return;
	switch (type) {
		case VARDATA_INT: delete value_int; break;
		case VARDATA_FLOAT: delete value_float; break;
		case VARDATA_TEXT: delete value_text; break;
		case VARDATA_BOOL: delete value_bool; break;
		case VARDATA_COLOUR: delete value_colour; break;
		case VARDATA_BLOCK: delete *value_block; delete value_block; break;
		default: break;
	}
	type = VARDATA_NONE;
	value = NULL;
}

template<class T> static inline VariableDataType get_type();

template<> inline VariableDataType get_type<int>() {
	return VARDATA_INT;
}
template<> inline VariableDataType get_type<double>() {
	return VARDATA_FLOAT;
}
template<> inline VariableDataType get_type<bool>() {
	return VARDATA_BOOL;
}
template<> inline VariableDataType get_type<wxString>() {
	return VARDATA_TEXT;
}
template<> inline VariableDataType get_type<wxColour>() {
	return VARDATA_COLOUR;
}
template<> inline VariableDataType get_type<AssDialogueBlockOverride *>() {
	return VARDATA_BLOCK;
}

template<class T>
void VariableData::Set(T param) {
	DeleteValue();
	type = get_type<T>();
	value = new T(param);
}
template void VariableData::Set<int>(int param);
template void VariableData::Set<double>(double param);
template void VariableData::Set<bool>(bool param);
template void VariableData::Set(wxString param);
template void VariableData::Set<wxColour>(wxColour param);
template void VariableData::Set<AssDialogueBlockOverride *>(AssDialogueBlockOverride * param);

/// @brief Resets a value with a string, preserving current type 
/// @param value 
void VariableData::ResetWith(wxString value) {
	switch (type) {
		case VARDATA_INT: {
			long temp = 0;
			value.ToLong(&temp);
			Set<int>(temp);
			break;
		}
		case VARDATA_FLOAT: {
			double temp = 0;
			value.ToDouble(&temp);
			Set(temp);
			break;
		}
		case VARDATA_BOOL:
			if (value == _T("1")) Set(true);
			else Set(false);
			break;
		case VARDATA_COLOUR: {
			long r=0,g=0,b=0;
			value.Mid(1,2).ToLong(&r,16);
			value.Mid(3,2).ToLong(&g,16);
			value.Mid(5,2).ToLong(&b,16);
			Set(wxColour(r,g,b));
			break;
		}
		default:
			Set(value);
			break;
	}
}

/// @brief Reads as an int 
/// @return 
template<> int VariableData::Get<int>() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_BOOL) return !!(*value_bool);
	if (type == VARDATA_INT) return *value_int;
	if (type == VARDATA_FLOAT) return (int)(*value_float);
	if (type == VARDATA_TEXT) return 0;
	throw _T("Wrong parameter type, should be int");
}

/// @brief Reads as a float 
/// @return 
template<> double VariableData::Get<double>() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_FLOAT) return *value_float;
	if (type == VARDATA_INT) return (float)(*value_int);
	if (type == VARDATA_TEXT) return 0.0f;
	throw _T("Wrong parameter type, should be float");
}

/// @brief Reads as a bool 
/// @return 
template<> bool VariableData::Get<bool>() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_BOOL) return *value_bool;
	if (type == VARDATA_INT) return ((*value_int)!=0);
	if (type == VARDATA_FLOAT) return ((*value_float)!=0);
	if (type == VARDATA_TEXT) return false;
	throw _T("Wrong parameter type, should be bool");
}

/// @brief Reads as a colour 
/// @return 
template<> wxColour VariableData::Get<wxColour>() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_COLOUR)	return *value_colour;
	else if (type == VARDATA_TEXT) {
		AssColor color;
		color.Parse(*value_text);
		return color.GetWXColor();
	}
	else throw _T("Wrong parameter type, should be colour");
}

/// @brief Reads as a block 
/// @return 
template<> AssDialogueBlockOverride *VariableData::Get<AssDialogueBlockOverride *>() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_BLOCK) throw _T("Wrong parameter type, should be block");
	return *value_block;
}

/// @brief Reads as a string 
/// @return 
template<> wxString VariableData::Get<wxString>() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_TEXT) {
		if (type == VARDATA_INT) return wxString::Format("%i",*value_int);
		else if (type == VARDATA_FLOAT) return wxString::Format("%g",*value_float);
		else if (type == VARDATA_COLOUR) return wxString::Format("#%02X%02X%02X",value_colour->Red(),value_colour->Green(),value_colour->Blue());
		else if (type == VARDATA_BOOL) return *value_bool ? "1" : "0";
		else if (type == VARDATA_BLOCK) return (*value_block)->GetText();
		else throw _T("Wrong parameter type, should be text");
	}
	return *value_text;
}

/// @brief Gets type 
/// @return 
VariableDataType VariableData::GetType() const {
	return type;
}

/// @brief Copy 
/// @param param 
void VariableData::operator= (const VariableData &param) {
	switch(param.GetType()) {
		case VARDATA_INT: Set(param.Get<int>()); break;
		case VARDATA_FLOAT: Set(param.Get<double>()); break;
		case VARDATA_TEXT: Set(param.Get<wxString>()); break;
		case VARDATA_BOOL: Set(param.Get<bool>()); break;
		case VARDATA_COLOUR: Set(param.Get<wxColor>()); break;
		case VARDATA_BLOCK: Set(param.Get<AssDialogueBlockOverride*>()); break;
		default: DeleteValue();
	}
}
