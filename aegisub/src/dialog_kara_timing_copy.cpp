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
//
// $Id$

/// @file dialog_kara_timing_copy.cpp
/// @brief Karaoke timing copier dialogue box and logic
/// @ingroup tools_ui kara_timing_copy
///


///////////
// Headers
#include "config.h"

#ifndef AGI_PRE
#include <deque>
#include <vector>

#include <wx/string.h>
#endif

#include "ass_file.h"
#include "ass_karaoke.h"
#include "ass_override.h"
#include "ass_style.h"
#include "dialog_kara_timing_copy.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"
#include "subs_grid.h"
#include "utils.h"
#include "validators.h"
#include "video_context.h"


/// DOCME
#define TEXT_LABEL_SOURCE _("Source: ")

/// DOCME
#define TEXT_LABEL_DEST _("Dest: ")

// IDs
enum {
	BUTTON_KTSTART = 2500,
	BUTTON_KTLINK,
	BUTTON_KTUNLINK,
	BUTTON_KTSKIPSOURCE,
	BUTTON_KTSKIPDEST,
	BUTTON_KTGOBACK,
	BUTTON_KTACCEPT,
	TEXT_SOURCE,
	TEXT_DEST
};

/// DOCME
/// @class KaraokeLineMatchDisplay
/// @brief DOCME
///
/// DOCME
class KaraokeLineMatchDisplay : public wxControl {

	/// DOCME
	struct MatchSyllable {

		/// DOCME
		size_t dur;

		/// DOCME
		wxString text;

		/// @brief DOCME
		/// @param _dur  
		/// @param _text 
		///
		MatchSyllable(int _dur, const wxString &_text) : dur(_dur), text(_text) { }
	};

	/// DOCME
	struct MatchGroup {

		/// DOCME
		std::vector<MatchSyllable> src;

		/// DOCME
		typedef std::vector<MatchSyllable>::iterator SrcIterator;

		/// DOCME
		wxString dst;

		/// DOCME
		size_t duration;

		/// DOCME
		int last_render_width;

		/// @brief DOCME
		///
		MatchGroup() : duration(0), last_render_width(0) { }
	};


	/// DOCME
	std::vector<MatchGroup> matched_groups;

	/// DOCME
	typedef std::vector<MatchGroup>::iterator MatchedGroupIterator;

	/// DOCME
	std::deque<MatchSyllable> unmatched_source;

	/// DOCME
	typedef std::deque<MatchSyllable>::iterator UnmatchedSourceIterator;

	/// DOCME
	wxString unmatched_destination;


	/// DOCME
	int last_total_matchgroup_render_width;


	/// DOCME
	size_t source_sel_length;

	/// DOCME
	size_t destination_sel_length;


	/// DOCME
	bool has_source, has_destination;

	void OnPaint(wxPaintEvent &event);
	void OnKeyboard(wxKeyEvent &event);
	void OnFocusEvent(wxFocusEvent &event);


	/// DOCME

	/// DOCME
	const wxString label_source, label_destination;

public:
	// Start processing a new line pair
	void SetInputData(const AssDialogue *src, const AssDialogue *dst);
	// Build and return the output line from the matched syllables
	wxString GetOutputLine();

	// Number of syllables not yet matched from source
	size_t GetRemainingSource();
	// Number of characters not yet matched from destination
	size_t GetRemainingDestination();

	// Adjust source and destination match lengths
	void IncreaseSourceMatch();
	void DecreaseSourceMatch();
	void IncreseDestinationMatch();
	void DecreaseDestinationMatch();
	// Attempt to treat source as Japanese romaji, destination as Japanese kana+kanji, and make an automatic match
	void AutoMatchJapanese();
	// Accept current selection and save match
	bool AcceptMatch();
	// Undo last match, adding it back to the unmatched input
	bool UndoMatch();


	// Constructor and destructor
	KaraokeLineMatchDisplay(DialogKanjiTimer *parent);
	~KaraokeLineMatchDisplay();

	wxSize GetBestSize();

	DECLARE_EVENT_TABLE()
};



/// @brief DOCME
/// @param parent 
///
KaraokeLineMatchDisplay::KaraokeLineMatchDisplay(DialogKanjiTimer *parent)
: wxControl(parent, -1, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE|wxWANTS_CHARS)
, label_source(TEXT_LABEL_SOURCE)
, label_destination(TEXT_LABEL_DEST)
{
	InheritAttributes();
	SetInputData(0, 0);

	wxSize best_size = GetBestSize();
	SetMaxSize(wxSize(-1, best_size.GetHeight()));
	SetMinSize(best_size);
}


