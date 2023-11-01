// Copyright (c) 2023, arch1t3cht <arch1t3cht@gmail.com>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#include "gui_wrap.h"

#include "libaegisub/log.h"
#include "libaegisub/format.h"
#include "libaegisub/util.h"

#include "compat.h"
#include "options.h"

#include <wx/msgdlg.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.


int wrapMessageBox(const wxString &message, const wxString &caption, long style, wxWindow *parent) {
	if (config::hasGui)
		return wxMessageBox(message, caption, style, parent);

	agi::log::Severity severity = agi::log::Info;
	if (style | wxICON_ERROR)
		severity = agi::log::Exception;
	if (style | wxICON_WARNING)
		severity = agi::log::Warning;

	LOG_SINK("agi", severity) << caption << ": " << message;
	return 0;
}

int wrapChoiceDialog(std::string const& key, const wxString& message, const wxString& caption, const wxArrayString& choices, wxWindow *parent) {
	if (config::hasGui)
		return wxGetSingleChoiceIndex(message, caption, choices, parent);

	if (!config::choice_indices.count(key) || config::choice_indices[key].empty()) {
		LOG_W("agi") << agi::format("No answer given for choice \"%s\". Using first choice \"%s\".", from_wx(caption), from_wx(choices[0]));
		return 0;
	}

	std::string choice = *config::choice_indices[key].rbegin();
	config::choice_indices[key].pop_back();

	auto index = std::find(choices.begin(), choices.end(), to_wx(choice));
	if (index != choices.end())
		return std::distance(choices.begin(), index);

	int result;
	if (!agi::util::try_parse(choice, &result)) {
		LOG_W("agi") << agi::format("Invalid answer \"%s\" given for choice \"%s\". Using first choice \"%s\".", choice, from_wx(caption), from_wx(choices[0]));
		return 0;
	}
	return result;
}

