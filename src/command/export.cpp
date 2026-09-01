// Copyright (c) 2026, arch1t3cht <arch1t3cht@gmail.com>
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
// Aegisub Project https://aegisub.org/

// Function for exporting a list of all commands to a yaml file that can be
// included on the website. Very quick and very dirty.
// This file is deliberately not linked into Aegisub on normal builds.

#include "export.h"

#include <iostream>

#include "command/command.h"
#include "compat.h"
#include "format.h"

#include <boost/algorithm/string/replace.hpp>

#include <wx/arrstr.h>
#include <wx/intl.h>

namespace cmd {

std::string yaml_quote(std::string_view input) {
	std::string escaped(input);
	boost::replace_all(escaped, "'", "''");
	return agi::format("'%s'", escaped);
}

void ExportCommands(agi::Context *c) {
	wxArrayString langs = wxTranslations::Get()->GetAvailableTranslations("aegisub");

	langs.Sort();
	langs.insert(langs.begin(), "en_US");

	auto commands = cmd::get_registered_commands();

	for (auto const& lang : langs) {
		auto info = wxLocale::FindLanguageInfo(lang);

		std::cout << agi::wxformat(" - locale: %s\n", lang);
		std::cout << agi::wxformat("   name: %s\n", info->Description);
		std::cout << "   commands:\n";

		wxTranslations *translations = new wxTranslations();
		wxTranslations::Set(translations);
		translations->SetLanguage(lang);
		translations->AddCatalog("aegisub");
		translations->AddStdCatalog();

		for (auto const& name : commands) {
			if (name.starts_with("automation/"))
				continue;

			auto command = cmd::get(name);

			std::cout << agi::format("    - name: %s\n", name);
			std::cout << agi::format("      display: %s\n", yaml_quote(from_wx(command->StrDisplay(c))));
			std::cout << agi::format("      help: %s\n", yaml_quote(from_wx(command->StrHelp())));
			std::cout << "\n";
		}
	}
}
}
