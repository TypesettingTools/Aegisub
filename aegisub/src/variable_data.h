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

/// @file variable_data.h
/// @see variable_data.cpp
/// @ingroup utility subs_storage
///

#pragma once

namespace agi { struct Color; }
class wxString;

enum VariableDataType {
	VARDATA_NONE,
	VARDATA_INT,
	VARDATA_FLOAT,
	VARDATA_TEXT,
	VARDATA_BOOL,
	VARDATA_COLOUR,
	VARDATA_BLOCK
};

class AssDialogueBlockOverride;

class VariableData {
private:
	union {
		void *value;
		int *value_int;
		double *value_float;
		bool *value_bool;
		wxString *value_text;
		agi::Color *value_colour;
		AssDialogueBlockOverride **value_block;
	};

	VariableDataType type;

protected:
	void DeleteValue();

public:
	VariableData();
	virtual ~VariableData();

	VariableDataType GetType() const;
	template<class T> void Set(T param);
	void ResetWith(wxString value);
	template<class T> T Get() const;
	template<class T> T Get(T def) const {
		return value ? Get<T>() : def;
	}

	void operator= (const VariableData &param);
};
