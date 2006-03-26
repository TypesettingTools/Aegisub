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
#include "about.h"
#include "version.h"
#include "options.h"
#include <wx/statline.h>


///////////////
// Constructor
AboutScreen::AboutScreen(wxWindow *parent,bool easter)
: wxDialog (parent, -1, _("About Aegisub"), wxDefaultPosition, wxSize(300,240), wxSTAY_ON_TOP | wxCAPTION | wxCLOSE_BOX , _("About Aegisub"))
{
	// Get splash
	wxBitmap splash;
	int splash_n = Options.AsInt(_T("Splash number"));
	if (splash_n < 1 || splash_n > 5) splash_n = (rand()%5)+1;
	if (splash_n == 1) splash = wxBITMAP(splash_01);
	if (splash_n == 2) splash = wxBITMAP(splash_02);
	if (splash_n == 3) splash = wxBITMAP(splash_03);
	if (splash_n == 4) splash = wxBITMAP(splash_04);
	if (splash_n == 5) splash = wxBITMAP(splash_05);

	// Picture
	wxSizer *PicSizer = new wxBoxSizer(wxHORIZONTAL);
	PicSizer->Add(new BitmapControl(this,splash));

	// Generate about string
	wxString aboutString;
	wxString translatorCredit = _("Translated into LANGUAGE by PERSON\n");
	if (translatorCredit == _T("Translated into LANGUAGE by PERSON\n")) translatorCredit.Clear();
	aboutString += wxString(_T("Aegisub ")) + GetAegisubVersionString() + _(" by ArchMage ZeratuL.\n");
	aboutString += _("Copyright (c) 2005-2006 - Rodrigo Braz Monteiro.\n\n");
	aboutString += _("Automation module is Copyright (c) 2005-2006 Niels Martin Hansen (aka jfs).\n");
	aboutString += _("Motion tracker module is Copyright (c) 2006 Hajo Krabbenhoeft (aka Tentacle).\n");
	aboutString += _("Coding by ArchMageZeratuL, jfs, Myrsloik, Tentacle and nmap.\n");
	aboutString += _("Manual by ArchMage ZeratuL, jfs, movax, Kobi, TheFluff and Jcubed.\n");
	aboutString += _("Forum and bug tracker hosting by Bot1.\n");
	aboutString += translatorCredit;
	aboutString += _("\nSee the help file for full credits.");

	// Text sizer
	wxSizer *TextSizer = new wxBoxSizer(wxVERTICAL);
	TextSizer->Add(new wxStaticText(this,-1,aboutString),1);

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
	ButtonSizer->Add(new wxButton(this,wxID_OK),0,0,0);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(PicSizer,0,wxBOTTOM,5);
	MainSizer->Add(TextSizer,0,wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT,5);
	MainSizer->Add(new wxStaticLine(this,wxID_ANY),0,wxEXPAND | wxALL,5);
	MainSizer->Add(ButtonSizer,0,wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT,5);

	// Set sizer
	MainSizer->SetSizeHints(this);
	SetSizer(MainSizer);

	// Draw logo
	Centre();
}


//////////////
// Destructor
AboutScreen::~AboutScreen () {
}
