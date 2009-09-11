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

/// @file dialog_progress.h
/// @see dialog_progress.cpp
/// @ingroup utility
///


#ifndef DIALOG_PROGRESS_H

/// DOCME
#define DIALOG_PROGRESS_H


///////////
// Headers
#ifndef AGI_PRE
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/stattext.h>
#endif


/// DOCME
/// @class DialogProgress
/// @brief DOCME
///
/// DOCME
class DialogProgress : public wxDialog {
private:

	/// DOCME
	volatile int count;

	/// DOCME
	int virtualMax;

	/// DOCME
	wxMutex mutex;


	/// DOCME
	wxGauge *gauge;

	/// DOCME
	wxStaticText *text;
	void OnCancel(wxCommandEvent &event);
	void OnUpdateProgress(wxCommandEvent &event);

public:

	/// DOCME
	volatile bool *canceled;

	DialogProgress(wxWindow *parent,wxString title,volatile bool *cancel,wxString message,int cur,int max);
	void SetProgress(int cur,int max);
	void SetText(wxString text);
	void Run();

	DECLARE_EVENT_TABLE()
};



/// DOCME
/// @class DialogProgressThread
/// @brief DOCME
///
/// DOCME
class DialogProgressThread : public wxThread {
	DialogProgressThread(wxWindow *parent,wxString title,volatile bool *canceled,wxString message,int cur,int max);

public:

	/// DOCME
	DialogProgress *dialog;

	~DialogProgressThread();
	wxThread::ExitCode Entry();
	void Close();
};


#endif


