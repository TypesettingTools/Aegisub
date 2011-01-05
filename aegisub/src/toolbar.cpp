// Copyright (c) 2010, Amar Takhar <verm@aegisub.org>
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
// $Id$

/// @file menutool.cpp
/// @brief Dynamic menu toolbar generator.
/// @ingroup toolbar menu

#include "config.h"

#ifndef AGI_PRE
#include <math.h>

#include <memory>
#endif

#include <libaegisub/io.h>
#include <libaegisub/json.h>
#include <libaegisub/log.h>

#include "aegisub/toolbar.h"
#include "libresrc/libresrc.h"
#include "command/command.h"


namespace toolbar {

Toolbar *toolbar;

Toolbar::Toolbar() {}

Toolbar::~Toolbar() {}


void Toolbar::GetToolbar(std::string name, wxToolBar *toolbar) {

//	TbMap::iterator index;

//	if ((index = map.find(name)) != map.end()) {
//		return index->second;
//	}

//	throw ToolbarInvalidName("Unknown index");

	LOG_D("toolbar/init") << "Generating " << name << " toolbar.";

    std::istringstream *stream = new std::istringstream(GET_DEFAULT_CONFIG(default_toolbar));
    json::UnknownElement toolbar_root = agi::json_util::parse(stream);

	const json::Array& arr = toolbar_root[name];

	BuildToolbar(toolbar, arr);
//	map.insert(TbPair(name, toolbar));
}


void Toolbar::BuildToolbar(wxToolBar *toolbar, const json::Array& array) {

	for (json::Array::const_iterator index(array.Begin()); index != array.End(); index++) {

		const json::Object& obj = *index;
		const json::Number& type_number = obj["type"];
		int type = type_number.Value();

		const json::String& command = obj["command"];
		cmd::Command *cmd = cmd::get(command.Value());

		// this is dumb.
		wxBitmap *bitmap = cmd->Icon(24);
		wxBitmap icon = bitmap->GetSubBitmap(wxRect(0, 0, bitmap->GetWidth(), bitmap->GetHeight()));

		switch (type) {
			case Toolbar::Standard: {
				toolbar->AddTool(cmd::id(command.Value()),  cmd->StrMenu(), icon, cmd->StrHelp(), wxITEM_NORMAL);
			}
			break;
		}

	} // for index

}

} // namespace toolbar


