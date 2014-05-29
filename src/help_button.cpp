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

#include "help_button.h"

#include <libaegisub/exception.h>

#include <boost/container/flat_map.hpp>

namespace {
static boost::container::flat_map<wxString, wxString> pages;

void init_static() {
	if (pages.empty()) {
		pages["Attachment Manager"] = "Attachment_Manager";
		pages["Automation Manager"] = "Automation/Manager";
		pages["Colour Picker"] = "Colour_Picker";
		pages["Dummy Video"] = "Video#dummyvideo";
		pages["Export"] = "Exporting";
		pages["Fonts Collector"] = "Fonts_Collector";
		pages["Kanji Timer"] = "Kanji_Timer";
		pages["Main"] = "Main_Page";
		pages["Options"] = "Options";
		pages["Paste Over"] = "Paste_Over";
		pages["Properties"] = "Properties";
		pages["Resample resolution"] = "Resolution_Resampler";
		pages["Shift Times"] = "Shift_Times";
		pages["Select Lines"] = "Select_Lines";
		pages["Spell Checker"] = "Spell_Checker";
		pages["Style Editor"] = "Styles";
		pages["Styles Manager"] = "Styles";
		pages["Styling Assistant"] = "Styling_Assistant";
		pages["Timing Processor"] = "Timing_Post-Processor";
		pages["Translation Assistant"] = "Translation_Assistant";
		pages["Visual Typesetting"] = "Visual_Typesetting";
	}
}
}

HelpButton::HelpButton(wxWindow *parent, wxString const& page, wxPoint position, wxSize size)
: wxButton(parent, wxID_HELP, "", position, size)
{
	Bind(wxEVT_BUTTON, [=](wxCommandEvent&) { OpenPage(page); });
	init_static();
	if (pages.find(page) == pages.end())
		throw agi::InternalError("Invalid help page");
}

void HelpButton::OpenPage(wxString const& pageID) {
	init_static();

	wxString page = pages[pageID];
	wxString section;
	page = page.BeforeFirst('#', &section);

	wxLaunchDefaultBrowser(wxString::Format("http://docs.aegisub.org/3.1/%s/#%s", page, section));
}
