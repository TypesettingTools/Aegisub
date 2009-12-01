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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include "config.h"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/fontdlg.h>
#include "browse_button.h"
#include "utils.h"
#include "standard_paths.h"


///////////////
// Constructor
BrowseButton::BrowseButton(wxWindow *parent,int id,wxString text,BrowseType _type,wxPoint position,wxSize size)
: wxButton (parent,id,text == wxString(_T("")) ? wxString(_("Browse...")) : text,position,size)
{
	type = _type;
	ctrl[0] = NULL;
	ctrl[1] = NULL;
	Connect(GetId(),wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(BrowseButton::OnPressed));
}


////////
// Bind
void BrowseButton::Bind(wxTextCtrl *control,int pos) {
	ctrl[pos] = control;
}


///////////
// Pressed
void BrowseButton::OnPressed(wxCommandEvent &event) {
	// Folder
	if (type == BROWSE_FOLDER) {
		// For some reason I can't make this work on Mac... -jfs
		wxString def = StandardPaths::DecodePathMaybeRelative(ctrl[0]->GetValue(), _T("?user/"));
		wxDirDialog dlg(0, _("Please choose the folder:"), def);
		if (dlg.ShowModal() == wxID_OK) {
			wxString dir = StandardPaths::EncodePath(dlg.GetPath());
			if (dir != _T("")) ctrl[0]->SetValue(dir);
		}
	}

	// File
	else if (type == BROWSE_FILE) {
	}

	// Font
	else if (type == BROWSE_FONT) {
		wxFont font;
		long size;
		ctrl[1]->GetValue().ToLong(&size);
		font.SetFaceName(ctrl[0]->GetValue());
		font.SetPointSize(size);
		font = wxGetFontFromUser(NULL,font);
		if (font.IsOk()) {
			ctrl[0]->SetValue(font.GetFaceName());
			ctrl[1]->SetValue(wxString::Format(_T("%i"),font.GetPointSize()));
		}
	}
}


