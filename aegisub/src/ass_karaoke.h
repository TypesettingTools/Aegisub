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

/// @file ass_karaoke.h
/// @see ass_karaoke.cpp
/// @ingroup subs_storage
///


#include <map>
#include <set>
#include <vector>

#include <wx/string.h>

#include <libaegisub/signal.h>

namespace agi { struct Context; }
class AssDialogue;

/// @class AssKaraoke
/// @brief Karaoke parser and parsed karaoke data model
class AssKaraoke {
public:
	/// Parsed syllable data
	struct Syllable {
		int start_time; ///< Start time relative to time zero (not line start) in milliseconds
		int duration;   ///< Duration in milliseconds
		wxString text;  ///< Stripped syllable text
		wxString tag_type; ///< \k, \kf or \ko
		/// Non-karaoke override tags in this syllable. Key is an index in text
		/// before which the value should be inserted
		std::map<size_t, wxString> ovr_tags;

		/// Get the text of this line with override tags and optionally the karaoke tag
		wxString GetText(bool k_tag) const;
	};
private:
	typedef std::map<size_t, wxString>::iterator ovr_iterator;
	std::vector<Syllable> syls;
	AssDialogue *active_line;

	bool no_announce;

	agi::signal::Signal<> AnnounceSyllablesChanged;
	void ParseSyllables(AssDialogue *line, Syllable &syl);

public:
	/// Constructor
	/// @param line Initial line
	/// @param auto_split Should the line automatically be split on spaces if there are no k tags?
	/// @param normalize Should the total duration of the syllables be forced to equal the line duration?
	AssKaraoke(AssDialogue *line = 0, bool auto_split = false, bool normalize = true);

	/// Parse a dialogue line
	void SetLine(AssDialogue *line, bool auto_split = false, bool normalize = true);

	/// Add a split before character pos in syllable syl_idx
	void AddSplit(size_t syl_idx, size_t pos);
	/// Remove the split at the given index
	void RemoveSplit(size_t syl_idx);
	/// Set the start time of a syllable in ms
	void SetStartTime(size_t syl_idx, int time);
	/// Adjust the line's start and end times without shifting the syllables
	void SetLineTimes(int start_time, int end_time);

	typedef std::vector<Syllable>::const_iterator iterator;

	iterator begin() const { return syls.begin(); }
	iterator end() const { return syls.end(); }
	size_t size() const { return syls.size(); }

	/// Get the line's text with k tags
	wxString GetText() const;

	/// Get the karaoke tag type used, with leading slash
	/// @returns "\k", "\kf", or "\ko"
	wxString GetTagType() const;
	/// Set the tag type for all karaoke tags in this line
	void SetTagType(wxString const& new_type);

	/// Split lines so that each syllable is its own line
	/// @param lines Lines to split
	/// @param c Project context
	static void SplitLines(std::set<AssDialogue*> const& lines, agi::Context *c);

	DEFINE_SIGNAL_ADDERS(AnnounceSyllablesChanged, AddSyllablesChangedListener)
};
