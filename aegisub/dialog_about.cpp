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
#include <wx/statline.h>
#include "dialog_about.h"
#include "version.h"
#include "options.h"
#include "setup.h"


///////////////
// Constructor
AboutScreen::AboutScreen(wxWindow *parent)
: wxDialog (parent, -1, _("About Aegisub"), wxDefaultPosition, wxSize(300,240), wxSTAY_ON_TOP | wxCAPTION | wxCLOSE_BOX , _("About Aegisub"))
{
	// Get splash
	wxBitmap splash = wxBITMAP(splash);
	SetBackgroundColour(wxColour(255,255,255));
	SetForegroundColour(wxColour(0,0,0));

	// Picture
	wxSizer *PicSizer = new wxBoxSizer(wxHORIZONTAL);
	PicSizer->Add(new BitmapControl(this,splash));

	// Generate library string
	wxString libString = _("This build of Aegisub uses the following C/C++ libraries:\n");
	libString += _T("wxWidgets - Copyright (c) 1998-2006 Julian Smart, Robert Roebling et al;\n");
	libString += _T("wxScintilla - Copyright (c) 2004 wxCode;\n");
	libString += _T("Lua - Copyright (c) 1994-2006 Lua.org, PUC-Rio;\n");
#if USE_HUNSPELL == 1
	libString += _T("Hunspell - Copyright (c) Kevin Hendricks;\n");
#endif
#if USE_PORTAUDIO == 1
	libString += _T("PortAudio - Copyright (c) 1999-2000 Ross Bencina, Phil Burk;\n");
#endif
#if USE_LAVC == 1
	libString += _T("FFmpeg - Copyright (c) ??;\n");
#endif
#if USE_LIBASS == 1
	libString += _T("libass - Copyright (c) 2006, Evgeniy Stepanov;\n");
#endif
#if USE_ASA == 1
	libString += _T("asa - Copyright (c) 2004-2007, David Lamparter;\n");
#endif
	libString += _T("MyThes - Copyright (c) 2003 Kevin B. Hendricks, Stratford, Ontario, Canada\n");

	// Generate about string
	wxString aboutString;
	wxString translatorCredit = _("Translated into LANGUAGE by PERSON\n");
	if (translatorCredit == _T("Translated into LANGUAGE by PERSON\n")) translatorCredit.Clear();
	aboutString += wxString(_T("Aegisub ")) + GetAegisubShortVersionString() + _(" by ArchMage ZeratuL.\n");
	aboutString += _T("Copyright (c) 2005-2007 - Rodrigo Braz Monteiro.\n\n");
	aboutString += _T("Automation - Copyright (c) 2005-2007 Niels Martin Hansen (aka jfs).\n");
	aboutString += _T("Motion Tracker - Copyright (c) 2006 Hajo Krabbenhoeft (aka Tentacle).\n");
	aboutString += _("Programmers: ");
	aboutString += _T(" ArchMageZeratuL, jfs, Myrsloik, equinox, Tentacle, Yuvi,\n     Azzy, Pomyk, Motoko-chan.\n");
	aboutString += _("Manual by: ");
	aboutString += _T("ArchMage ZeratuL, jfs, movax, Kobi, TheFluff, Jcubed.\n");
	aboutString += _("Forum, wiki and bug tracker hosting by: ");
	aboutString += _T("Bot1.\n");
	aboutString += _("SVN hosting by BerliOS and Mentar.\n");
	aboutString += translatorCredit;
	aboutString += _T("\n") + libString;
	aboutString += _("\nSee the help file for full credits.\n");
	aboutString += wxString::Format(_("Built by %s on %s."), GetAegisubBuildCredit().c_str(), GetAegisubBuildTime().c_str());

	// Replace copyright symbol
	wxChar copySymbol = 0xA9;
	aboutString.Replace(_T("(c)"),wxString(copySymbol));

	// Text sizer
	wxSizer *TextSizer = new wxBoxSizer(wxVERTICAL);
	TextSizer->Add(new wxStaticText(this,-1,aboutString),1);

	// Button sizer
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifndef __WXMAC__
	ButtonSizer->Add(new wxButton(this,wxID_OK),0,0,0);
#else
	wxButton *okButton = new wxButton(this,wxID_OK);
	ButtonSizer->Add(okButton,0,0,0);
	okButton->SetDefault();
#endif

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
