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
#include <wx/wxprec.h>
#include <wx/colour.h>


/////////////////
// Enum of types
enum VariableDataType {
	VARDATA_NONE,
	VARDATA_INT,
	VARDATA_FLOAT,
	VARDATA_TEXT,
	VARDATA_BOOL,
	VARDATA_COLOUR,
	VARDATA_BLOCK
};


//////////////
// Prototypes
class AssDialogueBlockOverride;


//////////////////////////////////////
// Class to store variable data types
class VariableData {
private:
	union {
		void *value;
		int *value_int;
		double *value_float;
		bool *value_bool;
		wxString *value_text;
		wxColour *value_colour;
		AssDialogueBlockOverride *value_block;
	};
	VariableDataType type;

protected:
	void DeleteValue();

public:
	VariableData();
	virtual ~VariableData();

	VariableDataType GetType() const;

	void SetInt(int param);
	void SetFloat(double param);
	void SetBool(bool param);
	void SetText(wxString param);
	void SetColour(wxColour param);
	void SetBlock(AssDialogueBlockOverride *param);
	void ResetWith(wxString value);

	int AsInt() const;
	double AsFloat() const;
	bool AsBool() const;
	wxString AsText() const;
	wxColour AsColour() const;
	AssDialogueBlockOverride *AsBlock() const;

	void operator= (const VariableData &param);
};
