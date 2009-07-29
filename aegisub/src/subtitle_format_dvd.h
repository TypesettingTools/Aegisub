// Copyright (c) 2008, Rodrigo Braz Monteiro
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

/// @file subtitle_format_dvd.h
/// @see subtitle_format_dvd.cpp
/// @ingroup subtitle_io vobsub
///


#pragma once


///////////
// Headers
#include "subtitle_format.h"


//////////////////
// Helper classes
struct SubPicture {
	//wxImage img;
	std::vector<unsigned char> data[2];
	int x,y;
	int w,h;
	int start,end;
};

struct RLEGroup {
	int col;
	int len;
	bool eol;
	RLEGroup(int _col,int _len,bool _eol) { col = _col; len = _len; eol = _eol; }
};


//////////////////////////
// DVD subpictures writer
class DVDSubtitleFormat : public SubtitleFormat {
private:
	void GetSubPictureList(std::vector<SubPicture> &pics);

public:
	wxString GetName();
	wxArrayString GetWriteWildcards();
	bool CanWriteFile(wxString filename);
	void WriteFile(wxString filename,wxString encoding);
};

