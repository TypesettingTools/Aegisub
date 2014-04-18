// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
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

#include "grid_column.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "compat.h"
#include "include/aegisub/context.h"
#include "options.h"
#include "utils.h"
#include "video_context.h"

#include <wx/dc.h>

int WidthHelper::operator()(boost::flyweight<std::string> const& str) {
	if (str.get().empty()) return 0;
	auto it = widths.find(str);
	if (it != end(widths)) return it->second;
	int width = dc.GetTextExtent(to_wx(str)).GetWidth();
	widths[str] = width;
	return width;
}

int WidthHelper::operator()(std::string const& str) {
	return dc.GetTextExtent(to_wx(str)).GetWidth();
}

int WidthHelper::operator()(wxString const& str) {
	return dc.GetTextExtent(str).GetWidth();
}

namespace {
#define COLUMN_HEADER(value) \
	private: const wxString header = value; \
	public: wxString const& Header() const override { return header; }
#define COLUMN_DESCRIPTION(value) \
	private: const wxString description = value; \
	public: wxString const& Description() const override { return description; }

struct GridColumnLineNumber final : GridColumn {
	COLUMN_HEADER(_("#"))
	COLUMN_DESCRIPTION(_("Line Number"))
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context * = nullptr) const override {
		return std::to_wstring(d->Row + 1);
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return helper(Value(&c->ass->Events.back()));
	}
};

template<typename T>
T max_value(T AssDialogueBase::*field, EntryList<AssDialogue> const& lines) {
	T value = 0;
	for (AssDialogue const& line : lines) {
		if (line.*field > value)
			value = line.*field;
	}
	return value;
}

struct GridColumnLayer final : GridColumn {
	COLUMN_HEADER(_("L"))
	COLUMN_DESCRIPTION(_("Layer"))
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return d->Layer ? wxString(std::to_wstring(d->Layer)) : wxString();
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		int max_layer = max_value(&AssDialogue::Layer, c->ass->Events);
		return max_layer == 0 ? 0 : helper(std::to_wstring(max_layer));
	}
};

struct GridColumnStartTime final : GridColumn {
	COLUMN_HEADER(_("Start"))
	COLUMN_DESCRIPTION(_("Start Time"))
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *c) const override {
		if (c)
			return std::to_wstring(c->videoController->FrameAtTime(d->Start, agi::vfr::START));
		return to_wx(d->Start.GetAssFormated());
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool by_frame) const override {
		if (!by_frame)
			return helper(wxS("0:00:00.00"));
		int frame = c->videoController->FrameAtTime(max_value(&AssDialogue::Start, c->ass->Events), agi::vfr::START);
		return helper(std::to_wstring(frame));
	}
};

struct GridColumnEndTime final : GridColumn {
	COLUMN_HEADER(_("End"))
	COLUMN_DESCRIPTION(_("End Time"))
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *c) const override {
		if (c)
			return std::to_wstring(c->videoController->FrameAtTime(d->End, agi::vfr::END));
		return to_wx(d->End.GetAssFormated());
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool by_frame) const override {
		if (!by_frame)
			return helper(wxS("0:00:00.00"));
		int frame = c->videoController->FrameAtTime(max_value(&AssDialogue::End, c->ass->Events), agi::vfr::END);
		return helper(std::to_wstring(frame));
	}
};

template<typename T>
int max_width(T AssDialogueBase::*field, EntryList<AssDialogue> const& lines, WidthHelper &helper) {
	int w = 0;
	for (AssDialogue const& line : lines) {
		auto const& v = line.*field;
		if (v.get().empty()) continue;
		int width = helper(v);
		if (width > w)
			w = width;
	}
	return w;
}

struct GridColumnStyle final : GridColumn {
	COLUMN_HEADER(_("Style"))
	COLUMN_DESCRIPTION(_("Style"))
	bool Centered() const override { return false; }

	wxString Value(const AssDialogue *d, const agi::Context *c) const override {
		return to_wx(d->Style);
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return max_width(&AssDialogue::Style, c->ass->Events, helper);
	}
};

struct GridColumnEffect final : GridColumn {
	COLUMN_HEADER(_("Effect"))
	COLUMN_DESCRIPTION(_("Effect"))
	bool Centered() const override { return false; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return to_wx(d->Effect);
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return max_width(&AssDialogue::Effect, c->ass->Events, helper);
	}
};

struct GridColumnActor final : GridColumn {
	COLUMN_HEADER(_("Actor"))
	COLUMN_DESCRIPTION(_("Actor"))
	bool Centered() const override { return false; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return to_wx(d->Actor);
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return max_width(&AssDialogue::Actor, c->ass->Events, helper);
	}
};

