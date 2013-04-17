// Copyright (c) 2005, Dan Donovan (Dansolo)
// Copyright (c) 2009, Niels Martin Hansen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Aegisub Project http://www.aegisub.org/

/// @file dialog_kara_timing_copy.cpp
/// @brief Karaoke timing copier dialogue box and logic
/// @ingroup tools_ui kara_timing_copy
///

#include "config.h"

#include "dialog_kara_timing_copy.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "kana_table.h"
#include "libresrc/libresrc.h"
#include "options.h"
#include "selection_controller.h"
#include "utils.h"

#include <deque>

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dcclient.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/string.h>

#define TEXT_LABEL_SOURCE _("Source: ")
#define TEXT_LABEL_DEST _("Dest: ")

// IDs
enum {
	BUTTON_KTSTART = 2500,
	BUTTON_KTLINK,
	BUTTON_KTUNLINK,
	BUTTON_KTSKIPSOURCE,
	BUTTON_KTSKIPDEST,
	BUTTON_KTGOBACK,
	BUTTON_KTACCEPT
};

class KaraokeLineMatchDisplay : public wxControl {
	typedef AssKaraoke::Syllable MatchSyllable;

	struct MatchGroup {
		std::vector<MatchSyllable> src;
		std::string dst;
		int last_render_width;
		MatchGroup() : last_render_width(0) { }
	};

	std::vector<MatchGroup> matched_groups;
	std::deque<MatchSyllable> unmatched_source;
	std::string unmatched_destination;

	int last_total_matchgroup_render_width;

	size_t source_sel_length;
	size_t destination_sel_length;

	void OnPaint(wxPaintEvent &event);

	const wxString label_source, label_destination;

public:
	/// Start processing a new line pair
	void SetInputData(AssDialogue *src, AssDialogue *dst);
	/// Build and return the output line from the matched syllables
	std::string GetOutputLine() const;

	/// Number of syllables not yet matched from source
	size_t GetRemainingSource() const { return unmatched_source.size(); }
	/// Number of characters not yet matched from destination
	size_t GetRemainingDestination() const { return unmatched_destination.size(); }

	// Adjust source and destination match lengths
	void IncreaseSourceMatch();
	void DecreaseSourceMatch();
	void IncreseDestinationMatch();
	void DecreaseDestinationMatch();
	/// Attempt to treat source as Japanese romaji, destination as Japanese kana+kanji, and make an automatic match
	void AutoMatchJapanese();
	/// Accept current selection and save match
	bool AcceptMatch();
	/// Undo last match, adding it back to the unmatched input
	bool UndoMatch();


	KaraokeLineMatchDisplay(wxWindow *parent);

	wxSize GetBestSize() const;
};

KaraokeLineMatchDisplay::KaraokeLineMatchDisplay(wxWindow *parent)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxWANTS_CHARS)
, label_source(TEXT_LABEL_SOURCE)
, label_destination(TEXT_LABEL_DEST)
{
	InheritAttributes();
	SetInputData(0, 0);

	wxSize best_size = GetBestSize();
	SetMaxSize(wxSize(-1, best_size.GetHeight()));
	SetMinSize(best_size);

	Bind(wxEVT_SET_FOCUS, std::bind(&wxControl::Refresh, this, true, nullptr));
	Bind(wxEVT_KILL_FOCUS, std::bind(&wxControl::Refresh, this, true, nullptr));
	Bind(wxEVT_PAINT, &KaraokeLineMatchDisplay::OnPaint, this);
}

wxSize KaraokeLineMatchDisplay::GetBestSize() const
{
	int w_src, h_src, w_dst, h_dst;
	GetTextExtent(label_source, &w_src, &h_src);
	GetTextExtent(label_destination, &w_dst, &h_dst);

	int min_width = std::max(w_dst, w_src);

	// Magic number 7:
	// 1 pixel for top black border, 1 pixel white top border in first row, 1 pixel white bottom padding in first row
	// 1 pixel middle border, 1 pixel top and 1 pixel bottom padding in second row, 1 pixel bottom border
	return wxSize(min_width * 2, h_src + h_dst + 7);
}

