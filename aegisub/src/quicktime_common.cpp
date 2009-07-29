// Copyright (c) 2009, Karl Blomster
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

/// @file quicktime_common.cpp
/// @brief Common code between QuickTime-based video and audio providers
/// @ingroup quicktime
///


#include "quicktime_common.h"

#ifdef WITH_QUICKTIME
#include <wx/wxprec.h>



/// DOCME
int QuickTimeProvider::qt_initcount = 0;

/// DOCME
GWorldPtr QuickTimeProvider::default_gworld = NULL;



/// @brief DOCME
///
void QuickTimeProvider::InitQuickTime() {
	OSErr qt_err;
#ifdef WIN32
	qt_err = InitializeQTML(0L);
	QTCheckError(qt_err, wxString(_T("Failed to initialize QTML (do you have QuickTime installed?)")));
#endif

	qt_err = EnterMovies();
	QTCheckError(qt_err, wxString(_T("EnterMovies failed")));

	// have we been inited before?
	if (qt_initcount <= 0) { 
		// We haven't, allocate an offscreen render target.
		// We need to do this before we actually open anything, or quicktime may crash. (heh)
		Rect def_box;
		def_box.top = 0;
		def_box.left = 0;
		def_box.bottom = 320;	// pick some random dimensions for now;
		def_box.right = 240;	// we won't actually use this buffer to render anything
		QDErr qd_err = NewGWorld(&default_gworld, 32, &def_box, NULL, NULL, keepLocal);
		if (qd_err != noErr)
			throw wxString(_T("Failed to initialize temporary offscreen drawing buffer"));
		SetGWorld(default_gworld, NULL);
	}

	qt_initcount++;
}



/// @brief DOCME
///
void QuickTimeProvider::DeInitQuickTime() {
#ifdef WIN32
	// calls to InitializeQTML() must be balanced with an equal number of calls to TerminateQTML()
	TerminateQTML();
#endif
	qt_initcount--;

	if (qt_initcount <= 0) {
		ExitMovies();
		DisposeGWorld(default_gworld);
	}
}



/// @brief convert a wxstring containing a filename to a QT data reference
/// @param string       
/// @param dataref      
/// @param dataref_type 
///
void QuickTimeProvider::wxStringToDataRef(const wxString &string, Handle *dataref, OSType *dataref_type) {
	// convert filename, first to a CFStringRef...
	wxString wx_filename = wxFileName(string).GetShortPath();
	CFStringRef qt_filename = CFStringCreateWithCString(NULL, wx_filename.utf8_str(), kCFStringEncodingUTF8);
	
	// and then to a data reference
	OSErr qt_err = QTNewDataReferenceFromFullPathCFString(qt_filename, kQTNativeDefaultPathStyle, 0,
		dataref, dataref_type);
	QTCheckError(qt_err, wxString(_T("Failed to convert filename to data reference")));
}



/// @brief check if this error code signifies an error, and if it does, throw an exception
/// @param err    
/// @param errmsg 
///
void QuickTimeProvider::QTCheckError(OSErr err, wxString errmsg) {
	if (err != noErr)
		throw errmsg;
	/* CheckError(err, errmsg.c_str()); // I wonder if this actually works on Mac, and if so, what it does */
}

/// @brief DOCME
/// @param err    
/// @param errmsg 
/// @return 
///
void QuickTimeProvider::QTCheckError(OSStatus err, wxString errmsg) {
	if (err != noErr)
		throw errmsg;
}



/// @brief return true if QT considers file openable
/// @param dataref      
/// @param dataref_type 
///
bool QuickTimeProvider::CanOpen(const Handle& dataref, const OSType dataref_type) {
	Boolean can_open;
	Boolean prefer_img;
	OSErr qt_err = CanQuickTimeOpenDataRef(dataref, dataref_type, NULL, &can_open, &prefer_img, 0);
	QTCheckError(qt_err, wxString(_T("Checking if file is openable failed")));

	// don't bother trying to open things quicktime considers images
	if (can_open && !prefer_img)
		return true;
	else
		return false;
}


#endif /* WITH_QUICKTIME */


