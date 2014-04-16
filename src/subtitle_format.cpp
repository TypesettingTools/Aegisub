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

/// @file subtitle_format.cpp
/// @brief Base class for subtitle format handlers
/// @ingroup subtitle_io
///

#include "subtitle_format.h"

#include <wx/intl.h>
#include <wx/choicdlg.h> // Keep this last so wxUSE_CHOICEDLG is set.

#include "ass_attachment.h"
#include "ass_dialogue.h"
#include "ass_file.h"
#include "ass_style.h"
#include "compat.h"
#include "subtitle_format_ass.h"
#include "subtitle_format_ebu3264.h"
#include "subtitle_format_encore.h"
#include "subtitle_format_microdvd.h"
#include "subtitle_format_mkv.h"
#include "subtitle_format_srt.h"
#include "subtitle_format_transtation.h"
#include "subtitle_format_ttxt.h"
#include "subtitle_format_txt.h"
#include "video_context.h"

#include <libaegisub/fs.h>
#include <libaegisub/util.h>

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace {
	std::vector<std::unique_ptr<SubtitleFormat>> formats;
}

SubtitleFormat::SubtitleFormat(std::string name)
: name(std::move(name))
{
}

SubtitleFormat::~SubtitleFormat() { }

bool SubtitleFormat::CanReadFile(agi::fs::path const& filename, std::string const&) const {
	auto wildcards = GetReadWildcards();
	return any_of(begin(wildcards), end(wildcards),
		[&](std::string const& ext) { return agi::fs::HasExtension(filename, ext); });
}

bool SubtitleFormat::CanWriteFile(agi::fs::path const& filename) const {
	auto wildcards = GetWriteWildcards();
	return any_of(begin(wildcards), end(wildcards),
		[&](std::string const& ext) { return agi::fs::HasExtension(filename, ext); });
}

bool SubtitleFormat::CanSave(const AssFile *subs) const {
	if (!subs->Attachments.empty())
		return false;

	std::string defstyle = AssStyle().GetEntryData();
	for (auto const& line : subs->Styles) {
		if (line.GetEntryData() != defstyle)
			return false;
	}

	for (auto const& line : subs->Events) {
		if (line.GetStrippedText() != line.Text)
			return false;
	}

	return true;
}

