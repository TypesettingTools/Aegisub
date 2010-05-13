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
///


////////////
// Includes
#include "config.h"

#include "ass_dialogue.h"
#include "ass_style.h"
#include "utils.h"
#include "variable_data.h"


/// @brief Constructor 
///
VariableData::VariableData () {
	type = VARDATA_NONE;
	value = NULL;
}



/// @brief Destructor 
///
VariableData::~VariableData () {
	DeleteValue ();
}



/// @brief Deletes the stored value 
/// @return 
///
void VariableData::DeleteValue () {
	if (!value) return;
	if (type == VARDATA_NONE) return;
	switch (type) {
		case VARDATA_INT: delete value_int; break;
		case VARDATA_FLOAT: delete value_float; break;
		case VARDATA_TEXT: delete value_text; break;
		case VARDATA_BOOL: delete value_bool; break;
		case VARDATA_COLOUR: delete value_colour; break;
		case VARDATA_BLOCK: delete value_block; break;
		default: break;
	}
	type = VARDATA_NONE;
	value = NULL;
}



/// @brief Sets to an integer 
/// @param param 
///
void VariableData::SetInt(int param) {
	DeleteValue();
	type = VARDATA_INT;
	value_int = new int(param);
}



/// @brief Sets to a float 
/// @param param 
///
void VariableData::SetFloat(double param) {
	DeleteValue();
	type = VARDATA_FLOAT;
	value_float = new double(param);
}



/// @brief Sets to a boolean 
/// @param param 
///
void VariableData::SetBool(bool param) {
	DeleteValue();
	type = VARDATA_BOOL;
	value_bool = new bool(param);
}



/// @brief Sets to a string 
/// @param param 
///
void VariableData::SetText(wxString param) {
	DeleteValue();
	type = VARDATA_TEXT;
	value_text = new wxString (param);
}



/// @brief Sets to a colour 
/// @param param 
///
void VariableData::SetColour(wxColour param) {
	DeleteValue();
	type = VARDATA_COLOUR;
	value_colour = new wxColour (param);
}



/// @brief Sets to a block 
/// @param param 
///
void VariableData::SetBlock(AssDialogueBlockOverride *param) {
	DeleteValue();
	type = VARDATA_BLOCK;
	value_block = param;
}



/// @brief Resets a value with a string, preserving current type 
/// @param value 
///
void VariableData::ResetWith(wxString value) {
	switch (type) {
		case VARDATA_INT: {
			long temp = 0;
			value.ToLong(&temp);
			SetInt(temp);
			break;
		}
		case VARDATA_FLOAT: {
			double temp = 0;
			value.ToDouble(&temp);
			SetFloat(temp);
			break;
		}
		case VARDATA_BOOL:
			if (value == _T("1")) SetBool(true);
			else SetBool(false);
			break;
		case VARDATA_COLOUR: {
			long r=0,g=0,b=0;
			value.Mid(1,2).ToLong(&r,16);
			value.Mid(3,2).ToLong(&g,16);
			value.Mid(5,2).ToLong(&b,16);
			SetColour(wxColour(r,g,b));
			break;
		}
		default:
			SetText(value);
			break;
	}
}



/// @brief Reads as an int 
/// @return 
///
int VariableData::AsInt() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_BOOL) return (*value_bool)?1:0;
	if (type == VARDATA_INT) return *value_int;
	if (type == VARDATA_FLOAT) return (int)(*value_float);
	if (type == VARDATA_TEXT) return 0;
	throw _T("Wrong parameter type, should be int");
}



/// @brief Reads as a float 
/// @return 
///
double VariableData::AsFloat() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_FLOAT) return *value_float;
	if (type == VARDATA_INT) return (float)(*value_int);
	if (type == VARDATA_TEXT) return 0.0f;
	throw _T("Wrong parameter type, should be float");
}



/// @brief Reads as a bool 
/// @return 
///
bool VariableData::AsBool() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_BOOL) return *value_bool;
	if (type == VARDATA_INT) return ((*value_int)!=0);
	if (type == VARDATA_FLOAT) return ((*value_float)!=0);
	if (type == VARDATA_TEXT) return false;
	throw _T("Wrong parameter type, should be bool");
}



/// @brief Reads as a colour 
/// @return 
///
wxColour VariableData::AsColour() const {
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
///
AssDialogueBlockOverride *VariableData::AsBlock() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_BLOCK) throw _T("Wrong parameter type, should be block");
	return value_block;
}



/// @brief Reads as a string 
/// @return 
///
wxString VariableData::AsText() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_TEXT) {
		if (type == VARDATA_INT) return wxString::Format(_T("%i"),*value_int);
		else if (type == VARDATA_FLOAT) return wxString::Format(_T("%g"),*value_float);
		else if (type == VARDATA_COLOUR) return wxString::Format(_T("#%02X%02X%02X"),value_colour->Red(),value_colour->Green(),value_colour->Blue());
		else if (type == VARDATA_BOOL) {
			if (*value_bool) return _T("1");
			else return _T("0");
		}
		else if (type == VARDATA_BLOCK) return value_block->GetText();
		else throw _T("Wrong parameter type, should be text");
	}
	return *value_text;
}



/// @brief Gets type 
/// @return 
///
VariableDataType VariableData::GetType() const {
	return type;
}



/// @brief Copy 
/// @param param 
///
void VariableData::operator= (const VariableData &param) {
	switch(param.GetType()) {
		case VARDATA_INT: SetInt(param.AsInt()); break;
		case VARDATA_FLOAT: SetFloat(param.AsFloat()); break;
		case VARDATA_TEXT: SetText(param.AsText()); break;
		case VARDATA_BOOL: SetBool(param.AsBool()); break;
		case VARDATA_COLOUR: SetColour(param.AsColour()); break;
		case VARDATA_BLOCK: SetBlock(param.AsBlock()); break;
		default: DeleteValue();
	}
}


