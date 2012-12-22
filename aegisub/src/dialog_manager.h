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

/// @file dialog_manager.h
/// @brief Manager for dialogs
/// @ingroup utility

#include <map>
#include <typeinfo>

#include <wx/dialog.h>

#include "utils.h"

/// @brief A manager for dialogs
///
/// DialogManager keeps track of modal and modeless dialogs which have been
/// created, so that commands can be send to the appropriate places and so that
/// the same dialog can't be opened twice at once.
class DialogManager {
	/// Comparer for pointers to std::type_info
	struct type_info_lt {
		bool operator()(const std::type_info *lft, const std::type_info *rgt) const {
			return !!lft->before(*rgt);
		}
	};

	typedef std::map<const std::type_info *, wxDialog *, type_info_lt> DialogMap;

	/// Dialogs which currently exist
	DialogMap created_dialogs;

	/// Close handler which deletes and unregisters closed modeless dialogs
	void OnClose(wxCloseEvent &evt) {
		evt.Skip();
		wxDialog *dialog = static_cast<wxDialog*>(evt.GetEventObject());
		dialog->Destroy();

		for (auto it = created_dialogs.begin(); it != created_dialogs.end(); ++it) {
			if (it->second == dialog) {
				created_dialogs.erase(it);
				return;
			}
		}
	}

public:
	/// Show a modeless dialog of the given type, creating it if needed
	/// @tparam DialogType Type of dialog to show
	template<class DialogType>
	void Show(agi::Context *c) {
		auto it = created_dialogs.find(&typeid(DialogType));

		if (it != created_dialogs.end()) {
			it->second->Show();
			it->second->SetFocus();
		}
		else {
			try {
				wxDialog *d = new DialogType(c);
				created_dialogs[&typeid(DialogType)] = d;
				d->Bind(wxEVT_CLOSE_WINDOW, &DialogManager::OnClose, this);
				d->Show();
				SetFloatOnParent(d);
			}
			catch (agi::UserCancelException const&) { }
		}
	}

	/// Show a modal dialog of the given type, creating it if needed
	/// @tparam DialogType Type of dialog to show
	template<class DialogType>
	void ShowModal(agi::Context *c) {
		DialogType diag(c);
		created_dialogs[&typeid(DialogType)] = &diag;
		try {
			diag.ShowModal();
		}
		catch (...) {
			created_dialogs.erase(&typeid(DialogType));
			throw;
		}
		created_dialogs.erase(&typeid(DialogType));
	}

	/// Get the dialog of the given type
	/// @tparam DialogType Type of dialog to get
	/// @return A pointer to a DialogType or nullptr if no dialog of the given type has been created
	template<class DialogType>
	DialogType *Get() const {
		DialogMap::const_iterator it = created_dialogs.find(&typeid(DialogType));
		return it != created_dialogs.end() ? static_cast<DialogType*>(it->second) : 0;
	}

	~DialogManager() {
		for (auto const& it : created_dialogs) {
			it.second->Unbind(wxEVT_CLOSE_WINDOW, &DialogManager::OnClose, this);
			it.second->Destroy();
		}
		created_dialogs.clear();
	}
};
