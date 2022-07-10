// Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "utils.h"

#include <libaegisub/exception.h>

#include <typeinfo>
#include <vector>
#include <wx/dialog.h>

namespace agi {
struct Context;
}

/// @brief A manager for dialogs
///
/// DialogManager keeps track of modal and modeless dialogs which have been
/// created, so that commands can be send to the appropriate places and so that
/// the same dialog can't be opened twice at once.
class DialogManager {
	using dialog_pair = std::pair<const std::type_info*, wxDialog*>;
	/// Dialogs which currently exist
	std::vector<dialog_pair> created_dialogs;

	/// Close handler which deletes and unregisters closed modeless dialogs
	template <typename Event> void OnClose(Event& evt) {
		evt.Skip();
		Destroy(static_cast<wxWindow*>(evt.GetEventObject()));
	}

	void Destroy(wxWindow* dialog) {
		while(!dialog->IsTopLevel())
			dialog = dialog->GetParent();
		dialog->Destroy();

		for(auto it = created_dialogs.begin(); it != created_dialogs.end(); ++it) {
			if(it->second == dialog) {
				created_dialogs.erase(it);
				return;
			}
		}
	}

	std::vector<dialog_pair>::iterator Find(std::type_info const& type) {
		for(auto it = begin(created_dialogs); it != end(created_dialogs); ++it) {
			if(*it->first == type) return it;
		}
		return end(created_dialogs);
	}

  public:
	/// Show a modeless dialog of the given type, creating it if needed
	/// @tparam DialogType Type of dialog to show
	template <class DialogType> void Show(agi::Context* c) {
		for(auto const& diag : created_dialogs) {
			if(*diag.first == typeid(DialogType)) {
				diag.second->Show();
				diag.second->SetFocus();
			}
		}

		try {
			wxDialog* d = new DialogType(c);
			created_dialogs.emplace_back(&typeid(DialogType), d);
			d->Bind(wxEVT_CLOSE_WINDOW, &DialogManager::OnClose<wxCloseEvent>, this);
			d->Bind(wxEVT_BUTTON, &DialogManager::OnClose<wxCommandEvent>, this, wxID_CANCEL);
			d->Show();
			SetFloatOnParent(d);
		} catch(agi::UserCancelException const&) {
		}
	}

	/// Show a modal dialog of the given type, creating it if needed
	/// @tparam DialogType Type of dialog to show
	template <class DialogType> void ShowModal(agi::Context* c) {
		DialogType diag(c);
		created_dialogs.emplace_back(&typeid(DialogType), &diag);
		try {
			diag.ShowModal();
		} catch(...) {
			created_dialogs.erase(Find(typeid(DialogType)));
			throw;
		}
		created_dialogs.erase(Find(typeid(DialogType)));
	}

	/// Get the dialog of the given type
	/// @tparam DialogType Type of dialog to get
	/// @return A pointer to a DialogType or nullptr if no dialog of the given type has been created
	template <class DialogType> DialogType* Get() const {
		auto it = const_cast<DialogManager*>(this)->Find(typeid(DialogType));
		return it != created_dialogs.end() ? static_cast<DialogType*>(it->second) : nullptr;
	}

	~DialogManager() {
		for(auto const& it : created_dialogs) {
			it.second->Unbind(wxEVT_CLOSE_WINDOW, &DialogManager::OnClose<wxCloseEvent>, this);
			it.second->Unbind(wxEVT_BUTTON, &DialogManager::OnClose<wxCommandEvent>, this,
			                  wxID_CANCEL);
			it.second->Destroy();
		}
	}
};
