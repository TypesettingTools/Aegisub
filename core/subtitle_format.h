// Copyright (c) 2006, Rodrigo Braz Monteiro
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
#include <list>


//////////////
// Prototypes
class AssFile;
class AssEntry;


///////////////////
// Subtitle reader
class SubtitleFormat {
private:
	bool isCopy;
	AssFile *assFile;

	void Register();
	void Remove();

	static std::list<SubtitleFormat*> formats;
	static bool loaded;

protected:
	std::list<AssEntry*> *Line;
	void CreateCopy();
	void ClearCopy();

	void Clear();
	void LoadDefault();
	AssFile *GetAssFile() { return assFile; }
	int AddLine(wxString data,wxString group,int lasttime,bool &IsSSA,wxString *outgroup=NULL);

	virtual wxString GetName()=0;
	virtual wxArrayString GetReadWildcards();
	virtual wxArrayString GetWriteWildcards();

public:
	SubtitleFormat();
	virtual ~SubtitleFormat();
	void SetTarget(AssFile *file);

	static wxString GetWildcards(int mode);

	virtual bool CanReadFile(wxString filename) { return false; };
	virtual bool CanWriteFile(wxString filename) { return false; };
	virtual void ReadFile(wxString filename,wxString forceEncoding=_T("")) { };
	virtual void WriteFile(wxString filename,wxString encoding=_T("")) { };

	static SubtitleFormat *GetReader(wxString filename);
	static SubtitleFormat *GetWriter(wxString filename);
	static void LoadFormats();
	static void DestroyFormats();
};
