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

#include <libaegisub/io.h>
#include <libaegisub/cajun/elements.h>
#include <libaegisub/cajun/writer.h>


#include "report.h"
#include "include/platform.h"
#include "aegisub.h"

/// @brief Contstructor
Report::Report() {

	json::Object root;

	Platform *p = Platform::GetPlatform();
	Aegisub a;

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
	root["General"] = general;




	try {
		json::Object aegisub;
		aegisub["Last Version"] = json::String(a.GetString("Version/Last Version"));
		aegisub["Spelling Language"] = json::String(a.GetString("Tool/Spell Checker/Language"));
		aegisub["Thesaurus Language"] = json::String(a.GetString("Tool/Thesaurus/Language"));
		aegisub["Audio Player"] = json::String(a.GetString("Audio/Player"));
		aegisub["Audio Provider"] = json::String(a.GetString("Audio/Provider"));
		aegisub["Video Provider"] = json::String(a.GetString("Video/Provider"));
		aegisub["Subtitles Provider"] = json::String(a.GetString("Subtitle/Provider"));
		aegisub["Save Charset"] = json::String(a.GetString("App/Save Charset"));
		aegisub["Grid Font Size"] = json::Number(a.GetInt("Grid/Font Size"));
		aegisub["Edit Font Size"] = json::Number(a.GetInt("Subtitle/Edit Box/Font Size"));
		aegisub["Spectrum Enabled"] = json::Boolean(a.GetBool("Audio/Spectrum"));
		aegisub["Spectrum Quality"] = json::Number(a.GetInt("Audio/Renderer/Spectrum/Quality"));
		aegisub["Call Tips Enabled"] = json::Boolean(a.GetBool("App/Call Tips"));
		aegisub["Medusa Hotkeys Enabled"] = json::Boolean(a.GetBool("Audio/Medusa Timing Hotkeys"));

		root["Aegisub"] = aegisub;
	} catch(...) {
		root["Aegisub"]["Error"] = json::String("Config file is corrupted");
	}



	json::Object hardware;
	hardware["Memory Size"] = json::Number();
	root["Hardware"] = hardware;

	json::Object cpu;
	cpu["Id"] = json::String(p->CPUId());
	cpu["Speed"] = json::String(p->CPUSpeed());
	cpu["Count"] = json::Number(p->CPUCount());
	cpu["Cores"] = json::Number(p->CPUCores());
	cpu["Features"] = json::String(p->CPUFeatures());
	cpu["Features2"] = json::String(p->CPUFeatures2());
	root["CPU"] = cpu;

	json::Object display;
	display["Depth"] = json::Number(p->DisplayDepth());
	display["Size"] = json::String(p->DisplaySize());
	display["Pixels Per Inch"] = json::String(p->DisplayPPI());

		json::Object gl;
		gl["Vendor"] = json::String(p->OpenGLVendor());
		gl["Renderer"] = json::String(p->OpenGLRenderer());
		gl["Version"] = json::String(p->OpenGLVersion());
		gl["Extensions"] = json::String(p->OpenGLExt());
		display["OpenGL"] = gl;

	root["Display"] = display;


#ifdef __WINDOWS__
	json::Object windows;
	windows["Service Pack"] = json::String();
	windows["Graphics Driver Version"] = json::String();
	windows["DirectShow Filters"] = json::String();
	windows["AntiVirus Installed"] = json::Boolean();
	windows["Firewall Installed"] = json::Boolean();
	windows["DLL"] = json::String();
	root["Windows"] = windows;

#endif

#ifdef __UNIX__
	json::Object u_nix;
	u_nix["Desktop Environment"] = json::String(p->DesktopEnvironment());
	u_nix["Libraries"] = json::String(p->UnixLibraries());
	root["Unix"] = u_nix;
#endif

#ifdef __APPLE__
	json::Object osx;
	osx["Patch"] = json::String(p->PatchLevel());
	osx["QuickTime Extensions"] = json::String(p->QuickTimeExt());
	osx["Model"] = json::String(p->HardwareModel());
	root["OS X"] = osx;
#endif

	agi::io::Save file("./t.json");
	json::Writer::Write(root, file.Get());

}

/// @brief Return Report as Text for the Clipboard.
void Report::Save(std::string filename) {
//	agi::io::Save file(filename);
//	json::Writer::Write(root, file.Get());
}
