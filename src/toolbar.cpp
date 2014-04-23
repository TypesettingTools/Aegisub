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

/// @file menutool.cpp
/// @brief Dynamic menu toolbar generator.
/// @ingroup toolbar menu

#include "include/aegisub/toolbar.h"

#include "command/command.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "include/aegisub/hotkey.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "retina_helper.h"
#include "utils.h"

#include <libaegisub/hotkey.h>
#include <libaegisub/json.h>
#include <libaegisub/log.h>
#include <libaegisub/signal.h>
#include <libaegisub/make_unique.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <vector>

#include <wx/frame.h>
#include <wx/toolbar.h>

namespace {
	json::Object const& get_root() {
		static json::Object root;
		if (root.empty()) {
			boost::interprocess::ibufferstream stream((const char *)default_toolbar, sizeof(default_toolbar));
			root = agi::json_util::parse(stream);
		}
		return root;
	}

	class Toolbar final : public wxToolBar {
		/// Window ID of first toolbar control
		static const int TOOL_ID_BASE = 5000;

		/// Toolbar name in config file
		std::string name;
		/// Project context
		agi::Context *context;
		/// Commands for each of the buttons
		std::vector<cmd::Command *> commands;
		/// Hotkey context
		std::string ht_context;

		RetinaHelper retina_helper;

		/// Current icon size
		int icon_size;

		/// Listener for icon size change signal
		agi::signal::Connection icon_size_slot;

		/// Listener for hotkey change signal
		agi::signal::Connection hotkeys_changed_slot;

		/// Enable/disable the toolbar buttons
		void OnIdle(wxIdleEvent &) {
			for (size_t i = 0; i < commands.size(); ++i) {
				if (commands[i]->Type() & cmd::COMMAND_VALIDATE)
					EnableTool(TOOL_ID_BASE + i, commands[i]->Validate(context));
				if (commands[i]->Type() & cmd::COMMAND_TOGGLE || commands[i]->Type() & cmd::COMMAND_RADIO)
					ToggleTool(TOOL_ID_BASE + i, commands[i]->IsActive(context));
			}
		}

		/// Toolbar button click handler
		void OnClick(wxCommandEvent &evt) {
			(*commands[evt.GetId() - TOOL_ID_BASE])(context);
		}

		/// Regenerate the toolbar when the icon size changes
		void OnIconSizeChange(agi::OptionValue const& opt) {
			icon_size = opt.GetInt();
			RegenerateToolbar();
		}

		/// Clear the toolbar and recreate it
		void RegenerateToolbar() {
			Unbind(wxEVT_IDLE, &Toolbar::OnIdle, this);
			ClearTools();
			commands.clear();
			Populate();
		}

		/// Populate the toolbar with buttons
		void Populate() {
			json::Object const& root = get_root();
			auto root_it = root.find(name);
			if (root_it == root.end()) {
				// Toolbar names are all hardcoded so this should never happen
				throw agi::InternalError("Toolbar named " + name + " not found.", nullptr);
			}

			json::Array const& arr = root_it->second;
			commands.reserve(arr.size());
			bool needs_onidle = false;
			bool last_was_sep = false;

			for (json::String const& command_name : arr) {
				if (command_name.empty()) {
					if (!last_was_sep)
						AddSeparator();
					last_was_sep = true;
					continue;
				}

				cmd::Command *command;
				try {
					command = cmd::get(command_name);
				}
				catch (cmd::CommandNotFound const&) {
					LOG_W("toolbar/command/not_found") << "Command '" << command_name << "' not found; skipping";
					continue;
				}

				last_was_sep = false;

				int flags = command->Type();
				wxItemKind kind =
					flags & cmd::COMMAND_RADIO ? wxITEM_RADIO :
					flags & cmd::COMMAND_TOGGLE ? wxITEM_CHECK :
					wxITEM_NORMAL;

				wxBitmap const& bitmap = command->Icon(icon_size);
				AddTool(TOOL_ID_BASE + commands.size(), command->StrDisplay(context), bitmap, GetTooltip(command), kind);

				commands.push_back(command);
				needs_onidle = needs_onidle || flags != cmd::COMMAND_NORMAL;
			}

			// Only bind the update function if there are actually any dynamic tools
			if (needs_onidle) {
				Bind(wxEVT_IDLE, &Toolbar::OnIdle, this);
			}

			Realize();
		}

		wxString GetTooltip(cmd::Command *command) {
			wxString ret = command->StrHelp();

			std::vector<std::string> hotkeys = hotkey::get_hotkey_strs(ht_context, command->name());
			if (!hotkeys.empty())
				ret += to_wx(" (" + boost::join(hotkeys, "/") + ")");

			return ret;
		}

	public:
		Toolbar(wxWindow *parent, std::string name, agi::Context *c, std::string ht_context, bool vertical)
		: wxToolBar(parent, -1, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER | wxTB_FLAT | (vertical ? wxTB_VERTICAL : wxTB_HORIZONTAL))
		, name(std::move(name))
		, context(c)
		, ht_context(std::move(ht_context))
		, retina_helper(parent)
		, icon_size(OPT_GET("App/Toolbar Icon Size")->GetInt())
		, icon_size_slot(OPT_SUB("App/Toolbar Icon Size", &Toolbar::OnIconSizeChange, this))
		, hotkeys_changed_slot(hotkey::inst->AddHotkeyChangeListener(&Toolbar::RegenerateToolbar, this))
		{
			Populate();
			Bind(wxEVT_TOOL, &Toolbar::OnClick, this);
		}

		Toolbar(wxFrame *parent, std::string name, agi::Context *c, std::string ht_context)
		: wxToolBar(parent, -1, wxDefaultPosition, wxDefaultSize, wxTB_FLAT | wxTB_HORIZONTAL)
		, name(std::move(name))
		, context(c)
		, ht_context(std::move(ht_context))
		, retina_helper(parent)
#ifndef __WXMAC__
		, icon_size(OPT_GET("App/Toolbar Icon Size")->GetInt())
		, icon_size_slot(OPT_SUB("App/Toolbar Icon Size", &Toolbar::OnIconSizeChange, this))
#else
		, icon_size(32 * retina_helper.GetScaleFactor())
		, icon_size_slot(retina_helper.AddScaleFactorListener([=](double scale) {
			icon_size = 32 * retina_helper.GetScaleFactor();
			RegenerateToolbar();
		}))
#endif
		, hotkeys_changed_slot(hotkey::inst->AddHotkeyChangeListener(&Toolbar::RegenerateToolbar, this))
		{
			parent->SetToolBar(this);
			Populate();
			Bind(wxEVT_TOOL, &Toolbar::OnClick, this);
		}
	};
}

namespace toolbar {
	void AttachToolbar(wxFrame *frame, std::string const& name, agi::Context *c, std::string const& hotkey) {
		new Toolbar(frame, name, c, hotkey);
	}

	wxToolBar *GetToolbar(wxWindow *parent, std::string const& name, agi::Context *c, std::string const& hotkey, bool vertical) {
		return new Toolbar(parent, name, c, hotkey, vertical);
	}
}
