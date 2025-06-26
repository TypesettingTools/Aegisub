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
// Aegisub Project http://www.aegisub.org/

#include <memory>
#include <wx/dataview.h>

class HotkeyModelItem;
class HotkeyModelRoot;
class Preferences;

/// @class HotkeyDataViewModel
/// @brief A wxDataViewModel for hotkeys
class HotkeyDataViewModel final : public wxDataViewModel {
	std::unique_ptr<HotkeyModelRoot> root;
	Preferences *parent;
	bool has_pending_changes = false;

	/// Get the real item from the wrapper, or root if it's wrapping nullptr
	const HotkeyModelItem *get(wxDataViewItem const& item) const;
	/// Get the real item from the wrapper, or root if it's wrapping nullptr
	HotkeyModelItem *get(wxDataViewItem const& item);
public:
	HotkeyDataViewModel(Preferences *parent);

	/// Create a new hotkey in the current context
	/// @param item A context or hotkey entry
	/// @return The new hotkey
	wxDataViewItem New(wxDataViewItem item);
	/// Delete the currently selected hotkey
	void Delete(wxDataViewItem const& item);
	/// Update the hotkeys with changes made to the model
	void Apply();

	/// Only display hotkeys containing filter, or all if filter is empty
	void SetFilter(wxString const& filter);

	unsigned int GetChildren(wxDataViewItem const& item, wxDataViewItemArray &children) const override;
	wxDataViewItem GetParent(wxDataViewItem const& item) const override;
	void GetValue(wxVariant &variant, wxDataViewItem const& item, unsigned int col) const override;
	bool IsContainer(wxDataViewItem const& item) const override;
	bool SetValue(wxVariant const& variant, wxDataViewItem const& item, unsigned int col) override;
};
