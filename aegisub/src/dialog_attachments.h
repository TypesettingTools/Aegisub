// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file dialog_attachments.h
/// @see dialog_attachments.cpp
/// @ingroup tools_ui
///


#pragma once


///////////
// Headers
#include <wx/wxprec.h>


//////////////
// Prototypes
class wxListView;
class wxListEvent;



/// DOCME
/// @class DialogAttachments
/// @brief DOCME
///
/// DOCME
class DialogAttachments : public wxDialog {
private:

	/// DOCME
	wxListView *listView;

	/// DOCME
	wxButton *extractButton;

	/// DOCME
	wxButton *deleteButton;

	void OnAttachFont(wxCommandEvent &event);
	void OnAttachGraphics(wxCommandEvent &event);
	void OnExtract(wxCommandEvent &event);
	void OnDelete(wxCommandEvent &event);
	void OnListClick(wxListEvent &event);

	void UpdateList();

public:
	DialogAttachments(wxWindow *parent);
	~DialogAttachments();

	DECLARE_EVENT_TABLE()
};


///////
// IDs
enum {

	/// DOCME
	BUTTON_ATTACH_FONT = 1300,

	/// DOCME
	BUTTON_ATTACH_GRAPHICS,

	/// DOCME
	BUTTON_EXTRACT,

	/// DOCME
	BUTTON_DELETE,

	/// DOCME
	ATTACHMENT_LIST
};


