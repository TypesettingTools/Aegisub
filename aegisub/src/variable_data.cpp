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

/// @file variable_data.cpp
/// @brief A variant-type implementation
/// @ingroup utility subs_storage

#include "config.h"

#include "variable_data.h"

#include "ass_dialogue.h"
#include "ass_style.h"
#include "compat.h"
#include "utils.h"

#include <libaegisub/color.h>

VariableData::VariableData() {
	type = VARDATA_NONE;
	value = nullptr;
}

VariableData::VariableData(VariableData&& o)
: value(o.value)
, type(o.type)
{
}

VariableData::~VariableData() {
	DeleteValue();
}

void VariableData::DeleteValue() {
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
	value = nullptr;
}

template<class T> static inline VariableDataType get_type();

template<> inline VariableDataType get_type<int>() {
	return VARDATA_INT;
}
template<> inline VariableDataType get_type<float>() {
	return VARDATA_FLOAT;
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
template<> inline VariableDataType get_type<agi::Color>() {
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
template void VariableData::Set<float>(float param);
template void VariableData::Set<double>(double param);
template void VariableData::Set<bool>(bool param);
template void VariableData::Set(wxString param);
template void VariableData::Set<agi::Color>(agi::Color param);
template void VariableData::Set<AssDialogueBlockOverride *>(AssDialogueBlockOverride * param);

template<> int VariableData::Get<int>() const {
	if (!value) throw "Null parameter";
	if (type == VARDATA_BOOL) return !!(*value_bool);
	if (type == VARDATA_INT) return *value_int;
	if (type == VARDATA_FLOAT) return (int)(*value_float);
	if (type == VARDATA_TEXT) return 0;
	throw "Wrong parameter type, should be int";
}

template<> float VariableData::Get<float>() const {
	if (!value) throw "Null parameter";
	if (type == VARDATA_FLOAT) return (float)*value_float;
	if (type == VARDATA_INT) return (float)(*value_int);
	if (type == VARDATA_TEXT) return 0.0f;
	throw "Wrong parameter type, should be float";
}
template<> double VariableData::Get<double>() const {
	if (!value) throw "Null parameter";
	if (type == VARDATA_FLOAT) return *value_float;
	if (type == VARDATA_INT) return (float)(*value_int);
	if (type == VARDATA_TEXT) return 0.0;
	throw "Wrong parameter type, should be float";
}

template<> bool VariableData::Get<bool>() const {
	if (!value) throw "Null parameter";
	if (type == VARDATA_BOOL) return *value_bool;
	if (type == VARDATA_INT) return ((*value_int)!=0);
	if (type == VARDATA_FLOAT) return ((*value_float)!=0);
	if (type == VARDATA_TEXT) return false;
	throw "Wrong parameter type, should be bool";
}

template<> agi::Color VariableData::Get<agi::Color>() const {
	if (!value) throw "Null parameter";
	if (type == VARDATA_COLOUR) return *value_colour;
	else if (type == VARDATA_TEXT) {
		return agi::Color(from_wx(*value_text));
	}
	else throw "Wrong parameter type, should be colour";
}

template<> AssDialogueBlockOverride *VariableData::Get<AssDialogueBlockOverride *>() const {
	if (!value) throw "Null parameter";
	if (type != VARDATA_BLOCK) throw "Wrong parameter type, should be block";
	return *value_block;
}

template<> wxString VariableData::Get<wxString>() const {
	if (!value) throw "Null parameter";
	if (type != VARDATA_TEXT) {
		if (type == VARDATA_INT) return wxString::Format("%i",*value_int);
		if (type == VARDATA_FLOAT) return wxString::Format("%g",*value_float);
		if (type == VARDATA_COLOUR) return to_wx(value_colour->GetHexFormatted());
		if (type == VARDATA_BOOL) return *value_bool ? "1" : "0";
		if (type == VARDATA_BLOCK) {
			wxString str((*value_block)->GetText());
			str.Replace("{", "");
			str.Replace("}", "");
			return str;
		}
		throw "Wrong parameter type, should be text";
	}
	return *value_text;
}