/// @brief DOCME
///
KaraokeLineMatchDisplay::~KaraokeLineMatchDisplay()
{
	// Nothing to do
}


/// @brief DOCME
/// @return 
///
wxSize KaraokeLineMatchDisplay::GetBestSize()
{
	int w_src, h_src, w_dst, h_dst;
	GetTextExtent(label_source, &w_src, &h_src);
	GetTextExtent(label_destination, &w_dst, &h_dst);

	int min_width = (w_dst > w_src) ? w_dst : w_src;

	// Magic number 7:
	// 1 pixel for top black border, 1 pixel white top border in first row, 1 pixel white bottom padding in first row
	// 1 pixel middle border, 1 pixel top and 1 pixel bottom padding in second row, 1 pixel bottom border
	return wxSize(min_width * 2, h_src + h_dst + 7);
}


BEGIN_EVENT_TABLE(KaraokeLineMatchDisplay,wxControl)
	EVT_PAINT(KaraokeLineMatchDisplay::OnPaint)
	EVT_KEY_DOWN(KaraokeLineMatchDisplay::OnKeyboard)
	EVT_SET_FOCUS(KaraokeLineMatchDisplay::OnFocusEvent)
	EVT_KILL_FOCUS(KaraokeLineMatchDisplay::OnFocusEvent)
END_EVENT_TABLE()



/// @brief DOCME
/// @param dc  
/// @param txt 
/// @param x   
/// @param y   
/// @return 
///
int DrawBoxedText(wxDC &dc, const wxString &txt, int x, int y)
{
	int tw, th;
	// Assume the pen, brush and font properties have already been set in the DC.
	// Return the advance width, including box margins, borders etc

	if (txt == _T(""))
	{
		// Empty string gets special handling:
		// The box is drawn in shorter width, to emphasize it's empty
		// GetTextExtent has to be called with a non-empty string, otherwise it returns the wrong height
		dc.GetTextExtent(_T(" "), &tw, &th);
		dc.DrawRectangle(x, y-2, 4, th+4);
		return 3;
	}
	else
	{
		dc.GetTextExtent(txt, &tw, &th);
		dc.DrawRectangle(x, y-2, tw+4, th+4);
		dc.DrawText(txt, x+2, y);
		return tw+3;
	}
}


