// Copyright (c) 2006, Rodrigo Braz Monteiro
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

/// @file subtitle_format.cpp
/// @brief Base class for subtitle format handlers
/// @ingroup subtitle_io
///

#include "config.h"

#include "subtitle_format.h"

#ifndef AGI_PRE
#include <wx/intl.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.
#endif

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_ebu3264.h"
#include "subtitle_format_encore.h"
#include "subtitle_format_microdvd.h"
#include "subtitle_format_mkv.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_transtation.h"
#include "subtitle_format_ttxt.h"
#include "subtitle_format_txt.h"
#include "utils.h"
#include "video_context.h"

using namespace std::tr1::placeholders;

SubtitleFormat::SubtitleFormat(wxString const& name)
: name(name)
{
	formats.push_back(this);
}

SubtitleFormat::~SubtitleFormat() {
	formats.remove(this);
}

bool SubtitleFormat::CanReadFile(wxString const& filename) const {
	return GetReadWildcards().Index(filename.AfterLast('.'), false) != wxNOT_FOUND;
}

bool SubtitleFormat::CanWriteFile(wxString const& filename) const {
	return GetWriteWildcards().Index(filename.AfterLast('.'), false) != wxNOT_FOUND;
}

bool SubtitleFormat::CanSave(const AssFile *subs) const {
	AssStyle defstyle;
	for (std::list<AssEntry*>::const_iterator cur = subs->Line.begin(); cur != subs->Line.end(); ++cur) {
		// Check style, if anything non-default is found, return false
		if (const AssStyle *curstyle = dynamic_cast<const AssStyle*>(*cur)) {
			if (curstyle->GetEntryData() != defstyle.GetEntryData())
				return false;
		}

		// Check for attachments, if any is found, return false
		if (dynamic_cast<const AssAttachment*>(*cur)) return false;

		// Check dialog
		if (const AssDialogue *curdiag = dynamic_cast<const AssDialogue*>(*cur)) {
			if (curdiag->GetStrippedText() != curdiag->Text)
				return false;
		}
	}

	return true;
}

agi::vfr::Framerate SubtitleFormat::AskForFPS(bool allow_vfr, bool show_smpte) {
	wxArrayString choices;

	// Video FPS
	VideoContext *context = VideoContext::Get();
	bool vidLoaded = context->TimecodesLoaded();
	if (vidLoaded) {
		if (!context->FPS().IsVFR())
			choices.Add(wxString::Format(_("From video (%g)"), context->FPS().FPS()));
		else if (allow_vfr)
			choices.Add(_("From video (VFR)"));
		else
			vidLoaded = false;
	}

	// Standard FPS values
	choices.Add(_("15.000 FPS"));
	choices.Add(_("23.976 FPS (Decimated NTSC)"));
	choices.Add(_("24.000 FPS (FILM)"));
	choices.Add(_("25.000 FPS (PAL)"));
	choices.Add(_("29.970 FPS (NTSC)"));
	if (show_smpte)
		choices.Add(_("29.970 FPS (NTSC with SMPTE dropframe)"));
	choices.Add(_("30.000 FPS"));
	choices.Add(_("50.000 FPS (PAL x2)"));
	choices.Add(_("59.940 FPS (NTSC x2)"));
	choices.Add(_("60.000 FPS"));
	choices.Add(_("119.880 FPS (NTSC x4)"));
	choices.Add(_("120.000 FPS"));

	wxEndBusyCursor();
	int choice = wxGetSingleChoiceIndex(_("Please choose the appropriate FPS for the subtitles:"), _("FPS"), choices);
	wxBeginBusyCursor();

	using agi::vfr::Framerate;
	if (choice == -1)
		return Framerate();

	// Get FPS from choice
	if (vidLoaded)
		--choice;
	if (!show_smpte && choice > 4)
		--choice;

	switch (choice) {
		case -1: return context->FPS();          break; // VIDEO
		case 0:  return Framerate(15, 1);        break;
		case 1:  return Framerate(24000, 1001);  break;
		case 2:  return Framerate(24, 1);        break;
		case 3:  return Framerate(25, 1);        break;
		case 4:  return Framerate(30000, 1001);  break;
		case 5:  return Framerate(30000, 1001, true); break;
		case 6:  return Framerate(30, 1);        break;
		case 7:  return Framerate(50, 1);        break;
		case 8:  return Framerate(60000, 1001);  break;
		case 9:  return Framerate(60, 1);        break;
		case 10: return Framerate(120000, 1001); break;
		case 11: return Framerate(120, 1);       break;
	}

	assert(false);
	return Framerate();
}

