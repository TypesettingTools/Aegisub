// Copyright (c) 2007, Rodrigo Braz Monteiro
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

/// @file dialog_version_check.h
/// @see dialog_version_check.cpp
/// @ingroup configuration_ui
///


#pragma once


///////////
// Headers
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>


//////////////
// Prototypes
class DialogVersionCheck;



/// DOCME
/// @class VersionCheckThread
/// @brief DOCME
///
/// DOCME
class VersionCheckThread : public wxThread {
private:

	/// DOCME
	DialogVersionCheck *parent;

public:

	/// DOCME
	bool alive;

	VersionCheckThread(DialogVersionCheck *parent);
	wxThread::ExitCode Entry();
};



/// DOCME
/// @class DialogVersionCheck
/// @brief DOCME
///
/// DOCME
class DialogVersionCheck : public wxDialog {
	friend class VersionCheckThread;

private:

	/// DOCME
	static bool dialogRunning;


	/// DOCME
	wxTextCtrl *logBox;

	/// DOCME
	VersionCheckThread *thread;

	/// DOCME
	bool visible;

	void CheckVersion();
	void OnClose(wxCloseEvent &event);
	void OnOK(wxCommandEvent &event);
	void OnURL(wxTextUrlEvent &event);

public:
	DialogVersionCheck(wxWindow *parent,bool hidden);
	~DialogVersionCheck();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	Log_Box = 1000
};


