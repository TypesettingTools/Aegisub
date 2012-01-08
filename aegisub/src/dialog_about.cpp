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
// Aegisub Project http://www.aegisub.org/
//
// $Id$

/// @file dialog_about.cpp
/// @brief About-dialogue box
/// @ingroup configuration_ui
///


////////////
// Includes
#include "config.h"

#ifndef AGI_PRE
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#endif

#include "dialog_about.h"
#include "libresrc/libresrc.h"
#include "version.h"


/// @brief Constructor
/// @param parent Parent frame.
///
AboutScreen::AboutScreen(wxWindow *parent)
: wxDialog (parent, -1, _("About Aegisub"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX , _("About Aegisub"))
{
	// Generate library string
	wxString libString = "Aegisub includes portions from the following other projects:\n";
	libString += "    wxWidgets - Copyright (c) Julian Smart, Robert Roebling et al;\n";
	libString += "    wxStyledTextCtrl - Copyright (c) Robin Dunn, Neil Hodgson;\n";
	libString += "    Scintilla - Copyright (c) Neil Hodgson;\n";
	libString += "    UniversalCharDet - Copyright (c) Netscape Communications Corp.;\n";
#ifdef WITH_AUTO4_LUA
	libString += "    Lua - Copyright (c) Lua.org, PUC-Rio;\n";
#endif
#ifdef WITH_HUNSPELL
	libString += "    Hunspell - Copyright (c) Kevin Hendricks;\n";
#endif
#ifdef WITH_PORTAUDIO
	libString += "    PortAudio - Copyright (c) Ross Bencina, Phil Burk;\n";
#endif
#ifdef WITH_FFMS2
	libString += "    FFmpeg - Copyright (c) Fabrice Bellard;\n";
	libString += "    FFMS2 - Copyright (c) Fredrik Mellbin;\n";
#endif
#ifdef WITH_AVISYNTH
	libString += "    Avisynth 2.5 - Copyright (c) Ben Rudiak-Gould et al;\n";
#endif
#ifdef WITH_CSRI
	libString += "    csri - Copyright (c) David Lamparter;\n";
# ifdef __WINDOWS__
	libString += "    vsfilter - Copyright (c) Gabest;\n";
# endif
#endif
#ifdef WITH_LIBASS
	libString += "    libass - Copyright (c) Evgeniy Stepanov, Grigori Goronzy;\n";
#endif
#ifdef __WINDOWS__
	libString += "    Matroska Parser - Copyright (c) Mike Matsnev;\n";
#endif
#ifdef WITH_FREETYPE2
	libString += "    Freetype - Copyright (c) David Turner, Robert Wilhelm, Werner Lemberg;\n";
#endif
#ifdef WITH_FFTW3
	libString += "    FFTW - Copyright (c) Matteo Frigo, Massachusetts Institute of Technology;\n";
#endif

	wxString translatorCredit = _("Translated into LANGUAGE by PERSON\n");
	if (translatorCredit == "Translated into LANGUAGE by PERSON\n") translatorCredit.Clear();

	// Generate about string
	wxString aboutString;
	aboutString += wxString("Aegisub ") + GetAegisubShortVersionString() + ".\n";
	aboutString += "Copyright (c) 2005-2012 Rodrigo Braz Monteiro, Niels Martin Hansen, Thomas Goyne et al.\n\n";
	aboutString += "Programmers:\n";
	aboutString += "    Alysson Souza e Silva\n";
	aboutString += "    Amar Takhar\n";
	aboutString += "    Dan Donovan\n";
	aboutString += "    Daniel Moscoviter\n";
	aboutString += "    David Conrad\n";
	aboutString += "    David Lamparter\n";
	aboutString += "    Eric Batalitzky\n";
	aboutString += "    Evgeniy Stepanov\n";
	aboutString += "    Fredrik Mellbin\n";
	aboutString += "    Grigori Goronzy\n";
	aboutString += "    Karl Blomster\n";
	aboutString += "    Mike Matsnev\n";
	aboutString += "    Moritz Brunner\n";
	aboutString += "    Muhammad Lukman Nasaruddin\n";
	aboutString += "    Niels Martin Hansen\n";
	aboutString += "    Patryk Pomykalski\n";
	aboutString += "    Ravi Pinjala\n";
	aboutString += "    Rodrigo Braz Monteiro\n";
	aboutString += "    Simone Cociancich\n";
	aboutString += "    Thomas Goyne\n";
	aboutString += "User manual written by:\n";
	aboutString += "    Karl Blomster\n";
	aboutString += "    Niels Martin Hansen\n";
	aboutString += "    Rodrigo Braz Monteiro\n";
	aboutString += "Icons by:\n";
	aboutString += "    Philip Cash\n";
	aboutString += "Additional thanks to:\n";
	aboutString += "    Mentar\n";
	aboutString += "    Sigurd Tao Lyngse\n";
	aboutString += "    Everyone in the Aegisub IRC channel\n";
	aboutString += "    Everyone who ever reported a bug\n";
	aboutString += translatorCredit;
	aboutString += "\n" + libString;
	aboutString += _("\nSee the help file for full credits.\n");
	aboutString += wxString::Format(_("Built by %s on %s."), GetAegisubBuildCredit(), GetAegisubBuildTime());

	// Replace copyright symbol
	wxChar copySymbol = 0xA9;
	aboutString.Replace("(c)",wxString(copySymbol));

	wxTextCtrl *textctrl = new wxTextCtrl(this, -1, aboutString, wxDefaultPosition, wxSize(-1,200), wxTE_MULTILINE|wxTE_READONLY|wxBORDER_NONE);

	wxSizer *MainSizer = new wxBoxSizer(wxVERTICAL);
	MainSizer->Add(new wxStaticBitmap(this, -1, GETIMAGE(splash)), 0, wxCENTER, 0);
	MainSizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND|wxALL, 0);
	MainSizer->Add(textctrl, 0, wxEXPAND|wxALL, 0);
	MainSizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND|wxALL, 0);
	MainSizer->Add(CreateButtonSizer(wxOK), 0, wxEXPAND|wxALL, 6);

	SetSizerAndFit(MainSizer);
	CentreOnParent();
}


/// @brief Destructor
///
AboutScreen::~AboutScreen () {
}


