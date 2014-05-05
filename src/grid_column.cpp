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
#include "video_context.h"

#include <libaegisub/character_count.h>

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

void GridColumn::UpdateWidth(const agi::Context *c, WidthHelper &helper) {
	if (!visible) {
		width = 0;
		return;
	}

	width = Width(c, helper);
	if (width) // 10 is an arbitrary amount of padding
		width = 10 + std::max(width, helper(Header()));
}

void GridColumn::Paint(wxDC &dc, int x, int y, const AssDialogue *d, const agi::Context *c) const {
	wxString str = Value(d, c);
	if (Centered())
		x += (width - 6 - dc.GetTextExtent(str).GetWidth()) / 2;
	dc.DrawText(str, x + 4, y + 2);
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

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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

	int Width(const agi::Context *c, WidthHelper &helper) const override {
		int max_layer = max_value(&AssDialogue::Layer, c->ass->Events);
		return max_layer == 0 ? 0 : helper(std::to_wstring(max_layer));
	}
};

struct GridColumnTime : GridColumn {
	bool by_frame = false;

	bool Centered() const override { return true; }
	void SetByFrame(bool by_frame) override { this->by_frame = by_frame; }
};

struct GridColumnStartTime final : GridColumnTime {
	COLUMN_HEADER(_("Start"))
	COLUMN_DESCRIPTION(_("Start Time"))

	wxString Value(const AssDialogue *d, const agi::Context *c) const override {
		if (by_frame)
			return std::to_wstring(c->videoController->FrameAtTime(d->Start, agi::vfr::START));
		return to_wx(d->Start.GetAssFormated());
	}

	int Width(const agi::Context *c, WidthHelper &helper) const override {
		if (!by_frame)
			return helper(wxS("0:00:00.00"));
		int frame = c->videoController->FrameAtTime(max_value(&AssDialogue::Start, c->ass->Events), agi::vfr::START);
		return helper(std::to_wstring(frame));
	}
};

struct GridColumnEndTime final : GridColumnTime {
	COLUMN_HEADER(_("End"))
	COLUMN_DESCRIPTION(_("End Time"))

	wxString Value(const AssDialogue *d, const agi::Context *c) const override {
		if (by_frame)
			return std::to_wstring(c->videoController->FrameAtTime(d->End, agi::vfr::END));
		return to_wx(d->End.GetAssFormated());
	}

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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

	int Width(const agi::Context *c, WidthHelper &helper) const override {
		return max_width(&AssDialogue::Actor, c->ass->Events, helper);
	}
};

template<int Index>
struct GridColumnMargin : GridColumn {
	bool Centered() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return d->Margin[Index] ? wxString(std::to_wstring(d->Margin[Index])) : wxString();
	}

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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

struct GridColumnMarginRight final : GridColumnMargin<1> {
	COLUMN_HEADER(_("Right"))
	COLUMN_DESCRIPTION(_("Right Margin"))
};

struct GridColumnMarginVert final : GridColumnMargin<2> {
	COLUMN_HEADER(_("Vert"))
	COLUMN_DESCRIPTION(_("Vertical Margin"))
};

wxColor blend(wxColor fg, wxColor bg, double alpha) {
	return wxColor(
		wxColor::AlphaBlend(fg.Red(), bg.Red(), alpha),
		wxColor::AlphaBlend(fg.Green(), bg.Green(), alpha),
		wxColor::AlphaBlend(fg.Blue(), bg.Blue(), alpha));
}

class GridColumnCPS final : public GridColumn {
	const agi::OptionValue *ignore_whitespace = OPT_GET("Subtitle/Character Counter/Ignore Whitespace");
	const agi::OptionValue *ignore_punctuation = OPT_GET("Subtitle/Character Counter/Ignore Punctuation");
	const agi::OptionValue *cps_warn = OPT_GET("Subtitle/Character Counter/CPS Warning Threshold");
	const agi::OptionValue *cps_error = OPT_GET("Subtitle/Character Counter/CPS Error Threshold");
	const agi::OptionValue *bg_color = OPT_GET("Colour/Subtitle Grid/CPS Error");

public:
	COLUMN_HEADER(_("CPS"))
	COLUMN_DESCRIPTION(_("Characters Per Second"))
	bool Centered() const override { return true; }
	bool RefreshOnTextChange() const override { return true; }

	wxString Value(const AssDialogue *d, const agi::Context *) const override {
		return wxS("");
	}

	int CPS(const AssDialogue *d) const {
		int duration = d->End - d->Start;
		auto const& text = d->Text.get();

		if (duration <= 100 || text.size() > static_cast<size_t>(duration))
			return -1;

		int ignore = agi::IGNORE_BLOCKS;
		if (ignore_whitespace->GetBool())
			ignore |= agi::IGNORE_WHITESPACE;
		if (ignore_punctuation->GetBool())
			ignore |= agi::IGNORE_PUNCTUATION;

		return agi::CharacterCount(text, ignore) * 1000 / duration;
	}

	int Width(const agi::Context *c, WidthHelper &helper) const override {
		return helper(wxS("999"));
	}

	void Paint(wxDC &dc, int x, int y, const AssDialogue *d, const agi::Context *) const {
		int cps = CPS(d);
		if (cps < 0 || cps > 100) return;

		wxString str = std::to_wstring(cps);
		wxSize ext = dc.GetTextExtent(str);
		auto tc = dc.GetTextForeground();

		int cps_min = cps_warn->GetInt();
		int cps_max = std::max<int>(cps_min, cps_error->GetInt());
		if (cps > cps_min) {
			double alpha = std::min((double)(cps - cps_min + 1) / (cps_max - cps_min + 1), 1.0);
			dc.SetBrush(wxBrush(blend(to_wx(bg_color->GetColor()), dc.GetBrush().GetColour(), alpha)));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.DrawRectangle(x, y + 1, width, ext.GetHeight() + 3);
			dc.SetTextForeground(blend(*wxBLACK, tc, alpha));
		}

		x += (width + 2 - ext.GetWidth()) / 2;
		dc.DrawText(str, x, y + 2);
		dc.SetTextForeground(tc);
	}
};

class GridColumnText final : public GridColumn {
	const agi::OptionValue *override_mode;
	wxString replace_char;

	agi::signal::Connection replace_char_connection;

public:
	GridColumnText()
	: override_mode(OPT_GET("Subtitle/Grid/Hide Overrides"))
	, replace_char(to_wx(OPT_GET("Subtitle/Grid/Hide Overrides Char")->GetString()))
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
		int mode = override_mode->GetInt();

		// Show overrides
		if (mode == 0)
			str = to_wx(d->Text);
		// Hidden overrides
		else {
			auto const& text = d->Text.get();
			str.reserve(text.size());
			size_t start = 0, pos;
			while ((pos = text.find('{', start)) != std::string::npos) {
				str += to_wx(text.substr(start, pos - start));
				if (mode == 1)
					str += replace_char;
				start = text.find('}', pos);
				if (start != std::string::npos) ++start;
			}
			if (start != std::string::npos)
				str += to_wx(text.substr(start));
		}

		// Cap length and set text
		if (str.size() > 512)
			str = str.Left(512) + "...";
		return str;
	}

	int Width(const agi::Context *c, WidthHelper &helper) const override {
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
	ret.push_back(make<GridColumnActor>());
	ret.push_back(make<GridColumnEffect>());
	ret.push_back(make<GridColumnMarginLeft>());
	ret.push_back(make<GridColumnMarginRight>());
	ret.push_back(make<GridColumnMarginVert>());
	ret.push_back(make<GridColumnText>());
	return ret;
}
