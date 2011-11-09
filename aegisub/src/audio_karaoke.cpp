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
//
// $Id$

/// @file audio_karaoke.cpp
/// @brief Karaoke table UI in audio box (not in audio display)
/// @ingroup audio_ui
///

#include "config.h"

#include "audio_karaoke.h"

#ifndef AGI_PRE
#include <algorithm>
#include <numeric>

#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/tokenzr.h>
#endif

#include "include/aegisub/context.h"

#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_karaoke.h"
#include "ass_override.h"
#include "audio_box.h"
#include "audio_timing.h"
#include "libresrc/libresrc.h"
#include "main.h"
#include "selection_controller.h"

template<class Container, class Value>
static inline size_t last_lt_or_eq(Container const& c, Value const& v) {
	typename Container::const_iterator it = lower_bound(c.begin(), c.end(), v);
	// lower_bound gives first >=
	if (it == c.end() || *it > v)
		--it;
	return distance(c.begin(), it);
};

AudioKaraoke::AudioKaraoke(wxWindow *parent, agi::Context *c)
: wxWindow(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_SUNKEN)
, c(c)
, file_changed(c->ass->AddCommitListener(&AudioKaraoke::OnFileChanged, this))
, active_line(0)
, kara(new AssKaraoke)
, enabled(false)
{
	using std::tr1::bind;

	wxSizer *main_sizer = new wxBoxSizer(wxHORIZONTAL);

	cancel_button = new wxBitmapButton(this, -1, GETIMAGE(kara_split_cancel_16));
	cancel_button->SetToolTip(_("Discard all uncommitted splits"));
	cancel_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&AudioKaraoke::CancelSplit, this));
	main_sizer->Add(cancel_button);

	accept_button = new wxBitmapButton(this, -1, GETIMAGE(kara_split_accept_16));
	accept_button->SetToolTip(_("Commit splits"));
	accept_button->Bind(wxEVT_COMMAND_BUTTON_CLICKED, bind(&AudioKaraoke::AcceptSplit, this));
	main_sizer->Add(accept_button);

	split_area = new wxPanel(this);
	main_sizer->Add(split_area, wxSizerFlags(1).Expand());

	SetSizerAndFit(main_sizer);

	/// @todo subscribe
	split_font.SetFaceName(OPT_GET("Audio/Karaoke/Font Face")->GetString());
	split_font.SetPointSize(OPT_GET("Audio/Karaoke/Font Size")->GetInt());

	Bind(wxEVT_SIZE, bind(&AudioKaraoke::Refresh, this, false, (const wxRect*)0));
	split_area->Bind(wxEVT_PAINT, &AudioKaraoke::OnPaint, this);
	split_area->Bind(wxEVT_LEFT_DOWN, &AudioKaraoke::OnMouse, this);
	split_area->Bind(wxEVT_LEFT_UP, &AudioKaraoke::OnMouse, this);
	split_area->Bind(wxEVT_MOTION, &AudioKaraoke::OnMouse, this);
	split_area->Bind(wxEVT_LEAVE_WINDOW, &AudioKaraoke::OnMouse, this);
	split_area->Bind(wxEVT_CONTEXT_MENU, &AudioKaraoke::OnContextMenu, this);

	c->selectionController->AddSelectionListener(this);

	accept_button->Enable(false);
	cancel_button->Enable(false);
	enabled = false;
	c->audioController->SetTimingController(CreateDialogueTimingController(c));
}

AudioKaraoke::~AudioKaraoke() {
	c->selectionController->RemoveSelectionListener(this);
}

void AudioKaraoke::OnActiveLineChanged(AssDialogue *new_line) {
	active_line = new_line;
	if (enabled) {
		LoadFromLine();
		split_area->Refresh(false);
	}
}

void AudioKaraoke::OnFileChanged(int type) {
	if (enabled && (type & AssFile::COMMIT_DIAG_FULL)) {
		LoadFromLine();
		split_area->Refresh(false);
	}
}

void AudioKaraoke::SetEnabled(bool en) {
	enabled = en;

	c->audioBox->ShowKaraokeBar(enabled);
	split_area->SetSize(GetSize().GetWidth(), -1);

	if (enabled) {
		LoadFromLine();
		c->audioController->SetTimingController(CreateKaraokeTimingController(c, kara.get(), file_changed));
		Refresh(false);
	}
	else {
		c->audioController->SetTimingController(CreateDialogueTimingController(c));
	}
}

void AudioKaraoke::OnPaint(wxPaintEvent &evt) {
	int w, h;
	split_area->GetClientSize(&w, &h);

	wxPaintDC dc(split_area);
	wxMemoryDC bmp_dc(rendered_line);

	// Draw the text and split lines
	dc.Blit(0, 0, w, h, &bmp_dc, 0, 0);

	// Draw the split line under the mouse
	dc.SetPen(*wxRED);
	dc.DrawLine(mouse_pos, 0, mouse_pos, h);
}

