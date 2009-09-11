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

/// @file quicktime_common.h
/// @see quicktime_common.cpp
/// @ingroup quicktime
///



#include "config.h"

#ifdef WITH_QUICKTIME

#ifndef AGI_PRE
#include <wx/thread.h>
#endif

#include "include/aegisub/aegisub.h"


// QT stuff
#ifdef _MSC_VER

/// DOCME
#define _STDINT_H				// avoid conflicts between MSVC's stdint.h and QT's stdint.h
#pragma warning(disable: 4004)	// get MSVC to shut up about a macro redefinition in QT's ConditionalMacros.h
#endif
extern "C" {
#ifdef WIN32
#include <QTML.h>
#include <Movies.h>
#include <Files.h>
#include <QDOffscreen.h>
#else
#include <QuickTime/QuickTime.h> // not sure about this path, someone on mac needs to test it
#endif
}



/// DOCME
/// @class QuickTimeProvider
/// @brief DOCME
///
/// DOCME
class QuickTimeProvider {
public:
	void InitQuickTime();
	void DeInitQuickTime();
	void wxStringToDataRef(const wxString &string, Handle *dataref, OSType *dataref_type);
	bool CanOpen(const Handle& dataref, const OSType dataref_type);


	void QTCheckError(OSErr err, wxString errmsg);
	void QTCheckError(OSStatus err, wxString errmsg);


	/// DOCME
	static int qt_initcount;

	/// DOCME
	static GWorldPtr default_gworld;


	/// @brief DOCME
	///
	virtual ~QuickTimeProvider() {};
};

#endif /* WITH_QUICKTIME */



