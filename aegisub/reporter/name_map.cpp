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

/// @@file name_map.cpp
/// @brief XML element -> human readable name mappings.

#include "report.h"

#define _TT(x) x	/// No-op macro for nMap.

/// @brief Load readable names of XML elements.
const Report::nameMap Report::HumanNames() {
	nameMap nMap;

	nMap.insert(nPair("antivirus",		_TT("AntiVirus Installed")));
	nMap.insert(nPair("arch",			_TT("Architecture")));
	nMap.insert(nPair("colour",			_TT("Colour Screen")));
	nMap.insert(nPair("colour",			_TT("Colour")));
	nMap.insert(nPair("cores",			_TT("Cores")));
	nMap.insert(nPair("count",			_TT("Count")));
	nMap.insert(nPair("cpu",			_TT("CPU")));
	nMap.insert(nPair("depth",			_TT("Depth")));
	nMap.insert(nPair("display",		_TT("Display")));
	nMap.insert(nPair("dll",			_TT("DLL")));
	nMap.insert(nPair("dshowfilter",	_TT("DirectShow Filters")));
	nMap.insert(nPair("features",		_TT("Features")));
	nMap.insert(nPair("features2",		_TT("Features2")));
	nMap.insert(nPair("firewall",		_TT("Firewall Installed")));
	nMap.insert(nPair("general",		_TT("General")));
	nMap.insert(nPair("graphicsver",	_TT("Graphics Driver Version")));
	nMap.insert(nPair("hardware",		_TT("Hardware")));
	nMap.insert(nPair("id",				_TT("Id")));
	nMap.insert(nPair("lang",			_TT("Language")));
	nMap.insert(nPair("lib",			_TT("Libraries")));
	nMap.insert(nPair("locale",			_TT("Locale")));
	nMap.insert(nPair("memory",			_TT("Memory")));
	nMap.insert(nPair("model",			_TT("Model")));
	nMap.insert(nPair("osendian",		_TT("Endian")));
	nMap.insert(nPair("osfamily",		_TT("OS Family")));
	nMap.insert(nPair("osname",			_TT("OS Name")));
	nMap.insert(nPair("osx",			_TT("OS X")));
	nMap.insert(nPair("osversion",		_TT("OS Version")));
	nMap.insert(nPair("patch",			_TT("Patch")));
	nMap.insert(nPair("ppi",			_TT("Pixels Per Inch")));
	nMap.insert(nPair("quicktimeext",	_TT("QuickTime Extensions")));
	nMap.insert(nPair("size",			_TT("Size")));
	nMap.insert(nPair("sp",				_TT("Service Pack")));
	nMap.insert(nPair("speed",			_TT("Speed")));
	nMap.insert(nPair("syslang",		_TT("System Language")));
	nMap.insert(nPair("unix",			_TT("Unix")));
	nMap.insert(nPair("video",			_TT("Video")));
	nMap.insert(nPair("windows",		_TT("Windows")));
	nMap.insert(nPair("wxversion",		_TT("wx Version")));
	nMap.insert(nPair("signature",		_TT("Signature")));
	nMap.insert(nPair("date",			_TT("Date")));
	nMap.insert(nPair("aegisub",		_TT("Aegisub")));
	nMap.insert(nPair("spelllang",		_TT("Spelling Language")));

	nMap.insert(nPair("lastversion",		_TT("Last Version")));
	nMap.insert(nPair("thesauruslang",		_TT("Thesaurus Language")));
	nMap.insert(nPair("audioplayer",		_TT("Audio Player")));
	nMap.insert(nPair("audioprovider",		_TT("Audio Provider")));
	nMap.insert(nPair("videoprovider",		_TT("Video Provider")));
	nMap.insert(nPair("subtitleprovider",	_TT("Subtitles Provider")));
	nMap.insert(nPair("savecharset",		_TT("Save Charset")));
	nMap.insert(nPair("gridfontsize",		_TT("Grid Font Size")));
	nMap.insert(nPair("editfontsize",		_TT("Edit Font Size")));
	nMap.insert(nPair("spectrum",			_TT("Spectrum Enabled")));
	nMap.insert(nPair("spectrumqual",		_TT("Spectrum Quality")));
	nMap.insert(nPair("calltips",			_TT("Call Tips Enabled")));
	nMap.insert(nPair("medusakeys",			_TT("Medusa Hotkeys Enabled")));
	nMap.insert(nPair("desktopenv",			_TT("Desktop Environment")));

	return nMap;
}
