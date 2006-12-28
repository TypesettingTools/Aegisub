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


///////////
// Headers
#include "dialog_options.h"
#ifdef wxUSE_TREEBOOK
#include <wx/treebook.h>
#endif


///////////////
// Constructor
DialogOptions::DialogOptions(wxWindow *parent)
: wxDialog(parent, -1, _T("Options"), wxDefaultPosition, wxDefaultSize)
{
#ifdef wxUSE_TREEBOOK
	// Create book
	book = new wxTreebook(this,-1,wxDefaultPosition,wxSize(500,300));

	// List book
	book->AddPage(new wxPanel(book,-1),_T("General"),true);
	book->AddSubPage(new wxPanel(book,-1),_T("File Save/Load"),true);
	book->AddSubPage(new wxPanel(book,-1),_T("Subtitles Grid"),true);
	book->AddSubPage(new wxPanel(book,-1),_T("Subtitles Edit Box"),true);
	book->AddPage(new wxPanel(book,-1),_T("Video"),true);
	book->AddPage(new wxPanel(book,-1),_T("Audio"),true);
	book->AddSubPage(new wxPanel(book,-1),_T("Display"),true);
	book->AddPage(new wxPanel(book,-1),_T("Automation"),true);

	// Buttons Sizer
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(new wxButton(this,wxID_OK),0,wxRIGHT,5);
	buttonSizer->Add(new wxButton(this,wxID_CANCEL),0,wxRIGHT,5);

	// Main Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book,1,wxEXPAND | wxALL,5);
	mainSizer->Add(buttonSizer,0,wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM,5);
	SetSizer(mainSizer);
#endif
}


//////////////
// Destructor
DialogOptions::~DialogOptions() {
}
