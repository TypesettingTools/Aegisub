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

#include "format.h"

#include <libaegisub/exception.h>

#include <algorithm>

static const char *pages[][2] = {
	{"Attachment Manager", "Attachment_Manager"},
	{"Automation Manager", "Automation/Manager"},
	{"Colour Picker", "Colour_Picker"},
	{"Dummy Video", "Video#dummy-video"},
	{"Export", "Exporting"},
	{"Fonts Collector", "Fonts_Collector"},
	{"Kanji Timer", "Kanji_Timer"},
	{"Main", "Main_Page"},
	{"Options", "Options"},
	{"Paste Over", "Paste_Over"},
	{"Properties", "Properties"},
	{"Resample resolution", "Resolution_Resampler"},
	{"Resolution mismatch", "Script_Resolution#automatic-resolution-change"},
	{"Shift Times", "Shift_Times"},
	{"Select Lines", "Select_Lines"},
	{"Spell Checker", "Spell_Checker"},
	{"Style Editor", "Styles"},
	{"Styles Manager", "Styles"},
	{"Styling Assistant", "Styling_Assistant"},
	{"Timing Processor", "Timing_Post-Processor"},
	{"Translation Assistant", "Translation_Assistant"},
	{"Visual Typesetting", "Visual_Typesetting"},
};

namespace {
	const char *url(const char *page) {
		auto it = std::lower_bound(std::begin(pages), std::end(pages), page, [](const char *pair[], const char *page) {
			return strcmp(pair[0], page) < 0;
		});
		return it == std::end(pages) ? nullptr : (*it)[1];
	}
}

HelpButton::HelpButton(wxWindow *parent, const char *page, wxPoint position, wxSize size)
: wxButton(parent, wxID_HELP, "", position, size)
{
	Bind(wxEVT_BUTTON, [=](wxCommandEvent&) { OpenPage(page); });
	if (!url(page))
		throw agi::InternalError("Invalid help page");
}

void HelpButton::OpenPage(const char *pageID) {
	auto page = url(pageID);
	auto sep = strchr(page, '#');
	if (sep)
		wxLaunchDefaultBrowser(fmt_wx("http://docs.aegisub.org/3.2/%.*s/%s", sep - page, page, sep));
	else
		wxLaunchDefaultBrowser(fmt_wx("http://docs.aegisub.org/3.2/%s/", page));
}