void SubtitleFormat::StripTags(LineList &lines) {
	for (LineList::iterator cur = lines.begin(); cur != lines.end(); ++cur) {
		if (AssDialogue *current = dynamic_cast<AssDialogue*>(*cur)) {
			current->StripTags();
		}
	}
}

void SubtitleFormat::ConvertNewlines(LineList &lines, wxString const& newline, bool mergeLineBreaks) {
	for (LineList::iterator cur = lines.begin(); cur != lines.end(); ++cur) {
		if (AssDialogue *current = dynamic_cast<AssDialogue*>(*cur)) {
			current->Text.Replace("\\h", " ");
			current->Text.Replace("\\n", newline);
			current->Text.Replace("\\N", newline);
			if (mergeLineBreaks) {
				while (current->Text.Replace(newline+newline, newline));
			}
		}
	}
}

void SubtitleFormat::StripComments(LineList &lines) {
	for (LineList::iterator it = lines.begin(); it != lines.end(); ) {
		AssDialogue *diag = dynamic_cast<AssDialogue*>(*it);
		if (!diag || (!diag->Comment && diag->Text.size()))
			++it;
		else {
			delete *it;
			lines.erase(it++);
		}
	}
}

void SubtitleFormat::StripNonDialogue(LineList &lines) {
	for (LineList::iterator it = lines.begin(); it != lines.end(); ) {
		if (dynamic_cast<AssDialogue*>(*it))
			++it;
		else {
			delete *it;
			lines.erase(it++);
		}
	}
}

static bool dialog_start_lt(AssEntry *pos, AssDialogue *to_insert) {
	AssDialogue *diag = dynamic_cast<AssDialogue*>(pos);
	return diag && diag->Start > to_insert->Start;
}

/// @brief Split and merge lines so there are no overlapping lines
///
/// Algorithm described at http://devel.aegisub.org/wiki/Technical/SplitMerge
void SubtitleFormat::RecombineOverlaps(LineList &lines) {
	LineList::iterator cur, next = lines.begin();
	cur = next++;

	for (; next != lines.end(); cur = next++) {
		AssDialogue *prevdlg = dynamic_cast<AssDialogue*>(*cur);
		AssDialogue *curdlg = dynamic_cast<AssDialogue*>(*next);

		if (!curdlg || !prevdlg) continue;
		if (prevdlg->End <= curdlg->Start) continue;

		// Use names like in the algorithm description and prepare for erasing
		// old dialogues from the list
		LineList::iterator prev = cur;
		cur = next;
		next++;

		// std::list::insert() inserts items before the given iterator, so
		// we need 'next' for inserting. 'prev' and 'cur' can safely be erased
		// from the list now.
		lines.erase(prev);
		lines.erase(cur);

		//Is there an A part before the overlap?
		if (curdlg->Start > prevdlg->Start) {
			// Produce new entry with correct values
			AssDialogue *newdlg = dynamic_cast<AssDialogue*>(prevdlg->Clone());
			newdlg->Start = prevdlg->Start;
			newdlg->End = curdlg->Start;
			newdlg->Text = prevdlg->Text;

			lines.insert(find_if(next, lines.end(), bind(dialog_start_lt, _1, newdlg)), newdlg);
		}

		// Overlapping A+B part
		{
			AssDialogue *newdlg = dynamic_cast<AssDialogue*>(prevdlg->Clone());
			newdlg->Start = curdlg->Start;
			newdlg->End = (prevdlg->End < curdlg->End) ? prevdlg->End : curdlg->End;
			// Put an ASS format hard linewrap between lines
			newdlg->Text = curdlg->Text + "\\N" + prevdlg->Text;

			lines.insert(find_if(next, lines.end(), bind(dialog_start_lt, _1, newdlg)), newdlg);
		}

		// Is there an A part after the overlap?
		if (prevdlg->End > curdlg->End) {
			// Produce new entry with correct values
			AssDialogue *newdlg = dynamic_cast<AssDialogue*>(prevdlg->Clone());
			newdlg->Start = curdlg->End;
			newdlg->End = prevdlg->End;
			newdlg->Text = prevdlg->Text;

			lines.insert(find_if(next, lines.end(), bind(dialog_start_lt, _1, newdlg)), newdlg);
		}

		// Is there a B part after the overlap?
		if (curdlg->End > prevdlg->End) {
			// Produce new entry with correct values
			AssDialogue *newdlg = dynamic_cast<AssDialogue*>(prevdlg->Clone());
			newdlg->Start = prevdlg->End;
			newdlg->End = curdlg->End;
			newdlg->Text = curdlg->Text;

			lines.insert(find_if(next, lines.end(), bind(dialog_start_lt, _1, newdlg)), newdlg);
		}

		next--;
	}
}

