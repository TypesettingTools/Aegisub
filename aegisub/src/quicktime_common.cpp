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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


#include <wx/wxprec.h>
#include "config.h"

#ifdef WITH_QUICKTIME
#include "quicktime_common.h"


void QuickTimeProvider::wxStringToDataRef(const wxString &string, Handle *dataref, OSType *dataref_type) {
	// convert filename, first to a CFStringRef...
	wxString wx_filename = wxFileName(string).GetShortPath();
	CFStringRef qt_filename = CFStringCreateWithCString(NULL, wx_filename.utf8_str(), kCFStringEncodingUTF8);
	
	// and then to a data reference
	OSErr qt_err = QTNewDataReferenceFromFullPathCFString(qt_filename, kQTNativeDefaultPathStyle, 0,
		dataref, dataref_type);
	QTCheckError(qt_err, wxString(_T("Failed to convert filename to data reference")));
}


void QuickTimeProvider::QTCheckError(OSErr err, wxString errmsg) {
	if (err != noErr)
		throw errmsg;
	/* CheckError(err, errmsg.c_str()); // I wonder if this actually works on Mac, and if so, what it does */
}

#endif /* WITH_QUICKTIME */