void AudioKaraoke::RenderText() {
	int w, h;
	split_area->GetClientSize(&w, &h);

	if (!rendered_line.IsOk() || split_area->GetClientSize() != rendered_line.GetSize()) {
		rendered_line = wxBitmap(w, h);
	}

	wxMemoryDC dc(rendered_line);

	// Draw background
	dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.DrawRectangle(0, 0, w, h);

	dc.SetFont(split_font);
	dc.SetTextForeground(wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));

	// Draw each character in the line
	int y = (h - char_height) / 2;
	for (size_t i = 0; i < spaced_text.size(); ++i) {
		dc.DrawText(spaced_text[i], char_x[i], y);
	}

	// Draw the lines between each syllable
	dc.SetPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	for (size_t i = 0; i < syl_lines.size(); ++i) {
		dc.DrawLine(syl_lines[i], 0, syl_lines[i], h);
	}
}

void AudioKaraoke::AddMenuItem(wxMenu &menu, wxString const& tag, wxString const& help, wxString const& selected) {
	wxMenuItem *item = menu.AppendCheckItem(-1, tag, help);
	menu.Bind(wxEVT_COMMAND_MENU_SELECTED, std::tr1::bind(&AudioKaraoke::SetTagType, this, tag), item->GetId());
	item->Check(tag == selected);
}

void AudioKaraoke::OnContextMenu(wxContextMenuEvent&) {
	if (!enabled) return;

	wxMenu context_menu(_("Karaoke tag"));
	wxString type = kara->GetTagType();

	AddMenuItem(context_menu, "\\k", _("Change karaoke tag to \\k"), type);
	AddMenuItem(context_menu, "\\kf", _("Change karaoke tag to \\kf"), type);
	AddMenuItem(context_menu, "\\ko", _("Change karaoke tag to \\ko"), type);

	PopupMenu(&context_menu);
}

void AudioKaraoke::OnMouse(wxMouseEvent &event) {
	if (!enabled) return;

	mouse_pos = event.GetX();

	if (event.Leaving())
		mouse_pos = -1;

	if (!event.LeftDown()) {
		// Erase the old line and draw the new one
		split_area->Refresh(false);
		return;
	}

	// Character to insert the new split point before
	int split_pos = std::min<int>((mouse_pos - char_width / 2) / char_width, spaced_text.size());

	// Syllable this character is in
	int syl = last_lt_or_eq(syl_start_points, split_pos);

	// If the click is sufficiently close to a line of a syllable split,
	// remove that split rather than adding a new one
	if ((syl > 0 && mouse_pos <= syl_lines[syl - 1] + 2) || (syl < (int)syl_lines.size() && mouse_pos >= syl_lines[syl] - 2)) {
		kara->RemoveSplit(syl);
	}
	else {
		kara->AddSplit(syl, split_pos - syl_start_points[syl]);
	}

	SetDisplayText();
	accept_button->Enable(true);
	cancel_button->Enable(true);
	split_area->Refresh(false);
}

void AudioKaraoke::LoadFromLine() {
	kara->SetLine(active_line, true);
	SetDisplayText();
	accept_button->Enable(false);
	cancel_button->Enable(false);
}

void AudioKaraoke::SetDisplayText() {
	// Insert spaces between each syllable to avoid crowding
	spaced_text.clear();
	syl_start_points.clear();
	syl_start_points.reserve(kara->size());
	for (AssKaraoke::iterator it = kara->begin(); it != kara->end(); ++it) {
		syl_start_points.push_back(spaced_text.size());
		spaced_text += " " + it->text;
	}

	// Get the x-coordinates of the right edge of each character
	wxMemoryDC dc;
	dc.SetFont(split_font);
	wxArrayInt p_char_x;
	dc.GetPartialTextExtents(spaced_text, p_char_x);

	// Convert the partial sub to the the width of each character
	std::adjacent_difference(p_char_x.begin(), p_char_x.end(), p_char_x.begin());

	// Get the maximum character width
	char_width = *std::max_element(p_char_x.begin(), p_char_x.end());

	// Center each character within the space available to it
	char_x.resize(p_char_x.size());
	for (size_t i = 0; i < p_char_x.size(); ++i) {
		char_x[i] =  i * char_width + (char_width - p_char_x[i]) / 2;
	}

	// Calculate the positions of the syllable divider lines
	syl_lines.resize(syl_start_points.size() - 1);
	for (size_t i = 1; i < syl_start_points.size(); ++i) {
		syl_lines[i - 1] = syl_start_points[i] * char_width + char_width / 2;
	}

	// Get line height
	wxSize extents = dc.GetTextExtent(spaced_text);
	char_height = extents.GetHeight();

	RenderText();
}

void AudioKaraoke::CancelSplit() {
	LoadFromLine();
	split_area->Refresh(false);
}

void AudioKaraoke::AcceptSplit() {
	active_line->Text = kara->GetText();
	file_changed.Block();
	c->ass->Commit(_("karaoke split"), AssFile::COMMIT_DIAG_TEXT);
	file_changed.Unblock();

	accept_button->Enable(false);
	cancel_button->Enable(false);
}

void AudioKaraoke::SetTagType(wxString new_tag) {
	kara->SetTagType(new_tag);
	AcceptSplit();
}
