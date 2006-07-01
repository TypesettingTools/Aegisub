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
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:zeratul@cellosoft.com
//


///////////
// Headers
#include <wx/listctrl.h>
#include "dialog_attachments.h"
#include "ass_file.h"
#include "ass_attachment.h"
#include "utils.h"


///////////////
// Constructor
DialogAttachments::DialogAttachments(wxWindow *parent)
: wxDialog(parent,-1,_("Attachment List"),wxDefaultPosition,wxDefaultSize)
{
	// List view
	listView = new wxListView(this,-1,wxDefaultPosition,wxSize(400,200));
	listView->InsertColumn(0, _("Attachment name"), wxLIST_FORMAT_LEFT, 200);
	listView->InsertColumn(1, _("Size"), wxLIST_FORMAT_LEFT, 90);
	listView->InsertColumn(2, _("Group"), wxLIST_FORMAT_LEFT, 90);

	// Fill list
	AssAttachment *attach;
	for (std::list<AssEntry*>::iterator cur = AssFile::top->Line.begin();cur != AssFile::top->Line.end();cur++) {
		attach = AssEntry::GetAsAttachment(*cur);
		if (attach) {
			// Add item
			int row = listView->GetItemCount();
			listView->InsertItem(row,attach->filename);
			listView->SetItem(row,1,PrettySize(attach->GetData().size()));
			listView->SetItem(row,2,attach->group);
			listView->SetItemData(row,(long)attach);
		}
	}

	// Buttons sizer
	wxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(new wxButton(this,BUTTON_ATTACH_FONT,_("&Attach Font")),3,0,0);
	buttonSizer->Add(new wxButton(this,BUTTON_EXTRACT,_("E&xtract")),2,0,0);
	buttonSizer->Add(new wxButton(this,BUTTON_DELETE,_("&Delete")),2,0,0);
	buttonSizer->Add(new wxButton(this,BUTTON_CLOSE,_("&Close")),2,wxLEFT,5);

	// Main sizer
	wxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(listView,1,wxTOP | wxLEFT | wxRIGHT | wxEXPAND,5);
	mainSizer->Add(buttonSizer,0,wxALL | wxEXPAND,5);
	mainSizer->SetSizeHints(this);
	SetSizer(mainSizer);
}


//////////////
// Destructor
DialogAttachments::~DialogAttachments() {
}


///////////////
// Event table
BEGIN_EVENT_TABLE(DialogAttachments,wxDialog)
	EVT_BUTTON(BUTTON_ATTACH_FONT,DialogAttachments::OnAttachFont)
	EVT_BUTTON(BUTTON_EXTRACT,DialogAttachments::OnExtract)
	EVT_BUTTON(BUTTON_DELETE,DialogAttachments::OnDelete)
	EVT_BUTTON(BUTTON_CLOSE,DialogAttachments::OnClose)
END_EVENT_TABLE()


///////////////
// Attach font
void DialogAttachments::OnAttachFont(wxCommandEvent &event) {
}


///////////
// Extract
void DialogAttachments::OnExtract(wxCommandEvent &event) {
}


//////////
// Delete
void DialogAttachments::OnDelete(wxCommandEvent &event) {
}


/////////
// Close
void DialogAttachments::OnClose(wxCommandEvent &event) {
	EndModal(0);
}