int DrawBoxedText(wxDC &dc, const std::string &txt, int x, int y)
{
	int tw, th;
	// Assume the pen, brush and font properties have already been set in the DC.
	// Return the advance width, including box margins, borders etc

	if (txt.empty())
	{
		// Empty string gets special handling:
		// The box is drawn in shorter width, to emphasize it's empty
		// GetTextExtent has to be called with a non-empty string, otherwise it returns the wrong height
		dc.GetTextExtent(" ", &tw, &th);
		dc.DrawRectangle(x, y-2, 4, th+4);
		return 3;
	}
	else
	{
		wxString wxtxt(to_wx(txt));
		dc.GetTextExtent(wxtxt, &tw, &th);
		dc.DrawRectangle(x, y-2, tw+4, th+4);
		dc.DrawText(wxtxt, x+2, y);
		return tw+3;
	}
}

void KaraokeLineMatchDisplay::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);

	wxColour outer_text(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	wxColour outer_back(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	wxColour outer_frame(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
	wxColour inner_back(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	wxColour inner_text(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	wxColour sel_back(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	wxColour sel_text(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));

	// Y coordinates of the top and bottom lines
	int y_line1, y_line2, y_line3;
	// Next X coordinate to draw a matched syllable at
	int next_x;

	wxSize client_size = GetClientSize();

	// Calculate the text line positions
	{
		int x, y, x2;
		GetTextExtent(label_source, &x, &y);
		y_line1 = 2;
		y_line2 = y_line1 + y + 3;
		y_line3 = y_line2 + y + 3;
		GetTextExtent(label_destination, &x2, &y);
		next_x = (x2 > x) ? x2 : x;
		next_x += 3;
	}

	// Paint the labels
	dc.SetTextBackground(outer_back);
	if (FindFocus() == this)
		dc.SetTextForeground(outer_text);
	else
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
	dc.SetFont(GetFont());
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.DrawText(label_source, wxPoint(0, y_line1));
	dc.DrawText(label_destination, wxPoint(0, y_line2));

	// Horizontal lines through the width of the control
	dc.SetPen(wxPen(outer_frame));
	dc.DrawLine(next_x-1, y_line1-2, client_size.GetWidth(), y_line1-2);
	dc.DrawLine(next_x-1, y_line2-2, client_size.GetWidth(), y_line2-2);
	dc.DrawLine(next_x-1, y_line3-2, client_size.GetWidth(), y_line3-2);

	// Draw matched groups
	int this_total_matchgroup_render_width = 0;
	bool scroll_arrows_drawn = false;
	for (auto& grp : matched_groups)
	{
		int prev_x = next_x;

		// Skip groups that would cause the input part to be too far right
		this_total_matchgroup_render_width += grp.last_render_width;
		if (last_total_matchgroup_render_width - this_total_matchgroup_render_width > client_size.x / 2)
		{
			// If we're skipping some syllables, show an arrow as feedback that something is scrolled off
			if (!scroll_arrows_drawn)
			{
				dc.SetBrush(wxBrush(outer_frame));
				wxPoint triangle[3];
				int height = y_line2 - y_line1;
				triangle[0] = wxPoint(next_x-3, height/2);
				triangle[1] = wxPoint(next_x-3+height/2, 0);
				triangle[2] = wxPoint(next_x-3+height/2, height);
				dc.DrawPolygon(3, triangle, 0, 0);
				dc.DrawPolygon(3, triangle, 0, height);
				next_x += height/2 - 4;
			}
			scroll_arrows_drawn = true;
			continue;
		}

		dc.SetPen(wxPen(outer_frame));
		dc.SetBrush(wxBrush(inner_back));
		dc.SetTextBackground(inner_back);
		dc.SetTextForeground(inner_text);

		// Matched source syllables
		int syl_x = next_x;
		for (auto const& syl : grp.src)
			syl_x += DrawBoxedText(dc, syl.text, syl_x, y_line1);

		// Matched destination text
		{
			const int adv = DrawBoxedText(dc, grp.dst, next_x, y_line2);

			// Adjust next_x here while we have the text_w
			next_x = syl_x > next_x + adv ? syl_x : next_x + adv;
		}

		// Spacing between groups
		next_x += 3;
		grp.last_render_width = next_x - prev_x;
	}

	last_total_matchgroup_render_width = this_total_matchgroup_render_width;

	// Spacing between grouped and ungrouped parts
	next_x += 4;

	// Remaining source syllables
	int syl_x = next_x;
	// Start out with highlight colours
	dc.SetTextBackground(sel_back);
	dc.SetTextForeground(sel_text);
	dc.SetBrush(wxBrush(sel_back));
	for (size_t j = 0; j < unmatched_source.size(); ++j)
	{
		// Switch to regular colours after all selected syllables
		if (j == source_sel_length)
		{
			dc.SetTextBackground(inner_back);
			dc.SetTextForeground(inner_text);
			dc.SetBrush(wxBrush(inner_back));
		}

		syl_x += DrawBoxedText(dc, unmatched_source[j].text, syl_x, y_line1);
	}

	// Remaining destination
	if (!unmatched_destination.empty())
	{
		dc.SetTextBackground(sel_back);
		dc.SetTextForeground(sel_text);
		dc.SetBrush(wxBrush(sel_back));
		next_x += DrawBoxedText(dc, unmatched_destination.substr(0, destination_sel_length), next_x, y_line2);
	}

	if (destination_sel_length < unmatched_destination.size())
	{
		dc.SetTextBackground(inner_back);
		dc.SetTextForeground(inner_text);
		dc.SetBrush(wxBrush(inner_back));
		DrawBoxedText(dc, unmatched_destination.substr(destination_sel_length), next_x, y_line2);
	}
}

void KaraokeLineMatchDisplay::SetInputData(AssDialogue *src, AssDialogue *dst)
{
	last_total_matchgroup_render_width = 0;

	matched_groups.clear();

	unmatched_source.clear();
	source_sel_length = 0;
	if (src)
	{
		AssKaraoke kara(src);
		copy(kara.begin(), kara.end(), back_inserter(unmatched_source));
		source_sel_length = 1;
	}

	unmatched_destination = dst ? dst->GetStrippedText() : "";
	destination_sel_length = std::max<size_t>(1, unmatched_destination.size());

	Refresh(true);
}

std::string KaraokeLineMatchDisplay::GetOutputLine() const
{
	std::string res;

	for (auto const& match : matched_groups)
	{
		int duration = 0;
		for (auto const& syl : match.src)
			duration += syl.duration;
		res += "{\\k" + std::to_string(duration / 10) + "}" + match.dst;
	}

	return res;
}

void KaraokeLineMatchDisplay::IncreaseSourceMatch()
{
	source_sel_length = std::min(source_sel_length + 1, GetRemainingSource());
	Refresh(true);
}

void KaraokeLineMatchDisplay::DecreaseSourceMatch()
{
	source_sel_length = std::max<size_t>(source_sel_length, 1) - 1;
	Refresh(true);
}

void KaraokeLineMatchDisplay::IncreseDestinationMatch()
{
	destination_sel_length = std::min(destination_sel_length + 1, GetRemainingDestination());
	Refresh(true);
}

void KaraokeLineMatchDisplay::DecreaseDestinationMatch()
{
	destination_sel_length = std::max<size_t>(destination_sel_length, 1) - 1;
	Refresh(true);
}

/// Kana interpolation, in characters, unset to disable
#define KANA_SEARCH_DISTANCE 3

void KaraokeLineMatchDisplay::AutoMatchJapanese()
{
	if (unmatched_source.size() < 1) return;

	// Quick escape: If there's no destination left, take all remaining source.
	// (Usually this means the user made a mistake.)
	if (unmatched_destination.empty())
	{
		source_sel_length = unmatched_source.size();
		destination_sel_length = 0;
		return;
	}

	// We'll first see if we can do something with the first unmatched source syllable
	wxString src(to_wx(unmatched_source[0].text).Lower());
	wxString dst(to_wx(unmatched_destination));
	source_sel_length = 1; // we're working on the first, assume it was matched
	destination_sel_length = 0;

	// Quick escape: If the source syllable is empty, return with first source syllable and empty destination
	if (src.empty()) return;

	// Try to match the next source syllable against the destination.  Do it
	// "inverted": try all kana from the table and prefix-match them against
	// the destination, then if it matches a prefix, try to match the hepburn
	// for it agast the source; eat if it matches.  Keep trying to match as
	// long as there's text left in the source syllable or matching fails.
	while (src.size() > 0)
	{
		wxString dst_hira_rest, dst_kata_rest, src_rest;
		bool matched = false;
		for (const KanaEntry *ke = KanaTable; ke->hiragana; ++ke)
		{
			if (src.StartsWith(ke->hepburn, &src_rest))
			{
				bool hira_matches = dst.StartsWith(ke->hiragana, &dst_hira_rest) && *ke->hiragana;
				bool kata_matches = dst.StartsWith(ke->katakana, &dst_kata_rest);

				if (hira_matches || kata_matches)
				{
					matched = true;
					src = src_rest;
					dst = hira_matches ? dst_hira_rest : dst_kata_rest;
					destination_sel_length += wcslen(hira_matches ? ke->hiragana : ke->katakana);
					break;
				}
			}
		}
		if (!matched) break;
	}

	// The source might be empty now: That's good!
	// That means we managed to match it all against destination text
	if (src.empty()) return;
	// destination_sel_length already has the appropriate value
	// and source_sel_length was already 1

	// Now the source syllable might consist of just whitespace.
	// Eat all whitespace at the start of the destination.
	if (StringEmptyOrWhitespace(src))
	{
		wxString str(to_wx(unmatched_destination.substr(destination_sel_length)));
		destination_sel_length += std::distance(str.begin(), std::find_if_not(str.begin(), str.end(), IsWhitespace));
		// Now we've eaten all spaces in the destination as well
		// so the selection lengths should be good
		return;
	}

	// If there's just one character left in the destination at this point,
	// (and the source doesn't begin with space syllables, see test above)
	// assume it's safe to take all remaining source to match the single
	// remaining destination.
	if (unmatched_destination.size() == 1)
	{
		source_sel_length = unmatched_source.size();
		destination_sel_length = 1;
		return;
	}

#ifdef KANA_SEARCH_DISTANCE
	// Try to look up to KANA_SEARCH_DISTANCE characters ahead in destination,
	// see if any of those are recognised kana. If there are any within the
	// range, see if it matches a following syllable, at most 5 source
	// syllables per character in source we're ahead.
	// The number 5 comes from the kanji with the longest readings:
	// 'uketamawa.ru' and 'kokorozashi' which each have a reading consisting of
	// five kana.
	// Only match the found kana in destination against the beginning of source
	// syllables, not the middle of them.
	// If a match is found, make a guess at how much source and destination
	// should be selected based on the distances it was found at.
	dst = to_wx(unmatched_destination);
	for (size_t lookahead = 0; lookahead < KANA_SEARCH_DISTANCE; ++lookahead)
	{
		// Eat dst at the beginning, don't test for the first character being kana
		dst = dst.Mid(1);
		// Find a position where hiragana or katakana matches
		wxString matched_roma;
		wxString matched_kana;
		for (const KanaEntry *ke = KanaTable; ke->hiragana; ++ke)
		{
			if (*ke->hiragana && dst.StartsWith(ke->hiragana))
			{
				matched_roma = ke->hepburn;
				matched_kana = ke->hiragana;
				break;
			}
			if (*ke->katakana && dst.StartsWith(ke->katakana))
			{
				matched_roma = ke->hepburn;
				matched_kana = ke->katakana;
				break;
			}
		}
		// If we didn't match any kana against dst, move to next char in dst
		if (!matched_kana)
			continue;
		// Otherwise look for a match for the romaji
		// For the magic number 5, see big comment block above
		int src_lookahead_max = (lookahead+1)*5;
		int src_lookahead_pos = 0;
		for (auto const& syl : unmatched_source)
		{
			// Check if we've gone too far ahead in the source
			if (src_lookahead_pos++ >= src_lookahead_max) break;
			// Otherwise look for a match
			if (to_wx(syl.text).StartsWith(matched_roma))
			{
				// Yay! Time to interpolate.
				// Special case: If the last source syllable before the matching one is
				// empty or contains just whitespace, don't include that one.
				if (src_lookahead_pos > 1 && StringEmptyOrWhitespace(to_wx(unmatched_source[src_lookahead_pos-2].text)))
					src_lookahead_pos -= 1;
				// Special case: Just one source syllable matching, pick all destination found
				if (src_lookahead_pos == 2)
				{
					source_sel_length = 1;
					destination_sel_length = lookahead+1;
					return;
				}
				// Otherwise try to split the eaten source syllables evenly between the eaten
				// destination characters, and do a regular rounding.
				float src_per_dst = (float)(src_lookahead_pos-1)/(float)(lookahead+1);
				source_sel_length = (int)(src_per_dst + 0.5);
				destination_sel_length = 1;
				return;
			}
		}
	}
#endif

	// Okay so we didn't match anything. Aww.
	// Just fail...
	// We know from earlier that we do have both some source and some destination.
	source_sel_length = 1;
	destination_sel_length = 1;
	return;
}

bool KaraokeLineMatchDisplay::AcceptMatch()
{
	// Completely empty match
	if (source_sel_length == 0 && destination_sel_length == 0) return false;

	MatchGroup match;

	assert(source_sel_length <= unmatched_source.size());
	copy(unmatched_source.begin(), unmatched_source.begin() + source_sel_length, back_inserter(match.src));
	unmatched_source.erase(unmatched_source.begin(), unmatched_source.begin() + source_sel_length);
	source_sel_length = 0;

	assert(destination_sel_length <= unmatched_destination.size());
	match.dst = unmatched_destination.substr(0, destination_sel_length);
	unmatched_destination.erase(0, destination_sel_length);
	destination_sel_length = 0;

	matched_groups.emplace_back(std::move(match));

	IncreaseSourceMatch();
	IncreseDestinationMatch();
	Refresh(true);

	return true;
}

bool KaraokeLineMatchDisplay::UndoMatch()
{
	if (matched_groups.empty())
		return false;

	MatchGroup &group = matched_groups.back();

	source_sel_length = group.src.size();
	destination_sel_length = group.dst.size();

	copy(group.src.rbegin(), group.src.rend(), front_inserter(unmatched_source));
	group.src.clear();

	unmatched_destination = group.dst + unmatched_destination;

	matched_groups.pop_back();

	Refresh(true);

	return true;
}

DialogKanjiTimer::DialogKanjiTimer(agi::Context *c)
: wxDialog(c->parent, -1, _("Kanji timing"))
, subs(c->ass)
, currentSourceLine(nullptr)
, currentDestinationLine(nullptr)
{
	SetIcon(GETICON(kara_timing_copier_16));

	wxSizer *DisplayBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Text"));
	wxSizer *StylesBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Styles"));
	wxFlexGridSizer *StylesGridSizer = new wxFlexGridSizer(2, 2, 6, 6);
	wxSizer *HelpBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Shortcut Keys"));
	wxSizer *ButtonsBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Commands"));
	wxSizer *MainStackSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *BottomShelfSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *BottomLeftStackSizer = new wxBoxSizer(wxVERTICAL);

	display = new KaraokeLineMatchDisplay(this);

	//Checkbox
	Interpolate = new wxCheckBox(this,-1,_("Attempt to &interpolate kanji."),wxDefaultPosition,wxDefaultSize,wxALIGN_LEFT);
	Interpolate->SetValue(OPT_GET("Tool/Kanji Timer/Interpolation")->GetBool());

	wxArrayString styles = to_wx(subs->GetStyles());
	SourceStyle = new wxComboBox(this, -1, "", wxDefaultPosition, wxSize(160, -1), styles, wxCB_READONLY);
	DestStyle = new wxComboBox(this, -1, "", wxDefaultPosition, wxSize(160, -1), styles, wxCB_READONLY);

	wxStaticText *ShortcutKeys = new wxStaticText(this,-1,_("When the destination textbox has focus, use the following keys:\n\nRight Arrow: Increase dest. selection length\nLeft Arrow: Decrease dest. selection length\nUp Arrow: Increase source selection length\nDown Arrow: Decrease source selection length\nEnter: Link, accept line when done\nBackspace: Unlink last"));

	//Buttons
	wxButton *Start = new wxButton(this,BUTTON_KTSTART,_("S&tart!"));
	wxButton *Link = new wxButton(this,BUTTON_KTLINK,_("&Link"));
	wxButton *Unlink = new wxButton(this,BUTTON_KTUNLINK,_("&Unlink"));
	wxButton *SkipSourceLine = new wxButton(this,BUTTON_KTSKIPSOURCE,_("Skip &Source Line"));
	wxButton *SkipDestLine = new wxButton(this,BUTTON_KTSKIPDEST,_("Skip &Dest Line"));
	wxButton *GoBackLine = new wxButton(this,BUTTON_KTGOBACK,_("&Go Back a Line"));
	wxButton *AcceptLine = new wxButton(this,BUTTON_KTACCEPT,_("&Accept Line"));
	wxButton *CloseKT = new wxButton(this,wxID_CLOSE,_("&Close"));

	//Frame: Text
	DisplayBoxSizer->Add(display, 0, wxEXPAND|wxALL, 6);
	DisplayBoxSizer->Add(Interpolate, 0, wxEXPAND|wxALL, 6);
	//Frame: Styles
	StylesGridSizer->Add(new wxStaticText(this, -1, TEXT_LABEL_SOURCE), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	StylesGridSizer->Add(SourceStyle, 1, wxEXPAND);
	StylesGridSizer->Add(new wxStaticText(this, -1, TEXT_LABEL_DEST), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	StylesGridSizer->Add(DestStyle, 1, wxEXPAND);
	StylesBoxSizer->Add(StylesGridSizer, 1, wxEXPAND|wxALL, 6);
	//Frame: Shortcut Keys
	HelpBoxSizer->Add(ShortcutKeys, 1, wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 6);
	//Frame: Commands
	ButtonsBoxSizer->AddStretchSpacer(1);
	ButtonsBoxSizer->Add(Start, 0, wxEXPAND|wxALL, 6);
	ButtonsBoxSizer->Add(Link, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->Add(Unlink, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->Add(SkipSourceLine, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->Add(SkipDestLine, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->Add(GoBackLine, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->Add(AcceptLine, 0, wxEXPAND|(wxALL&~wxTOP), 6);
	ButtonsBoxSizer->AddStretchSpacer(1);

	// Button sizer
	wxStdDialogButtonSizer *buttonSizer = new wxStdDialogButtonSizer();
	buttonSizer->AddButton(new HelpButton(this,"Kanji Timer"));
	buttonSizer->SetAffirmativeButton(CloseKT);
	buttonSizer->Realize();

	// Layout it all
	BottomLeftStackSizer->Add(StylesBoxSizer, 0, wxEXPAND|wxBOTTOM, 6);
	BottomLeftStackSizer->Add(HelpBoxSizer, 1, wxEXPAND, 6);
	BottomShelfSizer->Add(BottomLeftStackSizer, 1, wxEXPAND|wxRIGHT, 6);
	BottomShelfSizer->Add(ButtonsBoxSizer, 0, wxEXPAND, 6);
	MainStackSizer->Add(DisplayBoxSizer, 0, wxEXPAND|wxALL, 6);
	MainStackSizer->Add(BottomShelfSizer, 1, wxEXPAND|wxLEFT|wxRIGHT, 6);
	MainStackSizer->Add(buttonSizer, 0, wxEXPAND|wxALL, 6);

	SetSizerAndFit(MainStackSizer);
	CenterOnParent();

	display->Bind(wxEVT_KEY_DOWN, &DialogKanjiTimer::OnKeyDown, this);
}

BEGIN_EVENT_TABLE(DialogKanjiTimer,wxDialog)
	EVT_BUTTON(wxID_CLOSE,DialogKanjiTimer::OnClose)
	EVT_BUTTON(BUTTON_KTSTART,DialogKanjiTimer::OnStart)
	EVT_BUTTON(BUTTON_KTLINK,DialogKanjiTimer::OnLink)
	EVT_BUTTON(BUTTON_KTUNLINK,DialogKanjiTimer::OnUnlink)
	EVT_BUTTON(BUTTON_KTSKIPSOURCE,DialogKanjiTimer::OnSkipSource)
	EVT_BUTTON(BUTTON_KTSKIPDEST,DialogKanjiTimer::OnSkipDest)
	EVT_BUTTON(BUTTON_KTGOBACK,DialogKanjiTimer::OnGoBack)
	EVT_BUTTON(BUTTON_KTACCEPT,DialogKanjiTimer::OnAccept)
	EVT_KEY_DOWN(DialogKanjiTimer::OnKeyDown)
END_EVENT_TABLE()

void DialogKanjiTimer::OnClose(wxCommandEvent &) {
	OPT_SET("Tool/Kanji Timer/Interpolation")->SetBool(Interpolate->IsChecked());

	for (auto& line : LinesToChange)
		line.first->Text = line.second;

	if (LinesToChange.size()) {
		subs->Commit(_("kanji timing"), AssFile::COMMIT_DIAG_TEXT);
		LinesToChange.clear();
	}
	Close();
}

void DialogKanjiTimer::OnStart(wxCommandEvent &) {
	if (SourceStyle->GetValue().empty() || DestStyle->GetValue().empty())
		wxMessageBox(_("Select source and destination styles first."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else if (SourceStyle->GetValue() == DestStyle->GetValue())
		wxMessageBox(_("The source and destination styles must be different."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else {
		currentSourceLine = FindNextStyleMatch(&*subs->Line.begin(), from_wx(SourceStyle->GetValue()));
		currentDestinationLine = FindNextStyleMatch(&*subs->Line.begin(), from_wx(DestStyle->GetValue()));
		ResetForNewLine();
	}
	LinesToChange.clear();
}

void DialogKanjiTimer::OnLink(wxCommandEvent &) {
	if (display->AcceptMatch())
		TryAutoMatch();
	else
		wxBell();
}

void DialogKanjiTimer::OnUnlink(wxCommandEvent &) {
	if (!display->UndoMatch())
		wxBell();
	// Don't auto-match here, undoing sets the selection to the undone match
}

void DialogKanjiTimer::OnSkipSource(wxCommandEvent &) {
	currentSourceLine = FindNextStyleMatch(currentSourceLine, from_wx(SourceStyle->GetValue()));
	ResetForNewLine();
}

void DialogKanjiTimer::OnSkipDest(wxCommandEvent &) {
	currentDestinationLine = FindNextStyleMatch(currentDestinationLine, from_wx(DestStyle->GetValue()));
	ResetForNewLine();
}

void DialogKanjiTimer::OnGoBack(wxCommandEvent &) {
	if (LinesToChange.size())
		LinesToChange.pop_back(); //If we go back, then take out the modified line we saved.

	currentSourceLine = FindPrevStyleMatch(currentSourceLine, from_wx(SourceStyle->GetValue()));
	currentDestinationLine = FindPrevStyleMatch(currentDestinationLine, from_wx(DestStyle->GetValue()));
	ResetForNewLine();
}

void DialogKanjiTimer::OnAccept(wxCommandEvent &) {
	if (!currentDestinationLine) return;

	if (display->GetRemainingSource() > 0)
		wxMessageBox(_("Group all of the source text."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else if (AssDialogue *destLine = dynamic_cast<AssDialogue*>(currentDestinationLine)) {
		LinesToChange.push_back(std::make_pair(destLine, display->GetOutputLine()));

		currentSourceLine = FindNextStyleMatch(currentSourceLine, from_wx(SourceStyle->GetValue()));
		currentDestinationLine = FindNextStyleMatch(currentDestinationLine, from_wx(DestStyle->GetValue()));
		ResetForNewLine();
	}
}

void DialogKanjiTimer::OnKeyDown(wxKeyEvent &event) {
	wxCommandEvent evt;
	switch(event.GetKeyCode()) {
		case WXK_ESCAPE :
			OnClose(evt);
			break;
		case WXK_BACK :
			OnUnlink(evt);
			break;
		case WXK_RIGHT : //inc dest selection len
			display->IncreseDestinationMatch();
			break;
		case WXK_LEFT : //dec dest selection len
			display->DecreaseDestinationMatch();
			break;
		case WXK_UP : //inc source selection len
			display->IncreaseSourceMatch();
			break;
		case WXK_DOWN : //dec source selection len
			display->DecreaseSourceMatch();
			break;
		case WXK_RETURN :
			if (display->GetRemainingSource() == 0 && display->GetRemainingDestination() == 0)
				OnAccept(evt);
			else
				OnLink(evt);
			break;
		default :
			event.Skip();
	}
}

void DialogKanjiTimer::ResetForNewLine()
{
	AssDialogue *src = nullptr;
	AssDialogue *dst = nullptr;

	if (currentSourceLine)
		src = dynamic_cast<AssDialogue*>(currentSourceLine);
	if (currentDestinationLine)
		dst = dynamic_cast<AssDialogue*>(currentDestinationLine);

	if (!src || !dst)
	{
		src = dst = nullptr;
		wxBell();
	}

	display->SetInputData(src, dst);

	TryAutoMatch();

	display->SetFocus();
}

void DialogKanjiTimer::TryAutoMatch()
{
	if (Interpolate->GetValue())
		display->AutoMatchJapanese();
}

template<typename Iterator>
static AssEntry *find_next(Iterator from, Iterator to, std::string const& style_name) {
	for (++from; from != to; ++from)
	{
		AssDialogue *dlg = dynamic_cast<AssDialogue*>(&*from);
		if (dlg && dlg->Style == style_name)
			return dlg;
	}

	return nullptr;
}

AssEntry *DialogKanjiTimer::FindNextStyleMatch(AssEntry *search_from, const std::string &search_style)
{
	if (!search_from) return search_from;
	return find_next(subs->Line.iterator_to(*search_from), subs->Line.end(), search_style);
}

AssEntry *DialogKanjiTimer::FindPrevStyleMatch(AssEntry *search_from, const std::string &search_style)
{
	if (!search_from) return search_from;
	return find_next(EntryList::reverse_iterator(subs->Line.iterator_to(*search_from)), subs->Line.rend(), search_style);
}
