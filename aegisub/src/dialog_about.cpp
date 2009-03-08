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
#include "config.h"

#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include "dialog_about.h"
#include "version.h"
#include "options.h"


///////////////
// Constructor
AboutScreen::AboutScreen(wxWindow *parent)
: wxDialog (parent, -1, _("About Aegisub"), wxDefaultPosition, wxSize(300,240), wxCAPTION | wxCLOSE_BOX , _("About Aegisub"))
{
	// Get splash
	wxBitmap splash = wxBITMAP(splash);
	SetOwnBackgroundColour(wxColour(255,255,255));
	SetForegroundColour(wxColour(0,0,0));

	// Picture
	wxSizer *PicSizer = new wxBoxSizer(wxHORIZONTAL);
	PicSizer->Add(new BitmapControl(this,splash));

	// Generate library string
	wxString libString = _("This build of Aegisub uses the following C/C++ libraries:\n");
	libString += _T("    wxWidgets - Copyright (c) 1998-2008 Julian Smart, Robert Roebling et al;\n");
	libString += _T("    wxStyledTextCtrl - Copyright (c) 2004 wxCode;\n");
#ifdef WITH_AUTOMATION
	libString += _T("    Lua - Copyright (c) 1994-2008 Lua.org, PUC-Rio;\n");
#endif
#ifdef WITH_PERL
	libString += _T("    Perl - Copyright (c) 1987-2007 Larry Wall et al;\n");
#endif
#ifdef WITH_RUBY
	// Insert ruby string here
#endif
#ifdef WITH_HUNSPELL
	libString += _T("    Hunspell - Copyright (c) Kevin Hendricks;\n");
#endif
#ifdef WITH_PORTAUDIO
	libString += _T("    PortAudio - Copyright (c) 1999-2000 Ross Bencina, Phil Burk;\n");
#endif
#ifdef WITH_FFMPEG
#define _FFMPEG_ANY
#endif
#ifdef WITH_FFMPEGSOURCE
#define _FFMPEG_ANY
#endif
#ifdef _FFMPEG_ANY
	libString += _T("    FFmpeg - Copyright (c) 2001 Fabrice Bellard,;\n");
#endif
#ifdef WITH_AVISYNTH
	libString += _T("    Avisynth 2.5 - Copyright 2002 Ben Rudiak-Gould et al;\n");
#endif
#ifdef WITH_CSRI
	libString += _T("    csri - Copyright (c) 2004-2008 David Lamparter;\n");
#endif
#ifdef __WINDOWS__
	libString += _T("    vsfilter - Copyright (c) Gabest;\n");
#endif
#ifdef WITH_LIBASS
	libString += _T("    libass - Copyright (c) 2006-2008 Evgeniy Stepanov;\n");
#endif
#ifdef WITH_UNIVCHARDET
	libString += _T("    UniversalCharDet - Copyright (c) 1998 Netscape Communications Corp.;\n");
#endif
#ifdef __WINDOWS__
	libString += _T("    Matroska Parser and VideoSink - Copyright (c) 2004-2008 Mike Matsnev;\n");
#endif
#ifdef WITH_FREETYPE2
	libString += _T("    Freetype - Copyright (c) 1996-2001, 2006 David Turner, Robert Wilhelm,\n    and Werner Lemberg;\n");
#endif
	libString += _T("    MyThes - Copyright (c) 2003 Kevin B. Hendricks, Stratford, Ontario, Canada.\n");

	// Generate about string
	wxString aboutString;
	wxString translatorCredit = _("Translated into LANGUAGE by PERSON\n");
	if (translatorCredit == _T("Translated into LANGUAGE by PERSON\n")) translatorCredit.Clear();
	aboutString += wxString(_T("Aegisub ")) + GetAegisubShortVersionString() + _T(".\n");
	aboutString += _T("Copyright (c) 2005-2008 Rodrigo Braz Monteiro, Niels Martin Hansen et al.\n\n");
	aboutString += _T("Automation - Copyright (c) 2005-2008 Niels Martin Hansen.\n");
	aboutString += _("Programmers:");
	aboutString += _T(" Rodrigo Braz Monteiro, Niels Martin Hansen, David Lamparter,\n");
	aboutString += _T("    Dan Donovan, Alysson Souza e Silva, Karl Blomster, Simone Cociancich,\n");
	aboutString += _T("    Fredrik Mellbin, Patryk Pomykalski, ai-chan, Evgeniy Stepanov,\n");
	aboutString += _T("    Mike Matsnev, 2points, p-static, David Conrad, Daniel Moscoviter.\n");
	aboutString += _("Manual by:");
	aboutString += _T(" Karl Blomster, Niels Martin Hansen, Rodrigo Braz Monteiro.\n");
	aboutString += _("Forum and wiki hosting by:");
	aboutString += _T(" Sigurd Tao Lyngse.\n");
	aboutString += _("SVN hosting by:");
	aboutString += _T(" DeathWolf, David Lamparter, BerliOS, Mentar.\n");
	aboutString += _("Bug tracker hosting by:");
	aboutString += _T(" Niels Martin Hansen.\n");
	aboutString += translatorCredit;
	aboutString += _T("\n") + libString;
	aboutString += _("\nSee the help file for full credits.\n");
	aboutString += wxString::Format(_("Built by %s on %s."), GetAegisubBuildCredit().c_str(), GetAegisubBuildTime().c_str());

	// Replace copyright symbol
	wxChar copySymbol = 0xA9;
	aboutString.Replace(_T("(c)"),wxString(copySymbol));

	// Text sizer
	wxSizer *TextSizer = new wxBoxSizer(wxVERTICAL);
	//TextSizer->Add(new wxStaticText(this,-1,aboutString),1);
	TextSizer->Add(new wxTextCtrl(this,-1,aboutString,wxDefaultPosition,wxSize(410,200),wxTE_MULTILINE | wxTE_READONLY),1,wxEXPAND);

	// Buttons panel
	wxPanel *buttonPanel = new wxPanel(this,-1,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL);
	wxSizer *ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	ButtonSizer->AddStretchSpacer(1);
#ifndef __APPLE__
	ButtonSizer->Add(new wxButton(buttonPanel,wxID_OK),0,wxALIGN_RIGHT | wxALL,7);
#else
	wxButton *okButton = new wxButton(buttonPanel,wxID_OK);
	ButtonSizer->Add(okButton,0,wxALIGN_RIGHT | wxALL,7);
	okButton->SetDefault();
#endif
	ButtonSizer->SetSizeHints(buttonPanel);
	buttonPanel->SetSizer(ButtonSizer);

	// Main sizer
	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(PicSizer,0,wxCENTER,0);
	MainSizer->Add(TextSizer,0,wxEXPAND | wxALL,0);
	MainSizer->Add(new wxStaticLine(this,wxID_ANY),0,wxEXPAND | wxALL,0);
	MainSizer->Add(buttonPanel,0,wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT,0);

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
