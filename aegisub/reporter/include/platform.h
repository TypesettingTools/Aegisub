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

/// @file platform.h
/// @brief API for gathering platform information.
/// @see platform.cpp

#ifndef R_PRECOMP
#include <wx/platinfo.h>
#include <wx/string.h>
#endif

/// @brief Grab platform-specific information.
class Platform {
public:

	/// @brief Constructor
	Platform() {};

	/// @brief Destructor
	virtual ~Platform() {};

	// Get platform instance.
	static Platform* GetPlatform();

	// These are platform agnostic.
	//   From wxPlatformInfo
	wxString ArchName();			/// Architecture
	wxString OSFamily();			/// OS Family
	wxString OSName();			/// OS Name
	wxString Endian();			/// Endian

	//   From <wx/gdicmn.h>
	wxString DisplayColour();		/// Is the display colour?
	wxString DisplayDepth();		/// Depth
	wxString DisplaySize();			/// Size
	wxString DisplayPPI();			/// Pixels Per Inch

	//   Misc
	wxString Signature();			/// Report signature.
	wxString wxVersion();			/// wxWidgets version.
	wxString Locale();				/// Locale
	wxString Language();			/// Language currently in use
	wxString SystemLanguage();		/// System language
	wxString Date();				/// Date
	wxString Time();				/// Time
	wxString TimeZone();			/// TimeZone

	// The following are all platform-specific.
	//   Misc
	virtual wxString OSVersion()=0;

	//   Hardware
	virtual wxString CPUId()=0;
	virtual wxString CPUSpeed()=0;
	virtual wxString CPUCount()=0;
	virtual wxString CPUCores()=0;
	virtual wxString CPUFeatures()=0;
	virtual wxString CPUFeatures2()=0;
	virtual wxString Memory()=0;
	virtual wxString Video()=0;

	//   Windows
#ifdef __WINDOWS__
	virtual wxString ServicePack()=0;
	virtual wxString DriverGraphicsVersion()=0;
	virtual wxString DirectShowFilters()=0;
	virtual wxString AntiVirus()=0;
	virtual wxString FireWall()=0;
	virtual wxString DLLVersions()=0;
#endif

	//   Unix
#ifdef __UNIX__
	virtual wxString UnixLibraries()=0;
	virtual wxString DesktopEnvironment()=0;
#endif

	//   OS X
#ifdef __OSX__
	virtual wxString PatchLevel()=0;
	virtual wxString QuickTimeExt()=0;
	virtual wxString HardwareModel()=0;
#endif

private:
	void Init();
	const wxPlatformInfo plat;
	wxLocale *locale;
};
