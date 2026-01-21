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

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "compat.h"
#include "help_button.h"
#include "include/aegisub/context.h"
#include "libresrc/libresrc.h"
#include "options.h"

#include <libaegisub/ass/karaoke.h>
#include <libaegisub/karaoke_matcher.h>
#include <libaegisub/unicode.h>

#include <deque>
#include <vector>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/dcclient.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/string.h>

namespace {
#define TEXT_LABEL_SOURCE _("Source: ")
#define TEXT_LABEL_DEST _("Dest: ")

class KaraokeLineMatchDisplay final : public wxControl {
	agi::KaraokeMatcher matcher;
	std::vector<int> matchgroup_widths;
	int last_total_matchgroup_render_width = 0;
	using MatchSyllable = agi::ass::KaraokeSyllable;

	struct MatchGroup {
		std::vector<MatchSyllable> src;
		std::string dst;
		int last_render_width = 0;
	};

	void OnPaint(wxPaintEvent &event);

	wxString const& label_source = TEXT_LABEL_SOURCE;
	wxString const& label_destination = TEXT_LABEL_DEST;

public:
	/// Start processing a new line pair
	void SetInputData(AssDialogue *src, AssDialogue *dst);
	/// Build and return the output line from the matched syllables
	std::string GetOutputLine() const { return matcher.GetOutputLine(); }

	/// Number of syllables not yet matched from source
	size_t GetRemainingSource() const { return matcher.UnmatchedSource().size() + matcher.CurrentSourceSelection().size(); }
	/// Number of characters not yet matched from destination
	size_t GetRemainingDestination() const { return matcher.UnmatchedDestination().size() + matcher.CurrentDestinationSelection().size(); }