agi::vfr::Framerate SubtitleFormat::AskForFPS(bool allow_vfr, bool show_smpte, agi::vfr::Framerate const& fps) {
	wxArrayString choices;

	bool vidLoaded = false;
	if (fps.IsLoaded()) {
		vidLoaded = true;
		if (!fps.IsVFR())
			choices.Add(wxString::Format(_("From video (%g)"), fps.FPS()));
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

	bool was_busy = wxIsBusy();
	if (was_busy) wxEndBusyCursor();
	int choice = wxGetSingleChoiceIndex(_("Please choose the appropriate FPS for the subtitles:"), _("FPS"), choices);
	if (was_busy) wxBeginBusyCursor();

	using agi::vfr::Framerate;
	if (choice == -1)
		return Framerate();

	// Get FPS from choice
	if (vidLoaded)
		--choice;
	if (!show_smpte && choice > 4)
		--choice;

	switch (choice) {
		case -1: return fps;                     break;
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
	throw agi::InternalError("Out of bounds result from wxGetSingleChoiceIndex?", nullptr);
}

void SubtitleFormat::StripTags(AssFile &file) {
	for (auto& current : file.Events)
		current.StripTags();
}

void SubtitleFormat::ConvertNewlines(AssFile &file, std::string const& newline, bool mergeLineBreaks) {
	for (auto& current : file.Events) {
		std::string repl = current.Text;
		boost::replace_all(repl, "\\h", " ");
		boost::ireplace_all(repl, "\\n", newline);
		if (mergeLineBreaks) {
			std::string dbl(newline + newline);
			size_t pos = 0;
			while ((pos = repl.find(dbl, pos)) != std::string::npos)
				boost::replace_all(repl, dbl, newline);
		}
		current.Text = repl;
	}
}

void SubtitleFormat::StripComments(AssFile &file) {
	file.Events.remove_and_dispose_if([](AssDialogue const& diag) {
		return diag.Comment || diag.Text.get().empty();
	}, [](AssDialogue *e) { delete e; });
}

/// @brief Split and merge lines so there are no overlapping lines
///
/// Algorithm described at http://devel.aegisub.org/wiki/Technical/SplitMerge
void SubtitleFormat::RecombineOverlaps(AssFile &file) {
	auto cur = file.Events.begin();
	for (auto next = std::next(cur); next != file.Events.end(); cur = std::prev(next)) {
		if (next == file.Events.begin() || cur->End <= next->Start) {
			++next;
			continue;
		}

		std::unique_ptr<AssDialogue> prevdlg(&*cur);
		std::unique_ptr<AssDialogue> curdlg(&*next);
		++next;

		auto insert_line = [&](AssDialogue *newdlg) {
			file.Events.insert(std::find_if(next, file.Events.end(), [&](AssDialogue const& pos) {
				return pos.Start >= newdlg->Start;
			}), *newdlg);
		};

		//Is there an A part before the overlap?
		if (curdlg->Start > prevdlg->Start) {
			// Produce new entry with correct values
			auto newdlg = new AssDialogue(*prevdlg);
			newdlg->Start = prevdlg->Start;
			newdlg->End = curdlg->Start;
			newdlg->Text = prevdlg->Text;
			insert_line(newdlg);
		}

		// Overlapping A+B part
		{
			auto newdlg = new AssDialogue(*prevdlg);
			newdlg->Start = curdlg->Start;
			newdlg->End = (prevdlg->End < curdlg->End) ? prevdlg->End : curdlg->End;
			// Put an ASS format hard linewrap between lines
			newdlg->Text = curdlg->Text.get() + "\\N" + prevdlg->Text.get();
			insert_line(newdlg);
		}

		// Is there an A part after the overlap?
		if (prevdlg->End > curdlg->End) {
			// Produce new entry with correct values
			auto newdlg = new AssDialogue(*prevdlg);
			newdlg->Start = curdlg->End;
			newdlg->End = prevdlg->End;
			newdlg->Text = prevdlg->Text;
			insert_line(newdlg);
		}

		// Is there a B part after the overlap?
		if (curdlg->End > prevdlg->End) {
			// Produce new entry with correct values
			auto newdlg = new AssDialogue(*prevdlg);
			newdlg->Start = prevdlg->End;
			newdlg->End = curdlg->End;
			newdlg->Text = curdlg->Text;
			insert_line(newdlg);
		}
	}
}

/// @brief Merge identical lines that follow each other
void SubtitleFormat::MergeIdentical(AssFile &file) {
	auto next = file.Events.begin();
	auto cur = next++;

	for (; next != file.Events.end(); cur = next++) {
		if (cur->End == next->Start && cur->Text == next->Text) {
			// Merge timing
			next->Start = std::min(next->Start, cur->Start);
			next->End = std::max(next->End, cur->End);

			// Remove duplicate line
			delete &*cur;
		}
	}
}

void SubtitleFormat::LoadFormats() {
	if (formats.empty()) {
		formats.emplace_back(agi::util::make_unique<AssSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<Ebu3264SubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<EncoreSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<MKVSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<MicroDVDSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<SRTSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<TTXTSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<TXTSubtitleFormat>());
		formats.emplace_back(agi::util::make_unique<TranStationSubtitleFormat>());
	}
}

template<class Cont, class Pred>
SubtitleFormat *find_or_throw(Cont &container, Pred pred) {
	auto it = find_if(container.begin(), container.end(), pred);
	if (it == container.end())
		throw UnknownSubtitleFormatError("Subtitle format for extension not found", nullptr);
	return it->get();
}

const SubtitleFormat *SubtitleFormat::GetReader(agi::fs::path const& filename, std::string const& encoding) {
	LoadFormats();
	return find_or_throw(formats, [&](std::unique_ptr<SubtitleFormat> const& f) {
		return f->CanReadFile(filename, encoding);
	});
}

const SubtitleFormat *SubtitleFormat::GetWriter(agi::fs::path const& filename) {
	LoadFormats();
	return find_or_throw(formats, [&](std::unique_ptr<SubtitleFormat> const& f) {
		return f->CanWriteFile(filename);
	});
}

std::string SubtitleFormat::GetWildcards(int mode) {
	LoadFormats();

	std::vector<std::string> all;
	std::string final;

	for (auto const& format : formats) {
		auto cur = mode == 0 ? format->GetReadWildcards() : format->GetWriteWildcards();
		if (cur.empty()) continue;

		for (auto& str : cur) str.insert(0, "*.");
		all.insert(all.end(), begin(cur), end(cur));
		final += "|" + format->GetName() + " (" + boost::join(cur, ",") + ")|" + boost::join(cur, ";");
	}

	return from_wx(_("All Supported Formats")) + " (" + boost::join(all, ",") + ")|" + boost::join(all, ";") + final;
}
