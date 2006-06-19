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
#include <wx/notebook.h>
#include "dialog_options.h"


///////////////
// Constructor
DialogOptions::DialogOptions(wxWindow *parent)
: wxDialog(parent, -1, _T("Options"), wxDefaultPosition, wxSize(400,300))
{
	// List book
	book = new wxNotebook(this,-1,wxDefaultPosition,wxSize(300,200));
	wxWindow *page1 = new wxTextCtrl(book,-1,_T(""),wxDefaultPosition,wxSize(100,50));
	wxWindow *page2 = new wxPanel(book,-1,wxDefaultPosition,wxSize(100,50));
	wxSizer *testSizer = new wxBoxSizer(wxVERTICAL);
	wxWindow *testWindow = new wxPanel(book,-1);
	testSizer->Add(new wxTextCtrl(testWindow,-1,_T(""),wxDefaultPosition,wxSize(100,50)),1,wxEXPAND | wxALL,5);
	testSizer->Add(new wxButton(testWindow,wxID_OK),0,wxTOP,5);
	testWindow->SetSizer(testSizer);
	wxWindow *page3 = testWindow;
	book->AddPage(page1,_T("Test"),true);
	book->AddPage(page2,_T("Test2"),true);
	book->AddPage(page3,_T("Why, hello there"),true);

	// Sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(book,1,wxEXPAND | wxALL,5);
	//mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);
}


//////////////
// Destructor
DialogOptions::~DialogOptions() {
}
