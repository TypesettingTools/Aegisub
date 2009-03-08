// Copyright (c) 2005, Niels Martin Hansen
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

#include "export_clean_info.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_override.h"


///////////////
// Constructor
AssTransformCleanInfoFilter::AssTransformCleanInfoFilter() {
	initialized = false;
}


////////
// Init
void AssTransformCleanInfoFilter::Init() {
	if (initialized) return;
	initialized = true;
	autoExporter = false;
	Register(_("Clean Script Info"),0);
	description = _("Removes all but the absolutely required fields from the Script Info section. You might want to run this on files that you plan to distribute in original form.");
}


///////////
// Process
void AssTransformCleanInfoFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog) {
	using std::list;
	AssEntry *curEntry;
	entryIter cur, next = subs->Line.begin();
	while (next != subs->Line.end()) {
		cur = next++;

		curEntry = *cur;
		if (curEntry->group != _T("[Script Info]")) {
			continue;
		}
		if (curEntry->GetEntryData().IsEmpty()) {
			continue;
		}
		if (curEntry->GetEntryData() == _T("[Script Info]")) {
			continue;
		}
		if (curEntry->GetEntryData().Left(1) == _T(";")) {
			continue;
		}

		wxString field = curEntry->GetEntryData().Left(curEntry->GetEntryData().Find(_T(':'))).Lower();
		if (field != _T("scripttype") &&
			field != _T("collisions") &&
			field != _T("playresx") &&
			field != _T("playresy") &&
			field != _T("wrapstyle") &&
			field != _T("scaledborderandshadow")) {
			delete curEntry;
			subs->Line.erase(cur);
		}
	}
}


//////////////
// Get dialog
wxWindow *AssTransformCleanInfoFilter::GetConfigDialogWindow(wxWindow *parent) {
	return 0;
}


/////////////////
// Load settings
void AssTransformCleanInfoFilter::LoadSettings(bool IsDefault) {
}


///////////////////
// Global instance
AssTransformCleanInfoFilter AssTransformCleanInfoFilter::instance;
