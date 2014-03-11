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

	json::Object& general = root["General"];
	general["Signature"] = p->Signature();
	general["Date"] = p->Date();
	general["Architecture"] = p->ArchName();
	general["OS Family"] = p->OSFamily();
	general["OS Name"] = p->OSName();
	general["Endian"] = p->Endian();
	general["OS Version"] = p->OSVersion();
	general["wx Version"] = p->wxVersion();
	general["Locale"] = p->Locale();
	general["Language"] = p->Language();
	general["System Language"] = p->SystemLanguage();

	try {
		json::Object& aegisub = root["Aegisub"];
		aegisub["Last Version"] = a.GetString("Version/Last Version");
		aegisub["Spelling Language"] = a.GetString("Tool/Spell Checker/Language");
		aegisub["Thesaurus Language"] = a.GetString("Tool/Thesaurus/Language");
		aegisub["Audio Player"] = a.GetString("Audio/Player");
		aegisub["Audio Provider"] = a.GetString("Audio/Provider");
		aegisub["Video Provider"] = a.GetString("Video/Provider");
		aegisub["Subtitles Provider"] = a.GetString("Subtitle/Provider");
		aegisub["Save Charset"] = a.GetString("App/Save Charset");
		aegisub["Grid Font Size"] = a.GetInt("Grid/Font Size");
		aegisub["Edit Font Size"] = a.GetInt("Subtitle/Edit Box/Font Size");
		aegisub["Spectrum Enabled"] = a.GetBool("Audio/Spectrum");
		aegisub["Spectrum Quality"] = a.GetInt("Audio/Renderer/Spectrum/Quality");
		aegisub["Call Tips Enabled"] = a.GetBool("App/Call Tips");
		aegisub["Medusa Hotkeys Enabled"] = a.GetBool("Audio/Medusa Timing Hotkeys");
	} catch(...) {
		root["Aegisub"]["Error"] = json::String("Config file is corrupted");
	}

	json::Object& hardware = root["Hardware"];
	hardware["Memory Size"] = 0;

	json::Object& cpu = root["CPU"];
	cpu["Id"] = p->CPUId();
	cpu["Speed"] = p->CPUSpeed();
	cpu["Count"] = p->CPUCount();
	cpu["Cores"] = p->CPUCores();
	cpu["Features"] = p->CPUFeatures();
	cpu["Features2"] = p->CPUFeatures2();

	json::Object& display = root["Display"];
	display["Depth"] = p->DisplayDepth();
	display["Size"] = p->DisplaySize();
	display["Pixels Per Inch"] = p->DisplayPPI();

	json::Object& gl = display["OpenGL"];
	gl["Vendor"] = p->OpenGLVendor();
	gl["Renderer"] = p->OpenGLRenderer();
	gl["Version"] = p->OpenGLVersion();
	gl["Extensions"] = p->OpenGLExt();
	display["OpenGL"] = gl;

#ifdef __WINDOWS__
	json::Object& windows = root["Windows"];
	windows["Service Pack"] = json::String();
	windows["Graphics Driver Version"] = json::String();
	windows["DirectShow Filters"] = json::String();
	windows["AntiVirus Installed"] = json::Boolean();
	windows["Firewall Installed"] = json::Boolean();
	windows["DLL"] = json::String();
#endif

#ifdef __UNIX__
	json::Object& u_nix = root["Unix"];
	u_nix["Desktop Environment"] = p->DesktopEnvironment();
	u_nix["Libraries"] = p->UnixLibraries();
#endif

#ifdef __APPLE__
	json::Object& osx = root["OS X"];
	osx["Patch"] = p->PatchLevel();
	osx["Model"] = p->HardwareModel();
#endif

	agi::io::Save file("./t.json");
	json::Writer::Write(root, file.Get());

}

/// @brief Return Report as Text for the Clipboard.
void Report::Save(std::string filename) {
//	agi::io::Save file(filename);
//	json::Writer::Write(root, file.Get());
}