template<int Index>
struct GridColumnMargin : GridColumn {
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return d->Margin[Index] ? wxString(std::to_wstring(d->Margin[Index])) : wxString();
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		int max = 0;
		for (AssDialogue const& line : c->ass->Events) {
			if (line.Margin[Index] > max)
				max = line.Margin[Index];
		}
		return max == 0 ? 0 : helper(std::to_wstring(max));
	}
};

struct GridColumnMarginLeft final : GridColumnMargin<0> {
	COLUMN_HEADER(_("Left"))
	COLUMN_DESCRIPTION(_("Left Margin"))
};

struct GridColumnMarginRight final : GridColumnMargin<0> {
	COLUMN_HEADER(_("Right"))
	COLUMN_DESCRIPTION(_("Right Margin"))
};

struct GridColumnMarginVert final : GridColumnMargin<0> {
	COLUMN_HEADER(_("Vert"))
	COLUMN_DESCRIPTION(_("Vertical Margin"))
};

struct GridColumnCPS final : GridColumn {
	COLUMN_HEADER(_("CPS"))
	COLUMN_DESCRIPTION(_("Characters Per Second"))
	bool Centered() const override { return true; }
	bool RefreshOnTextChange() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		int characters = 0;

		auto const& text = d->Text.get();
		auto pos = begin(text);
		do {
			auto it = std::find(pos, end(text), '{');
			characters += CharacterCount(pos, it, true);
			if (it == end(text)) break;

			pos = std::find(pos, end(text), '}');
			if (pos == end(text)) {
				characters += CharacterCount(it, pos, true);
				break;
			}
		} while (++pos != end(text));

		int duration = d->End - d->Start;
		if (duration <= 0 || characters * 1000 / duration >= 1000)
			return wxS("");
		else
			return std::to_wstring(characters * 1000 / duration);
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return helper(wxS("999"));
	}
};

class GridColumnText final : public GridColumn {
	int override_mode;
	wxString replace_char;

	agi::signal::Connection override_mode_connection;
	agi::signal::Connection replace_char_connection;

public:
	GridColumnText()
	: override_mode(OPT_GET("Subtitle/Grid/Hide Overrides")->GetInt())
	, replace_char(to_wx(OPT_GET("Subtitle/Grid/Hide Overrides Char")->GetString()))
	, override_mode_connection(OPT_SUB("Subtitle/Grid/Hide Overrides",
		[&](agi::OptionValue const& v) { override_mode = v.GetInt(); }))
	, replace_char_connection(OPT_SUB("Subtitle/Grid/Hide Overrides Char",
		[&](agi::OptionValue const& v) { replace_char = to_wx(v.GetString()); }))
	{
	}

	COLUMN_HEADER(_("Text"))
	COLUMN_DESCRIPTION(_("Text"))
	bool Centered() const override { return false; }
	bool CanHide() const override { return false; }
	bool RefreshOnTextChange() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		wxString str;

		// Show overrides
		if (override_mode == 0)
			str = to_wx(d->Text);
		// Hidden overrides
		else {
			str.reserve(d->Text.get().size());
			size_t start = 0, pos;
			while ((pos = d->Text.get().find('{', start)) != std::string::npos) {
				str += to_wx(d->Text.get().substr(start, pos - start));
				if (override_mode == 1)
					str += replace_char;
				start = d->Text.get().find('}', pos);
				if (start != std::string::npos) ++start;
			}
			if (start != std::string::npos)
				str += to_wx(d->Text.get().substr(start));
		}

		// Cap length and set text
		if (str.size() > 512)
			str = str.Left(512) + "...";
		return str;
	}

	int Width(const agi::Context *c, WidthHelper &helper, bool) const override {
		return 5000;
	}
};

template<typename T>
std::unique_ptr<GridColumn> make() {
	return std::unique_ptr<GridColumn>(new T);
}

}

std::vector<std::unique_ptr<GridColumn>> GetGridColumns() {
	std::vector<std::unique_ptr<GridColumn>> ret;
	ret.push_back(make<GridColumnLineNumber>());
	ret.push_back(make<GridColumnLayer>());
	ret.push_back(make<GridColumnStartTime>());
	ret.push_back(make<GridColumnEndTime>());
	ret.push_back(make<GridColumnCPS>());
	ret.push_back(make<GridColumnStyle>());
	ret.push_back(make<GridColumnEffect>());
	ret.push_back(make<GridColumnActor>());
	ret.push_back(make<GridColumnMarginLeft>());
	ret.push_back(make<GridColumnMarginRight>());
	ret.push_back(make<GridColumnMarginVert>());
	ret.push_back(make<GridColumnText>());
	return ret;
}
