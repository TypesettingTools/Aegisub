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
#include <map>
#include "variable_data.h"


//////////////////////
// Modification types
enum ModType {
	MOD_OFF = -1,
	MOD_AUTOMATIC,
	MOD_RESTART
};


/////////////////////////////
// Class that stores options
class OptionsManager {
private:
	ModType curModType;
	bool modified;
	wxString filename;
	std::map<wxString,VariableData> opt;
	std::map<wxString,ModType> optType;

	void SetModificationType(ModType type);

public:
	OptionsManager();
	~OptionsManager();

	void SetFile(wxString file);
	void Save();
	void Load();
	void LoadDefaults();
	void AddToRecentList (wxString entry,wxString list);
	wxArrayString GetRecentList (wxString list);

	void SetInt(wxString key,int param);
	void SetFloat(wxString key,double param);
	void SetBool(wxString key,bool param);
	void SetText(wxString key,wxString param);
	void SetColour(wxString key,wxColour param);
	void ResetWith(wxString key,wxString param);

	bool IsDefined(wxString key);
	int AsInt(wxString key);
	double AsFloat(wxString key);
	bool AsBool(wxString key);
	wxString AsText(wxString key);
	wxColour AsColour(wxString key);
	ModType GetModType(wxString key);
};


///////////////////
// Global instance
extern OptionsManager Options;
