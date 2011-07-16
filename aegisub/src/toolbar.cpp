// Copyright (c) 2011, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "command/command.h"
#include "include/aegisub/context.h"
#include "include/aegisub/toolbar.h"
#include "libresrc/libresrc.h"
#include "main.h"

#ifndef AGI_PRE
#include <vector>

#include <wx/frame.h>
#include <wx/toolbar.h>
#endif

#include <libaegisub/json.h>
#include <libaegisub/log.h>
#include <libaegisub/signal.h>

namespace {
	json::Object const& get_root() {
		static json::Object root;
		if (root.Empty()) {
			root = agi::json_util::parse(new std::istringstream(GET_DEFAULT_CONFIG(default_toolbar)));
		}
		return root;
	}

	class Toolbar : public wxToolBar {
		/// Window ID of first toolbar control
		static const int TOOL_ID_BASE = 5000;

		/// Toolbar name in config file
		std::string name;
		/// Project context
		agi::Context *context;
		/// Commands for each of the buttons
		std::vector<cmd::Command *> commands;

		/// Listener for icon size change signal
		agi::signal::Connection icon_size_slot;

		/// Enable/disable the toolbar buttons
		void OnIdle(wxIdleEvent &) {
			for (size_t i = 0; i < commands.size(); ++i) {
				if (commands[i]->Type() & cmd::COMMAND_VALIDATE) {
					EnableTool(TOOL_ID_BASE + i, commands[i]->Validate(context));
				}
				if (commands[i]->Type() & cmd::COMMAND_TOGGLE) {
					ToggleTool(TOOL_ID_BASE + i, commands[i]->IsActive(context));
				}
			}
		}

		/// Toolbar button click handler
		void OnClick(wxCommandEvent &evt) {
			(*commands[evt.GetId() - TOOL_ID_BASE])(context);
		}

		/// Clear the toolbar and recreate it with the new icon size
		void OnIconSizeChanged() {
			Unbind(wxEVT_IDLE, &Toolbar::OnIdle, this);
			ClearTools();
			commands.clear();
			Populate();
		}

		/// Populate the toolbar with buttons
		void Populate() {
			json::Object const& root = get_root();
			json::Object::const_iterator it = root.Find(name);
			if (it == root.End()) {
				// Toolbar names are all hardcoded so this should never happen
				throw agi::InternalError("Toolbar named " + name + " not found.", 0);
			}

			int icon_size = OPT_GET("App/Toolbar Icon Size")->GetInt();

			json::Array arr = it->element;
			commands.reserve(arr.Size());
			bool needs_onidle = false;

			for (json::Array::const_iterator it = arr.Begin(); it != arr.End(); ++it) {
				json::String const& command_name = *it;

				if (command_name.Value().empty()) {
					AddSeparator();
				}
				else {
					cmd::Command *command;
					try {
						command = cmd::get(command_name.Value());
					}
					catch (CommandNotFound const&) {
						LOG_W("toolbar/command/not_found") << "Command '" << command_name.Value() << "' not found; skipping";
						continue;
					}

					wxBitmap const& bitmap = command->Icon(icon_size);
					// this hack is needed because ???
					wxBitmap icon = bitmap.GetSubBitmap(wxRect(0, 0, bitmap.GetWidth(), bitmap.GetHeight()));

					int flags = command->Type();
					wxItemKind kind =
						flags & cmd::COMMAND_RADIO ? wxITEM_RADIO :
						flags & cmd::COMMAND_TOGGLE ? wxITEM_CHECK :
						wxITEM_NORMAL;

					AddTool(TOOL_ID_BASE + commands.size(), command->StrMenu(), icon, command->StrHelp(), kind);

					commands.push_back(command);
					needs_onidle = needs_onidle || flags != cmd::COMMAND_NORMAL;
				}
			}

			// Only bind the update function if there are actually any dynamic tools
			if (needs_onidle) {
				Bind(wxEVT_IDLE, &Toolbar::OnIdle, this);
			}

			Realize();
		}
	public:
		Toolbar(wxWindow *parent, std::string const& name, agi::Context *c)
		: wxToolBar(parent, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_HORIZONTAL)
		, name(name)
		, context(c)
		, icon_size_slot(OPT_SUB("App/Toolbar Icon Size", &Toolbar::OnIconSizeChanged, this))
		{
			Populate();
			Bind(wxEVT_COMMAND_TOOL_CLICKED, &Toolbar::OnClick, this);
		}
	};
}

namespace toolbar {
	void AttachToolbar(wxFrame *frame, std::string const& name, agi::Context *c) {
		frame->SetToolBar(new Toolbar(frame, name, c));
	}
}
