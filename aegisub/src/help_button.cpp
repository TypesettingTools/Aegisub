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

/// @file help_button.cpp
/// @brief Push-button opening the help file at a specified section
/// @ingroup custom_control
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/mimetype.h>
#endif

#include "help_button.h"
#include "standard_paths.h"
#include "utils.h"


/// @brief Constructor 
/// @param parent   
/// @param _page    
/// @param position 
/// @param size     
///
HelpButton::HelpButton(wxWindow *parent,wxString _page,wxPoint position,wxSize size)
: wxButton (parent,wxID_HELP,"",position,size)
{
	id = _page;
	Connect(GetId(),wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(HelpButton::OnPressed));
}



/// @brief Pressed 
/// @param event 
/// @return 
///
void HelpButton::OnPressed(wxCommandEvent &event) {
	// Verify if the page is valid
	if (id.IsEmpty()) {
		wxLogMessage("TODO");
		return;
	}

	// Open
	OpenPage(id);
}



/// @brief Open a page 
/// @param pageID 
///
void HelpButton::OpenPage(const wxString pageID) {
	// Transcode
	InitStatic();
	wxString page = (*pages)[pageID];

	// Get the file type
	wxFileType *type = wxTheMimeTypesManager->GetFileTypeFromExtension("html");
	if (type) {
		//wxString path = StandardPaths::DecodePath(wxString::Format("http://docs.aegisub.net/%s",page.c_str()));
		wxString docsPath = StandardPaths::DecodePath("?data/docs");
#ifdef __WINDOWS__
		docsPath.Replace("\\","/");
		docsPath = "/" + docsPath;
#endif
		wxString path = wxString::Format("file://%s/%s.html",docsPath,page);
		wxLaunchDefaultBrowser(path);
	}
}



/// DOCME
std::map<wxString,wxString> *HelpButton::pages = NULL;


/// @brief DOCME
///
void HelpButton::InitStatic() {
	if (!pages) {
		pages = new std::map<wxString,wxString>;
		std::map<wxString,wxString> &page = *pages;
		page["Attachment Manager"] = "Attachment_Manager";
		page["Automation Manager"] = "Automation_Manager";
		page["Colour Picker"] = "Colour_Picker";
		page["Dummy Video"] = "Dummy_video";
		page["Export"] = "Exporting";
		page["Fonts Collector"] = "Fonts_Collector";
		page["Kanji Timer"] = "Kanji_Timer";
		page["Main"] = "Main_Page";
		page["Options"] = "Options";
		page["Paste Over"] = "Paste_Over";
		page["Properties"] = "Properties";
		page["Resample resolution"] = "Resolution_Resampler";
		page["Shift Times"] = "Shift_Times";
		page["Select Lines"] = "Select_Lines";
		page["Spell Checker"] = "Spell_Checker";
		page["Style Editor"] = "Styles";
		page["Styles Manager"] = "Styles";
		page["Styling Assistant"] = "Styling_Assistant";
		page["Timing Processor"] = "Timing_Post-Processor";
		page["Translation Assistant"] = "Translation_Assistant";
		page["Visual Typesetting"] = "Visual_Typesetting";
	}
}


/// @brief DOCME
///
void HelpButton::ClearPages() {
	if (pages) delete pages;
}


