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

/// @file export_fixstyle.cpp
/// @brief Fix Styles export filter
/// @ingroup export
///

#include "config.h"

#ifndef AGI_PRE
#include <algorithm>
#include <functional>
#endif

#include "export_fixstyle.h"
#include "ass_file.h"
#include "ass_dialogue.h"
#include "ass_style.h"

/// @brief Constructor 
AssFixStylesFilter::AssFixStylesFilter() {
	initialized = false;
}

/// @brief Init 
void AssFixStylesFilter::Init() {
	if (initialized) return;
	initialized = true;
	autoExporter = true;
	Register(_("Fix Styles"),-5000);
	description = _("Fixes styles by replacing any style that isn't available on file with Default.");
}

/// @brief Process 
/// @param subs          
/// @param export_dialog 
void AssFixStylesFilter::ProcessSubs(AssFile *subs, wxWindow *export_dialog) {
	wxArrayString styles = subs->GetStyles();
	styles.Sort();
	std::for_each(styles.begin(), styles.end(), std::mem_fun_ref(&wxString::MakeLower));
	size_t n = styles.Count();

	for (entryIter cur=subs->Line.begin();cur!=subs->Line.end();cur++) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(*cur);
		if (diag) {
			if (!std::binary_search(styles.begin(), styles.end(), diag->Style.Lower())) {
				diag->Style = L"Default";
			}
		}
	}
}

/// DOCME
AssFixStylesFilter AssFixStylesFilter::instance;
