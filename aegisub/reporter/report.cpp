// Copyright (c) 2009, Amar Takhar <verm@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// $Id$

/// @file report.cpp
/// @brief Generation and manipulation of reports.
/// @ingroup base

#ifndef R_PRECOMP
#include <wx/intl.h>
#include <wx/log.h>
#endif

#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/writer.h>


#include "report.h"
#include "include/platform.h"
#include "aegisub.h"

/// @brief Contstructor
Report::Report() {
	ReportCreate();
}

/// @brief Create report layout and add contents.
/// @return Document.
Report::XMLReport Report::ReportCreate() {


	json::Object root;

	Platform *p = Platform::GetPlatform();

	json::Object general;
	general["Signature"] = json::String(p->Signature());
	general["Date"] = json::String(p->Date());
	general["Architecture"] = json::String(p->ArchName());
	general["OS Family"] = json::String(p->OSFamily());
	general["OS Name"] = json::String(p->OSName());
	general["Endian"] = json::String(p->Endian());
	general["OS Version"] = json::String(p->OSVersion());
	general["wx Version"] = json::String(p->wxVersion());
	general["Locale"] = json::String(p->Locale());
	general["Language"] = json::String(p->Language());
	general["System Language"] = json::String(p->SystemLanguage());



	json::Object aegisub;

	/// I'll fix these at the end.
	/*
	Last Version
	Spelling Language
	Thesaurus Language
	Audio Player
	Audio Provider
	Video Provider
	Subtitles Provider
	Save Charset
	Grid Font Size
	Edit Font Size
	Spectrum Enabled
	Spectrum Quality
	Call Tips Enabled
	Medusa Hotkeys Enabled
	*/


	json::Object hardware;
	hardware["Memory Size"] = json::Number();


	json::Object cpu;
	cpu["Id"] = json::String(p->CPUId());
	cpu["Speed"] = json::String(p->CPUSpeed());
	cpu["Count"] = json::Number(p->CPUCount());
	cpu["Cores"] = json::Number(p->CPUCores());
	cpu["Features"] = json::String(p->CPUFeatures());
	cpu["Features2"] = json::String(p->CPUFeatures2());


	json::Object display;
//	display["Depth"] = json::Number(p->DisplayDepth());
//	display["Colour"] = json::Number(p->DisplayColour());
//	display["Size"] = json::String(p->DisplaySize());
//	display["Pixels Per Inch"] = json::Number(p->DisplayPPI());


//	json::Object gl;
//	gl["Vendor"] = json::String(p->OpenGLVendor());
//	gl["Renderer"] = json::String(p->OpenGLRenderer());
//	gl["Version"] = json::String(p->OpenGLVersion());
//	gl["Extensions"] = json::String(p->OpenGLExt());
//	display["OpenGL"] = gl;


#ifdef __WINDOWS__
	json::Object windows;
	windows["Service Pack"] = json::String();
	windows["Graphics Driver Version"] = json::String();
	windows["DirectShow Filters"] = json::String();
	windows["AntiVirus Installed"] = json::Boolean();
	windows["Firewall Installed"] = json::Boolean();
	windows["DLL"] = json::String();

#endif

#ifdef __UNIX__
	json::Object u_nix;
//	u_nix["Desktop Environment"] = json::String(p->DesktopEnvironment());
//	u_nix["Libraries"] = json::String(p->UnixLibraries());
#endif

#ifdef __APPLE__
	json::Object osx;
	osx["Patch"] = json::String(p->PatchLevel());
	osx["QuickTime Extensions"] = json::String(p->QuickTimeExt());
	osx["Model"] = json::String(p->HardwareModel());
#endif

	return doc;
}

/// @brief Return Report as Text for the Clipboard.
void Report::Save(wxString file) {
	doc.doc->Save(file);
}