	// Adjust source and destination match lengths
	void IncreaseSourceMatch();
	void DecreaseSourceMatch();
	void IncreaseDestinationMatch();
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
{
	InheritAttributes();
	SetInputData(nullptr, nullptr);

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

int DrawBoxedText(wxDC &dc, wxString const& txt, int x, int y)
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
		dc.GetTextExtent(txt, &tw, &th);
		dc.DrawRectangle(x, y-2, tw+4, th+4);
		dc.DrawText(txt, x+2, y);
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

	auto groups = matcher.MatchedGroups();
	matchgroup_widths.resize(groups.size());
	for (size_t i = 0; i < groups.size(); ++i) {
		auto& grp = groups[i];
		int prev_x = next_x;

		// Skip groups that would cause the input part to be too far right
		this_total_matchgroup_render_width += matchgroup_widths[i];
		if (last_total_matchgroup_render_width - this_total_matchgroup_render_width > client_size.x / 2)
		{
			// If we're skipping some syllables, show an arrow as feedback that something is scrolled off
			if (!scroll_arrows_drawn)
			{
				dc.SetBrush(wxBrush(outer_frame));
				int height = y_line2 - y_line1;
				wxPoint triangle[] = {
					wxPoint(next_x-3, height/2),
					wxPoint(next_x-3+height/2, 0),
					wxPoint(next_x-3+height/2, height)
				};
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
			syl_x += DrawBoxedText(dc, to_wx(syl.text), syl_x, y_line1);

		// Matched destination text
		{
			const int adv = DrawBoxedText(dc, to_wx(grp.dst), next_x, y_line2);

			// Adjust next_x here while we have the text_w
			next_x = syl_x > next_x + adv ? syl_x : next_x + adv;
		}

		// Spacing between groups
		next_x += 3;
		matchgroup_widths[i] = next_x - prev_x;
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
	for (auto syl : matcher.CurrentSourceSelection()) {
		syl_x += DrawBoxedText(dc, to_wx(syl.text), syl_x, y_line1);
	}

	// Switch to regular colours after all selected syllables
	dc.SetTextBackground(inner_back);
	dc.SetTextForeground(inner_text);
	dc.SetBrush(wxBrush(inner_back));
	for (auto syl : matcher.UnmatchedSource()) {
		syl_x += DrawBoxedText(dc, to_wx(syl.text), syl_x, y_line1);
	}

	// Remaining destination
	if (auto dest = matcher.CurrentDestinationSelection(); !dest.empty()) {
		dc.SetTextBackground(sel_back);
		dc.SetTextForeground(sel_text);
		dc.SetBrush(wxBrush(sel_back));
		next_x += DrawBoxedText(dc, to_wx(dest), next_x, y_line2);
	}

	if (auto dest = matcher.UnmatchedDestination(); !dest.empty()) {
		dc.SetTextBackground(inner_back);
		dc.SetTextForeground(inner_text);
		dc.SetBrush(wxBrush(inner_back));
		DrawBoxedText(dc, to_wx(dest), next_x, y_line2);
	}
}

void KaraokeLineMatchDisplay::SetInputData(AssDialogue *src, AssDialogue *dst)
{
	last_total_matchgroup_render_width = 0;
	matcher.SetInputData(ParseKaraokeSyllables(src), dst ? dst->GetStrippedText() : "");
	Refresh(true);
}

void KaraokeLineMatchDisplay::IncreaseSourceMatch()
{
	if (matcher.IncreaseSourceMatch()) Refresh(true);
}

void KaraokeLineMatchDisplay::DecreaseSourceMatch()
{
	if (matcher.DecreaseSourceMatch()) Refresh(true);
}

void KaraokeLineMatchDisplay::IncreaseDestinationMatch()
{
	if (matcher.IncreaseDestinationMatch()) Refresh(true);
}

void KaraokeLineMatchDisplay::DecreaseDestinationMatch()
{
	if (matcher.DecreaseDestinationMatch()) Refresh(true);
}

void KaraokeLineMatchDisplay::AutoMatchJapanese()
{
	matcher.AutoMatchJapanese();
}

bool KaraokeLineMatchDisplay::AcceptMatch()
{
	if (matcher.AcceptMatch()) {
		Refresh(true);
		return true;
	}
	return false;
}

bool KaraokeLineMatchDisplay::UndoMatch()
{
	if (matcher.UndoMatch()) {
		Refresh(true);
		return true;
	}
	return false;
}

class DialogKanjiTimer final : public wxDialog {
	AssFile *subs;

	KaraokeLineMatchDisplay *display;

	wxComboBox *SourceStyle, *DestStyle;
	wxCheckBox *Interpolate;

	std::vector<std::pair<AssDialogue*, std::string>> LinesToChange;

	AssDialogue *currentSourceLine = nullptr;
	AssDialogue *currentDestinationLine = nullptr;

	void OnClose(wxCommandEvent &event);
	void OnStart(wxCommandEvent &event);
	void OnLink(wxCommandEvent &event);
	void OnUnlink(wxCommandEvent &event);
	void OnSkipSource(wxCommandEvent &event);
	void OnSkipDest(wxCommandEvent &event);
	void OnGoBack(wxCommandEvent &event);
	void OnAccept(wxCommandEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	void ResetForNewLine();
	void TryAutoMatch();

	AssDialogue *FindNextStyleMatch(AssDialogue *search_from, const std::string &search_style);
	AssDialogue *FindPrevStyleMatch(AssDialogue *search_from, const std::string &search_style);

public:
	DialogKanjiTimer(agi::Context *context);
};

DialogKanjiTimer::DialogKanjiTimer(agi::Context *c)
: wxDialog(c->parent, -1, _("Kanji timer"))
, subs(c->ass.get())
{
	SetIcons(GETICONS(kara_timing_copier));

	wxStaticBoxSizer *DisplayBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Text"));
	wxStaticBoxSizer *StylesBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Styles"));
	auto StylesGridSizer = new wxFlexGridSizer(2, 2, 6, 6);
	wxStaticBoxSizer *HelpBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Shortcut Keys"));
	wxStaticBoxSizer *ButtonsBoxSizer = new wxStaticBoxSizer(wxVERTICAL,this,_("Commands"));
	wxSizer *MainStackSizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *BottomShelfSizer = new wxBoxSizer(wxHORIZONTAL);
	wxSizer *BottomLeftStackSizer = new wxBoxSizer(wxVERTICAL);

	wxWindow *DisplayBox = DisplayBoxSizer->GetStaticBox();
	wxWindow *StylesBox = StylesBoxSizer->GetStaticBox();
	wxWindow *HelpBox = HelpBoxSizer->GetStaticBox();
	wxWindow *ButtonsBox = ButtonsBoxSizer->GetStaticBox();

	display = new KaraokeLineMatchDisplay(DisplayBox);

	//Checkbox
	Interpolate = new wxCheckBox(DisplayBox,-1,_("Attempt to &interpolate kanji"),wxDefaultPosition,wxDefaultSize,wxALIGN_LEFT);
	Interpolate->SetValue(OPT_GET("Tool/Kanji Timer/Interpolation")->GetBool());

	wxArrayString styles = to_wx(subs->GetStyles());
	SourceStyle = new wxComboBox(StylesBox, -1, "", wxDefaultPosition, wxSize(160, -1), styles, wxCB_READONLY);
	DestStyle = new wxComboBox(StylesBox, -1, "", wxDefaultPosition, wxSize(160, -1), styles, wxCB_READONLY);

	wxStaticText *ShortcutKeys = new wxStaticText(HelpBox,-1,_("When the destination textbox has focus, use the following keys:\n\nRight Arrow: Increase dest. selection length\nLeft Arrow: Decrease dest. selection length\nUp Arrow: Increase source selection length\nDown Arrow: Decrease source selection length\nEnter: Link, accept line when done\nBackspace: Unlink last"));

	//Buttons
	wxButton *Start = new wxButton(ButtonsBox, -1,_("S&tart!"));
	wxButton *Link = new wxButton(ButtonsBox, -1,_("&Link"));
	wxButton *Unlink = new wxButton(ButtonsBox, -1,_("&Unlink"));
	wxButton *SkipSourceLine = new wxButton(ButtonsBox, -1,_("Skip &Source Line"));
	wxButton *SkipDestLine = new wxButton(ButtonsBox, -1,_("Skip &Dest Line"));
	wxButton *GoBackLine = new wxButton(ButtonsBox, -1,_("&Go Back a Line"));
	wxButton *AcceptLine = new wxButton(ButtonsBox, -1,_("&Accept Line"));
	wxButton *CloseKT = new wxButton(this,wxID_CLOSE,_("&Close"));

	//Frame: Text
	DisplayBoxSizer->Add(display, 0, wxEXPAND|wxALL, 6);
	DisplayBoxSizer->Add(Interpolate, 0, wxEXPAND|wxALL, 6);
	//Frame: Styles
	StylesGridSizer->Add(new wxStaticText(StylesBox, -1, TEXT_LABEL_SOURCE), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
	StylesGridSizer->Add(SourceStyle, 1, wxEXPAND);
	StylesGridSizer->Add(new wxStaticText(StylesBox, -1, TEXT_LABEL_DEST), 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL);
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
	auto buttonSizer = new wxStdDialogButtonSizer();
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

	Bind(wxEVT_KEY_DOWN, &DialogKanjiTimer::OnKeyDown, this);
	display->Bind(wxEVT_KEY_DOWN, &DialogKanjiTimer::OnKeyDown, this);

	CloseKT->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnClose, this);
	Start->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnStart, this);
	Link->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnLink, this);
	Unlink->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnUnlink, this);
	SkipSourceLine->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnSkipSource, this);
	SkipDestLine->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnSkipDest, this);
	GoBackLine->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnGoBack, this);
	AcceptLine->Bind(wxEVT_BUTTON, &DialogKanjiTimer::OnAccept, this);
}

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
		currentDestinationLine = currentSourceLine = &*subs->Events.begin();
		auto sourceStyle = from_wx(SourceStyle->GetValue());
		auto destStyle = from_wx(DestStyle->GetValue());
		if (currentSourceLine->Style != sourceStyle)
			currentSourceLine = FindNextStyleMatch(currentSourceLine, sourceStyle);
		if (currentDestinationLine->Style != destStyle)
			currentDestinationLine = FindNextStyleMatch(currentDestinationLine, destStyle);
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
	else {
		LinesToChange.emplace_back(currentDestinationLine, display->GetOutputLine());

		currentSourceLine = FindNextStyleMatch(currentSourceLine, from_wx(SourceStyle->GetValue()));
		currentDestinationLine = FindNextStyleMatch(currentDestinationLine, from_wx(DestStyle->GetValue()));
		ResetForNewLine();
	}
}