/// @brief DOCME
/// @param event 
///
void KaraokeLineMatchDisplay::OnPaint(wxPaintEvent &event)
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
	for (size_t i = 0; i < matched_groups.size(); ++i)
	{
		MatchGroup &grp = matched_groups[i];
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
		for (size_t j = 0; j < grp.src.size(); ++j)
		{
			syl_x += DrawBoxedText(dc, grp.src[j].text, syl_x, y_line1);
		}

		// Matched destination text
		{
			int adv = DrawBoxedText(dc, grp.dst, next_x, y_line2);
			
			// Adjust next_x here while we have the text_w
			if (syl_x > next_x + adv)
				next_x = syl_x;
			else
				next_x = next_x + adv;
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
	dc.SetTextBackground(sel_back);
	dc.SetTextForeground(sel_text);
	dc.SetBrush(wxBrush(sel_back));
	wxString txt = unmatched_destination.Left(destination_sel_length);
	if (txt != _T(""))
		next_x += DrawBoxedText(dc, txt, next_x, y_line2);

	dc.SetTextBackground(inner_back);
	dc.SetTextForeground(inner_text);
	dc.SetBrush(wxBrush(inner_back));
	txt = unmatched_destination.Mid(destination_sel_length);
	if (txt != _T(""))
		DrawBoxedText(dc, txt, next_x, y_line2);
}


/// @brief DOCME
/// @param event 
///
void KaraokeLineMatchDisplay::OnKeyboard(wxKeyEvent &event)
{
	((DialogKanjiTimer*)GetParent())->OnKeyDown(event);
}


/// @brief DOCME
/// @param event 
///
void KaraokeLineMatchDisplay::OnFocusEvent(wxFocusEvent &event)
{
	Refresh(true);
}



/// @brief DOCME
/// @param src 
/// @param dst 
///
void KaraokeLineMatchDisplay::SetInputData(const AssDialogue *src, const AssDialogue *dst)
{
	has_source = src != 0;
	has_destination = dst != 0;

	last_total_matchgroup_render_width = 0;

	matched_groups.clear();

	unmatched_source.clear();
	source_sel_length = 0;
	if (src)
	{
		AssDialogue *varsrc = dynamic_cast<AssDialogue*>(src->Clone());
		varsrc->ParseASSTags();
		AssKaraokeVector kara;
		ParseAssKaraokeTags(varsrc, kara);
		// Start from 1 instead of 0: The first syllable is actually everything before the first
		for (size_t i = 1; i < kara.size(); ++i)
		{
			unmatched_source.push_back(MatchSyllable(kara[i].duration, kara[i].text));
		}
		delete varsrc;
	}

	unmatched_destination.clear();
	destination_sel_length = 0;
	if (dst)
	{
		unmatched_destination = dst->GetStrippedText();
	}

	IncreaseSourceMatch();
	IncreseDestinationMatch();
	Refresh(true);
}



/// @brief DOCME
/// @return 
///
wxString KaraokeLineMatchDisplay::GetOutputLine()
{
	wxString res;

	for (size_t i = 0; i < matched_groups.size(); ++i)
	{
		MatchGroup &match = matched_groups[i];
		res = wxString::Format(_T("%s{\\k%d}%s"), res.c_str(), match.duration, match.dst.c_str());
	}

	return res;
}



/// @brief DOCME
/// @return 
///
size_t KaraokeLineMatchDisplay::GetRemainingSource()
{
	return unmatched_source.size();
}


/// @brief DOCME
/// @return 
///
size_t KaraokeLineMatchDisplay::GetRemainingDestination()
{
	return unmatched_destination.size();
}



/// @brief DOCME
///
void KaraokeLineMatchDisplay::IncreaseSourceMatch()
{
	source_sel_length += 1;
	if (source_sel_length > GetRemainingSource())
		source_sel_length = GetRemainingSource();
	Refresh(true);
}


/// @brief DOCME
///
void KaraokeLineMatchDisplay::DecreaseSourceMatch()
{
	if (source_sel_length > 0)
		source_sel_length -= 1;
	Refresh(true);
}


/// @brief DOCME
///
void KaraokeLineMatchDisplay::IncreseDestinationMatch()
{
	destination_sel_length += 1;
	if (destination_sel_length > GetRemainingDestination())
		destination_sel_length = GetRemainingDestination();
	Refresh(true);
}


/// @brief DOCME
///
void KaraokeLineMatchDisplay::DecreaseDestinationMatch()
{
	if (destination_sel_length > 0)
		destination_sel_length -= 1;
	Refresh(true);
}



/// DOCME
#define KANA_SEARCH_DISTANCE 3 //Kana interpolation, in characters, unset to disable

/// DOCME
#define ROMAJI_SEARCH_DISTANCE 4 //Romaji interpolation, in karaoke groups, unset to disable


/// @brief DOCME
/// @return 
///
void KaraokeLineMatchDisplay::AutoMatchJapanese()
{
	if (unmatched_source.size() < 1) return;

	// Quick escape: If there's no destination left, take all remaining source.
	// (Usually this means the user made a mistake.)
	if (unmatched_destination.size() == 0)
	{
		source_sel_length = unmatched_source.size();
		destination_sel_length = 0;
		return;
	}

	KanaTable kt;
	typedef std::list<KanaEntry>::const_iterator KanaIterator;

	// We'll first see if we can do something with the first unmatched source syllable
	wxString src(unmatched_source[0].text.Lower());
	wxString dst(unmatched_destination);
	source_sel_length = 1; // we're working on the first, assume it was matched
	destination_sel_length = 0;

	// Quick escape: If the source syllable is empty, return with first source syllable and empty destination
	if (src.size() == 0)
	{
		return;
	}

	// Try to match the next source syllable against the destination.
	// Do it "inverted", try all kana from the table and prefix-match them against the destination,
	// then if it matches a prefix, try to match the hepburn for it agast the source; eat if it matches.
	// Keep trying to match as long as there's text left in the source syllable or matching fails.
	while (src.size() > 0)
	{
		wxString dst_hira_rest, dst_kata_rest, src_rest;
		bool matched = false;
		for (KanaIterator ke = kt.entries.begin(); ke != kt.entries.end(); ++ke)
		{
			bool hira_matches = dst.StartsWith(ke->hiragana, &dst_hira_rest) && !ke->hiragana.IsEmpty();
			bool kata_matches = dst.StartsWith(ke->katakana, &dst_kata_rest);
			bool roma_matches = src.StartsWith(ke->hepburn, &src_rest);

			if ((hira_matches || kata_matches) && roma_matches)
			{
				matched = true;
				src = src_rest;
				dst = hira_matches ? dst_hira_rest : dst_kata_rest;
				destination_sel_length += (hira_matches?ke->hiragana:ke->katakana).size();
				break;
			}
		}
		if (!matched) break;
	}

	// The source might be empty now: That's good!
	// That means we managed to match it all against destination text
	if (src.size() == 0)
	{
		// destination_sel_length already has the appropriate value
		// and source_sel_length was alredy 1
		return;
	}

	// Now the source syllable might consist of just whitespace.
	// Eat all whitespace at the start of the destination.
	if (StringEmptyOrWhitespace(src))
	{
trycatchingmorespaces:
		// ASCII space
		if (unmatched_destination[destination_sel_length] == L'\x20') { ++destination_sel_length; goto trycatchingmorespaces; }
		// Japanese fullwidth ('ideographic') space
		if (unmatched_destination[destination_sel_length] == L'\u3000') { ++destination_sel_length; goto trycatchingmorespaces; }
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
	// Try to look up to KANA_SEARCH_DISTANCE characters ahead in destination, see if any of those
	// are recognised kana. If there are any within the range, see if it matches a following syllable,
	// at most 5 source syllables per character in source we're ahead.
	// The number 5 comes from the kanji with the longest readings: 'uketamawa.ru' and 'kokorozashi'
	// which each has a reading consisting of five kana.
	// Only match the found kana in destination against the beginning of source syllables, not the
	// middle of them.
	// If a match is found, make a guess at how much source and destination should be selected based
	// on the distances it was found at.
	dst = unmatched_destination;
	for (size_t lookahead = 0; lookahead < KANA_SEARCH_DISTANCE; ++lookahead)
	{
		// Eat dst at the beginning, don't test for the first character being kana
		dst = dst.Mid(1);
		// Find a position where hiragana or katakana matches
		wxString matched_roma;
		wxString matched_kana;
		for (KanaIterator ke = kt.entries.begin(); ke != kt.entries.end(); ++ke)
		{
			if (!!ke->hiragana && dst.StartsWith(ke->hiragana))
			{
				matched_roma = ke->hepburn;
				matched_kana = ke->hiragana;
				break;
			}
			if (!!ke->katakana && dst.StartsWith(ke->katakana))
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
		for (UnmatchedSourceIterator ss = unmatched_source.begin(); ss != unmatched_source.end(); ++ss)
		{
			// Check if we've gone too far ahead in the source
			if (src_lookahead_pos++ >= src_lookahead_max) break;
			// Otherwise look for a match
			if (ss->text.StartsWith(matched_roma))
			{
				// Yay! Time to interpolate.
				// Special case: If the last source syllable before the matching one is
				// empty or contains just whitespace, don't include that one.
				if (src_lookahead_pos > 1 && StringEmptyOrWhitespace(unmatched_source[src_lookahead_pos-2].text))
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

#ifdef ROMAJI_SEARCH_DISTANCE
	// Not re-implemented (yet?)
	// So far testing shows that the kana-based interpolation works just fine by itself.
#endif

	// Okay so we didn't match anything. Aww.
	// Just fail...
	// We know from earlier that we do have both some source and some destination.
	source_sel_length = 1;
	destination_sel_length = 1;
	return;
}



/// @brief DOCME
/// @return 
///
bool KaraokeLineMatchDisplay::AcceptMatch()
{
	MatchGroup match;

	if (source_sel_length == 0 && destination_sel_length == 0)
	{
		// Completely empty match
		return false;
	}

	assert(source_sel_length <= unmatched_source.size());
	for (; source_sel_length > 0; source_sel_length--)
	{
		match.src.push_back(unmatched_source.front());
		match.duration += unmatched_source.front().dur;
		unmatched_source.pop_front();
	}
	assert(source_sel_length == 0);

	assert(destination_sel_length <= unmatched_destination.size());
	match.dst = unmatched_destination.Left(destination_sel_length);
	unmatched_destination = unmatched_destination.Mid(destination_sel_length);
	destination_sel_length = 0;

	matched_groups.push_back(match);

	IncreaseSourceMatch();
	IncreseDestinationMatch();
	Refresh(true);

	return true;
}


/// @brief DOCME
/// @return 
///
bool KaraokeLineMatchDisplay::UndoMatch()
{
	if (matched_groups.empty())
		return false;

	MatchGroup &group = matched_groups.back();

	source_sel_length = group.src.size();
	destination_sel_length = group.dst.size();

	while (group.src.size() > 0)
	{
		unmatched_source.push_front(group.src.back());
		group.src.pop_back();
	}

	unmatched_destination = group.dst + unmatched_destination;

	matched_groups.pop_back();

	Refresh(true);

	return true;
}





/// @brief Constructor 
/// @param parent 
/// @param _grid  
///
DialogKanjiTimer::DialogKanjiTimer(agi::Context *c)
: wxDialog(c->parent,-1,_("Kanji timing"),wxDefaultPosition)
{
	// Set icon
	SetIcon(BitmapToIcon(GETIMAGE(kara_timing_copier_24)));

	// Variables
	subs = c->ass;
	grid = c->subsGrid;
	currentSourceLine = subs->Line.begin();
	currentDestinationLine = subs->Line.begin();

	//Sizers
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
	Interpolate = new wxCheckBox(this,-1,_("Attempt to interpolate kanji."),wxDefaultPosition,wxDefaultSize,wxALIGN_LEFT);
	Interpolate->SetValue(OPT_GET("Tool/Kanji Timer/Interpolation")->GetBool());

	SourceStyle=new wxComboBox(this,-1,_T(""),wxDefaultPosition,wxSize(160,-1),
								 subs->GetStyles(),wxCB_READONLY,wxDefaultValidator,_("Source Style"));
	DestStyle = new wxComboBox(this,-1,_T(""),wxDefaultPosition,wxSize(160,-1),
								 subs->GetStyles(),wxCB_READONLY,wxDefaultValidator,_("Dest Style"));

	wxStaticText *ShortcutKeys = new wxStaticText(this,-1,_("When the destination textbox has focus, use the following keys:\n\nRight Arrow: Increase dest. selection length\nLeft Arrow: Decrease dest. selection length\nUp Arrow: Increase source selection length\nDown Arrow: Decrease source selection length\nEnter: Link, accept line when done\nBackspace: Unlink last"));

	//Buttons
	wxButton *Start = new wxButton(this,BUTTON_KTSTART,_("Start!"));
	wxButton *Link = new wxButton(this,BUTTON_KTLINK,_("Link"));
	wxButton *Unlink = new wxButton(this,BUTTON_KTUNLINK,_("Unlink"));
	wxButton *SkipSourceLine = new wxButton(this,BUTTON_KTSKIPSOURCE,_("Skip Source Line"));
	wxButton *SkipDestLine = new wxButton(this,BUTTON_KTSKIPDEST,_("Skip Dest Line"));
	wxButton *GoBackLine = new wxButton(this,BUTTON_KTGOBACK,_("Go Back a Line"));
	wxButton *AcceptLine = new wxButton(this,BUTTON_KTACCEPT,_("Accept Line"));
	wxButton *CloseKT = new wxButton(this,wxID_CLOSE,_("Close"));

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
	buttonSizer->AddButton(new HelpButton(this,_T("Kanji Timer")));
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
}


///////////////
// Event table
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
	EVT_TEXT_ENTER(TEXT_SOURCE,DialogKanjiTimer::OnKeyEnter)
	EVT_TEXT_ENTER(TEXT_DEST,DialogKanjiTimer::OnKeyEnter)
END_EVENT_TABLE()


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnClose(wxCommandEvent &event) {
	OPT_SET("Tool/Kanji Timer/Interpolation")->SetBool(Interpolate->IsChecked());
	bool modified = !LinesToChange.empty();
	
	while(LinesToChange.empty()==false) {
		std::pair<entryIter,wxString> p = LinesToChange.back();
		LinesToChange.pop_back();
		AssDialogue *line = dynamic_cast<AssDialogue*>(*p.first);
		line->Text = p.second;
	}
	if (modified) {
		subs->Commit(_("kanji timing"));
		LinesToChange.clear();
	}
	Close();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnStart(wxCommandEvent &event) {
	if (SourceStyle->GetValue().Len() == 0 || DestStyle->GetValue().Len() == 0)
		wxMessageBox(_("Select source and destination styles first."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else if (SourceStyle->GetValue() == DestStyle->GetValue())
		wxMessageBox(_("The source and destination styles must be different."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else {
		currentSourceLine = FindNextStyleMatch(subs->Line.begin(), SourceStyle->GetValue());
		currentDestinationLine = FindNextStyleMatch(subs->Line.begin(), DestStyle->GetValue());
		ResetForNewLine();
	}
	LinesToChange.clear();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnLink(wxCommandEvent &event) {
	if (display->AcceptMatch())
		TryAutoMatch();
	else
		wxBell();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnUnlink(wxCommandEvent &event) {
	if (!display->UndoMatch())
		wxBell();
	// Don't auto-match here, undoing sets the selection to the undone match
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnSkipSource(wxCommandEvent &event) {
	currentSourceLine = FindNextStyleMatch(currentSourceLine, SourceStyle->GetValue());
	ResetForNewLine();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnSkipDest(wxCommandEvent &event) {
	currentDestinationLine = FindNextStyleMatch(currentDestinationLine, DestStyle->GetValue());
	ResetForNewLine();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnGoBack(wxCommandEvent &event) {
	if (LinesToChange.empty()==false)
		LinesToChange.pop_back(); //If we go back, then take out the modified line we saved.

	currentSourceLine = FindPrevStyleMatch(currentSourceLine, SourceStyle->GetValue());
	currentDestinationLine = FindPrevStyleMatch(currentDestinationLine, DestStyle->GetValue());
	ResetForNewLine();
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnAccept(wxCommandEvent &event) {
	if (display->GetRemainingSource() > 0)
		wxMessageBox(_("Group all of the source text."),_("Error"),wxICON_EXCLAMATION | wxOK);
	else {
		wxString OutputText = display->GetOutputLine();
		std::pair<entryIter,wxString> ins(currentDestinationLine, OutputText);
		LinesToChange.push_back(ins);

		currentSourceLine = FindNextStyleMatch(currentSourceLine, SourceStyle->GetValue());
		currentDestinationLine = FindNextStyleMatch(currentDestinationLine, DestStyle->GetValue());
		ResetForNewLine();
	}
}


/// @brief DOCME
/// @param event 
/// @return 
///
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
			OnKeyEnter(evt);
			break;
		default :
			event.Skip();
	}
}


/// @brief DOCME
/// @param event 
///
void DialogKanjiTimer::OnKeyEnter(wxCommandEvent &event) {
	wxCommandEvent evt;

	if (display->GetRemainingSource() == 0 && display->GetRemainingDestination() == 0)
	{
		OnAccept(evt);
	}
	else
	{
		OnLink(evt);
	}
}



/// @brief DOCME
///
void DialogKanjiTimer::ResetForNewLine()
{
	AssDialogue *src = 0;
	AssDialogue *dst = 0;

	if (currentSourceLine != subs->Line.end())
		src = dynamic_cast<AssDialogue*>(*currentSourceLine);
	if (currentDestinationLine != subs->Line.end())
		dst = dynamic_cast<AssDialogue*>(*currentDestinationLine);

	if (src == 0 || dst == 0)
	{
		src = dst = 0;
		wxBell();
	}

	display->SetInputData(src, dst);

	TryAutoMatch();

	display->SetFocus();
}


/// @brief DOCME
///
void DialogKanjiTimer::TryAutoMatch()
{
	if (Interpolate->GetValue())
		display->AutoMatchJapanese();
}


/// @brief DOCME
/// @param search_from  
/// @param search_style 
/// @return 
///
entryIter DialogKanjiTimer::FindNextStyleMatch(entryIter search_from, const wxString &search_style)
{
	if (search_from == subs->Line.end()) return search_from;

	while (++search_from != subs->Line.end())
	{
		AssDialogue *dlg = dynamic_cast<AssDialogue*>(*search_from);
		if (dlg && dlg->Style == search_style)
			break;
	}

	return search_from;
}


/// @brief DOCME
/// @param search_from  
/// @param search_style 
///
entryIter DialogKanjiTimer::FindPrevStyleMatch(entryIter search_from, const wxString &search_style)
{
	if (search_from == subs->Line.begin()) return search_from;

	while (--search_from != subs->Line.begin())
	{
		AssDialogue *dlg = dynamic_cast<AssDialogue*>(*search_from);
		if (dlg && dlg->Style == search_style)
			break;
	}

	return search_from;
}


