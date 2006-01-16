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
#include "variable_data.h"
#include "ass_dialogue.h"
#include "ass_style.h"
#include "utils.h"


///////////////
// Constructor
VariableData::VariableData () {
	type = VARDATA_NONE;
	value = NULL;
}


//////////////
// Destructor
VariableData::~VariableData () {
	DeleteValue ();
}


////////////////////////////
// Deletes the stored value
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
	}
	type = VARDATA_NONE;
	value = NULL;
}


//////////////////////
// Sets to an integer
void VariableData::SetInt(int param) {
	DeleteValue();
	type = VARDATA_INT;
	value_int = new int(param);
}


///////////////////
// Sets to a float
void VariableData::SetFloat(double param) {
	DeleteValue();
	type = VARDATA_FLOAT;
	value_float = new double(param);
}


/////////////////////
// Sets to a boolean
void VariableData::SetBool(bool param) {
	DeleteValue();
	type = VARDATA_BOOL;
	value_bool = new bool(param);
}


////////////////////
// Sets to a string
void VariableData::SetText(wxString param) {
	DeleteValue();
	type = VARDATA_TEXT;
	value_text = new wxString (param);
}


////////////////////
// Sets to a colour
void VariableData::SetColour(wxColour param) {
	DeleteValue();
	type = VARDATA_COLOUR;
	value_colour = new wxColour (param);
}


////////////////////
// Sets to a block
void VariableData::SetBlock(AssDialogueBlockOverride *param) {
	DeleteValue();
	type = VARDATA_BLOCK;
	value_block = param;
}


/////////////////////////////////////////////////////////
// Resets a value with a string, preserving current type
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


///////////////////
// Reads as an int
int VariableData::AsInt() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_INT) throw _T("Wrong parameter type, should be int");
	return *value_int;
}


////////////////////
// Reads as a float
double VariableData::AsFloat() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_FLOAT) throw _T("Wrong parameter type, should be float");
	return *value_float;
}


///////////////////
// Reads as a bool
bool VariableData::AsBool() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_BOOL) return *value_bool;
	else if (type == VARDATA_INT) return ((*value_int)!=0);
	else throw _T("Wrong parameter type, should be bool");
}


/////////////////////
// Reads as a colour
wxColour VariableData::AsColour() const {
	if (!value) throw _T("Null parameter");
	if (type == VARDATA_COLOUR)	return *value_colour;
	else if (type == VARDATA_TEXT) {
		AssColor color;
		color.ParseASS(*value_text);
		return color.GetWXColor();
	}
	else throw _T("Wrong parameter type, should be colour");
}


////////////////////
// Reads as a block
AssDialogueBlockOverride *VariableData::AsBlock() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_BLOCK) throw _T("Wrong parameter type, should be block");
	return value_block;
}


/////////////////////
// Reads as a string
wxString VariableData::AsText() const {
	if (!value) throw _T("Null parameter");
	if (type != VARDATA_TEXT) {
		if (type == VARDATA_INT) return wxString::Format(_T("%i"),*value_int);
		else if (type == VARDATA_FLOAT) return PrettyFloat(wxString::Format(_T("%f"),*value_float));
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


/////////////
// Gets type
VariableDataType VariableData::GetType() const {
	return type;
}


////////
// Copy
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