void DialogKanjiTimer::OnKeyDown(wxKeyEvent &event) {
	wxCommandEvent evt;
	switch(event.GetKeyCode()) {
		case WXK_ESCAPE:
			OnClose(evt);
			break;
		case WXK_BACK:
			OnUnlink(evt);
			break;
		case WXK_RIGHT: //inc dest selection len
			display->IncreaseDestinationMatch();
			break;
		case WXK_LEFT: //dec dest selection len
			display->DecreaseDestinationMatch();
			break;
		case WXK_UP: //inc source selection len
			display->IncreaseSourceMatch();
			break;
		case WXK_DOWN: //dec source selection len
			display->DecreaseSourceMatch();
			break;
		case WXK_RETURN:
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
	if (!currentSourceLine || !currentDestinationLine)
	{
		currentSourceLine = currentDestinationLine = nullptr;
		wxBell();
	}

	display->SetInputData(currentSourceLine, currentDestinationLine);

	TryAutoMatch();

	display->SetFocus();
}

void DialogKanjiTimer::TryAutoMatch()
{
	if (Interpolate->GetValue())
		display->AutoMatchJapanese();
}

template<typename Iterator>
static AssDialogue *find_next(Iterator from, Iterator to, std::string const& style_name) {
	for (; from != to; ++from) {
		if (from->Style == style_name && !from->Text.get().empty())
			return &*from;
	}

	return nullptr;
}

AssDialogue *DialogKanjiTimer::FindNextStyleMatch(AssDialogue *search_from, const std::string &search_style)
{
	if (!search_from) return search_from;
	return find_next(++subs->iterator_to(*search_from), subs->Events.end(), search_style);
}

AssDialogue *DialogKanjiTimer::FindPrevStyleMatch(AssDialogue *search_from, const std::string &search_style)
{
	if (!search_from) return search_from;
	return find_next(EntryList<AssDialogue>::reverse_iterator(subs->iterator_to(*search_from)), subs->Events.rend(), search_style);
}
}

void ShowKanjiTimerDialog(agi::Context *c) {
	DialogKanjiTimer(c).ShowModal();
}
