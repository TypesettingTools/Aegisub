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

#include "hotkey_data_view_model.h"

#include "command/command.h"
#include "compat.h"
#include "include/aegisub/hotkey.h"
#include "preferences.h"

#include <libaegisub/exception.h>
#include <libaegisub/hotkey.h>

#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <list>
#include <map>
#include <vector>

using namespace agi::hotkey;

/// @class HotkeyModelItem
/// @brief A base class for things exposed by HotkeyDataViewModel
class HotkeyModelItem {
protected:
	~HotkeyModelItem() = default;
public:
	virtual unsigned int GetChildren(wxDataViewItemArray &children) const=0;
	virtual wxDataViewItem GetParent() const=0;
	virtual void GetValue(wxVariant &variant, unsigned int col) const=0;
	virtual bool IsContainer() const=0;
	virtual bool SetValue(wxVariant const& variant, unsigned int col)=0;
};

class HotkeyModelRoot;
class HotkeyModelCategory;

/// @class HotkeyModelCombo
/// @brief A single hotkey exposed in the data view
///
/// All actual mutation of hotkeys happens through this class
class HotkeyModelCombo final : public HotkeyModelItem {
	HotkeyModelCategory *parent; ///< The containing category
	Combo combo;				 ///< The actual hotkey
	std::string cmd_name;
	std::string cmd_str;
public:
	HotkeyModelCombo(HotkeyModelCategory *parent, Combo const& combo)
	: parent(parent)
	, combo(combo)
	, cmd_name(combo.CmdName())
	, cmd_str(combo.Str())
	{
	}

	bool IsVisible(std::string const& filter) const {
		return boost::to_lower_copy(cmd_name).find(filter) != std::string::npos
			|| boost::to_lower_copy(cmd_str).find(filter) != std::string::npos;
	}

	void Apply(Hotkey::HotkeyMap *hk_map) {
		if (combo.CmdName().size() || combo.Str().size())
			hk_map->insert(make_pair(combo.CmdName(), combo));
	}

	unsigned int GetChildren(wxDataViewItemArray &) const override { return 0; }
	wxDataViewItem GetParent() const override { return wxDataViewItem(parent); }
	bool IsContainer() const override { return false; }

	void GetValue(wxVariant &variant, unsigned int col) const override {
		if (col == 0)
			variant = to_wx(combo.Str());
		else if (col == 1) {
			wxIcon icon;
			try {
				auto icon_bmp = cmd::get(combo.CmdName())->Icon(16);
				if (icon_bmp.IsOk())
					icon.CopyFromBitmap(icon_bmp);
			}
			catch (agi::Exception const&) {
				// Just use no icon; error is reported in the description column
			}
			variant << wxDataViewIconText(to_wx(combo.CmdName()), icon);
		}
		else if (col == 2) {
			try {
				variant = cmd::get(combo.CmdName())->StrHelp();
			}
			catch (agi::Exception const& e) {
				variant = to_wx(e.GetMessage());
			}
		}
		else
			throw agi::InternalError("HotkeyDataViewModel asked for an invalid column number");
	}

	bool SetValue(wxVariant const& variant, unsigned int col) override {
		if (col == 0) {
			combo = Combo(combo.Context(), combo.CmdName(), from_wx(variant.GetString()));
			cmd_str = combo.Str();
			return true;
		}
		else if (col == 1) {
			wxDataViewIconText text;
			text << variant;
			cmd_name = from_wx(text.GetText());
			combo = Combo(combo.Context(), cmd_name, combo.Str());
			return true;
		}
		return false;
	}
};

/// A hotkey context exposed in the data view
class HotkeyModelCategory final : public HotkeyModelItem {
	std::list<HotkeyModelCombo> children;
	wxDataViewModel *model;
	std::string name;
	wxString translated_name;
	wxDataViewItemArray visible_items;
public:
	HotkeyModelCategory(wxDataViewModel *model, std::string_view name)
	: model(model)
	, name(name)
	, translated_name(wxGetTranslation(to_wx(name)))
	{
	}

	wxDataViewItem AddChild(Combo const& combo) {
		children.emplace_back(this, combo);
		visible_items.push_back(wxDataViewItem(&children.back()));
		model->ItemAdded(wxDataViewItem(this), wxDataViewItem(&children.back()));
		return wxDataViewItem(&children.back());
	}

	void Delete(wxDataViewItem const& item) {
		for (auto it = children.begin(); it != children.end(); ++it) {
			if (&*it == item.GetID()) {
				model->ItemDeleted(wxDataViewItem(this), wxDataViewItem((void*)&*it));
				children.erase(it);
				return;
			}
		}
	}

	void Apply(Hotkey::HotkeyMap *hk_map) {
		for (auto& combo : children)
			combo.Apply(hk_map);
	}

	void SetFilter(std::string const& new_filter) {
		std::vector<HotkeyModelCombo *> old_visible;
		for (auto item : visible_items)
			old_visible.push_back(static_cast<HotkeyModelCombo*>(item.GetID()));
		sort(begin(old_visible), end(old_visible));

		visible_items.clear();

		wxDataViewItemArray added;
		wxDataViewItemArray removed;

		for (auto& combo : children) {
			bool was_visible = binary_search(begin(old_visible), end(old_visible), &combo);
			bool is_visible = combo.IsVisible(new_filter);

			if (is_visible)
				visible_items.push_back(wxDataViewItem(&combo));
			if (was_visible && !is_visible)
				removed.push_back(wxDataViewItem(&combo));
			if (is_visible && !was_visible)
				added.push_back(wxDataViewItem(&combo));
		}

		if (!added.empty())
			model->ItemsAdded(wxDataViewItem(this), added);
		if (!removed.empty())
			model->ItemsDeleted(wxDataViewItem(this), removed);
	}

