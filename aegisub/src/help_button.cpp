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

#include <wx/wxprec.h>
#include <wx/mimetype.h>
#include <wx/log.h>
#include <wx/filename.h>
#include <map>
#include "help_button.h"
#include "utils.h"
#include "standard_paths.h"


///////////////
// Constructor
HelpButton::HelpButton(wxWindow *parent,wxString _page,wxPoint position,wxSize size)
: wxButton (parent,wxID_HELP,_T(""),position,size)
{
	id = _page;
	Connect(GetId(),wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(HelpButton::OnPressed));
}


///////////
// Pressed
void HelpButton::OnPressed(wxCommandEvent &event) {
	// Verify if the page is valid
	if (id.IsEmpty()) {
		wxLogMessage(_T("TODO"));
		return;
	}

	// Open
	OpenPage(id);
}


///////////////
// Open a page
void HelpButton::OpenPage(const wxString pageID) {
	// Transcode
	InitStatic();
	wxString page = (*pages)[pageID];

	// Get the file type
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension(_T("html"));
	if (type) {
		//wxString path = StandardPaths::DecodePath(wxString::Format(_T("http://docs.aegisub.net/%s"),page.c_str()));
		wxString docsPath = StandardPaths::DecodePath(_T("?data/docs"));
#ifdef __WINDOWS__
		docsPath.Replace(_T("\\"),_T("/"));
		docsPath = _T("/") + docsPath;
#endif
		wxString path = StandardPaths::DecodePath(wxString::Format(_T("file://%s/%s.html"),docsPath.c_str(),page.c_str()));
		wxLaunchDefaultBrowser(path);
	}
}


//////////////
// Static map
std::map<wxString,wxString> *HelpButton::pages = NULL;

void HelpButton::InitStatic() {
	if (!pages) {
		pages = new std::map<wxString,wxString>;
		std::map<wxString,wxString> &page = *pages;
		page[_T("Attachment Manager")] = _T("Attachment_Manager");
		page[_T("Automation Manager")] = _T("Automation_Manager");
		page[_T("Colour Picker")] = _T("Colour_Picker");
		page[_T("Dummy Video")] = _T("Dummy_video");
		page[_T("Export")] = _T("Exporting");
		page[_T("Fonts Collector")] = _T("Fonts_Collector");
		page[_T("Kanji Timer")] = _T("Kanji_Timer");
		page[_T("Main")] = _T("Main_Page");
		page[_T("Options")] = _T("Options");
		page[_T("Paste Over")] = _T("Paste_Over");
		page[_T("Properties")] = _T("Properties");
		page[_T("Resample resolution")] = _T("Resolution_Resampler");
		page[_T("Shift Times")] = _T("Shift_Times");
		page[_T("Select Lines")] = _T("Select_Lines");
		page[_T("Spell Checker")] = _T("Spell_Checker");
		page[_T("Style Editor")] = _T("Styles");
		page[_T("Styles Manager")] = _T("Styles");
		page[_T("Styling Assistant")] = _T("Styling_Assistant");
		page[_T("Timing Processor")] = _T("Timing_Post-Processor");
		page[_T("Translation Assistant")] = _T("Translation_Assistant");
		page[_T("Visual Typesetting")] = _T("Visual_Typesetting");
	}
}

void HelpButton::ClearPages() {
	if (pages) delete pages;
}