/// @brief Merge identical lines that follow each other
void SubtitleFormat::MergeIdentical(LineList &lines) {
	LineList::iterator cur, next = lines.begin();
	cur = next++;

	for (; next != lines.end(); cur = next++) {
		AssDialogue *curdlg = dynamic_cast<AssDialogue*>(*cur);
		AssDialogue *nextdlg = dynamic_cast<AssDialogue*>(*next);

		if (curdlg && nextdlg && curdlg->End == nextdlg->Start && curdlg->Text == nextdlg->Text) {
			// Merge timing
			nextdlg->Start = std::min(nextdlg->Start, curdlg->Start);
			nextdlg->End = std::max(nextdlg->End, curdlg->End);

			// Remove duplicate line
			delete *cur;
			lines.erase(cur);
		}
	}
}

std::list<SubtitleFormat*> SubtitleFormat::formats;

void SubtitleFormat::LoadFormats() {
	if (formats.empty()) {
		new ASSSubtitleFormat;
		new Ebu3264SubtitleFormat;
		new EncoreSubtitleFormat;
		new MKVSubtitleFormat;
		new MicroDVDSubtitleFormat;
		new SRTSubtitleFormat;
		new TTXTSubtitleFormat;
		new TXTSubtitleFormat;
		new TranStationSubtitleFormat;
	}
}

void SubtitleFormat::DestroyFormats() {
	for (std::list<SubtitleFormat*>::iterator it = formats.begin(); it != formats.end(); )
		delete *it++;
}

template<class Cont, class Pred>
SubtitleFormat *find_or_null(Cont &container, Pred pred) {
	typename Cont::iterator it = find_if(container.begin(), container.end(), pred);
	if (it == container.end())
		return 0;
	return *it;
}

const SubtitleFormat *SubtitleFormat::GetReader(wxString const& filename) {
	LoadFormats();
	return find_or_null(formats, bind(&SubtitleFormat::CanReadFile, _1, filename));
}

const SubtitleFormat *SubtitleFormat::GetWriter(wxString const& filename) {
	LoadFormats();
	return find_or_null(formats, bind(&SubtitleFormat::CanWriteFile, _1, filename));
}

wxString SubtitleFormat::GetWildcards(int mode) {
	LoadFormats();

	wxArrayString all;
	wxString final;

	std::list<SubtitleFormat*>::iterator curIter;
	for (curIter=formats.begin();curIter!=formats.end();curIter++) {
		SubtitleFormat *format = *curIter;
		wxArrayString cur = mode == 0 ? format->GetReadWildcards() : format->GetWriteWildcards();
		if (cur.empty()) continue;

		for_each(cur.begin(), cur.end(), bind(&wxString::Prepend, _1, "*."));
		copy(cur.begin(), cur.end(), std::back_inserter(all));
		final += "|" + format->GetName() + " (" + wxJoin(cur, ',') + ")|" + wxJoin(cur, ';');
	}

	final.Prepend(_("All Supported Formats") + " (" + wxJoin(all, ',') + ")|" + wxJoin(all, ';'));

	return final;
}