	std::string const& GetName() const { return name; }

	wxDataViewItem GetParent() const override { return wxDataViewItem(nullptr); }
	bool IsContainer() const override { return true; }
	bool SetValue(wxVariant const&, unsigned int) override { return false; }
	void GetValue(wxVariant &variant, unsigned int col) const override {
		if (col == 1)
			variant << wxDataViewIconText(translated_name);
		else
			variant = translated_name;
	}

	unsigned int GetChildren(wxDataViewItemArray &out) const override {
		out = visible_items;
		return out.size();
	}
};

/// The root containing the hotkey contexts
class HotkeyModelRoot final : public HotkeyModelItem {
	std::list<HotkeyModelCategory> categories;
public:
	HotkeyModelRoot(wxDataViewModel *model) {
		Hotkey::HotkeyMap const& hk_map = hotkey::inst->GetHotkeyMap();
		std::map<std::string, HotkeyModelCategory*, std::less<>> cat_map;

		for (auto const& category : hk_map) {
			auto cat_name = category.second.Context();
			HotkeyModelCategory *cat;
			if (auto it = cat_map.find(cat_name); it != cat_map.end())
				cat = it->second;
			else {
				categories.emplace_back(model, cat_name);
				cat = cat_map.emplace(cat_name, &categories.back()).first->second;
			}

			cat->AddChild(category.second);
		}
	}

	void Apply(Hotkey::HotkeyMap *hk_map) {
		for (auto& category : categories)
			category.Apply(hk_map);
	}

	void SetFilter(std::string const& filter) {
		for (auto& category : categories)
			category.SetFilter(filter);
	}

	wxDataViewItem GetParent() const override { return wxDataViewItem(nullptr); }
	bool IsContainer() const override { return true; }
	bool SetValue(wxVariant const&, unsigned int) override { return false; }
	void GetValue(wxVariant &, unsigned int) const override { }

	unsigned int GetChildren(wxDataViewItemArray &out) const override {
		out.reserve(categories.size());
		for (auto const& category : categories)
			out.push_back(wxDataViewItem((void*)&category));
		return out.size();
	}
};

HotkeyDataViewModel::HotkeyDataViewModel(Preferences *parent)
: root(std::make_unique<HotkeyModelRoot>(this))
, parent(parent)
{
}

const HotkeyModelItem * HotkeyDataViewModel::get(wxDataViewItem const& item) const {
	if (item.IsOk())
		return static_cast<HotkeyModelItem*>(item.GetID());
	return root.get();
}

HotkeyModelItem * HotkeyDataViewModel::get(wxDataViewItem const& item) {
	if (item.IsOk())
		return static_cast<HotkeyModelItem*>(item.GetID());
	return root.get();
}

unsigned int HotkeyDataViewModel::GetChildren(wxDataViewItem const& item, wxDataViewItemArray &children) const {
	return get(item)->GetChildren(children);
}

wxDataViewItem HotkeyDataViewModel::GetParent(wxDataViewItem const& item) const {
	return get(item)->GetParent();
}

void HotkeyDataViewModel::GetValue(wxVariant &variant, wxDataViewItem const& item, unsigned int col) const {
	get(item)->GetValue(variant, col);
}

bool HotkeyDataViewModel::IsContainer(wxDataViewItem const& item) const {
	return get(item)->IsContainer();
}

bool HotkeyDataViewModel::SetValue(wxVariant const& variant, wxDataViewItem const& item, unsigned int col) {
	if (!has_pending_changes) {
		has_pending_changes = true;
		parent->AddPendingChange([=, this] { Apply(); });
	}
	return get(item)->SetValue(variant, col);
}

wxDataViewItem HotkeyDataViewModel::New(wxDataViewItem item) {
	if (!item.IsOk()) return wxDataViewItem();

	if (!IsContainer(item))
		item = GetParent(item);

	HotkeyModelCategory *ctx = static_cast<HotkeyModelCategory*>(item.GetID());
	return ctx->AddChild(Combo(ctx->GetName(), "", ""));
}

void HotkeyDataViewModel::Delete(wxDataViewItem const& item) {
	if (!item.IsOk() || IsContainer(item)) return;

	static_cast<HotkeyModelCategory*>(GetParent(item).GetID())->Delete(item);

	if (!has_pending_changes) {
		has_pending_changes = true;
		parent->AddPendingChange([=, this] { Apply(); });
	}
}

void HotkeyDataViewModel::Apply() {
	Hotkey::HotkeyMap hk_map;
	root->Apply(&hk_map);
	hotkey::inst->SetHotkeyMap(hk_map);
	has_pending_changes = false;
}

void HotkeyDataViewModel::SetFilter(wxString const& filter) {
	auto str = from_wx(filter);
	boost::to_lower(str);
	root->SetFilter(str);
}
