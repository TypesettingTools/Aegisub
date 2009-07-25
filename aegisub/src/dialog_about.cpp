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
#include "libresrc/libresrc.h"



///////////////
// Constructor
AboutScreen::AboutScreen(wxWindow *parent)
: wxDialog (parent, -1, _("About Aegisub"), wxDefaultPosition, wxSize(300,240), wxCAPTION | wxCLOSE_BOX , _("About Aegisub"))
{
	// Get splash
	wxBitmap splash = GETIMAGE(splash_misc);
	SetOwnBackgroundColour(wxColour(255,255,255));
	SetForegroundColour(wxColour(0,0,0));

	// Picture
	wxSizer *PicSizer = new wxBoxSizer(wxHORIZONTAL);
	PicSizer->Add(new BitmapControl(this,splash));

	// Generate library string
	wxString libString = _T("Aegisub includes portions from the following other projects:\n");
	libString += _T("    wxWidgets - Copyright (c) Julian Smart, Robert Roebling et al;\n");
	libString += _T("    wxStyledTextCtrl - Copyright (c) Robin Dunn, Neil Hodgson;\n");
	libString += _T("    Scintilla - Copyright (c) Neil Hodgson;\n");
#ifdef WITH_AUTO4_LUA
	libString += _T("    Lua - Copyright (c) Lua.org, PUC-Rio;\n");
#endif
#ifdef WITH_HUNSPELL
	libString += _T("    Hunspell - Copyright (c) Kevin Hendricks;\n");
#endif
#ifdef WITH_PORTAUDIO
	libString += _T("    PortAudio - Copyright (c) Ross Bencina, Phil Burk;\n");
#endif
#ifdef WITH_FFMPEGSOURCE
	libString += _T("    FFmpeg - Copyright (c) Fabrice Bellard;\n");
	libString += _T("    FFmpegSource - Copyright (c) Fredrik Mellbin;\n");
#endif
#ifdef WITH_AVISYNTH
	libString += _T("    Avisynth 2.5 - Copyright (c) Ben Rudiak-Gould et al;\n");
#endif
#ifdef WITH_CSRI
	libString += _T("    csri - Copyright (c) David Lamparter;\n");
# ifdef __WINDOWS__
	libString += _T("    vsfilter - Copyright (c) Gabest;\n");
# endif
#endif
#ifdef WITH_LIBASS
	libString += _T("    libass - Copyright (c) Evgeniy Stepanov, Grigori Goronzy;\n");
#endif
#ifdef WITH_UNIVCHARDET
	libString += _T("    UniversalCharDet - Copyright (c) Netscape Communications Corp.;\n");
#endif
#ifdef __WINDOWS__
	libString += _T("    Matroska Parser - Copyright (c) Mike Matsnev;\n");
#endif
#ifdef WITH_FREETYPE2
	libString += _T("    Freetype - Copyright (c) David Turner, Robert Wilhelm, Werner Lemberg;\n");
#endif
	libString += _T("    MyThes - Copyright (c) Kevin B. Hendricks, Stratford, Ontario, Canada.\n");

	wxString translatorCredit = _("Translated into LANGUAGE by PERSON\n");
	if (translatorCredit == _T("Translated into LANGUAGE by PERSON\n")) translatorCredit.Clear();

	// Generate about string
	wxString aboutString;
	aboutString += wxString(_T("Aegisub ")) + GetAegisubShortVersionString() + _T(".\n");
	aboutString += _T("Copyright (c) 2005-2009 Rodrigo Braz Monteiro, Niels Martin Hansen et al.\n\n");
	aboutString += _T("Programmers:\n");
	aboutString += _T("    Alysson Souza e Silva\n");
	aboutString += _T("    Amar Takhar\n");
	aboutString += _T("    Dan Donovan\n");
	aboutString += _T("    Daniel Moscoviter\n");
	aboutString += _T("    David Conrad\n");
	aboutString += _T("    David Lamparter\n");
	aboutString += _T("    Eric Batalitzky\n");
	aboutString += _T("    Evgeniy Stepanov\n");
	aboutString += _T("    Fredrik Mellbin\n");
	aboutString += _T("    Karl Blomster\n");
	aboutString += _T("    Mike Matsnev\n");
	aboutString += _T("    Muhammad Lukman Nasaruddin\n");
	aboutString += _T("    Niels Martin Hansen\n");
	aboutString += _T("    Patryk Pomykalski\n");
	aboutString += _T("    Rodrigo Braz Monteiro\n");
	aboutString += _T("    Simone Cociancich\n");
	aboutString += _T("    Thomas Goyne\n");
	aboutString += _T("    2points\n");
	aboutString += _T("    p-static\n");
	aboutString += _T("User manual written by:\n");
	aboutString += _T("    Karl Blomster\n");
	aboutString += _T("    Niels Martin Hansen\n");
	aboutString += _T("    Rodrigo Braz Monteiro\n");
	aboutString += _T("Additional thanks to:\n");
	aboutString += _T("    Mentar\n");
	aboutString += _T("    Sigurd Tao Lyngse\n");
	aboutString += _T("    Everyone in the Aegisub IRC channel\n");
	aboutString += _T("    Everyone who ever reported a bug\n");
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
