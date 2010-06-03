// Copyright (c) 2010, Amar Takhar
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

/// @file charset_detect.cpp
/// @brief Wrapper around text encoding detection library
/// @ingroup utility
///

#include "config.h"

#ifndef AGI_PRE
#include <fstream>
#include <list>

#include <wx/arrstr.h>
#include <wx/choicdlg.h>
#include <wx/intl.h>
#endif

#include <libaegisub/charset.h>
#include <libaegisub/log.h>

#include "charset_detect.h"
#include "compat.h"

namespace CharSetDetect {

wxString GetEncoding(wxString const& filename) {
	LOG_I("charset/file") << filename;
	bool unknown = 0;

	agi::charset::CharsetListDetected list;
	agi::charset::CharsetListDetected::const_iterator i_lst;

	try {
		agi::charset::DetectAll(STD_STR(filename), list);
	} catch (const agi::charset::UnknownCharset&) {
		unknown = 1;
	}

	/// @todo If the charset is unknown we need to display a complete list of character sets.
	if (list.size() > 1) {

		// Get choice from user
		wxArrayString choices;

		for (i_lst = list.begin(); i_lst != list.end(); ++i_lst) {
			choices.Add(lagi_wxString(i_lst->second));
		}

		int choice = wxGetSingleChoiceIndex(_("Aegisub could not narrow down the character set to a single one.\nPlease pick one below:"),_("Choose character set"),choices);
		if (choice == -1) throw _T("Canceled");
		return choices.Item(choice);
	}

	i_lst = list.begin();
	return i_lst->second;
}

}

